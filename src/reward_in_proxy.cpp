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
 * File:   reward_in_proxy.cpp
 * Author: Hoff
 *
 * This file is based on music_cont_in_proxy.cpp which is part of NEST
 * (Copyright (C) 2004 The NEST Initiative).
 * See: http://nest-initiative.org/
 */

#include "reward_in_proxy.h"

#include "spore.h"
#include "config.h"
#include "exceptions.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "arraydatum.h"
#include "spikecounter.h"
#include "universal_data_logger_impl.h"
#include "connection_updater.h"
#include "dictutils.h"
#include "music.hh"
#include "tracing_node.h"
#include "logging.h"

#include <numeric>
#include <iterator>

#include <stdio.h>

namespace spore
{
#ifndef HAVE_MUSIC
#error MUSIC bindings for NEST are required.
#endif

/* ----------------------------------------------------------------
 * Default constructors defining default parameters, state and buffer
 * ---------------------------------------------------------------- */

RewardInProxy::Parameters_::Parameters_()
: port_name_("reward_in")
, delay_(0.0)
{
}

RewardInProxy::State_::State_()
: published_(false)
, port_width_(-1)
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void RewardInProxy::Parameters_::get(DictionaryDatum& d) const
{
    (*d)[ nest::names::port_name ] = port_name_;
    (*d)[ "delay" ] = delay_;
}

void RewardInProxy::Parameters_::set(const DictionaryDatum& d, State_& s)
{
    if (!s.published_)
    {
        updateValue< string >(d, nest::names::port_name, port_name_);
        updateValue< float >(d, "delay", delay_);
    }
    /* TODO
    else
    {
        // TODO verify first argument
        throw MUSICPortAlreadyPublished("reward_in_proxy", port_name_);
    }
     */
}

void RewardInProxy::State_::get(DictionaryDatum& d) const
{
    (*d)[ nest::names::published ] = published_;
    (*d)[ nest::names::port_width ] = port_width_;
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

RewardInProxy::RewardInProxy()
: TracingNode()
, S_()
, P_()
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void RewardInProxy::init_state_(const Node& n)
{
    const RewardInProxy& pr = downcast< RewardInProxy >(n);
    S_ = pr.S_;
}

void RewardInProxy::init_buffers_()
{
}

void RewardInProxy::calibrate()
{
    // @todo: Remove this as soon as bug gets fixed!
    const size_t num_threads = nest::kernel().vp_manager.get_num_threads();
    if (num_threads > 1)
    {
        throw nest::BadProperty("Due to issue #696 in NEST RewardInProxy can currently only be used with"
                                " local_num_threads is set to 1!");
    }

    // only publish the port once
    if (!S_.published_)
    {
        MUSIC::Setup* s = nest::kernel().music_manager.get_music_setup();
        if (s == 0)
        {
            throw nest::MUSICSimulationHasRun(get_name());
        }

        reward_in_ = s->publishContInput(P_.port_name_);

        if (reward_in_->isConnected())
        {
            if (!reward_in_->hasWidth())
            {
                throw nest::MUSICPortHasNoWidth(get_name(), P_.port_name_);
            }

            S_.port_width_ = reward_in_->width();

            reward_in_buffer_ = std::vector< double >(S_.port_width_);
            MUSIC::ArrayData data_map(static_cast<void*> (&(reward_in_buffer_[ 0 ])), MPI::DOUBLE, 0, S_.port_width_);

            reward_in_->map(&data_map, P_.delay_, true);
            S_.published_ = true;

            init_traces(S_.port_width_);

            std::string msg = String::compose("Mapping MUSIC input port '%1' with width=%2.",
                                              P_.port_name_, S_.port_width_);
            LOG(nest::M_INFO, "reward_in_proxy::calibrate()", msg.c_str());
        }
        else
        {
            // throw nest::MUSICPortUnconnected(get_name(), P_.port_name_);
            std::string msg = String::compose("MUSIC port '%1' is unconnected.", P_.port_name_);
            LOG(nest::M_WARNING, "reward_in_proxy::calibrate()", msg.c_str());
        }
    }
}

void RewardInProxy::get_status(DictionaryDatum& d) const
{
    TracingNode::get_trace_status(d);
    
    P_.get(d);
    S_.get(d);

    (*d)[nest::names::element_type] = LiteralDatum(nest::names::other);
}

void RewardInProxy::set_status(const DictionaryDatum& d)
{
    Parameters_ ptmp = P_; // temporary copy in case of errors
    ptmp.set(d, S_); // throws if BadProperty
    P_ = ptmp;
}

void RewardInProxy::update(const nest::Time& origin, const long from, const long to)
{
    int n_channels = S_.port_width_;

    if (n_channels == -1)
    {
        return;
    }

    for (long lag = from; lag < to; ++lag)
    {
        nest::Time time = nest::Time::step(origin.get_steps() + lag);
        for (int channel = 0; channel < n_channels; channel++)
        {
            set_trace(time.get_steps(), reward_in_buffer_[channel], channel);
        }
    }

#if __SPORE_DEBUG__
    for (int i = 0; i < n_channels; i++)
    {
        std::cout << "trace #" << i << ":";
        TracingNode::const_iterator trace = get_trace(0, i);
        size_t steps = ConnectionUpdateManager::instance()->get_max_latency();
        while (steps--)
        {
            std::cout << " " << *trace;
            ++trace;
        }
        std::cout << std::endl;
    }
#endif
}
}
