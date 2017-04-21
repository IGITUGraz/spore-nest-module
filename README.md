# NEST module *SPORE*: Synaptic Plasticity with Online Reinforcement learning

SPORE is a custom-module for [NEST](http://www.nest-simulator.org/) that provides
a simulation framework for reward-based learning with spiking neurons. SPORE
utilizes [MUSIC](https://github.com/INCF/MUSIC) for communication with other
software components.

*Note:* this is an outdated version of SPORE for backward compatibility with
NEST 2.10 only. We strongly recommend to use the recent version of the module
which can be found [here](https://github.com/IGITUGraz/spore-nest-module).

## Installation

Requires NEST with MUSIC bindings.

```bash
./bootstrap.sh
./configure
make install
```
