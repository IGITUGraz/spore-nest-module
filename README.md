# NEST module SPORE:<br> Synaptic Plasticity with Online Reinforcement learning

SPORE is a custom-module for [NEST](http://www.nest-simulator.org/) that provides a simulation framework
for reward-based learning with spiking neurons. SPORE utilizes [MUSIC](https://github.com/INCF/MUSIC)
for communication with other software components.

The documentation of the module can be found [here](https://igitugraz.github.io/spore-nest-module/).

[Here](https://github.com/IGITUGraz/spore-nest-module/blob/master/examples/pattern_matching_showcase)
you find an example use case and a detailed description how to set up SPORE to run a first experiment.

[![Plotting Animation][1]][2]

[1]: https://cloud.githubusercontent.com/assets/22887425/24467479/f7235d1c-14b4-11e7-8ecf-ba19931d7f8d.gif
[2]: https://github.com/IGITUGraz/spore-nest-module/blob/master/examples/pattern_matching_showcase

## Quick Installation Guide

### SPORE for NEST v2.12.0+

SPORE does not support the release version v2.12.0 of NEST.
Instead, SPORE currently requires a fix on the development branch.
Please make sure your development version of NEST corresponds to
or is newer than revision
[b8ad1a51dd](https://github.com/nest/nest-simulator/commit/b8ad1a51dd8141190f1f06381a8a72499dfc453f).

Further, we assume NEST go compiled and installed with MUSIC
bindings (`-Dwith-music=ON`).

```bash
git clone https://github.com/IGITUGraz/spore-nest-module
cd spore-nest-module
mkdir build && cd build
cmake ..
make
make install
make test
```

### SPORE for NEST v2.10.0

We assume NEST got compiled and installed with MUSIC bindings
(`--with-music=$MUSIC_INSTALL_PATH`).

```bash
git clone -b v2.10.0 https://github.com/IGITUGraz/spore-nest-module
cd spore-nest-module
./bootstrap.sh
./configure.sh
make
make install
make test
```

## License

SPORE is open source software and is licensed under the [GNU General Public
License v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html) or later.
