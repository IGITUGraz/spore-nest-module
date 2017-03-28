#include "test_tracing_node.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "tracing_node.h"

namespace spore
{

/**
 * Constructor.
 */
TestTracingNode::TestTracingNode()
: SporeTestBase("test_tracing_node")
{
}

/**
 * Execute once at startup.
 */
void TestTracingNode::check(nest::delay time_step, TracingNode *node)
{
    test_assert(node->get_num_traces() == 5, "number of traces");

    for (size_t tid = 0; tid < node->get_num_traces(); tid++)
    {
        for (nest::delay t = 0; t < time_step; t++)
        {
            test_assert((*node->get_trace(t, tid)) == t + tid, "trace content");
        }
    }
}

}
