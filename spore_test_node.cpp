#include "spore_test_node.h"

#include "test_circular_buffer.h"
#include "test_tracing_node.h"

namespace spore
{
/**
 * Constructor.
 */
SporeTestNode::SporeTestNode()
:test_time_(-1.0)
{
    register_test(new TestCircularBuffer());
    register_test(new TestTracingNode());
}

/**
 * Destructor.
 */
SporeTestNode::~SporeTestNode()
{}


/**
 * Register a new test.
 */
void SporeTestNode::register_test(SporeTestBase *test)
{
    assert(tests_[test->get_name()] == 0);
    tests_[test->get_name()] = test;
}
    

/**
 * State initialization function
 */
void SporeTestNode::init_state_(const nest::Node& proto)
{}


/**
 * State initialization function
 */
void SporeTestNode::init_buffers_()
{
    init_traces(tests_[test_name_]->get_num_traces());
    tests_[test_name_]->init();
}


/**
 * Calibration function.
 */
void SporeTestNode::calibrate()
{}


/**
 * SpikeEvent handling.
 * @param e the event.
 */
void SporeTestNode::handle(nest::SpikeEvent & e)
{}


/**
 * Update function.
 */
void SporeTestNode::update(nest::Time const & origin, const long from, const long to)
{
    SporeTestBase* test = tests_[test_name_];
    
    for (long lag = from; lag < to; ++lag)
    {
        nest::Time time = nest::Time::step(origin.get_steps() + lag);
        
        for (TracingNode::trace_id tid=0; tid<test->get_num_traces(); tid++)
        {
            assert( time.get_ms() >= ConnectionUpdateManager::instance()->get_origin().get_ms() &&
                    time.get_ms() < ConnectionUpdateManager::instance()->get_origin().get_ms() +
                                    nest::kernel().connection_manager.get_min_delay() );
            
            set_trace(time.get_steps(), test->get_trace(time.get_steps(), tid), tid);
        }
        
        if ( (test_time_>=time.get_ms()) && (test_time_<(time+nest::Time::get_resolution()).get_ms()) )
        {
            test->check(time.get_steps(), this);
        }
    }
}

}
