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
 * File:   spore_test_base.h
 * Author: Kappel
 *
 * Created on November 9, 2016, 3:13 PM
 */

#ifndef SPORE_TEST_BASE_H
#define	SPORE_TEST_BASE_H

#include <string>

#include "nest.h"

#include "tracing_node.h"


namespace spore
{
/**
 * Base class to all tests.
 */
class SporeTestBase
{
public:
    SporeTestBase(const std::string &name, nest::delay t_max=1)
    :name_(name), t_max_(t_max)
    {};
    
    ~SporeTestBase()
    {};
    
    const std::string &get_name() const
    {
        return name_;
    };

    nest::delay get_t_max() const
    {
        return t_max_;
    };
    
    virtual void init() {};
    virtual void check(nest::delay time_step, TracingNode *node) {};
    virtual size_t get_num_traces() const {return 0;};
    virtual double get_trace(nest::delay time_step, TracingNode::trace_id id) { assert(false); };
    
protected:
    void test_assert(bool condition, const std::string &msg)
    {
        if (!condition)
            throw std::runtime_error(name_+" failed: "+msg);
    };
    
private:
    std::string name_;
    nest::delay t_max_;
};

}

#endif

