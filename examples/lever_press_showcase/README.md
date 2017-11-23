# Lever Press Experiment

This experiment demonstrates the capabilities of the [NEST](https://github.com/nest/nest-simulator) module
[SPORE](https://github.com/IGITUGraz/spore-nest-module) in interplay with coordination & communication framework [MUSIC](https://github.com/INCF/MUSIC).
Hereby, a spiking neural network has to learn to produce a movement pattern by controlling a virtual lever.

Run this experiment using:

```
python3 experiment.py
```

The neural network topology consists of a set of input, a set of hidden and a set of output neurons.
A subset of hidden neurons receives input from afferent input neurons and a disjoint subset is chosen to control the lever. The population of hidden neurons is recurrently connected and, in addition, inhibitory neurons are added.
Excitatory synapse adapts their synaptic weight and connectivity through reward-based synaptic sampling.

Incoming and outgoing spikes, as well as the incoming reward signal are communicated via MUSIC.
This framework allows a separate process, a python node, to generate the input spikes based on the current pattern and generate
the reward signal based on the current output activity of the network.
The network output is interpreted by splitting the output neurons into sub populations of equal size each of which corresponds to either upward or downward movement of the lever.
The spike activity of each sub population is then interpreted as the strength of the vote for the lever position.

