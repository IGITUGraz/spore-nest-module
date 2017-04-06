#! /usr/bin/env python
# -*- coding: utf-8 -*-

import logging
from config import *

import nest
import numpy as np
from mpi4py import MPI

from snn_utils.comm.music.node.nest import PyNestNode

logger = logging.getLogger(__name__)


class NetworkNode(PyNestNode):
    def __init__(self):
        PyNestNode.__init__(self, total_simulation_time, pre_run_barrier=True)

        logger.info("Setting up NEST and installing SPORE")

        if nest_n_threads is None:
            import multiprocessing
            n_threads = int(1.1 * (multiprocessing.cpu_count() - MPI.COMM_WORLD.Get_size()))
            logger.info("> n_threads = {} (autodetect)".format(n_threads))
        else:
            n_threads = nest_n_threads
            logger.info("> n_threads = {} (fixed configuration)".format(nest_n_threads))

        nest.SetKernelStatus({
            "resolution": s_to_ms(network_node_time_step),
            "overwrite_files": True,
            "local_num_threads": n_threads
        })

        nest.Install("sporemodule")

        logger.info("> Synaptic update interval: {}s".format(s_to_ms(synapse_update_interval)))
        nest.sli_func("InitSynapseUpdater", int(s_to_ms(synapse_update_interval)), int(s_to_ms(synapse_update_interval)))

        # initialize random values, see NEST documentation
        N_vp = nest.GetKernelStatus(["total_num_virtual_procs"])[0]
        nest.SetKernelStatus({"grng_seed": random_seed + N_vp})
        nest.SetKernelStatus({"rng_seeds": list(range(random_seed + N_vp + 1, random_seed + 2 * N_vp + 1))})

        # ------------- NETWORK CONSTRUCTION -------------

        logger.info("Constructing network")

        nest.CopyModel("poisson_dbl_exp_neuron", "hidden_neuron", hidden_neuron_properties)
        nest.CopyModel("static_synapse", "e_to_i_synapse")
        nest.CopyModel("static_synapse", "i_to_e_synapse")
        nest.CopyModel("synaptic_sampling_rewardgradient_synapse", "reward_synapse")

        logger.info("> Opening port and creating proxy for incoming reward signal")

        reward_in = nest.Create('reward_in_proxy')
        nest.SetStatus(reward_in, music_network_node_reward_in_port)

        synapse_properties["reward_transmitter"] = reward_in[0]
        nest.SetDefaults("reward_synapse", synapse_properties)

        logger.info("> Opening port and creating proxy for incoming spike patterns")
        input_neurons = self._create_event_in_proxy(n_input_neurons, **music_network_node_pattern_in_port)

        logger.info("> Creating hidden neurons")
        hidden_neurons = nest.Create("hidden_neuron", n_hidden_neurons)

        logger.info("> Establishing synaptic connection between input and hidden neurons")
        for i in range(n_max_connections_input_hidden):
            nest.Connect(input_neurons, hidden_neurons,
                         {
                             "rule": "pairwise_bernoulli",
                             "p": p_connection_input_hidden
                         },
                         {
                             "model": "reward_synapse",
                             "synaptic_parameter": {
                                 "distribution": "normal",
                                 "mu": synapse_init_mean,
                                 "sigma": synapse_init_std}
                         })

        # ################################################################################
        # status("> Establishing lateral inhibition on hidden neurons")
        #
        # lateral inhibition between hidden neurons
        # nest.Connect(hidden_neurons, hidden_neurons,
        #              {
        #                  "rule": "pairwise_bernoulli",
        #                  "p": 0.5,
        #                  "autapses": False
        #              },
        #              {
        #                  "model": "i_to_e_synapse",
        #                  "weight": {
        #                      "distribution": "normal_clipped",
        #                      "mu": -2.00,
        #                      "sigma": 0.2,
        #                      "high": 0.0},
        #                  "delay": 1.0,
        #                  "receptor_type": 1  # inhibitory synapse port
        #              })
        # ################################################################################

        logger.info("> Opening ports for outgoing spikes; Establishing connection to hidden neurons")
        self._create_event_out_proxy(hidden_neurons, **music_network_node_activity_out_port)

    def _run(self):
        logger.info("Starting simulation")
        nest.Simulate(s_to_ms(total_simulation_time))

if __name__ == '__main__':
    NetworkNode().main()
