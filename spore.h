/* 
 * File:   spore.h
 * Author: Kappel
 *
 * Created on November 9, 2016, 2:26 PM
 */

#ifndef SPORE_H
#define	SPORE_H

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
 *   port.
 * 
 * 
 * @see DiligentConnectorModel, TracingNode.
 */


// specify to enable spore debug tests.
#define __SPORE_DEBUG__ 0

} // namespace spore

#endif	/* SPORE_H */
