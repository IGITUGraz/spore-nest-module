/**
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
 * File:   diligent_connector_model.h
 * Author: Kappel
 *
 * This file is based on connector_model_impl.h which is part of NEST
 * (Copyright (C) 2004 The NEST Initiative).
 * See: http://nest-initiative.org/
 */

#ifndef DILIGENT_CONNECTOR_MODEL_H
#define DILIGENT_CONNECTOR_MODEL_H

#include "spore.h"
#include "nest_time.h"
#include "nest_timeconverter.h"
#include "dictutils.h"
#include "connector_model.h"
#include "connector_base.h"
#include "connection_manager_impl.h"
#include "connector_model_impl.h"
#include "kernel_manager.h"

#include "connection_updater.h"

namespace spore
{

/**
 * @brief Connector model for diligent connections.
 * 
 * Generic connector model for connections that require being updated on a
 * regular time grid. Use the function
 * <i>
 *   register_diligent_connection_model
 * <\i>
 * provided in diligent_connector_model.h, to register your synapse model at
 * the network, to use the diligent synapse model instead of NEST's build-in
 * synapse model.
 * 
 * Diligent connections are connection models that are updated on a regular
 * time grid (as opposed to NEST's standard lazy update). More precisely, the
 * API guarantees that the send function of the synapse is called at least once
 * in a certain time window. If no presynaptic spike is send via the synapse
 * during that time, a "dummy" spike is inserted to trigger a synapse update.
 * The synapse can distinguish between "dummy" spikes and "real" spikes through
 * the receptor port, which is set to the invalid value of -1 for dummy spikes.
 * 
 * The length of the update time window can be controlled using the
 * InitSynapseUpdater SLI function. It takes two arguments, the update interval
 * and the maximum synaptic update latency. Both are tuning parameters that can
 * be used to optimize the simulation performance. Long update intervals are
 * likely more time efficient, but require more memory.
 * 
 * The DiligentConnectorModel provides the interface to the API. New synapse
 * models that use the diligent connection framework should be registered using
 * the register_diligent_connection_model template function. Connections
 * instantiated from this model will be automatically registered at the global
 * ConnectionUpdateManager object that takes care of triggering the synapse
 * updates.
 * 
 * Another difference of diligent connectors is that the calibrate function of
 * their CommonSynapseProperties object is called whenever a simulation is
 * started (same behavior as for nest::Node objects). 
 * 
 * @see ConnectionUpdateManager, SynapseUpdateEvent, spore.h
 * 
 */
template < typename ConnectionT >
class DiligentConnectorModel : public nest::GenericConnectorModel<ConnectionT>
{
public:
    
    DiligentConnectorModel(const std::string name, bool is_primary=true, bool has_delay=true, bool requires_symmetric=false)
    : nest::GenericConnectorModel<ConnectionT>(name, is_primary, has_delay, requires_symmetric)
    {}   

    DiligentConnectorModel(const DiligentConnectorModel &other, const std::string name)
    : nest::GenericConnectorModel<ConnectionT>(other, name)
    {}

    virtual nest::ConnectorBase* add_connection( nest::Node& src, nest::Node& tgt, nest::ConnectorBase* conn,
                                                 nest::synindex syn_id, double weight,
                                                 double delay );

    virtual nest::ConnectorBase* add_connection( nest::Node& src, nest::Node& tgt, nest::ConnectorBase* conn,
                                                 nest::synindex syn_id, DictionaryDatum& d,
                                                 double weight, double delay );

    virtual nest::ConnectorBase* delete_connection( nest::Node& tgt, size_t target_thread,
                                                    nest::ConnectorBase* conn, nest::synindex syn_id );

    virtual nest::ConnectorModel* clone(std::string name) const;

protected:

    void register_connector( nest::ConnectorBase* new_conn, nest::ConnectorBase* old_conn, size_t target_thread, nest::synindex syn_id );

    nest::ConnectorBase* get_hom_connector( nest::ConnectorBase* conn, nest::synindex syn_id );

}; // DiligentConnectorModel


/**
 * Adds a new connection between two nodes.
 *
 * Delay and weight have the default value NAN.
 * NAN is a special value in cmath, which describes double values that
 * are not a number. If delay or weight is omitted in an add_connection call,
 * NAN indicates this and weight/delay are set only, if they are valid.
 *
 * @param tgt source node.
 * @param tgt target node.
 * @param target_thread thread of the target.
 * @param conn Connector Base from where the connection will be deleted.
 * @param syn_id Synapse type.
 * @param delay synaptic delay.
 * @param weight synaptic weight.
 * @return A new Connector, with the inserted new connection.
 */
template < typename ConnectionT >
nest::ConnectorBase*
DiligentConnectorModel< ConnectionT >::add_connection( nest::Node& src,
                                                       nest::Node& tgt,
                                                       nest::ConnectorBase* conn,
                                                       nest::synindex syn_id,
                                                       double delay,
                                                       double weight )
{
    nest::ConnectorBase* old_hom_conn = get_hom_connector( nest::validate_pointer( conn ), syn_id );
    nest::ConnectorBase* new_conn = nest::GenericConnectorModel< ConnectionT >::add_connection(src, tgt, conn, syn_id, delay, weight);
    nest::ConnectorBase* new_hom_conn = get_hom_connector( nest::validate_pointer( new_conn ), syn_id );
    register_connector(new_hom_conn, old_hom_conn, tgt.get_thread(), syn_id );
    return new_conn;
}


/**
 * Adds a new connection between two nodes.
 *
 * Delay and weight have the default value NAN.
 * NAN is a special value in cmath, which describes double values that
 * are not a number. If delay or weight is omitted in an add_connection call,
 * NAN indicates this and weight/delay are set only, if they are valid.
 *
 * @param tgt source node.
 * @param tgt target node.
 * @param target_thread thread of the target.
 * @param conn Connector Base from where the connection will be deleted.
 * @param syn_id Synapse type.
 * @param p dictionary with synapse paramters.
 * @param delay synaptic delay.
 * @param weight synaptic weight.
 * @return A new Connector, with the inserted new connection.
 */
template < typename ConnectionT >
nest::ConnectorBase*
DiligentConnectorModel< ConnectionT >::add_connection( nest::Node& src,
                                                       nest::Node& tgt,
                                                       nest::ConnectorBase* conn,
                                                       nest::synindex syn_id,
                                                       DictionaryDatum& p,
                                                       double delay,
                                                       double weight )
{
    nest::ConnectorBase* old_hom_conn = get_hom_connector( nest::validate_pointer( conn ), syn_id );
    nest::ConnectorBase* new_conn = nest::GenericConnectorModel< ConnectionT >::add_connection(src, tgt, conn, syn_id, p, delay, weight);
    nest::ConnectorBase* new_hom_conn = get_hom_connector( nest::validate_pointer( new_conn ), syn_id );
    register_connector(new_hom_conn, old_hom_conn, tgt.get_thread(), syn_id );
    return new_conn;
}


/**
 * Delete a connection of a given type directed to a defined target Node
 *
 * @param tgt Target node
 * @param target_thread Thread of the target
 * @param conn Connector Base from where the connection will be deleted
 * @param syn_id Synapse type
 * @return A new Connector, equal to the original but with an erased
 * connection to the defined target.
 */
template < typename ConnectionT >
nest::ConnectorBase*
DiligentConnectorModel< ConnectionT >::delete_connection( nest::Node& tgt,
                                                          size_t target_thread,
                                                          nest::ConnectorBase* conn,
                                                          nest::synindex syn_id )
{
    nest::ConnectorBase* old_hom_conn = get_hom_connector( nest::validate_pointer( conn ), syn_id );
    nest::ConnectorBase* new_conn = nest::GenericConnectorModel< ConnectionT >::delete_connection(tgt, target_thread, conn, syn_id);
    nest::ConnectorBase* new_hom_conn = get_hom_connector( nest::validate_pointer( new_conn ), syn_id );
    register_connector(new_hom_conn, old_hom_conn, tgt.get_thread(), syn_id );
    return new_conn;
}


/**
 * Creates a clone of the connector model.
 */
template < typename ConnectionT >
nest::ConnectorModel*
DiligentConnectorModel< ConnectionT >::clone( std::string name ) const
{
    return new DiligentConnectorModel< ConnectionT >( *this, name ); // calls copy construtor
}


/**
 * Registers the connector at the ConnectionUpdateManager.
 */
template < typename ConnectionT >
void DiligentConnectorModel< ConnectionT >::register_connector( nest::ConnectorBase* new_conn,
                                                                nest::ConnectorBase* old_conn,
                                                                size_t target_thread,
                                                                nest::synindex syn_id )
{
    ConnectionUpdateManager::instance()->register_connector( new_conn, old_conn,
                                                             target_thread, this, syn_id );
}


/**
 * Helper function to extract the homogeneous connector with given type
 * from a given connector. If the given connector is homogeneous the
 * type is checked and it is returned. If the type does not match the
 * function fails. If the connector is heterogeneous a connector of given
 * is searched within it and returned if found. Returns 0 if a 0-pointer
 * is passed.
 *
 * @param conn the connector to be checked, can be 0.
 * @param syn_id the type id of the connector to be found.
 * @return the homogeneous connector, 0 if none found.
 */
template < typename ConnectionT >
nest::ConnectorBase* DiligentConnectorModel< ConnectionT >::get_hom_connector( nest::ConnectorBase* conn,
                                                                                    nest::synindex syn_id )
{
    if (!conn)
    {
        return 0;
    }
    else if (conn->homogeneous_model())
    {
        if (conn->get_syn_id() == syn_id)
            return conn;
    }
    else
    {
        // connector is heterogeneous - go through all entries and search for correct syn_id
        nest::HetConnector* hc = static_cast< nest::HetConnector* >( conn );
        for ( size_t i = 0; i < hc->size(); i++ )
        {
            // need to cast to vector_like to access syn_id
            if ( ( *hc )[ i ]->get_syn_id() == syn_id ) // find entry for this type
            {
                return ( *hc )[ i ];
            }
        }
    }
    
    return 0;
}


/**
 * @brief Convenience function to register diligent synapses.
 */
template <class ConnectionT>
void register_diligent_connection_model(const std::string &name, bool requires_symmetric = false)
{
    nest::kernel().model_manager.register_connection_model< ConnectionT, DiligentConnectorModel > (name, requires_symmetric);
}

}

#endif
