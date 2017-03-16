#include "tracing_node.h"

#include "connection_updater.h"
#include "network.h"

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
};


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

} // namespace
