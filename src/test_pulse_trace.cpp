/* 
 * This file is part of SPORE.
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
 * File:   test_pulse_trace.cpp
 * Author: Kappel
 *
 * Created on April 4, 2017, 6:23 PM
 */

#include "test_pulse_trace.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "nest_time.h"

#include "tracing_node.h"


namespace spore
{

/**
 * Constructor.
 */
TestPulseTrace::TestPulseTrace()
: SporeTestBase("test_pulse_trace"),
offset_(0.0),
weight_(1.0)
{
}

/**
 * @return the current value of the trace.
 */
double TestPulseTrace::get_trace(nest::delay time_step, TracingNode::trace_id id)
{
    assert(id == 0);
    const double t_ms = nest::Time::delay_steps_to_ms( time_step );
    const double resolution = nest::Time::get_resolution().get_ms();
    bool in_spike = false;
    for ( std::vector< double >::const_iterator it = spike_times_.begin(); it != spike_times_.end(); ++it )
    {
        if ( (t_ms >= *it) && (t_ms < *it + resolution) )
        {
            in_spike = true;
            break;
        }
    }
    return offset_ + (in_spike ? weight_ : 0.0);
}

/**
 * Status setter function.
 */
void TestPulseTrace::set_status(const DictionaryDatum &d)
{
    const bool updated_spike_times = d->known( nest::names::spike_times );
    
    if (updated_spike_times)
    {
        const std::vector< double > d_times =
            getValue < std::vector< double > > ( d->lookup( nest::names::spike_times ) );

        const size_t n_spikes = d_times.size();
        spike_times_.clear();
        spike_times_.reserve( n_spikes );

        std::copy(d_times.begin(), d_times.end(), spike_times_.begin());
    }
    
    updateValue<double>(d, "offset", offset_);
    updateValue<double>(d, "weight", weight_);
}

}

