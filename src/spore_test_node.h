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
 * File:   spore_test_node.h
 * Author: Kappel
 *
 * Created on November 9, 2016, 2:30 PM
 */

#ifndef SPORE_TEST_MODULE_H
#define SPORE_TEST_MODULE_H

#include "dictutils.h"

#include "tracing_node.h"
#include "spore_test_base.h"

namespace spore
{

/**
 * @brief Node to test the SPORE module.
 */
class SporeTestNode : public TracingNode
{
public:
    SporeTestNode();
    ~SporeTestNode();

    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &);

    /**
     * Import sets of overloaded virtual functions.
     * @see Technical Issues / Virtual Functions: Overriding, Overloading, and Hiding
     */
    using nest::Node::handle;
    using nest::Node::handles_test_event;

    void handle(nest::SpikeEvent &);
    nest::port handles_test_event(nest::SpikeEvent&, nest::rport);
    nest::port send_test_event(nest::Node&, nest::rport, nest::synindex, bool);


protected:
    void register_test(SporeTestBase *test);

private:
    void init_state_(const nest::Node& proto);
    void init_buffers_();
    void calibrate();

    void update(nest::Time const &, const long, const long);

    std::string test_name_;
    double test_time_;

    std::map<std::string, SporeTestBase*> tests_;
};

/**
 * PoissonDblExpNeuron test event.
 */
inline
nest::port SporeTestNode::send_test_event(nest::Node& target, nest::rport receptor_type, nest::synindex, bool)
{
    nest::SpikeEvent e;
    e.set_sender(*this);

    return target.handles_test_event(e, receptor_type);
}

/**
 * PoissonDblExpNeuron test event.
 */
inline
nest::port SporeTestNode::handles_test_event(nest::SpikeEvent&, nest::rport receptor_type)
{
    return 0;
}

/**
 * Status getter function.
 */
inline
void SporeTestNode::get_status(DictionaryDatum &d) const
{
    def<std::string>(d, "test_name", test_name_);
    def<double>(d, "test_time", test_time_);
    if (not test_name_.empty())
    {
        const SporeTestBase *test = tests_.at(test_name_);
        assert(test);
        test->get_status(d);
    }
}

/**
 * Status setter function.
 */
inline
void SporeTestNode::set_status(const DictionaryDatum &d)
{
    std::string test_name;
    if (updateValue<std::string>(d, "test_name", test_name))
    {
        if (tests_.find(test_name) == tests_.end())
            throw nest::BadParameter("test '" + test_name + "' does not exist!");

        test_name_ = test_name;
    }

    
    updateValue<double>(d, "test_time", test_time_);
    
    if (not test_name_.empty())
    {
        tests_[test_name_]->set_status(d);
    }
}

}

#endif

