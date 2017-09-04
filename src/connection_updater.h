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
#include "dictdatum.h"

#include "connection_manager_impl.h"
#include "connector_model_impl.h"
#include "kernel_manager.h"


namespace spore
{

/**
 * @brief Class that manages updating diligent connections.
 *
 * Singleton class that manages updating connections that require regular
 * updates (diligent connections). The object is supposed to be updated in a
 * regular interval (defined by get_interval()). Synapses that were not updated
 * for longer than specified by get_acceptable_latency() will by updated.
 * The maximum update latency that may occur can be retrieved through the
 * get_max_latency() method.
 *
 * Synapses that use the functionality of the update manager should be
 * registered using the register_connector() method. Synapse Models that
 * are instantiated using the DiligentConnectorModel do this automatically.
 *
 * <b>Initializing the Update Manager</b>
 *
 * If diligent connections are used the SLI function \a InitSynapseUpdater
 * must be called before the first call to \a Connect. This is done
 * automatically when the SPORE module is loaded but must be done manually
 * after the NEST kernel has been reset using \a ResetKernel or its state
 * has changed using \a SetKernelStatus. Therefore a line of code
 * \code
 *   <update_interval> <acceptable_latency> InitSynapseUpdater
 * \endcode
 * is required, where <update_interval> is an integer value (default: 100)
 * that defines the interval (in number of nest time steps) of invoking the
 * update manager and <acceptable_latency> is an integer value (default: 100)
 * defining the acceptable latency for synapses. These parameters can be used
 * to optimize the simulation performance. Long update intervals are likely
 * more time efficient, but require more memory. Equivalently, if using python,
 * add a line
 * \code
 *   nest.sli_func('InitSynapseUpdater',<update_interval>,<acceptable_latency>)
 * \endcode
 * after each call to \a ResetKernel / \a SetKernelStatus.
 * \a InitSynapseUpdater may be called multiple times to change the parameters
 * of the update manager, but must not be invoked again after the first call to
 * \a Simulate (see: setup()).
 *
 * <b>Garbage Collection</b>
 *
 * ConnectionUpdateManager also provides a mechanism to removed synapses that
 * become nonfunctional (garbage collection). The synapse must take care of
 * invoking the garbage collector by calling the trigger_garbage_collector()
 * method. After a call to this the synapse must return \c true in its
 * \a is_degenerated method. The synapse is not deleted immediately. If
 * trigger_garbage_collector() was triggered from within the \a send function
 * of the synapse the function is guaranteed to be finished before the synapse
 * is removed. The \a send function may also be triggered again multiple times
 * after a call to trigger_garbage_collector() before the synapse is finally
 * removed. The synapse should in that case not call trigger_garbage_collector()
 * again or change its behavior of returning \c true from \a is_degenerated, as
 * this would lead to undefined behavior.
 *
 * @node The garbage collection feature should not be used together with structural
 * plasticity mechanisms of NEST which \em delete synapses of the same synapse
 * type, since these may interfere with the garbage collector. Using structural
 * plasticity mechanisms which only \em add synapses is however possible.
 *
 * @see DiligentConnectorModel, SporeModule::InitSynapseUpdater_i_i_Function
 */
class ConnectionUpdateManager
{
public:
    ConnectionUpdateManager();
    ~ConnectionUpdateManager();

    void init(nest::index cu_model_id);
    void setup(long interval, long exceptable_latency);

    void register_connector(nest::ConnectorBase* new_conn, nest::ConnectorBase* old_conn, nest::index sender_gid,
                            nest::thread th, nest::synindex syn_id);

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
        return (interval_ > 0) && (acceptable_latency_ >= 0) && (cu_id_ != nest::invalid_index);
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

    static ConnectionUpdateManager* instance();

private:
    friend class ConnectionUpdater;

    /**
     * @brief ConnectionUpdateManager is a singleton. Copy constructor is private and not implement.
     */
    ConnectionUpdateManager(const ConnectionUpdateManager&);

    void execute_garbage_collector(nest::thread th);
    void update(const nest::Time& time, nest::thread th);
    void calibrate(nest::thread th);
    void finalize(nest::thread th);
    void prepare();
    void reset();
    void finalize();

    /**
     * @brief Class for connection entries.
     */
    class ConnectionEntry
    {
    public:
        ConnectionEntry( nest::ConnectorBase* connector, nest::Node* sender=0 )
        : connector_(connector),
          sender_(sender)
        {
        }

        ConnectionEntry( const ConnectionEntry& src )
        : connector_(src.connector_),
          sender_(src.sender_)
        {
            assert(sender_);
        }

        bool operator==(const ConnectionEntry& conn) const
        {
            return (conn.connector_ == connector_);
        }

        bool operator!=(const ConnectionEntry& conn) const
        {
            return (conn.connector_ != connector_);
        }

        bool operator<=(const ConnectionEntry& conn) const
        {
            return (conn.connector_ <= connector_);
        }

        bool operator>=(const ConnectionEntry& conn) const
        {
            return (conn.connector_ >= connector_);
        }

        bool operator<(const ConnectionEntry& conn) const
        {
            return (conn.connector_ < connector_);
        }

        bool operator>(const ConnectionEntry& conn) const
        {
            return (conn.connector_ > connector_);
        }

        const ConnectionEntry& operator=(const ConnectionEntry& conn)
        {
            assert(sender_);
            connector_ = conn.connector_;
            sender_ = conn.sender_;
            return *this;
        }

        nest::ConnectorBase* get_connector() const
        {
            return connector_;
        }

        nest::Node& get_sender() const
        {
            assert(sender_);
            return *sender_;
        }

    private:
        nest::ConnectorBase* connector_;
        nest::Node* sender_;
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

        GarbageCollectorEntry(const GarbageCollectorEntry& src)
        : target_gid_(src.target_gid_),
          sender_gid_(src.sender_gid_),
          syn_id_(src.syn_id_)
        {
        }

        const GarbageCollectorEntry& operator=(const GarbageCollectorEntry& src)
        {
            target_gid_ = src.target_gid_;
            sender_gid_ = src.sender_gid_;
            syn_id_ = src.syn_id_;
            return *this;
        }

        nest::index get_target_gid() const
        {
            return target_gid_;
        }

        nest::index get_sender_gid() const
        {
            return sender_gid_;
        }

        nest::synindex get_syn_id() const
        {
            return syn_id_;
        }

    private:
        nest::index target_gid_;
        nest::index sender_gid_;
        nest::synindex syn_id_;
    };

    /**
     * @brief set of all connections under control by the manager.
     */
    std::vector< std::set< ConnectionEntry > > connectors_;

    /**
     * @brief set of connection models that are in use by the manager.
     */
    std::vector< std::set< nest::synindex > > used_models_;

    /**
     * @brief connections waiting for garbage collection.
     */
    std::vector< std::vector< GarbageCollectorEntry > > garbage_pile_;

    long acceptable_latency_;
    long interval_;
    nest::index cu_model_id_;
    nest::index cu_id_;
    bool has_connections_;
    bool is_initialized_;

    static ConnectionUpdateManager* instance_;
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

    virtual void update(nest::Time const& origin, const long from, const long to);
    virtual void calibrate();
    virtual void finalize();

    void get_status(DictionaryDatum&) const;
    void set_status(const DictionaryDatum&);

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

    virtual void init_buffers_();
    virtual void init_state_(const nest::Node& proto);
};

/**
 * @brief The dummy event class used to trigger synapse updates.
 *
 * The event sent to the synapse for updates. This is a dummy event, that must
 * not be communicated to the postsynaptic neuron. Trying to send or clone an
 * instance of SynapseUpdateEvent will raise an exception. SynapseUpdateEvent
 * should have an invalid \a rport of -1.
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
