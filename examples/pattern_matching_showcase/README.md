# Detailed installation guide to the SPORE software framework

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

