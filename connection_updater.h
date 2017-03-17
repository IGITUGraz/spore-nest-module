/*
 * File:   connection_updater.h
 * Author: Kappel
 *
 * Created on October 11, 2016, 7:09 PM
 */

#ifndef CONNECTION_UPDATER_H
#define	CONNECTION_UPDATER_H

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
 * @see DiligentConnectorModel
 */
class ConnectionUpdateManager
{
public:
    ConnectionUpdateManager();
    ~ConnectionUpdateManager();

    void init(nest::index cu_model_id);
    void setup(long interval, long exceptable_latency);

    void register_connector( nest::ConnectorBase* new_conn, nest::ConnectorBase* old_conn,
                             nest::thread th, nest::ConnectorModel* cm, nest::synindex syn_id );

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
        return get_origin() - nest::Time( nest::Time::step(interval_ + acceptable_latency_) ); 
    }
    
    /**
     * @return true if the ConnectionUpdateManager is set up correctly.
     */
    inline bool is_valid() const
    {
        return (interval_>0) && (acceptable_latency_>=0);
    }

    /**
     * @return true if at least one connection has been registered.
     */
    inline bool has_connections() const
    {
        return has_connections_;
    }

    static ConnectionUpdateManager *instance();

private:
    friend class ConnectionUpdater;
    
    void update(const nest::Time &time, nest::thread th);
    void calibrate(nest::thread th);
    void reset();
    
    ConnectionUpdateManager(const ConnectionUpdateManager&)
    {}

    std::vector< std::set<nest::ConnectorBase*> > connectors_;

    std::vector< std::vector<nest::ConnectorModel*> > models_;

    long acceptable_latency_;
    long interval_;
    nest::index cu_model_id_;
    nest::index cu_id_;
    bool has_connections_;

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
 * should have an invalid sender and rport of -1.
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

} // namespace spore

#endif	/* CONNECTION_UPDATER_H */

