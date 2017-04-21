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
 * File:   test_pulse_trace.h
 * Author: Kappel
 *
 * Created on April 4, 2017, 6:23 PM
 */

#ifndef TEST_PULSE_TRACE_H
#define	TEST_PULSE_TRACE_H

#include "spore_test_base.h"

namespace spore
{

/**
 * @brief Test class to generate a pulsed trace for testing synapse models.
 */
class TestPulseTrace : public SporeTestBase
{
public:
    TestPulseTrace();

    virtual double get_trace(nest::delay time_step, TracingNode::trace_id id);
    virtual void set_status(const DictionaryDatum &d);

    virtual size_t get_num_traces() const
    {
        return 1;
    };

private:
    std::vector< double > spike_times_;
    double offset_;
    double weight_;
};

}

#endif
