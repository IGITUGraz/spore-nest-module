#ifndef SAMBA_TEST_CONNECTION
#define SAMBA_TEST_CONNECTION

#include <cmath>
#include "nest.h"
#include "connection.h"
#include "normal_randomdev.h"
#include "spikecounter.h"

#include "tracing_node.h"


namespace spore
{

/**
 * Class containing the common properties for all synapses of type SporeTestConnection.
 */
class SporeTestConnectionCommonProperties : public nest::CommonSynapseProperties
{
public:
    /**
     * Constructor.
     */
    SporeTestConnectionCommonProperties()
    :nest::CommonSynapseProperties(),
     weight_update_time_(100.0),
     bap_trace_id_(0),
     resolution_unit_(-1)
    {}

    /**
     * Destructor.
     */
    ~SporeTestConnectionCommonProperties()
    {}

    /**
     * Object status getter.
     */
    void get_status(DictionaryDatum & d) const
    {}

    /**
     * Object status setter.
     */
    void set_status(const DictionaryDatum & d, nest::ConnectorModel& cm)
    {
        updateValue<double>(d, "weight_update_time", weight_update_time_);
        updateValue<long>(d, "bap_trace_id", bap_trace_id_);
    }
    
    /**
     * Calibrate all time objects, which are contained in this object.
     * This function is called when the time resolution changes and on
     * simulation startup.
     */
    void calibrate( const nest::TimeConverter &tc )
    {
        resolution_unit_ = nest::Time::get_resolution().get_ms();
    }

    /**
     * Check spike event.
     */    
    void check_event(nest::SpikeEvent&)
    {}
    
    /**
     * @return GID of the reward transmitter associated to the synapse type.
     */
    long get_vt_gid() const
    {
        return -1;
    }

    /**
     * @return The reward transmitter associated to the synapse type.
     */    
    nest::Node* get_node()
    {
        return nest::CommonSynapseProperties::get_node();
    }    
    
    double weight_update_time_;
    TracingNode::trace_id bap_trace_id_;
    double resolution_unit_;
};

/**
 * @brief Connection for testing the spore module.
 * 
 * SporeTestConnection class provides an interface for writing unit tests
 * for the SPORE module.
 */
template<typename targetidentifierT>
class SporeTestConnection : public nest::Connection<targetidentifierT>
{
public:

    SporeTestConnection();
    SporeTestConnection(const SporeTestConnection<targetidentifierT>& rhs);
    ~SporeTestConnection();

    //! Type to use for representing common synapse properties
    typedef SporeTestConnectionCommonProperties CommonPropertiesType;

    //! Shortcut for base class
    typedef nest::Connection<targetidentifierT> ConnectionBase;

    class ConnTestDummyNode : public nest::ConnTestDummyNodeBase
    {
    public:
        using nest::ConnTestDummyNodeBase::handles_test_event;

        nest::port handles_test_event(nest::SpikeEvent&, nest::rport)
        {
            return nest::invalid_port_;
        }

        nest::port handles_test_event(nest::DSSpikeEvent&, nest::rport)
        {
            return nest::invalid_port_;
        }
    };

    void check_connection(nest::Node & s, nest::Node & t,
                          nest::rport receptor_type, double t_lastspike, const CommonPropertiesType &cp)
    {
        if (!dynamic_cast<TracingNode*> (&t))
        {
            throw nest::IllegalConnection("This synapse only works with nodes exposing their firing probability trace (e.g. TracingNode-Subclass)!");
        }
        
        ConnTestDummyNode dummy_target;
        ConnectionBase::check_connection_(dummy_target, s, t, receptor_type);
    }

    void get_status(DictionaryDatum & d) const;
    void set_status(const DictionaryDatum & d, nest::ConnectorModel &cm);

    void send(nest::Event& e, nest::thread t, double t_lastspike, const CommonPropertiesType &cp);

    using ConnectionBase::get_delay_steps;
    using ConnectionBase::get_delay;
    using ConnectionBase::get_rport;
    using ConnectionBase::get_target;

    void set_weight(double w)
    {
        weight_ = w;
    }

private:
    double weight_;
    double t_weight_;

    std::vector<double> recorder_times_;
    std::vector<double> recorder_values_;

    void update_synapse_state( double t_to,
                               double t_last_update,
                               TracingNode::const_iterator &bap_trace,
                               const CommonPropertiesType& cp );
    
    void update_synapic_weight( double time,
                                nest::thread thread,
                                TracingNode *target,
                                const CommonPropertiesType& cp);
};


/* ----------------------------------------------------------------
 * IMPLEMENTATION OF SYNAPSE
 * ---------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * Object lifecycle
 * ---------------------------------------------------------------- */

template <typename targetidentifierT>
SporeTestConnection<targetidentifierT>::SporeTestConnection()
:ConnectionBase(),
 weight_(0.0),
 t_weight_(0.0)
{}

template <typename targetidentifierT>
SporeTestConnection<targetidentifierT>::SporeTestConnection(const SporeTestConnection& rhs)
:ConnectionBase(rhs),
 weight_(rhs.weight_),
 t_weight_(rhs.t_weight_)
{}

template <typename targetidentifierT>
SporeTestConnection<targetidentifierT>::~SporeTestConnection()
{}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

template <typename targetidentifierT>
void SporeTestConnection<targetidentifierT>::get_status(DictionaryDatum & d) const
{
    ConnectionBase::get_status(d);
    (*d)["recorder_times"] = recorder_times_;
    (*d)["recorder_values"] = recorder_values_;
}

template <typename targetidentifierT>
void SporeTestConnection<targetidentifierT>::set_status(const DictionaryDatum & d, nest::ConnectorModel &cm)
{
    ConnectionBase::set_status(d, cm);
}

//
// Synapse event handling
//

/**
 * Send an event to the postsynaptic neuron. This will update the synapse state
 * and synaptic weights to the current slice origin and send the spike event.
 * This method is also triggered by the ConnectionUpdateManager to indicate
 * that the synapse is running out of data. In this case an invalid rport of -1
 * is passed and the spike is not delivered to the postsynaptic neuron.
 * 
 * @param e the spike event.
 * @param thread the id of the connections thread.
 * @param t_last_spike the time of the last spike.
 * @param cp the synapse type common properties.
 */
template <typename targetidentifierT>
void SporeTestConnection<targetidentifierT>::send( nest::Event& e,
                                                   nest::thread thread,
                                                   double t_last_spike,
                                                   const CommonPropertiesType &cp )
{
    const double t_to = e.get_stamp().get_ms(); // ConnectionUpdateManager::instance()->get_origin().get_ms();
    double t_from = t_last_spike;
    
    assert(cp.resolution_unit_>0.0);

    if (t_to>t_last_spike)
    {
        // prepare the pointer to the target neuron. We can safely static_cast
        // since the connection is checked when established.
        TracingNode* target = static_cast<TracingNode*> (get_target(thread));

        TracingNode::const_iterator bap_trace = target->get_trace( nest::delay(t_from/cp.resolution_unit_), cp.bap_trace_id_ + get_rport() );
    
        for ( double next_weight_time = t_weight_ + cp.weight_update_time_;
              next_weight_time <= t_to;
              next_weight_time += cp.weight_update_time_ )
        {
            update_synapse_state(next_weight_time, t_from, bap_trace, cp);
            update_synapic_weight(next_weight_time, thread, target, cp);
            t_from = next_weight_time;
        }

        if (t_to > t_from)
        {
            update_synapse_state(t_to, t_from, bap_trace, cp);
        }        
    }

    if (e.get_rport() >= 0)
    {
        if (weight_ > 0.0)
        {
            e.set_weight(weight_);

            e.set_delay(get_delay_steps());
            e.set_receiver(*get_target(thread));
            e.set_rport(get_rport());
            e();
        }
    }
}


/**
 * Updates the state of the synapse to the given time point. This method
 * expects the back propagating action potential BAP trace of the postsynaptic
 * neuron and the reward (dopamine) trace of the reward transmitter to be
 * passed. Iterators are expected to be positioned at time t_last_update and
 * will be advanced to t_to after the call.
 * 
 * @param t_to time to advance to.
 * @param t_last_update time of last update.
 * @param bap_trace iterator pointing to the current value of the BAP trace.
 * @param cp synapse type common properties.
 */
template <typename targetidentifierT>
void SporeTestConnection<targetidentifierT>::update_synapse_state( double t_to,
                                                                   double t_last_update,
                                                                   TracingNode::const_iterator &bap_trace,
                                                                   const CommonPropertiesType& cp )
{
    t_to -= cp.resolution_unit_/2.0; // exclude the last time step.
    
    for (double time = t_last_update; time < t_to; time += cp.resolution_unit_)
    {
        assert( time >= ConnectionUpdateManager::instance()->get_horizon().get_ms() &&
                time < ConnectionUpdateManager::instance()->get_origin().get_ms() );
        
        recorder_times_.push_back(time);
        recorder_values_.push_back(*bap_trace);
        ++bap_trace;
    }
}


/**
 * Updates the synaptic parameter and weight of the synapse.
 * 
 * @param time the time when the synapse is updated.
 * @param thread the thread of the synapse.
 * @param target the target node of the synapse.
 * @param cp the synapse type common properties.
 */
template <typename targetidentifierT>
void SporeTestConnection<targetidentifierT>::update_synapic_weight( double time,
                                                                    nest::thread thread,
                                                                    TracingNode *target,
                                                                    const CommonPropertiesType& cp )
{
    t_weight_ = time;
}

} // namespace

#endif
