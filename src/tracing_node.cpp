/*
 * This file is part of SPORE.
 *
 * Copyright (C) 2016, the SPORE team (see AUTHORS).
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
 * File:   tracing_node.cpp
 * Author: Kappel
 *
 * Created on November 3, 2016, 2:07 PM
 */

#include "tracing_node.h"
#include "connection_updater.h"

#include "kernel_manager.h"
#include "arraydatum.h"


namespace spore
{

/**
 * Constructor.
 */
TracingNode::TracingNode()
: nest::Node()
{
}

/**
 * Destructor.
 */
TracingNode::~TracingNode()
{
}

/**
 * Constructor.
 */
void TracingNode::init_traces(size_t num_traces)
{
    traces_.resize(num_traces);
    const size_t trace_length = ConnectionUpdateManager::instance()->get_max_latency();

    for (size_t i = 0; i < num_traces; i++)
    {
        traces_[i].resize(trace_length, 0.0);
    }
}

/**
 * Set status of tracing node.
 */
void TracingNode::set_status(const DictionaryDatum& d)
{
}

/**
 * Get status of tracing node.
 */
void TracingNode::get_status(DictionaryDatum& d) const
{
}

/**
 * Read traces into dictionary.
 */
void TracingNode::get_trace_status(DictionaryDatum& d) const
{
    nest::Time time = ConnectionUpdateManager::instance()->get_horizon();
    ArrayDatum traces;
    for (size_t trace_id = 0; trace_id < traces_.size(); trace_id++)
    {
        TracingNode::const_iterator it = get_trace(time, trace_id);
        std::vector<double> trace;
        const size_t trace_length = ConnectionUpdateManager::instance()->get_max_latency();
        trace.resize(trace_length);
        for (size_t i = 0; i < trace_length; i++)
        {
            trace[i] = *it;
            ++it; // back to the future
        }
        traces.push_back(trace);
    }
    (*d)["trace"] = traces;
}

}
