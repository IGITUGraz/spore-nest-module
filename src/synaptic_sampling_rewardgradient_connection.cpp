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
 * File:   synaptic_sampling_rewardgradient_connection.cpp
 * Author: Kappel, Hsieh
 *
 * This file is based on stdp_dopa_connection.h which is part of NEST
 * (Copyright (C) 2004 The NEST Initiative).
 * See: http://nest-initiative.org/
 */


#include "synaptic_sampling_rewardgradient_connection.h"


namespace spore
{

/**
 * Default constructor.
 */
SynapticSamplingRewardGradientCommonProperties::SynapticSamplingRewardGradientCommonProperties()
: nest::CommonSynapseProperties(),
  reward_transmitter_(0),
  resolution_unit_(-1.0),
  gamma_(0.0),
  lambda_(0.0),
  psp_faciliation_update_(0.0),
  psp_depression_update_(0.0),
  psp_scale_factor_(0.0),
  weight_update_steps_(0),
  std_wiener_(0.0),
  std_gradient_(0.0)
{
    SetDefault p;
    define_parameters < SetDefault > ( p );
}

/**
 * Destructor.
 */
SynapticSamplingRewardGradientCommonProperties::~SynapticSamplingRewardGradientCommonProperties()
{
}

/**
 * Status getter function.
 */
void SynapticSamplingRewardGradientCommonProperties::get_status(DictionaryDatum & d) const
{
    nest::CommonSynapseProperties::get_status(d);

    if (reward_transmitter_ != 0)
    {
        def<long>(d, "reward_transmitter", reward_transmitter_->get_gid());
    }
    else
    {
        def<long>(d, "reward_transmitter", -1);
    }
}

/**
 * Status setter function.
 */
void SynapticSamplingRewardGradientCommonProperties::set_status(const DictionaryDatum & d, nest::ConnectorModel& cm)
{
    nest::CommonSynapseProperties::set_status(d, cm);

    SetStatus p( d );
    define_parameters < SetStatus > ( p );

    long rtgid;
    if (updateValue<long>(d, "reward_transmitter", rtgid))
    {
        TracingNode* new_reward_transmitter = dynamic_cast<TracingNode*> (nest::kernel().node_manager.get_node(rtgid));

        if (new_reward_transmitter == 0)
        {
            throw nest::BadProperty("Reward transmitter must be of model TracingNode");
        }

        reward_transmitter_ = new_reward_transmitter;
    }
}

/**
 * Calibrate all time objects, which are contained in this object.
 * This function is called when the time resolution changes and on
 * simulation startup.
 *
 * @param tc time converter object.
 */
void SynapticSamplingRewardGradientCommonProperties::calibrate(const nest::TimeConverter &tc)
{
    // make sure this check is only performed shortly before the simulation starts.
    if (ConnectionUpdateManager::instance()->is_initialized() && not reward_transmitter_)
    {
        throw nest::BadProperty("Reward transmitter was not set at simulation startup!");
    }

    resolution_unit_ = nest::Time::get_resolution().get_ms();

    weight_update_steps_ = std::ceil(weight_update_time_ / resolution_unit_);

    const double l_rate = weight_update_time_*learning_rate_;
    std_wiener_ = std::sqrt(2.0 * temperature_ * l_rate);
    std_gradient_ = std::sqrt(2.0 * gradient_noise_ * l_rate);

    psp_faciliation_update_ = std::exp(-resolution_unit_ / psp_facilitation_rate_);
    psp_depression_update_ = std::exp(-resolution_unit_ / psp_depression_rate_);
    psp_scale_factor_ = (psp_facilitation_rate_ / (psp_facilitation_rate_ - psp_depression_rate_));
    gamma_ = (integration_time_ == 0.0) ? 0.0 : std::exp(-resolution_unit_ / integration_time_);
    lambda_ = (episode_length_ == 0.0) ? 0.0 : std::exp(-resolution_unit_ / episode_length_);
}

}
