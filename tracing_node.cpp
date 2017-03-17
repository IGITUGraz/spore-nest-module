#include "tracing_node.h"
#include "connection_updater.h"

#include "kernel_manager.h"


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
    nest::kernel().event_delivery_manager.send( *this, ev, lag );
}


/**
 * Convenience function to get the random number generator.
 */
librandom::RngPtr TracingNode::get_rng()
{
    return nest::kernel().rng_manager.get_rng( get_thread() );
}


/**
 * Convenience function to get the slice origin.
 */
nest::Time TracingNode::get_slice_origin() const
{
    return nest::kernel().simulation_manager.get_slice_origin();
}

} // namespace
