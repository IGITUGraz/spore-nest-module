# NEST module SPORE:<br> Synaptic Plasticity with Online Reinforcement learning

SPORE is a custom-module for [NEST](http://www.nest-simulator.org/) that provides a simulation framework
for reward-based learning with spiking neurons. SPORE utilizes [MUSIC](https://github.com/INCF/MUSIC)
for communication with other software components.

* A detailed installation guide for SPORE and all its dependencies can be found on the [SPORE wiki page](https://github.com/IGITUGraz/spore-nest-module/wiki).

* A detailed documentation of the module can be found in the [SPORE class reference](https://igitugraz.github.io/spore-nest-module/).

* We also provide a first [example](https://github.com/IGITUGraz/spore-nest-module/blob/master/examples/pattern_matching_showcase) that demonstrates a typical use case of SPORE.

[![Plotting Animation][1]][2]

[1]: https://cloud.githubusercontent.com/assets/22887425/24467479/f7235d1c-14b4-11e7-8ecf-ba19931d7f8d.gif
[2]: https://github.com/IGITUGraz/spore-nest-module/blob/master/examples/pattern_matching_showcase

## Quick Installation Guide

SPORE requires NEST installed with MUSIC (`-Dwith-music=ON`) and Python bindings. Please make sure your version of NEST corresponds to or is newer than revision
[58fd190f5e4](https://github.com/nest/nest-simulator/commit/58fd190f5e404f1e3e822c0d3915e2321d102ed5).
A detailed installation guide that also shows how to set up the dependencies is provided on the [SPORE wiki page](https://github.com/IGITUGraz/spore-nest-module/wiki).

```bash
git clone https://github.com/IGITUGraz/spore-nest-module
cd spore-nest-module
mkdir build && cd build
cmake ..
make
make install
make test
```

## License

SPORE is open source software and is licensed under the [GNU General Public
License v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html) or later.

