# Pattern Matching Experiment

This experiment demonstrates the capabilities of the [NEST](https://github.com/nest/nest-simulator) module
[SPORE](https://github.com/IGITUGraz/spore-nest-module) in interplay with coordination & communication framework [MUSIC](https://github.com/INCF/MUSIC).
Hereby, a spiking neural network has to learn to differentiate between a number of different spike patterns.

The neural network topology consists of a set of input and a set of output neurons.
These groups are all-to-all connected with several connections utilizing a synapse model with reward-based learning.
This type of synapse adapts its synaptic weight by taking into account the local spike history and a global neuromodulatory reward signal.

Incoming and outgoing spikes, as well as the incoming reward signal are communicated via MUSIC.
This framework allows a separate process, a python node, to generate the input spikes based on the current pattern and generate
the reward signal based on the current output activity of the network.
The network output is interpreted by splitting the output neurons into sub populations of equal size each of which corresponds to  one of the patterns (we use 2 patterns/sub populations in this example).
The spike activity of each sub population is then interpreted as the strength of the vote for the pattern corresponding to this sub population.
Consequently, the strongest active sub population determines which pattern the network has detected and the difference to the weaker activities can be
seen as the certainty of this decision.

The learning process is visualized by a third node, which is reading and, in turn, broadcasting all the information available via ZMQ to allow for live plotting in a process completely separate to the simulation.

[![Plotting Animation][2]][1]

[1]: https://cloud.githubusercontent.com/assets/22887425/24467479/f7235d1c-14b4-11e7-8ecf-ba19931d7f8d.gif
[2]: https://cloud.githubusercontent.com/assets/22887425/24467512/1aa0caa4-14b5-11e7-9217-29dcf48ac349.gif (Follow link for higher resolution and longer recording)

