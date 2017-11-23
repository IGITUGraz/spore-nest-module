# Center-out Reaching Experiment

This experiment demonstrates the capabilities of the [NEST](https://github.com/nest/nest-simulator) module
[SPORE](https://github.com/IGITUGraz/spore-nest-module) in interplay with coordination & communication framework [MUSIC](https://github.com/INCF/MUSIC).
Hereby, a spiking neural network has to learn to reach for a target region from an initial center position of a virtual cursor on a 2D surface.

Run this experiment using:

python3 experiment.py

The neural network topology consists of a set of input, a set of hidden and a set of output neurons.
A subset of hidden neurons receives input from afferent input neurons and a disjoint subset is chosen to control the lever. The population of hidden neurons is recurrently connected and, in addition, inhibitory neurons are added.
Excitatory synapse adapts their synaptic weight and connectivity through reward-based synaptic sampling.

Incoming and outgoing spikes, as well as the incoming reward signal are communicated via MUSIC.
This framework allows a separate process, a python node, to generate the input spikes based on the current pattern and generate
the reward signal based on the current output activity of the network.
The network output is interpreted by assigning to each output neuron a random movement direction for the x- or y-position of the cursor.
The combined spike activity of all output neurons is then interpreted as the movement direction of the cursor. In each trial the cursor is initialized at the center position and the target area has to be reached while a cue pattern is presented. After reaching the target the cursor has to be held in the target area for a brief holding period to receive a reward. Unsuccessful trials are aborted and no reward is delivered.

