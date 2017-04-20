/*
 * This file is part of SPORE.
 *
 * Copyright (c) 2016, Institute for Theoretical Computer Science,
 * Graz University of Technology
 *
 * SPORE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * SPORE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SPORE.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For more information see: https://github.com/IGITUGraz/spore-nest-module
 *
 * File:   synaptic_sampling_rewardgradient_connection.h
 * Author: Kappel, Hsieh
 *
 * This file is based on stdp_dopa_connection.h which is part of NEST
 * (Copyright (C) 2004 The NEST Initiative).
 * See: http://nest-initiative.org/
 */

#ifndef SYNAPTIC_SAMPLING_REWARDGRADIENT_CONNECTIO
#define SYNAPTIC_SAMPLING_REWARDGRADIENT_CONNECTIO

#include <cmath>
#include "nest.h"
#include "connection.h"
#include "normal_randomdev.h"
#include "spikecounter.h"

#include "tracing_node.h"
#include "connection_updater.h"
#include "connection_data_logger.h"
#include "param_utils.h"


namespace spore
{

/**
 * @brief Class holding the common properties for all synapses of type SynapticSamplingRewardGradientConnection.
 *
 * The parameters, their constraints and their default values are described
 * in detail in the documentation of SynapticSamplingRewardGradientConnection.
 */
class SynapticSamplingRewardGradientCommonProperties : public nest::CommonSynapseProperties
{
public:

    SynapticSamplingRewardGradientCommonProperties();
    ~SynapticSamplingRewardGradientCommonProperties();

    using CommonSynapseProperties::get_status;
    using CommonSynapseProperties::set_status;
    using CommonSynapseProperties::calibrate;

    void get_status(DictionaryDatum & d) const;
    void set_status(const DictionaryDatum & d, nest::ConnectorModel& cm);
    void calibrate(const nest::TimeConverter &tc);

    /**
     * Check spike event.
     */
    void check_event(nest::SpikeEvent&)
    {
    }

    /**
     * @return GID of the reward transmitter associated to the synapse type.
     */
    long get_vt_gid() const
    {
        if (reward_transmitter_ != 0)
            return reward_transmitter_->get_gid();
        else
            return -1;
    }

    /**
     * @return The reward transmitter associated to the synapse type.
     */
    nest::Node* get_node()
    {
        if (reward_transmitter_ == 0)
            return nest::CommonSynapseProperties::get_node();
        else
            return reward_transmitter_;
    }

    /**
     * Convenience function to get Gaussian random number for parameter noise.
     */
    double get_d_wiener(nest::thread thread) const
    {
        double result = 0.0;
        if (std_wiener_ > 0)
        {
            result = std_wiener_ * normal_dev_(nest::kernel().rng_manager.get_rng(thread));
        }
        return result;
    }

    /**
     * Convenience function to get Gaussian random number for gradient noise.
     */
    double get_gradient_noise(nest::thread thread) const
    {
        double result = 0.0;
        if (std_gradient_ > 0)
        {
            result = std_gradient_ * normal_dev_(nest::kernel().rng_manager.get_rng(thread));
        }
        return result;
    }

    /**
     * Convenience function to random number.
     */
    double drand(nest::thread thread) const
    {
        return nest::kernel().rng_manager.get_rng(thread)->drand();
    }

    // parameters
    double learning_rate_;
    double episode_length_;
    double psp_tau_rise_;
    double psp_tau_fall_;

    double temperature_;
    double gradient_noise_;
    double max_param_;
    double min_param_;
    double max_param_change_;
    double integration_time_;
    double direct_gradient_rate_;
    double parameter_mapping_offset_;
    double weight_scale_;
    double weight_update_interval_;
    double gradient_scale_;
    double psp_cutoff_amplitude_;

    long bap_trace_id_;
    long dopa_trace_id_;

    bool simulate_retracted_synapses_;
    bool delete_retracted_synapses_;
    bool verbose_;

    // state variables
    TracingNode* reward_transmitter_;

    double resolution_unit_;
    double reward_gradient_update_;
    double eligibility_trace_update_;

    double psp_faciliation_update_;
    double psp_depression_update_;
    double psp_scale_factor_;

    long weight_update_steps_;

private:

    double std_wiener_;
    double std_gradient_;
    librandom::NormalRandomDev normal_dev_;
};

/**
 * @brief Reward-based synaptic sampling connection class
 *
 * This connection type implements the Bayesian policy sampling algorithm
 * introduced in [1,2]. The target node to which synapses of this type
 * are connected must be derived from TracingNode. A second node which
 * is also derived from type TracingNode must be registered to the synapse
 * model at its \a reward_transmitter parameter. The synapse model performs
 * a stochastic policy search which tries to maximize the reward signal
 * provided by the \a reward_transmitter node. At the same time synaptic
 * weights are constraint by a Gaussian prior with mean \f$\mu\f$ and standard
 * deviation \f$\sigma\f$. This synapse type can not change its sign, i.e.
 * synapses are either excitatory or inhibitory depending on the sign of the
 * \a weight_scale parameter. If synaptic weights fall below a threshold
 * (determined by parameter \a parameter_mapping_offset) weights are clipped
 * to zero (retracted synapses). The synapse model also implements an optional
 * mechanism to automatically remove retracted synapses from the simulation.
 * This mechanism can be turned on using the \a delete_retracted_synapses
 * parameter.
 *
 * <b>Parameters and state variables</b>
 *
 * The following parameters can be set in the common properties dictionary
 * (default values and constraints are given in parentheses, corresponding symbols
 * in the equations given below and in [1,2] are given in braces):
 * <table>
 * <tr><th>name</th>                           <th>type</th>   <th>comment</th></tr>
 * <tr><td>\a learning_rate</td>               <td>double</td> <td>learning rate (5e-08, &ge;0.0)
 *                                                              {\f$\beta\f$}</td></tr>
 * <tr><td>\a temperature</td>                 <td>double</td> <td>amplitude of parameter noise (0.1, &ge;0.0)
 *                                                              {\f$T_\theta\f$}</td></tr>
 * <tr><td>\a gradient_noise</td>              <td>double</td> <td>amplitude of gradient noise (0.0, &ge;0.0)
 *                                                              {\f$T_g\f$}</td></tr>
 * <tr><td>\a psp_tau_rise</td>                <td>double</td> <td>double exponential PSP kernel rise (2.0, >0.0) [ms]
 *                                                              {\f$\tau_r\f$}</td></tr>
 * <tr><td>\a psp_tau_fall</td>                <td>double</td> <td>double exponential PSP kernel decay (20.0, >0.0)
 *                                                              [ms] {\f$\tau_m\f$}</td></tr>
 * <tr><td>\a psp_cutoff_amplitude</td>        <td>double</td> <td>psp is clipped to 0 below this value
 *                                                              (0.0001, &ge;0.0) {\f$\tau_m\f$}</td></tr>
 * <tr><td>\a integration_time</td>            <td>double</td> <td>time of gradient integration (50000.0, >0.0) [ms]
 *                                                              {\f$\tau_g\f$}</td></tr>
 * <tr><td>\a episode_length</td>              <td>double</td> <td>length of eligibility trace (1000.0, >0.0) [ms]
 *                                                              {\f$\tau_e\f$}</td></tr>
 * <tr><td>\a weight_update_interval</td>      <td>double</td> <td>interval of synaptic weight updates (100.0, >0.0)
 *                                                              [ms]</td></tr>
 * <tr><td>\a parameter_mapping_offset</td>    <td>double</td> <td>offset parameter for computing synaptic
 *                                                              weight (3.0) {\f$\theta_0\f$}</td></tr>
 * <tr><td>\a weight_scale</td>                <td>double</td> <td>scaling factor for the synaptic weight (1.0)
 *                                                              {\f$w_0\f$} </td></tr>
 * <tr><td>\a direct_gradient_rate</td>        <td>double</td> <td>rate of directly applying changes to the
 *                                                              synaptic parameter (0.0) {\f$c_e\f$} </td></tr>
 * <tr><td>\a gradient_scale</td>              <td>double</td> <td>scaling parameter for the gradient (1.0)
 *                                                              {\f$c_g\f$}</td></tr>
 * <tr><td>\a max_param</td>                   <td>double</td> <td>maximum synaptic parameter (5.0)</td></tr>
 * <tr><td>\a min_param</td>                   <td>double</td> <td>minimum synaptic parameter (-2.0)</td></tr>
 * <tr><td>\a max_param_change</td>            <td>double</td> <td>maximum synaptic parameter change (40.0, &ge;0.0)
 *                                                             </td></tr>
 * <tr><td>\a reward_transmitter</td>          <td>long</td>   <td>GID of the synapse's reward transmitter*</td></tr>
 * <tr><td>\a bap_trace_id</td>                <td>long</td>   <td>ID of the BAP trace (0, &ge;0)</td></tr>
 * <tr><td>\a dopa_trace_id</td>               <td>long</td>   <td>ID of the dopamine trace (0, &ge;0)</td></tr>
 * <tr><td>\a simulate_retracted_synapses</td> <td>bool</td>   <td>continue simulating retracted synapses
 *                                                              (false)</td></tr>
 * <tr><td>\a delete_retracted_synapses</td>   <td>bool</td>   <td>delete retracted synapses (false)</td></tr>
 * <tr><td>\a verbose</td>                     <td>bool</td>   <td>write status to the standard output (false)
 *                                                             </td></tr>
 * </table>
 *
 * *)  \a reward_transmitter must be set to the GID of a TracingNode before
 *        simulation startup.
 *
 * The following parameters can be set in the status dictionary:
 * <table>
 * <tr><th>name</th>                           <th>type</th>   <th>comment</th></tr>
 * <tr><td>\a synaptic_parameter</td>          <td>double</td> <td>current synaptic parameter {\f$\theta(t)\f$}
 *                                                             </td></tr>
 * <tr><td>\a weight</td>                      <td>double</td> <td>current synaptic weight {\f$w(t)\f$}</td></tr>
 * <tr><td>\a eligibility_trace</td>           <td>double</td> <td>current eligibility trace {\f$e(t)\f$}</td></tr>
 * <tr><td>\a reward_gradient</td>             <td>double</td> <td>current reward gradient {\f$g(t)\f$}</td></tr>
 * <tr><td>\a prior_mean</td>                  <td>double</td> <td>mean of the Gaussian prior {\f$\mu\f$}</td></tr>
 * <tr><td>\a prior_precision</td>             <td>double</td> <td>precision of the Gaussian prior {\f$c_p\f$}</td></tr>
 * <tr><td>\a recorder_times</td>              <td>[double]</td> <td>time points of parameter recordings*</td></tr>
 * <tr><td>\a weight_values</td>               <td>[double]</td> <td>array of recorded synaptic weight values*
 *                                                               </td></tr>
 * <tr><td>\a synaptic_parameter_values</td>   <td>[double]</td> <td>array of recorded synaptic parameter values*
 *                                                               </td></tr>
 * <tr><td>\a reward_gradient_values</td>      <td>[double]</td> <td>array of recorded reward gradient values*
 *                                                               </td></tr>
 * <tr><td>\a eligibility_trace_values</td>    <td>[double]</td> <td>array of recorded eligibility trace values*
 *                                                               </td></tr>
 * <tr><td>\a psp_values</td>                  <td>[double]</td> <td>array of recorded psp values*</td></tr>
 * <tr><td>\a recorder_interval</td>           <td>double</td> <td>interval of synaptic recordings [ms]</td></tr>
 * <tr><td>\a reset_recorder</td>              <td>bool</td>   <td>clear all recorded values now* (write only)
 *                                                             </td></tr>
 * </table>
 *
 * *) Recorder fields are read only. If \a reset_recorder is set to \c true
 *    all recorder fields will be cleared instantaneously.
 *
 * <b>Implementation Details</b>
 *
 * This connection type is a diligent synapse model, therefore updates are triggered
 * on a regular interval which is ensured by the ConnectionUpdateManager. The
 * state of each synapse consists of the variables
 * \f$y(t), e(t), g(t), \theta(t), w(t)\f$.
 * The variable \f$y(t)\f$ is the presynaptic spike train filtered with a PSP
 * kernel \f$\epsilon(t)\f$ of the form
 * \f[
 *     \epsilon(t) \;=\; \frac{\tau_r}{\tau_m - \tau_r}\left( e^{-\frac{1}{\tau_m}} - e^{-\frac{1}{\tau_r}} \right)\;.
 *     \hspace{24px} (1)
 * \f]
 * A node derived from type TracingNode must be registered to the synapse
 * model at its \a reward_transmitter parameter. The trace of this node at
 * id \a dopa_trace_id is used as reward signal \f$dopa(t)\f$. The trace
 * of the postsynaptic neuron with id \a bap_trace_id is used as
 * back-propagating signal \f$bap(t)\f$. The synapse then solves the following
 * set of differential equations:
 * \f[
 *     \frac{d e(t)}{dt} \;=\; -\frac{1}{\tau_e} e(t) \,+\, w(t)\,y(t)\,bap(t) \hspace{24px} (2)
 * \f]
 * \f[
 *     \frac{d g(t)}{dt} \;=\; -\frac{1}{\tau_g} g(t) \,+\, dopa(t)\,e(t) \,+\, T_g\,d \mathcal{W}_g \hspace{24px} (3)
 * \f]
 * \f[
 *     d \theta(t) \;=\; \beta\,\bigg( c_p (\mu - \theta(t)) + c_g\,g(t) +
 *                                     c_e \, dopa(t) \,e(t) \bigg) dt \,+\,
 *                       \sqrt{ 2 T_\theta \beta } \, \mathcal{W}_{\theta}  \hspace{24px} (4)
 * \f]
 * \f[
 *     w(t) \;=\; w_0 \, \exp ( \theta(t) - \theta_0 ) \hspace{24px} (5)
 * \f]
 * The precision of the prior in equation (4) relates to the standard deviation
 * as \f$c_p = 1/\sigma^2\f$. Setting \f$c_p=0\f$ corresponds to a
 * non-informative (flat) prior.
 *
 * The differential equations (2-5) are solved using Euler integration.
 * The dynamics of the postsynaptic term \f$y(t)\f$, the eligibility trace
 * \f$e(t)\f$ and the reward gradient \f$g(t)\f$ are updated at each NEST
 * time step. The dynamics of \f$\theta(t)\f$ and \f$w(t)\f$ are updated
 * on a time grid based on \a weight_update_interval. The synaptic weights
 * remain constant between two updates. The synapse recorder is only invoked
 * after each weight update which means that \a recorder_interval must be a
 * multiple of \a weight_update_interval. Synaptic parameters are clipped at
 * \a min_param and \a max_param. Parameter gradients are clipped at +/-
 * \a max_param_change. Synaptic weights of synapses for which \f$\theta(t)\f$
 * falls below 0 are clipped to 0 (retracted synapses). If
 * \a simulate_retracted_synapses is set to \c false simulation of
 * \f$y(t), e(t)\f$ and \f$g(t)\f$ is not continued for retracted synapse.
 * This means that only the stochastic dynamics of \f$\theta(t)\f$ are simulated
 * until the synapse is reformed again. During this time, the reward gradient
 * \f$g(t)\f$ is fixed to 0. If \a delete_retracted_synapses is set to \c true,
 * retracted synapses will be removed from the network using the garbage
 * collector of the ConnectionUpdateManager.
 *
 * <b>References</b>
 *
 * [1] David Kappel, Robert Legenstein, Stefan Habenschuss, Michael Hsieh and
 * Wolfgang Maass. <i>Reward-based self-configuration of neural circuits.</i> 2017.
 * https://arxiv.org/abs/1704.04238
 *
 * [2] Zhaofei Yu, David Kappel, Robert Legenstein, Sen Song, Feng Chen and
 * Wolfgang Maass. <i>CaMKII activation supports reward-based neural network
 * optimization through Hamiltonian sampling.</i> 2016.
 * https://arxiv.org/abs/1606.00157
 *
 * @author David Kappel, Michael Hsieh
 *
 * @see TracingNode, DiligentConnectorModel
 */
template<typename targetidentifierT>
class SynapticSamplingRewardGradientConnection : public nest::Connection<targetidentifierT>
{
public:

    SynapticSamplingRewardGradientConnection();
    SynapticSamplingRewardGradientConnection(const SynapticSamplingRewardGradientConnection<targetidentifierT>& rhs);
    ~SynapticSamplingRewardGradientConnection();

    //! Type to use for representing common synapse properties
    typedef SynapticSamplingRewardGradientCommonProperties CommonPropertiesType;

    //! Shortcut for base class
    typedef nest::Connection<targetidentifierT> ConnectionBase;

    /**
     * Checks if the type of the postsynaptic node is supported. Throws an
     * \a IllegalConnection exception if the postsynaptic node is not
     * derived from TracingNode.
     */
    void check_connection(nest::Node & s, nest::Node & t,
                          nest::rport receptor_type, double t_lastspike, const CommonPropertiesType &cp)
    {
        if (!dynamic_cast<TracingNode*> (&t))
        {
            throw nest::IllegalConnection("This synapse only works with nodes exposing their firing"
                                          " probability trace (i.e. TracingNode-Subclass)!");
        }

        ConnTestDummyNode dummy_target;
        ConnectionBase::check_connection_(dummy_target, s, t, receptor_type);
    }

    void get_status(DictionaryDatum & d) const;
    void set_status(const DictionaryDatum & d, nest::ConnectorModel &cm);

    void send(nest::Event& e, nest::thread t, double t_lastspike, const CommonPropertiesType &cp);

    using ConnectionBase::get_delay_steps;
    using ConnectionBase::get_delay;
    using ConnectionBase::get_rport;
    using ConnectionBase::get_target;

    /**
     * Sets the synaptic parameter to the given value.
     *
     * @note This function deviates from the default behavior of nest as it not directly sets the
     * weight of the synapse, but the value of the synaptic parameter \f$\theta$. The actual synaptic
     * weight of the synapse will only change after the next weight update is triggered. Use \c set_status
     * to set the weight directly.
     */
    void set_weight(double w)
    {
        synaptic_parameter_ = w;
    }

    /**
     * Access function to the current value of the eligibility trace.
     */
    double get_eligibility_trace() const
    {
        return eligibility_trace_;
    }

    /**
     * Access function to the current value of the postsynaptic potential.
     */
    double get_psp() const
    {
        return psp_facilitation_ - psp_depression_;
    }

    /**
     * Access function to the current value of the synaptic weight.
     */
    double get_weight() const
    {
        return weight_;
    }

    /**
     * Access function to the current value of the synaptic parameter.
     */
    double get_synaptic_parameter() const
    {
        return synaptic_parameter_;
    }

    /**
     * Access function to the current value of the reward gradient.
     */
    double get_reward_gradient() const
    {
        return reward_gradient_;
    }

    /**
     * @returns true if the synapse should be picked up by the garbage collector.
     */
    bool is_degenerated() const
    {
        return (psp_facilitation_ == -1.0);
    }

    static ConnectionDataLogger<SynapticSamplingRewardGradientConnection> *logger();

private:

    double weight_;
    double synaptic_parameter_;

    double psp_facilitation_;
    double psp_depression_;

    double eligibility_trace_;
    double reward_gradient_;

    double prior_mean_;
    double prior_precision_;

    nest::index recorder_port_;

    static ConnectionDataLogger<SynapticSamplingRewardGradientConnection> *logger_;

    void update_synapse_state(long t_to,
                              long t_last_update,
                              TracingNode::const_iterator &bap_trace,
                              TracingNode::const_iterator &dopa_trace,
                              const CommonPropertiesType& cp);

    void update_synapic_parameter(nest::thread thread, const CommonPropertiesType& cp);
    void update_synapic_weight(long time_step, const CommonPropertiesType& cp);

    class ConnTestDummyNode : public nest::ConnTestDummyNodeBase
    {
    public:
        using nest::ConnTestDummyNodeBase::handles_test_event;

        nest::port handles_test_event(nest::SpikeEvent&, nest::rport)
        {
            return nest::invalid_port_;
        }

        nest::port handles_test_event(nest::DSSpikeEvent&, nest::rport)
        {
            return nest::invalid_port_;
        }
    };
};


//
// Implementation of SynapticSamplingRewardGradientConnection
//

//
// Object life cycle
//

/**
 * Default Constructor.
 */
template <typename targetidentifierT>
SynapticSamplingRewardGradientConnection<targetidentifierT>::SynapticSamplingRewardGradientConnection()
: ConnectionBase(),
weight_(0.0),
synaptic_parameter_(0.0),
psp_facilitation_(0.0),
psp_depression_(0.0),
eligibility_trace_(0.0),
reward_gradient_(0.0),
prior_mean_(0.0),
prior_precision_(1.0),
recorder_port_(nest::invalid_index)
{
}

/**
 * Copy Constructor.
 */
template <typename targetidentifierT>
SynapticSamplingRewardGradientConnection<targetidentifierT>::
SynapticSamplingRewardGradientConnection(const SynapticSamplingRewardGradientConnection& rhs)
: ConnectionBase(rhs),
weight_(rhs.weight_),
synaptic_parameter_(rhs.synaptic_parameter_),
psp_facilitation_(rhs.psp_facilitation_),
psp_depression_(rhs.psp_depression_),
eligibility_trace_(rhs.eligibility_trace_),
reward_gradient_(rhs.reward_gradient_),
prior_mean_(rhs.prior_mean_),
prior_precision_(rhs.prior_precision_),
recorder_port_(nest::invalid_index)
{
}

/**
 * Destructor.
 */
template <typename targetidentifierT>
SynapticSamplingRewardGradientConnection<targetidentifierT>::~SynapticSamplingRewardGradientConnection()
{
}

//
// Instance of global data logger singleton.
//

/**
 * Pointer to the global instance of the data logger.
 */
template <typename targetidentifierT>
ConnectionDataLogger< SynapticSamplingRewardGradientConnection<targetidentifierT> >
*SynapticSamplingRewardGradientConnection<targetidentifierT>::logger_ = 0;

/**
 * Get the data logger singleton.
 *
 * @return the instance of the data logger.
 */
template <typename targetidentifierT>
ConnectionDataLogger< SynapticSamplingRewardGradientConnection<targetidentifierT> >
*SynapticSamplingRewardGradientConnection<targetidentifierT>::logger()
{
    if (!logger_)
    {
        logger_ = new ConnectionDataLogger<SynapticSamplingRewardGradientConnection>();

        logger_->register_recordable_variable("eligibility_trace",
                                              &SynapticSamplingRewardGradientConnection::get_eligibility_trace);
        logger_->register_recordable_variable("psp",
                                              &SynapticSamplingRewardGradientConnection::get_psp);
        logger_->register_recordable_variable("weight",
                                              &SynapticSamplingRewardGradientConnection::get_weight);
        logger_->register_recordable_variable("synaptic_parameter",
                                              &SynapticSamplingRewardGradientConnection::get_synaptic_parameter);
        logger_->register_recordable_variable("reward_gradient",
                                              &SynapticSamplingRewardGradientConnection::get_reward_gradient);
    }

    return logger_;
}

//
// Parameter and state extractions and manipulation functions
//

/**
 * Status getter function.
 */
template <typename targetidentifierT>
void SynapticSamplingRewardGradientConnection<targetidentifierT>::get_status(DictionaryDatum & d) const
{
    ConnectionBase::get_status(d);
    def<double>(d, nest::names::weight, weight_);
    def<double>(d, "synaptic_parameter", synaptic_parameter_);
    def<double>(d, "eligibility_trace", eligibility_trace_);
    def<double>(d, "reward_gradient", reward_gradient_);
    def<double>(d, "prior_mean", prior_mean_);
    def<double>(d, "prior_precision", prior_precision_);
    def<long>(d, nest::names::size_of, sizeof (*this));

    logger()->get_status(d, recorder_port_);
}

/**
 * @brief Status setter function.
 *
 * @note \a weight will be overwritten next time when the synapse is updated.
 */
template <typename targetidentifierT>
void SynapticSamplingRewardGradientConnection<targetidentifierT>::set_status(const DictionaryDatum & d,
                                                                             nest::ConnectorModel &cm)
{
    ConnectionBase::set_status(d, cm);
    updateValue<double>(d, nest::names::weight, weight_);
    updateValue<double>(d, "synaptic_parameter", synaptic_parameter_);
    updateValue<double>(d, "prior_mean", prior_mean_);
    updateValue<double>(d, "prior_precision", prior_precision_);

    logger()->set_status(d, recorder_port_);
}

//
// Synapse event handling
//

/**
 * Send an event to the postsynaptic neuron. This will update the synapse state
 * and synaptic weights to the current slice origin and send the spike event.
 * This method is also triggered by the ConnectionUpdateManager to indicate
 * that the synapse is running out of date. In this case an invalid rport of -1
 * is passed and the spike is not delivered to the postsynaptic neuron.
 *
 * @param e the spike event.
 * @param thread the id of the connections thread.
 * @param t_last_spike the time of the last spike.
 * @param cp the synapse type common properties.
 */
template <typename targetidentifierT>
void SynapticSamplingRewardGradientConnection<targetidentifierT>::send(nest::Event& e,
                                                                       nest::thread thread,
                                                                       double t_last_spike,
                                                                       const CommonPropertiesType &cp)
{
    if (is_degenerated())
    {
        // synapse is waiting for the garbage collector.
        return;
    }

    assert(cp.resolution_unit_ > 0.0);

    const long s_to = std::floor( e.get_stamp().get_ms() / cp.resolution_unit_ );
    long s_from = std::floor( t_last_spike / cp.resolution_unit_ );

    if (s_to > s_from)
    {
        if (s_from == 0)
        {
            update_synapic_weight(0, cp);
        }
        // prepare the pointer to the target neuron. We can safely static_cast
        // since the connection is checked when established.
        TracingNode* target = static_cast<TracingNode*> (get_target(thread));

        TracingNode::const_iterator bap_trace =
                target->get_trace(s_from, cp.bap_trace_id_);

        TracingNode::const_iterator dopa_trace =
                cp.reward_transmitter_->get_trace(s_from, cp.dopa_trace_id_);

        const double t_last_weight_update =
            std::floor(t_last_spike / cp.weight_update_interval_) * cp.weight_update_interval_;
        const long s_last_update = std::floor( t_last_weight_update/cp.resolution_unit_ );

        for (long next_weight_step = s_last_update + cp.weight_update_steps_;
             next_weight_step <= s_to;
             next_weight_step += cp.weight_update_steps_)
        {
            update_synapse_state(next_weight_step, s_from, bap_trace, dopa_trace, cp);
            update_synapic_parameter(thread, cp);
            update_synapic_weight(next_weight_step, cp);
            s_from = next_weight_step;
        }

        if (s_to > s_from)
        {
            update_synapse_state(s_to, s_from, bap_trace, dopa_trace, cp);
        }
    }

    if (cp.delete_retracted_synapses_ && (weight_==0.0))
    {
        // synapse prepares to be picked up by the garbage collector.
        // invalid value of -1.0 for psp_facilitation_ is used to indicate
        // synapses to be deleted. The synapse will be removed next time
        // when the garbage collector is invoked.
        psp_facilitation_ = -1.0;
        nest::synindex syn_id = nest::Connection<targetidentifierT>::get_syn_id();
        ConnectionUpdateManager::instance()->trigger_garbage_collector(get_target(thread)->get_gid(),
                                                                       e.get_sender_gid(), thread, syn_id );
        return;
    }

    // Make sure that the event is not a SynapseUpdateEvent.
    if (e.get_rport() >= 0)
    {
        // Apply presynaptic spike
        psp_facilitation_ += 1.0;
        psp_depression_ += 1.0;

        if (weight_ > 0.0)
        {
            e.set_weight(weight_);

            e.set_delay(get_delay_steps());
            e.set_receiver(*get_target(thread));
            e.set_rport(get_rport());
            e();
        }
    }
}

/**
 * Updates the state of the synapse to the given time point. This method
 * expects the back propagating action potential BAP trace of the postsynaptic
 * neuron and the reward (dopamine) trace of the reward transmitter to be
 * passed. Iterators are expected to be positioned at time t_last_update and
 * will be advanced to t_to after the call.
 *
 * This method implements equations (1-3).
 *
 * @param t_to time to advance to.
 * @param t_last_update time of last update.
 * @param bap_trace iterator pointing to the current value of the BAP trace.
 * @param dopa_trace iterator pointing to the current value of the dopamine trace.
 * @param cp synapse type common properties.
 */
template <typename targetidentifierT>
void SynapticSamplingRewardGradientConnection<targetidentifierT>::update_synapse_state(long t_to,
                                                                                       long t_last_update,
                                                                                       TracingNode::const_iterator
                                                                                       &bap_trace,
                                                                                       TracingNode::const_iterator
                                                                                       &dopa_trace,
                                                                                       const CommonPropertiesType& cp)
{
    if ((weight_ == 0.0) && not cp.simulate_retracted_synapses_)
    {
        // synapse is retracted. psps and eligibility traces are not going to be simulated.
        return;
    }

    assert(t_to >= t_last_update);
    long steps = t_to - t_last_update;

    const double sc_psp = weight_ * cp.psp_scale_factor_;
    const bool direct_gradient = cp.direct_gradient_rate_ > 0.0;
    bool psp_active = (psp_facilitation_ != 0.0);

    while( steps )
    {
        // This loop will - considering every call - iterate through EVERY time step (in steps of resolution)

        // decay eligibility trace
        eligibility_trace_ *= cp.eligibility_trace_update_;

        // decay gradient variable
        reward_gradient_ *= cp.reward_gradient_update_;

        // update postsynaptic spike potential
        if (psp_active)
        {
            psp_facilitation_ *= cp.psp_faciliation_update_;
            psp_depression_ *= cp.psp_depression_update_;

            eligibility_trace_ += sc_psp * (psp_facilitation_ - psp_depression_) * (*bap_trace);

            if (psp_facilitation_ < cp.psp_cutoff_amplitude_)
            {
                psp_facilitation_ = 0.0;
                psp_depression_ = 0.0;
                psp_active = false;
            }
        }

        reward_gradient_ += (*dopa_trace) * eligibility_trace_;

        if (direct_gradient)
        {
            synaptic_parameter_ += (*dopa_trace) * cp.learning_rate_ *
                                   cp.direct_gradient_rate_ * eligibility_trace_;
        }

        ++bap_trace;
        ++dopa_trace;
        --steps;
    }
}

/**
 * @brief Updates the synaptic parameter of the synapse.
 *
 * This method implements equation (4).
 *
 * @param thread the thread of the synapse.
 * @param cp the synapse type common properties.
 */
template < typename targetidentifierT >
void SynapticSamplingRewardGradientConnection< targetidentifierT >::
update_synapic_parameter(nest::thread thread, const CommonPropertiesType& cp)
{
    // update synaptic parameters
    const double l_rate = cp.weight_update_interval_ * cp.learning_rate_;

    // compute prior
    const double prior = prior_precision_ * (prior_mean_ - synaptic_parameter_);

    reward_gradient_ += cp.get_gradient_noise(thread);

    const double d_lik = std::max(-cp.max_param_change_,
                                  std::min(cp.max_param_change_, cp.gradient_scale_ * reward_gradient_));

    const double d_param = l_rate * (prior + d_lik) + cp.get_d_wiener(thread);

    synaptic_parameter_ = std::max(cp.min_param_, std::min(cp.max_param_, synaptic_parameter_ + d_param));
}

/**
 * @brief Updates the synaptic weight of the synapse and trigger recording.
 *
 * This method implements equation (5).
 *
 * @param time_step the current time step.
 * @param cp the synapse type common properties.
 */
template < typename targetidentifierT >
void SynapticSamplingRewardGradientConnection< targetidentifierT >::
update_synapic_weight(long time_step, const CommonPropertiesType& cp)
{
    const bool synapse_is_active = (weight_ != 0.0) || (time_step==0);

    // update synaptic weight
    if (synaptic_parameter_ >= 0.0)
    {
        weight_ = cp.weight_scale_ * std::exp(synaptic_parameter_ - cp.parameter_mapping_offset_);
    }
    else
    {
        weight_ = 0.0;
    }

    if (synapse_is_active && not cp.simulate_retracted_synapses_ && (weight_ == 0.0))
    {
        // synapse is entering the retracted state, eligibility trace is reset.
        psp_facilitation_ = 0.0;
        psp_depression_ = 0.0;
        eligibility_trace_ = 0.0;
        reward_gradient_ = 0.0;
    }

    logger()->record(time_step*cp.resolution_unit_, *this, recorder_port_);
}

}

#endif
