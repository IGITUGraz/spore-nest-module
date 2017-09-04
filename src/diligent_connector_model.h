/*
 * This file is part of SPORE.
 *
 * Copyright (C) 2016, the SPORE team (see AUTHORS).
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

#include "connector_model_impl.h"

#include "spore.h"
#include "connection_updater.h"


namespace spore
{

/**
 * @brief Connector model for diligent connections.
 *
 * Generic connector model for connections that require being updated on a
 * regular time grid. Use the function spore::register_diligent_connection_model
 * provided in diligent_connector_model.h, to register your synapse model at
 * the network, to use the diligent synapse model instead of NEST's build-in
 * synapse model.
 *
 * Diligent connections are connection models that are updated on a regular
 * time grid (as opposed to NEST's standard lazy update). More precisely, the
 * API guarantees that the \a send function of the synapse is called at least once
 * in a certain time window. If no presynaptic spike is send via the synapse
 * during that time, a "dummy" spike is inserted to trigger a synapse update.
 * The synapse can distinguish between "dummy" spikes and "real" spikes through
 * the receptor port, which is set to the invalid value of -1 for dummy spikes
 * (see SynapseUpdateEvent). The update interval can be controlled using the
 * \a InitSynapseUpdater SLI function (see ConnectionUpdateManager)
 *
 * The DiligentConnectorModel provides the interface to this mechanism. New synapse
 * models that use the diligent connection framework should be registered using
 * the register_diligent_connection_model template function. Connections
 * instantiated from this model will be automatically registered at the global
 * ConnectionUpdateManager object that takes care of triggering the synapse
 * updates.
 *
 * Synapses registered with the DiligentConnectorModel must implement a method
 * called \a is_degenerated that takes no arguments and returns a \c bool. This
 * is used by the garbage collector of the ConnectionUpdateManager to identify
 * synapses that need to be deleted. The mechanism is described in detail in
 * the documentation of ConnectionUpdateManager.
 *
 * Another difference diligent connectors and standard NEST connections is that
 * the calibrate function of their \a CommonSynapseProperties object is called
 * additionally on simulation startup (such as nest::Node objects).
 *
 * @see ConnectionUpdateManager, SynapseUpdateEvent, SynapticSamplingRewardGradientConnection
 *
 */
template < typename ConnectionT >
class DiligentConnectorModel : public nest::GenericConnectorModel<ConnectionT>
{
public:
    /**
     * Constructor.
     */
    DiligentConnectorModel(const std::string name, bool is_primary = true,
                           bool has_delay = true, bool requires_symmetric = false)
    : nest::GenericConnectorModel<ConnectionT>(name, is_primary, has_delay, requires_symmetric)
    {
    }

    /**
     * Constructor.
     */
    DiligentConnectorModel(const DiligentConnectorModel& other, const std::string name)
    : nest::GenericConnectorModel<ConnectionT>(other, name)
    {
    }

    virtual nest::ConnectorBase* add_connection(nest::Node& src, nest::Node& tgt, nest::ConnectorBase* conn,
                                                nest::synindex syn_id, double weight,
                                                double delay);

    virtual nest::ConnectorBase* add_connection(nest::Node& src, nest::Node& tgt, nest::ConnectorBase* conn,
                                                nest::synindex syn_id, DictionaryDatum& d,
                                                double weight, double delay);

    virtual nest::ConnectorBase* delete_connection(nest::Node& tgt, size_t target_thread,
                                                   nest::ConnectorBase* conn, nest::synindex syn_id);

    virtual nest::ConnectorModel* clone(std::string name) const;

protected:

    nest::ConnectorBase* cleanup_delete_connection(nest::Node& tgt, const size_t target_thread,
                                                   nest::ConnectorBase* const conn, const nest::synindex syn_id);

    void register_connector(nest::ConnectorBase* new_conn, nest::ConnectorBase* old_conn, nest::index sender_gid,
                            size_t target_thread, nest::synindex syn_id);

    nest::ConnectorBase* get_hom_connector(nest::ConnectorBase* conn, nest::synindex syn_id);

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
DiligentConnectorModel< ConnectionT >::add_connection(nest::Node& src,
                                                      nest::Node& tgt,
                                                      nest::ConnectorBase* conn,
                                                      nest::synindex syn_id,
                                                      double delay,
                                                      double weight)
{
    nest::ConnectorBase* old_hom_conn = get_hom_connector(nest::validate_pointer(conn), syn_id);
    nest::ConnectorBase* new_conn = nest::GenericConnectorModel< ConnectionT >::add_connection(src, tgt, conn,
                                                                                               syn_id, delay, weight);
    nest::ConnectorBase* new_hom_conn = get_hom_connector(nest::validate_pointer(new_conn), syn_id);
    register_connector(new_hom_conn, old_hom_conn, src.get_gid(), tgt.get_thread(), syn_id);
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
DiligentConnectorModel< ConnectionT >::add_connection(nest::Node& src,
                                                      nest::Node& tgt,
                                                      nest::ConnectorBase* conn,
                                                      nest::synindex syn_id,
                                                      DictionaryDatum& p,
                                                      double delay,
                                                      double weight)
{
    nest::ConnectorBase* old_hom_conn = get_hom_connector(nest::validate_pointer(conn), syn_id);
    nest::ConnectorBase* new_conn = nest::GenericConnectorModel< ConnectionT >::add_connection(src, tgt, conn, syn_id,
                                                                                               p, delay, weight);
    nest::ConnectorBase* new_hom_conn = get_hom_connector(nest::validate_pointer(new_conn), syn_id);
    register_connector(new_hom_conn, old_hom_conn, src.get_gid(), tgt.get_thread(), syn_id);
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
DiligentConnectorModel< ConnectionT >::delete_connection(nest::Node& tgt,
                                                         size_t target_thread,
                                                         nest::ConnectorBase* conn,
                                                         nest::synindex syn_id)
{
    nest::ConnectorBase* old_hom_conn = get_hom_connector(nest::validate_pointer(conn), syn_id);
    nest::ConnectorBase* new_conn = cleanup_delete_connection(tgt, target_thread, conn, syn_id);
    nest::ConnectorBase* new_hom_conn = get_hom_connector(nest::validate_pointer(new_conn), syn_id);
    register_connector(new_hom_conn, old_hom_conn, nest::invalid_index, tgt.get_thread(), syn_id);
    return new_conn;
}

/**
 * Creates a clone of the connector model.
 */
template < typename ConnectionT >
nest::ConnectorModel*
DiligentConnectorModel< ConnectionT >::clone(std::string name) const
{
    return new DiligentConnectorModel< ConnectionT >(*this, name); // calls copy construtor
}

/**
 * Registers the connector at the ConnectionUpdateManager.
 */
template < typename ConnectionT >
void DiligentConnectorModel< ConnectionT >::register_connector(nest::ConnectorBase* new_conn,
                                                               nest::ConnectorBase* old_conn,
                                                               nest::index sender_gid,
                                                               size_t target_thread,
                                                               nest::synindex syn_id)
{
    ConnectionUpdateManager::instance()->register_connector(new_conn, old_conn, sender_gid,
                                                            target_thread, syn_id);
}

/**
 * @brief Cleanup and delete the given connection.
 *
 * This deviates from the standard NEST implementation in that it
 * preferably deletes synapses that are marked for deletion instead of
 * taking the first synapse that matches the specifications. Synapses
 * indicate that they are marked for deletion by returning \c true from
 * their \a is_degenerated method. If no connection is found that
 * is marked for deletion this function behaves the same way as
 * \a delete_connection of NEST's generic connector model.
 *
 * @param tgt Target node
 * @param target_thread Thread of the target
 * @param conn Connector Base from where the connection will be deleted
 * @param syn_id Synapse type
 * @return A new Connector, equal to the original but with an erased
 * connection to the defined target.
 */
template < typename ConnectionT >
nest::ConnectorBase* DiligentConnectorModel< ConnectionT >::cleanup_delete_connection(nest::Node& tgt,
                                                                                      const size_t target_thread,
                                                                                      nest::ConnectorBase* const conn,
                                                                                      const nest::synindex syn_id)
{
    using nest::HetConnector;
    using nest::vector_like;
    using nest::pack_pointer;
    using nest::kernel;

    assert(conn != 0); // we should not delete not existing synapses
    bool found = false;
    vector_like< ConnectionT >* vc;

    const bool b_has_primary = has_primary(conn);
    const bool b_has_secondary = has_secondary(conn);

    nest::ConnectorBase* conn_vp = validate_pointer(conn);

    // from here on we can use conn as a valid pointer

    if (conn_vp->homogeneous_model())
    {
        assert(conn_vp->get_syn_id() == syn_id);
        vc = static_cast<vector_like< ConnectionT >*> (conn_vp);
        // delete the first Connection corresponding to the target
        for (size_t i = 0; i < vc->size(); i++)
        {
            ConnectionT* connection = &vc->at(i);

            // only remove synapse if marked for deletion.
            if ((connection->get_target(target_thread)->get_gid() == tgt.get_gid()) && connection->is_degenerated())
            {
                if (vc->get_num_connections() > 1)
                    conn_vp = &vc->erase(i);
                else
                {
                    delete vc;
                    conn_vp = 0;
                }
                const bool is_primary = DiligentConnectorModel< ConnectionT >::is_primary_;
                if (conn_vp != 0)
                {
                    conn_vp = pack_pointer(conn_vp, is_primary, !is_primary);
                }
                found = true;
                break;
            }
        }
    }
    else
    {
        // heterogeneous case
        // go through all entries and search for correct syn_id
        // if not found create new entry for this syn_id
        HetConnector* hc = static_cast< HetConnector* > (conn_vp);

        for (size_t i = 0; i < hc->size() && !found; i++)
        {
            // need to cast to vector_like to access syn_id there is already an entry
            // for this type
            if ((*hc)[ i ]->get_syn_id() == syn_id)
            {
                // here we know that the type is vector_like<connectionT>, because
                // syn_id agrees so we can safely static cast
                vector_like< ConnectionT >* vc =
                        static_cast< vector_like< ConnectionT >* > ((*hc)[ i ]);
                // Find and delete the first Connection corresponding to the target
                for (size_t j = 0; j < vc->size(); j++)
                {
                    ConnectionT* connection = &vc->at(j);

                    if ((connection->get_target(target_thread)->get_gid() == tgt.get_gid()) &&
                        connection->is_degenerated()) // only remove synapse if marked for deletion.
                    {
                        // Get rid of the ConnectionBase for this type of synapse if there
                        // is only this element left
                        if (vc->size() == 1)
                        {
                            (*hc).erase((*hc).begin() + i);
                            // Test if the homogeneous vector of connections went back to only
                            // 1 type of synapse... then go back to the simple vector_like
                            // case.
                            if (hc->size() == 1)
                            {
                                conn_vp = (*hc)[ 0 ];
                                const bool is_primary =
                                    kernel().model_manager.get_synapse_prototype(conn_vp->get_syn_id()).is_primary();
                                conn_vp = pack_pointer(conn_vp, is_primary, not is_primary);
                            }
                            else
                            {
                                conn_vp = pack_pointer(hc, b_has_primary, b_has_secondary);
                            }
                        } // Otherwise, just remove the desired connection
                        else
                        {
                            (*hc)[ i ] = &vc->erase(j);
                            conn_vp = pack_pointer(hc, b_has_primary, b_has_secondary);
                        }
                        found = true;
                        break;
                    }
                }
            }
        }
    }

    if (found)
    {
        return conn_vp;
    }
    else
    {
        return nest::GenericConnectorModel< ConnectionT >::delete_connection(tgt, target_thread, conn, syn_id);
    }
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
nest::ConnectorBase* DiligentConnectorModel< ConnectionT >::get_hom_connector(nest::ConnectorBase* conn,
                                                                              nest::synindex syn_id)
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
        nest::HetConnector* hc = static_cast<nest::HetConnector*> (conn);
        for (size_t i = 0; i < hc->size(); i++)
        {
            // need to cast to vector_like to access syn_id
            if ((*hc)[ i ]->get_syn_id() == syn_id) // find entry for this type
            {
                return (*hc)[ i ];
            }
        }
    }

    return 0;
}

/**
 * @function register_diligent_connection_model
 *
 * Convenience function to register diligent synapses. The template argument
 * specifies the new synapse type. Connections registered in this way must
 * implement a method \a is_degenerated() which returns true if the synapse
 * wishes to be picked up by the garbage collector.
 *
 * @param name name of the connection model.
 * @param requires_symmetric indicate that the model requires symmetric connections.
 *
 * @see DiligentConnectorModel, ConnectionUpdateManager
 */
template <class ConnectionT>
void register_diligent_connection_model(const std::string& name, bool requires_symmetric = false)
{
    nest::kernel().model_manager.register_connection_model< ConnectionT, DiligentConnectorModel >
            (name, requires_symmetric);
}

}

#endif
