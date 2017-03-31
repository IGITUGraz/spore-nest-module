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
 * File:   tracing_node.h
 * Author: Kappel
 *
 * Created on November 3, 2016, 2:07 PM
 */

#ifndef TRACING_NODE_H
#define TRACING_NODE_H

#include <vector>
#include <deque>

#include "nest.h"
#include "event.h"
#include "node.h"

#include "circular_buffer.h"
#include "connection_updater.h"


namespace spore
{

/**
 * @brief Base class to all nodes that record traces.
 *
 * Node that allows to record traces. A trace is a fixed-length buffer that
 * stores the most recent values of a real-valued time-dependent variable,
 * e.g., the neuron's membrane potential. Traces can be read by other nodes
 * or connections.
 */
class TracingNode : public nest::Node
{
public:
    typedef CircularBuffer<double>::const_iterator const_iterator;
    typedef size_t trace_id;

    TracingNode();
    ~TracingNode();

    virtual void get_status(DictionaryDatum& d) const;
    virtual void set_status(const DictionaryDatum& d);

    /**
     * @brief Base class to all nodes that record traces.
     * 
     * Get the trace at the given time step. Returns an iterator to a
     * circular buffer that allows to read the trace. Note that neither
     * this function nor the iterator checks for the limits of the trace.
     * Instead values wrap around at the buffer limits. Reading at or beyond
     * ConnectionUpdateManager::get_origin() (which is equal to the slice
     * origin), or values older than ConnectionUpdateManager::get_horizon()
     * will lead to undefined behavior.
     * 
     * @param steps the time point to be read.
     * @param id the index of the trace.
     * @return an iterator to the trace at the given time point.
     */
    inline
    const_iterator get_trace(nest::delay steps, trace_id id) const
    {
        assert(id < traces_.size());
        return traces_[id].get(steps);
    };

    /**
     * Convenience function. Time point of trace is passed here as a set::Time
     * object.
     * 
     * @param time the time point to be read.
     * @param id the index of the trace.
     * @return an iterator to the trace at the given time point.
     */
    inline
    const_iterator get_trace(nest::Time const &time, trace_id id) const
    {
        return get_trace(time.get_steps(), id);
    };

    /**
     * @return the number of traces that are recorded by this node.
     */
    inline
    size_t get_num_traces() const
    {
        return traces_.size();
    };

protected:
    void init_traces(size_t num_traces);

    /**
     * Set value of a trace at the given time point. Values are supposed to
     * be written only in the interval of the current slice.
     * 
     * @param steps time to write values to (in steps).
     * @param v value to be written.
     * @param id id of the trace (default is 0).
     */
    void set_trace(nest::delay steps, double v, trace_id id = 0)
    {
        assert(id < traces_.size());
        traces_[id][steps] = v;
    }
    
    virtual void get_trace_status(DictionaryDatum& d) const;

private:
    std::vector< CircularBuffer<double> > traces_;
};

}

#endif
