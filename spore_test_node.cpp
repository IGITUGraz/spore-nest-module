/*
 * This file is part of SPORE.
 *
 * Copyright (c) 2016, Institute for Theoretical Computer Science,
 * Graz University of Technology
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
 * File:   spore_test_node.cpp
 */

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
                                    nest::NestModule::get_network().get_min_delay() );
            
            set_trace(time.get_steps(), test->get_trace(time.get_steps(), tid), tid);
        }
        
        if ( (test_time_>=time.get_ms()) && (test_time_<(time+nest::Time::get_resolution()).get_ms()) )
        {
            test->check(time.get_steps(), this);
        }
    }
}

}
