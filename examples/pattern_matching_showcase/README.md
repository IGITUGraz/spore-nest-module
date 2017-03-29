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

The learning process is visualized by a third node, which is reading and, in turn, plotting all the information available.

[![Plotting Animation][2]][1]

[1]: https://cloud.githubusercontent.com/assets/22887425/24467479/f7235d1c-14b4-11e7-8ecf-ba19931d7f8d.gif
[2]: https://cloud.githubusercontent.com/assets/22887425/24467512/1aa0caa4-14b5-11e7-9217-29dcf48ac349.gif (Follow link for higher resolution and longer recording)

# Detailed installation guide for the SPORE software framework

This guide describes how to install the software framework that was built around the SPORE module for the NEST simulator. It has been tested with Debian GNU/Linux 8.7 (jessie).

This guide assumes that you want to install everything into the folder `$HOME/opt/`. It is further assumed that you checkout the software into a local folder that is used for development, which we refer to as `devel` folder.

## Preparation

First make these exports, which will be used by the bash commands below.

```bash
export TARGET_DIR=$HOME/opt/
export LOCAL_ENV_SCRIPT=~/.bashrc
export NUM_CORES=4
```

Run these lines of code in a terminal (will add some export commands to your `~/.bashrc` file)

```bash
echo "export PATH=\$PATH:$TARGET_DIR/bin" >> $LOCAL_ENV_SCRIPT
echo "export LIBRARY_PATH=\$LIBRARY_PATH:$TARGET_DIR/lib" >> $LOCAL_ENV_SCRIPT
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$TARGET_DIR/lib" >> $LOCAL_ENV_SCRIPT
echo "export PYTHONPATH=\$PYTHONPATH:$TARGET_DIR/lib/python2.7/site-packages" >> $LOCAL_ENV_SCRIPT
echo "export PYTHONPATH=\$PYTHONPATH:$TARGET_DIR/lib64/python2.7/site-packages" >> $LOCAL_ENV_SCRIPT
```

Best close the current terminal now and start a new session such that the changes get applied.

## Installing MUSIC

In your `devel` folder, check out the latest version of MUSIC from https://github.com/INCF/MUSIC

```bash
git clone git@github.com:INCF/MUSIC.git
```

Note that at the moment `--disable-isend` is required because
of a [critical problem](https://github.com/INCF/MUSIC/issues/35#issuecomment-280332573)
in the MUSIC scheduler.

In the folder `./MUSIC/` :

```bash
./autogen.sh
./configure --prefix=$TARGET_DIR --disable-isend
make -j$NUM_CORES
make install
```

## Installing NEST

In your `devel` folder, check out the latest version of NEST from https://github.com/nest/nest-simulator

```bash
git clone git@github.com:nest/nest-simulator.git
```
Then in the folder `./nest-simulator` :

```bash
mkdir build
cd build/
cmake -DCMAKE_INSTALL_PREFIX:PATH=$TARGET_DIR -Dwith-music=ON -Dwith-mpi=ON ..
make -j$NUM_CORES
make install
```

## Installing SPORE

In your `devel` folder, check out the latest version of SPORE from https://github.com/IGITUGraz/spore-nest-module

```bash
git clone git@github.com:IGITUGraz/spore-nest-module.git
```

Then in the folder `./spore-nest-module` :

```bash
mkdir build
cd build/
cmake ..
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

## Install ujson

Finally you have to install ujson using:

```bash
pip install --user ujson
```

## Optional installs

If you want to have nice plotting, you should upgrade `matplotlib`

```bash
pip install --user --upgrade matplotlib
```

## Running experiment

That should be it.

Now you should be able to execute the script `experiment.sh` in `spore-nest-module/examples/pattern_matching_showcase`.

