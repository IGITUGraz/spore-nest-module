/* 
 * File:   spore.h
 * Author: Kappel
 *
 * Created on November 9, 2016, 2:26 PM
 */

#ifndef SPORE_H
#define	SPORE_H

/**
 * NEST::SPORE (Synaptic Plasticity Online REinforcement learning)
 * is a NEST module that features a collection on neurons and synapses
 * that implement models based on ideas from probabilistic inference (PI)
 * via sampling. It also provides an API to develop such models
 * efficiently in NEST.
 * 
 * The main features of the module are:
 * 
 * - It introduces a synapse model that is guaranteed to be updated on a
 *   regular time grid (diligent connections).
 * 
 * - It introduces TracingNode, new class of nodes that can store a trace
 *   of a real-valued variable (e.g. the membrane potential) for a fixed time
 *   window. Connections may use these traces to update their state. This is
 *   more flexible and more suitable to implement synapse models based on
 *   PI via sampling, than NEST's plasticity models based on ArchivingNode
 *   which only allows to store spike events. 
 * 
 * @see DiligentConnectorModel, TracingNode.
 */


// specify to enable spore debug tests.
#define __SPORE_DEBUG__


#endif	/* SPORE_H */

