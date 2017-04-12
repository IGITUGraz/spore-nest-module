/*
 * This file is part of SPORE.
 *
 * SPORE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * SPORE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SPORE.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For more information see: https://github.com/IGITUGraz/spore-nest-module
 *
 * File:   spore.h
 * Author: Kappel
 *
 * Created on November 9, 2016, 2:26 PM
 */

#ifndef SPORE_H
#define SPORE_H

/**
 * @brief Global namespace holding all classes of the SPORE NEST module.
 *
 * @see DiligentConnectorModel, ConnectionUpdateManager, TracingNode, RewardInProxy
 */
namespace spore
{

/**
 * @mainpage
 * 
 * <b>SPORE</b> (<b>S</b>ynaptic <b>P</b>lasticity with <b>O</b>nline <b>Re</b>inforcement learning)
 * is a NEST module that features a collection of neuron and synapse models and
 * infrastructure targeting reward-based learning algorithms.
 * 
 * The source code of the project is available form here: https://github.com/IGITUGraz/spore-nest-module
 * 
 * The main features of the module are:
 * 
 * - It introduces a new class of synapse models that is guaranteed to be
 *   updated on a regular time grid (DiligentConnectorModel).
 * 
 * - It introduces TracingNode, a new class of NEST nodes that can store a
 *   history of a pice-wise constant real-valued time series (e.g. the membrane
 *   potential) for a fixed time window. We call these histories traces.
 *   Connections may use these traces to update their state, e.g. to implement
 *   a reward mechanism or for learning rules that depend on the somatic
 *   potential. This is in many cases more suitable to implementations of
 *   synapse models that were developed for reward-based learning.
 * 
 * - It introduces a MUSIC proxy that allows to receive traces from a MUSIC
 *   port (RewardInProxy).
 * 
 * 
 * @see DiligentConnectorModel, TracingNode, RewardInProxy
 */

// version number of the module
#define __SPORE_VERSION__ "2.12.0"

// specify to enable spore debug tests.
#define __SPORE_DEBUG__ 0

}

#endif
