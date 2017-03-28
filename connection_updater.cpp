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
 * File:   connection_updater.cpp
 * Author: Kappel
 *
 * Created on October 11, 2016, 7:09 PM
 */

#include "connection_updater.h"

#include "common_synapse_properties.h"
#include "connector_base.h"
#include "genericmodel.h"

namespace spore
{

/**
 * Constructor.
 */
ConnectionUpdateManager::ConnectionUpdateManager()
 :acceptable_latency_(-1.0),
  interval_(-1.0),
  cu_model_id_(nest::invalid_index),
  cu_id_(nest::invalid_index),
  has_connections_(false)
{}


/**
 * Destructor.
 */
ConnectionUpdateManager::~ConnectionUpdateManager()
{}


/**
 * Sets up the ConnectionUpdateManager with the given update interval and
 * acceptable latency.
 * 
 * This function must be called once at start up and after a
 * call to ConnectionUpdateManager::reset to inform the
 * ConnectionUpdateManager about the number of threads used
 * by NEST.
 *
 * This function may not be thread safe.
 *
 * @param interval the update interval used to update connections.
 * @param delay the maximum delay allowed for a connection.
 */
void ConnectionUpdateManager::setup(long interval, long acceptable_latency)
{
    const size_t num_threads = nest::kernel().vp_manager.get_num_threads();

    assert(num_threads>0);
    
    if (connectors_.size()==0)
    {
        connectors_.resize(num_threads);
        models_.resize(num_threads);
    }
    else
    {
        assert(connectors_.size() == num_threads);
    }
    
    if (cu_id_ == nest::invalid_index)
    {
        assert(cu_model_id_ != nest::invalid_index);
        cu_id_ = nest::kernel().node_manager.add_node(cu_model_id_,1);
    }

    interval_ = interval;
    acceptable_latency_ = acceptable_latency;
}


/**
 * Initializes the connection update manager. This must be called once
 * at start up and should be done within the initialization of the module
 * that uses the ConnectionUpdateManager.
 */
void ConnectionUpdateManager::init(nest::index cu_model_id)
{
    assert(cu_model_id_==nest::invalid_index);
    cu_model_id_ = cu_model_id;
}


/**
 * Check all child connectors and advance them if they required update.
 *
 * @param time the time point to advance to.
 * @param th the thread of the calling node.
 */
void ConnectionUpdateManager::update(const nest::Time &time, nest::thread th)
{
    if (!has_connections())
        return;
    
    const double t_trig = time.get_steps();

    SynapseUpdateEvent ev;
    ev.set_stamp(time);

    for ( std::set<nest::ConnectorBase*>::iterator it = connectors_[th].begin();
          it != connectors_[th].end();
          it++ )
    {
        assert(*it);
        
        if ( t_trig > (*it)->get_t_lastspike() )
        {
            (*it)->send( ev, th, models_[th] );
        }
    }
}


/**
 * Adds the given connector and removes the old one. New connector
 * can be 0; in that case only the old one is removed. The old
 * connector can be 0; in that case only the new one is added.
 *
 * @param: new_conn the connector object to be added.
 * @param: old_conn the old connector object to be removed.
 * @param: th the thread to which the connectors belong.
 * @param: cm the connector model that has created the new connector.
 * @param: syn_id the connection (synapse) type id.
 */
void ConnectionUpdateManager::register_connector( nest::ConnectorBase* new_conn, nest::ConnectorBase* old_conn,
                                                  nest::thread th, nest::ConnectorModel* cm, nest::synindex syn_id )
{
    assert(cm);
    
    if (connectors_.size() < static_cast<size_t>(th))
    {
        throw nest::BadProperty("Connection update manager was not set up correctly before the first call"
                                " to 'Connect'! Maybe you forgot to call 'InitSynapseUpdater'?");
    }
    
    if (models_[th].size() <= syn_id)
    {
        models_[th].resize(syn_id+1,0);
    }

    models_[th][syn_id] = cm;

    std::set<nest::ConnectorBase*> &conns = connectors_[th];

    if (new_conn == old_conn)
    {
        assert(conns.find(new_conn)!=conns.end());
    }
    else
    {
        if (old_conn)
        {
            // at this time the old connector is already deleted.
            // just check if it is still in the update set and
            // remove if found.
            std::set<nest::ConnectorBase*>::iterator it = conns.find(old_conn);
            assert(it!=conns.end());
            conns.erase(it);
        }
        
        if (new_conn)
        {
            conns.insert(new_conn);
        }
    }
    
    has_connections_ = true;
}


/**
 * Calibrates all registered connection models. This should be called
 * by the updater nodes when they are calibrated on simulation startup.
 *
 * @param: th the thread of the calling node.
 */
void ConnectionUpdateManager::calibrate(nest::thread th)
{
    nest::TimeConverter tc;
    
    for ( std::vector<nest::ConnectorModel*>::iterator it = models_[th].begin();
          it != models_[th].end();
          ++it )
    {
        if (*it)
            (*it)->calibrate(tc);
    }
}


/**
 * Reset the ConnectionUpdateManager. Removes all connectors that have
 * been registered.
 * 
 * This function may not be thread safe.
 */
void ConnectionUpdateManager::reset()
{
    has_connections_ = false;
    connectors_.clear();
    models_.clear();
    cu_id_ = nest::invalid_index;
}


/**
 * @return the only instance of ConnectionUpdateManager.
 */
ConnectionUpdateManager* ConnectionUpdateManager::instance()
{
    if (!instance_)
        instance_ = new ConnectionUpdateManager();

    return instance_;
}


/**
 * pointer to global instance.
 */
ConnectionUpdateManager* ConnectionUpdateManager::instance_=0;


//
// ConnectionUpdater implementation
//
    
/**
 * Constructur.
 */
ConnectionUpdater::ConnectionUpdater()
{}


/**
 * Copy Constructur.
 */
ConnectionUpdater::ConnectionUpdater(const ConnectionUpdater& n)
{}


/**
 * Destructur.
 */
ConnectionUpdater::~ConnectionUpdater()
{
    // The updater has been destroyed. This should only happen if
    // NESTs' kernel has been reset. We reset the update manager
    // here so that all connections are removed from it.
    if (get_thread()==0)
    {
        ConnectionUpdateManager::instance()->reset();
    }
}


/**
 * Update function of the ConnectionUpdater node. This will trigger the update
 * function of the ConnectionUpdateManager in regular time intervals given
 * by ConnectionUpdateManager::interval_.
 *
 * @param origin the time slice origin.
 * @param from from time offset.
 * @param to to time offset.
 */
void ConnectionUpdater::update(nest::Time const &origin, const long from, const long to)
{
    const long acceptable_latency = ConnectionUpdateManager::instance()->get_acceptable_latency();
    const long interval = ConnectionUpdateManager::instance()->get_interval();

    const long start_time = origin.get_steps() + from - acceptable_latency;
    const long slice = to - from;

    if ( (start_time%interval + slice) >= interval )
    {
        ConnectionUpdateManager::instance()->update( origin, get_thread() );
    }
}


/**
 * Will set the node frozen if the ConnectionUpdateManager does not need
 * updates.
 */
void ConnectionUpdater::calibrate()
{
    ConnectionUpdateManager::instance()->calibrate( get_thread() );
    
    set_frozen_( !ConnectionUpdateManager::instance()->is_valid() ||
                 !ConnectionUpdateManager::instance()->has_connections() );
}


/**
 */
void ConnectionUpdater::init_state_(const nest::Node& proto)
{}


/**
 */
void ConnectionUpdater::init_buffers_()
{}
    

/**
 */
void ConnectionUpdater::get_status(DictionaryDatum &) const
{}


/**
 */
void ConnectionUpdater::set_status(const DictionaryDatum &)
{}


}

