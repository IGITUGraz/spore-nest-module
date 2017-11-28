[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1043486.svg)](https://doi.org/10.5281/zenodo.1043486)

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

SPORE requires NEST installed with MUSIC (`-Dwith-music=ON`) and Python bindings. Please make sure your version of NEST corresponds to or is newer than version 2.14.
A detailed installation guide that also shows how to set up the dependencies is provided below and on the [SPORE wiki page](https://github.com/IGITUGraz/spore-nest-module/wiki).

```bash
git clone https://github.com/IGITUGraz/spore-nest-module
cd spore-nest-module
mkdir build && cd build
cmake ..
make
make install
make test
```

## Detailed Installation Guide

This guide describes how to install the SPORE module in version 2.14 for the NEST simulator in version 2.14. Combinining this version of SPORE with other versions of NEST may work but has not been tested.
We used MUSIC version 1.1.15 in revision [8e0a609b298](https://github.com/INCF/MUSIC/commit/8e0a609b29835be604ae556c1592aad9b4be1827) and [MPI](https://www.open-mpi.org/) (Open MPI 1.6.5).
Furthermore we need python with the packages `pyzmq`, `numpy`, `ujson` and `matplotlib`.
A guide to install these dependencies is provided below.
Finally we used SPORE ([version 2.14](https://github.com/IGITUGraz/spore-nest-module/releases/tag/v2.14.0), or later).
The installation procedure was tested on Debian GNU/Linux 8.7 (jessie).

This guide assumes that you want to install everything into your local home folder `$HOME/opt/`.
It is further assumed that you checkout the software into a local folder that is used for development,
which we refer to as `devel` folder.

### Preparation

Add the following lines to your `~/.bashrc` (or `~/.zshrc` or `~/.profile` etc.)

```bash
export TARGET_DIR=$HOME/opt/
export PATH=$PATH:$TARGET_DIR/bin
export LIBRARY_PATH=$LIBRARY_PATH:$TARGET_DIR/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TARGET_DIR/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TARGET_DIR/lib/nest  # required in some cases
export PYTHONPATH=$PYTHONPATH:$TARGET_DIR/lib/python2.7/site-packages
export PYTHONPATH=$PYTHONPATH:$TARGET_DIR/lib64/python2.7/site-packages
export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libmpi.so  # required in some cases (path only works for debian jessie)
export NUM_CORES=3  # number of CPU cores

```

Now run `source ~/.bashrc` or close the current terminal now and start a new session such that the changes get applied.

Now install all dependencies (for Debian Jessie). 

**Notes:**

* Use `pip install --user <package name>` to install the python packages locally.
* Use `sudo pip install <package name>` to install the python packages globally.
* The python packages can also be installed in a [virtualenv](https://virtualenv.pypa.io/en/stable/).
* For python3, use `pip3` to install python packages and install the python3 versions of the packages for NEST dependencies. 
* The use of multiple different python3 versions is discouraged.
  Make sure to consistently use *one* version for installing libraries and actually running experiments.
  When installing MUSIC, the specification of the precise executable (e.g. `python3.4`), rather than `python3`,
  may be necessary in ambigous cases (see [#9](https://github.com/IGITUGraz/spore-nest-module/issues/9)).

```bash
# For NEST and MUSIC
apt-get install build-essential cmake
apt-get install python python-dev python-pip
apt-get install libreadline-dev gsl-bin libgsl0-dev libncurses5-dev openmpi-bin

apt-get install automake libtool # BUILD Dependencies
apt-get install libopenmpi-dev libopenmpi1.6 # RUN Dependencies
apt-get install freeglut3-dev # Optional, for viewevents
pip install cython
pip install mpi4py # RUN Dependencies

# For SPORE
apt-get install libzmq3  # For realtime plotting
pip install pyzmq # For real-time plotting
pip install numpy  # For testing
pip install ujson
pip install --upgrade matplotlib  # (Optional, If you want to have nice plotting, you should upgrade `matplotlib` to the newest version)
```


### Installing MUSIC

In your `devel` folder, check out the latest version of MUSIC from https://github.com/INCF/MUSIC

```bash
git clone https://github.com/INCF/MUSIC.git
```

Note that at the moment `--disable-isend` is required because
of a [critical problem](https://github.com/INCF/MUSIC/issues/35#issuecomment-280332573)
in the MUSIC scheduler.

In the folder `./MUSIC/` :

```bash
./autogen.sh
PYTHON=/usr/bin/python ./configure --prefix=$TARGET_DIR --disable-isend  # Replace with python3[.x] if desired
make -j$NUM_CORES
make install
```

### Installing NEST

In your `devel` folder, check out the latest version of NEST from https://github.com/nest/nest-simulator

```bash
# use the latest developmental version of NEST. Release versions don't currently work with SPORE
git clone https://github.com/nest/nest-simulator.git
```
Then in the folder `./nest-simulator` :

```bash
mkdir build
cd build/
cmake -DCMAKE_INSTALL_PREFIX:PATH=$TARGET_DIR -Dwith-music=ON -Dwith-mpi=ON -Dwith-python=2 ..  # Change python version to 3 for Python 3
make -j$NUM_CORES
make install
```

### Installing SPORE

In your `devel` folder, check out the latest version of SPORE from https://github.com/IGITUGraz/spore-nest-module

```bash
git clone https://github.com/IGITUGraz/spore-nest-module.git
```

Then in the folder `./spore-nest-module` :

```bash
mkdir build
cd build/
cmake -Dwith-python=2 ..  # Change python version to 3 for Python 3, or provide a path to a python binary
make -j$NUM_CORES
make install
make test
```

In `ipython` (or `ipython3`) running `import nest` and then `nest.Install("sporemodule")` should now yield the following:

```
In [1]: import nest
[INFO] [.../rng_manager.cpp:238 @ Network::create_rngs_] : Creating default RNGs
[INFO] [.../rng_manager.cpp:233 @ Network::create_rngs_] : Deleting existing random number generators
[INFO] [.../rng_manager.cpp:238 @ Network::create_rngs_] : Creating default RNGs
[INFO] [.../rng_manager.cpp:284 @ Network::create_grng_] : Creating new default global RNG

              -- N E S T --

  Copyright (C) 2004 The NEST Initiative
  Version 2.14.0 Oct 30 2017 13:06:41

This program is provided AS IS and comes with
NO WARRANTY. See the file LICENSE for details.

Problems or suggestions?
  Visit http://www.nest-simulator.org

Type 'nest.help()' to find out more about NEST.

In [2]: nest.Install("sporemodule")

Oct 31 13:26:30 Install [Info]: 
    loaded module SPORE (version 2.14.0)
```

## Running a first experiment

That should be it.

Now you should be able to execute the script [`experiment.py`](https://github.com/IGITUGraz/spore-nest-module/blob/master/examples/pattern_matching_showcase/experiment.py) in

`spore-nest-module/examples/pattern_matching_showcase`.

## Cite SPORE

Please cite the current version of SPORE as

```
David Kappel, Michael Hoff, & Anand Subramoney. (2017, November 7).
IGITUGraz/spore-nest-module: SPORE version 2.14.0 (Version v2.14.0).
Zenodo. http://doi.org/10.5281/zenodo.1043486
```

## License

SPORE is open source software and is licensed under the [GNU General Public
License v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html) or later.

