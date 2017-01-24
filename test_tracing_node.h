/* 
 * File:   test_tracing_node.h
 * Author: kappel
 *
 * Created on November 9, 2016, 5:03 PM
 */

#ifndef TEST_TRACING_NODE_H
#define	TEST_TRACING_NODE_H

#include "spore_test_base.h"

namespace spore
{
/**
 * Test class for circular buffer container.
 */
class TestTracingNode : public SambaTestBase
{
public:
    TestTracingNode();
    virtual double get_trace(nest::delay time_step, TracingNode::trace_id id)
    {
        return double(time_step + id);
    };    
    virtual size_t get_num_traces() const
    {
        return 5;
    };

    virtual void check(nest::delay time_step, TracingNode *node);
};

}

#endif	/* TEST_TRACING_NODE_H */

