#include "tracing_node.h"
#include "connection_updater.h"

#if defined(__SPORE_WITH_NEST_2_12__)
#include "kernel_manager.h"
#endif

namespace spore
{

/**
 * Constructor.
 */
TracingNode::TracingNode()
:nest::Node()
{}

/**
 * Destructor.
 */
TracingNode::~TracingNode()
{}


/**
 * Constructor.
 */
void TracingNode::init_traces(size_t num_traces)
{
    traces_.resize(num_traces);
    const size_t trace_length = ConnectionUpdateManager::instance()->get_max_latency();
    
    for (size_t i=0; i<num_traces; i++)
    {
        traces_[i].resize(trace_length, 0.0);
    }
}


/**
 * Set status of tracing node.
 */
void TracingNode::set_status(const DictionaryDatum& d)
{
    nest::Node::set_status(d);
}


/**
 * Get status of tracing node.
 */
void TracingNode::get_status(DictionaryDatum& d) const
{
    nest::Node::get_status(d);
}


/**
 * Convenience function to send the given event with time lag.
 */
void TracingNode::send_event(nest::Event &ev, long lag)
{
#if defined(__SPORE_WITH_NEST_2_10__)
    network()->send(*this, ev, lag);
#elif defined(__SPORE_WITH_NEST_2_12__)
    nest::kernel().event_delivery_manager.send( *this, ev, lag );
#else
#error NEST version is not supported!
#endif
}


/**
 * Convenience function to get the random number generator.
 */
librandom::RngPtr TracingNode::get_rng()
{
#if defined(__SPORE_WITH_NEST_2_10__)
    return network()->get_rng(get_thread());
#elif defined(__SPORE_WITH_NEST_2_12__)
    return nest::kernel().rng_manager.get_rng( get_thread() );
#else
#error NEST version is not supported!
#endif
}


/**
 * Convenience function to get the slice origin.
 */
nest::Time TracingNode::get_slice_origin() const
{
#if defined(__SPORE_WITH_NEST_2_10__)
    return network()->get_slice_origin();
#elif defined(__SPORE_WITH_NEST_2_12__)
    return nest::kernel().simulation_manager.get_slice_origin();
#else 
#error NEST version is not supported!
#endif
}

} // namespace
