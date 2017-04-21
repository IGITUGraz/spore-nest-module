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

# Detailed installation guide for the SPORE software framework

This guide describes how to install the SPORE module for the NEST 2.12 simulator. Note that SPORE does not support the release version v2.12.0 of NEST. Instead, SPORE currently requires a fix on the development branch so please make sure your development version of NEST corresponds to or is newer than revision [58fd190f5e4](https://github.com/nest/nest-simulator/commit/58fd190f5e404f1e3e822c0d3915e2321d102ed5). We used MUSIC version 1.1.15 in revision [8e0a609b298](https://github.com/INCF/MUSIC/commit/8e0a609b29835be604ae556c1592aad9b4be1827) and we assume that [MPI](https://www.open-mpi.org/) (Open MPI 1.6.5) and python with `numpy` and `matplotlib` are preinstalled on your system. Finally we used revision [319937d6f1a](https://github.com/IGITUGraz/spore-nest-module/commit/319937d6f1ab1bdfca9c16a5daa9770229af5bc6) of SPORE (version 2.12). The installation procedure was tested on Debian GNU/Linux 8.7 (jessie).

This guide assumes that you want to install everything into your local home folder `$HOME/opt/`. It is further assumed that you checkout the software into a local folder that is used for development, which we refer to as `devel` folder.

## Preparation

Add the following lines to your `~/.bashrc` (or `~/.zshrc` or `~/.profile` etc.)

```bash
export TARGET_DIR=$HOME/opt/
export PATH=$PATH:$TARGET_DIR/bin
export LIBRARY_PATH=$LIBRARY_PATH:$TARGET_DIR/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TARGET_DIR/lib
export PYTHONPATH=$PYTHONPATH:$TARGET_DIR/lib/python2.7/site-packages
export PYTHONPATH=$PYTHONPATH:$TARGET_DIR/lib64/python2.7/site-packages
export LD_PRELOAD=/usr/lib/openmpi/lib/libmpi.so  # Required in some cases
export NUM_CORES=3

```

Now run `source ~/.bashrc` or close the current terminal now and start a new session such that the changes get applied.

Now install all dependencies (for Debian Jessie). 

Notes:

* Use `pip install --user <package name>` to install the python packages locally.
* Use `sudo pip install <package name>` to install the python packages globally.
* The python packages can also be installed in a [virtualenv](https://virtualenv.pypa.io/en/stable/).
* For python3, use `pip3` to install python packages and install the python3 versions of the packages for NEST dependencies. 
* Python3 is NOT officially supported by this package, but has been known to work occasionally.
* The use of multiple different python3 versions is discouraged.
  Make sure to consistently use *one* version for installing libraries and actually running experiments.
  When installing MUSIC, the specification of the precise executable (e.g. `python3.4`), rather than `python3`,
  may be necessary in ambigous cases (see [#9](https://github.com/IGITUGraz/spore-nest-module/issues/9)).

```bash
## For MUSIC
apt-get install automake libtool # BUILD Dependencies
apt-get install libopenmpi-dev libopenmpi1.6 # RUN Dependencies
apt-get install freeglut3-dev # Optional, for viewevents
pip install mpi4py # RUN Dependencies

## For NEST
apt-get install build-essential python python-dev python-pip libreadline-dev gsl-bin libgsl0-dev libncurses5-dev cmake openmpi-bin

## For SPORE
apt-get install libzmp3  # For realtime plotting
pip install pyzmq # For real-time plotting
pip install numpy  # For testing
pip install ujson
pip install --upgrade matplotlib  # (Optional, If you want to have nice plotting, you should upgrade `matplotlib` to the newest version)
```


## Installing MUSIC

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

## Installing NEST

In your `devel` folder, check out the latest version of NEST from https://github.com/nest/nest-simulator

```bash
git clone https://github.com/nest/nest-simulator.git  # NEST release versions don't currently work with SPORE
```
Then in the folder `./nest-simulator` :

```bash
mkdir build
cd build/
cmake -DCMAKE_INSTALL_PREFIX:PATH=$TARGET_DIR -Dwith-music=ON -Dwith-mpi=ON -Dwith-python=2 ..  # Change python version to 3 for Python 3
make -j$NUM_CORES
make install
```

## Installing SPORE

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

In `ipython` running `import nest` and then `nest.Install("sporemodule")` should now yield the following:

```
In [1]: import nest
[INFO] [2017.3.29 12:5:24 /home/YOU/devel/nest-simulator/nestkernel/rng_manager.cpp:226 @ Network::create_rngs_] : Creating default RNGs
[INFO] [2017.3.29 12:5:24 /home/YOU/devel/nest-simulator/nestkernel/rng_manager.cpp:221 @ Network::create_rngs_] : Deleting existing random number generators
[INFO] [2017.3.29 12:5:24 /home/YOU/devel/nest-simulator/nestkernel/rng_manager.cpp:226 @ Network::create_rngs_] : Creating default RNGs
[INFO] [2017.3.29 12:5:24 /home/YOU/devel/nest-simulator/nestkernel/rng_manager.cpp:272 @ Network::create_grng_] : Creating new default global RNG

              -- N E S T --

  Copyright (C) 2004 The NEST Initiative
  Version 2.12.0 Mar 29 2017 11:54:48

This program is provided AS IS and comes with
NO WARRANTY. See the file LICENSE for details.

Problems or suggestions?
  Visit http://www.nest-simulator.org

Type 'nest.help()' to find out more about NEST.

In [2]: nest.Install("sporemodule")

Mar 29 12:05:33 Install [Info]: 
    loaded module SPORE (version 2.12.0)
```

## Running experiment

That should be it.

Now you should be able to execute the script [`experiment.py`](https://github.com/IGITUGraz/spore-nest-module/blob/master/examples/pattern_matching_showcase/experiment.py) in

`spore-nest-module/examples/pattern_matching_showcase`.

