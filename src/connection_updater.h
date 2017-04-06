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
 * File:   connection_updater.h
 * Author: Kappel
 *
 * Created on October 11, 2016, 7:09 PM
 */

#ifndef CONNECTION_UPDATER_H
#define CONNECTION_UPDATER_H

#include <vector>
#include <set>

#include "spore.h"

#include "nest.h"
#include "event.h"
#include "node.h"
#include "nestmodule.h"
#include "connector_model.h"
#include "dictdatum.h"

#include "connection_manager_impl.h"
#include "connector_model_impl.h"
#include "kernel_manager.h"


namespace nest
{
class ConnectorBase;
}

namespace spore
{

/**
 * @brief Class that manages updating diligent synapses.
 * 
 * Singleton class that manages updating connections that require regular
 * updates (diligent connections). The object is supposed to be updated in a
 * regular interval (defined by get_interval()). Synapses that were not updated
 * for longer than specified by get_acceptable_latency() will by updated.
 * The maximum update latency that may occur can be retrieved through the
 * get_max_latency() method.
 *
 * Synapses that use the functionality of the update manager should be
 * registered using the \a register_connector method. Synapse Models that
 * are instantiated using the DiligentConnectorModel do this automatically.
 *
 * ConnectionUpdateManager also provides a mechanism to removed synapses that
 * become nonfunctional (garbage collection). The synapse must take care of
 * invoking the garbage collector by calling the trigger_garbage_collector
 * method. After a call to this the synapse must return \c true in its
 * \a is_degenerated method. The synapse is not deleted immediately. If
 * \a trigger_garbage_collector was triggered from within the \a send function
 * of the synapse the function is guaranteed to be finished before the synapse
 * is removed. The \a send function may also be triggered again multiple times
 * after a call to \a trigger_garbage_collector before the synapse is finally
 * removed. The synapse should in that case not call \a trigger_garbage_collector
 * again or change its behavior of returning \c true from \a is_degenerated, as
 * this would lead to undefined behavior.
 *
 * @see DiligentConnectorModel
 */
class ConnectionUpdateManager
{
public:
    ConnectionUpdateManager();
    ~ConnectionUpdateManager();

    void init(nest::index cu_model_id);
    void setup(long interval, long exceptable_latency);

    void register_connector(nest::ConnectorBase* new_conn, nest::ConnectorBase* old_conn, nest::index sender_gid,
                            nest::thread th, nest::ConnectorModel* cm, nest::synindex syn_id);

    void trigger_garbage_collector(nest::index target_gid, nest::index sender_gid,
                                   nest::thread target_thread, nest::synindex syn_id);

    /**
     * @return the update interval.
     */
    inline nest::delay get_interval() const
    {
        return interval_;
    }

    /**
     * @return the maximum allowed delay.
     */
    inline nest::delay get_acceptable_latency() const
    {
        return acceptable_latency_;
    }

    /**
     * @return the maximum latency of any registered connection.
     */
    inline nest::delay get_max_latency() const
    {
        return interval_ + acceptable_latency_ + nest::kernel().connection_manager.get_min_delay();
    }

    /**
     * @return the time limit up to which connections should be updated.
     */
    inline nest::Time get_origin() const
    {
        return nest::kernel().simulation_manager.get_slice_origin();
    }

    /**
     * @return the time limit up to which connections should be updated.
     */
    inline nest::Time get_horizon() const
    {
        return get_origin() - nest::Time(nest::Time::step(interval_ + acceptable_latency_));
    }

    /**
     * @return true if the ConnectionUpdateManager is set up correctly.
     */
    inline bool is_valid() const
    {
        return (interval_ > 0) && (acceptable_latency_ >= 0);
    }

    /**
     * @return true if at least one connection has been registered.
     */
    inline bool has_connections() const
    {
        return has_connections_;
    }

    /**
     * @return true if the connection manager is initialized and ready for the simulation.
     */
    inline bool is_initialized() const
    {
        return is_initialized_;
    }

    static ConnectionUpdateManager *instance();

private:
    friend class ConnectionUpdater;

    void execute_garbage_collector(nest::thread th);
    void update(const nest::Time &time, nest::thread th);
    void calibrate(nest::thread th);
    void prepare();
    void reset();

    ConnectionUpdateManager(const ConnectionUpdateManager&)
    {
    }

    /**
     * @brief Class for connection entries.
     */
    class Connection
    {
    public:
        Connection( nest::ConnectorBase* connector, nest::index sender_gid )
        : connector_(connector),
          sender_gid_(sender_gid)
        {
        }

        Connection( nest::ConnectorBase* connector )
        : connector_(connector),
          sender_gid_(nest::invalid_index)
        {
        }

        bool operator==(const Connection &conn) const
        {
            return (conn.connector_ == connector_);
        }

        bool operator!=(const Connection &conn) const
        {
            return (conn.connector_ != connector_);
        }
        
        bool operator<=(const Connection &conn) const
        {
            return (conn.connector_ <= connector_);
        }

        bool operator>=(const Connection &conn) const
        {
            return (conn.connector_ >= connector_);
        }

        bool operator<(const Connection &conn) const
        {
            return (conn.connector_ < connector_);
        }

        bool operator>(const Connection &conn) const
        {
            return (conn.connector_ > connector_);
        }

        const Connection& operator=(const Connection &conn)
        {
            connector_ = conn.connector_;
            sender_gid_ = conn.sender_gid_;
            return *this;
        }

        nest::ConnectorBase* connector_;
        nest::index sender_gid_;        
    };

    /**
     * @brief Class for garbage collection entries.
     */
    class GarbageCollectorEntry
    {
    public:
        GarbageCollectorEntry(nest::index target_gid, nest::index sender_gid, nest::synindex syn_id)
        : target_gid_(target_gid),
          sender_gid_(sender_gid),
          syn_id_(syn_id)
        {
        }

        GarbageCollectorEntry(const GarbageCollectorEntry &src)
        : target_gid_(src.target_gid_),
          sender_gid_(src.sender_gid_),
          syn_id_(src.syn_id_)
        {
        }

        const GarbageCollectorEntry &operator=(const GarbageCollectorEntry &src)
        {
            target_gid_ = src.target_gid_;
            sender_gid_ = src.sender_gid_;
            syn_id_ = src.syn_id_;
            return *this;
        }

        nest::index target_gid_;
        nest::index sender_gid_;
        nest::synindex syn_id_;
    };

    std::vector< std::set<Connection> > connectors_;  //!< set of all connections under control of the manager.
    std::vector< std::set<nest::synindex> > used_models_;  //!< set of connection models that are in use.
    std::vector< std::vector<GarbageCollectorEntry> > garbage_pile_;  //!< connections waiting for garbage collection.

    long acceptable_latency_;
    long interval_;
    nest::index cu_model_id_;
    nest::index cu_id_;
    bool has_connections_;
    bool is_initialized_;

    static ConnectionUpdateManager *instance_;
};

/**
 * @brief Nest node to handle synapse updates on regular time grid.
 */
class ConnectionUpdater : public nest::Node
{
public:
    ConnectionUpdater();
    ConnectionUpdater(const ConnectionUpdater& n);
    virtual ~ConnectionUpdater();

    virtual void update(nest::Time const &origin, const long from, const long to);

    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &);

    inline
    bool has_proxies() const
    {
        return false;
    }

    inline
    bool one_node_per_process() const
    {
        return false;
    }

private:

    virtual void calibrate();
    virtual void init_state_(const nest::Node& proto);
    virtual void init_buffers_();
};

/**
 * @brief The dummy event class used to trigger synapse updates.
 * 
 * The event sent to the synapse for updates. This is a dummy event, that must
 * not be communicated to the postsynaptic neuron. Trying to send or clone an
 * instance of SynapseUpdateEvent will raise an exception. SynapseUpdateEvent
 * should have an invalid rport of -1.
 */
class SynapseUpdateEvent : public nest::Event
{
public:
    // Constructor.

    SynapseUpdateEvent()
    {
        set_rport(-1);
    };

    // SynapseUpdateEvent are dummy events. Trying to clone them will raise
    // an exception!

    virtual Event* clone() const
    {
        throw nest::KernelException("Trying to call clone() of a SynapseUpdateEvent is illegal!");
        return 0;
    }

    // SynapseUpdateEvent are dummy events. Trying to send them will raise
    // an exception!

    virtual void operator()()
    {
        throw nest::KernelException("Trying to send a SynapseUpdateEvent is illegal!");
    }
};

}

#endif

