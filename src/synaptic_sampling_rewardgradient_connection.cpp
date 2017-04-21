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

/*
 * Define default values and constraints for synaptic parameters.
 */
template < typename T, typename C >
    static void define_parameters( T & p, C &v )
{
    p.parameter( v.learning_rate_, "learning_rate", 5e-08, pc::MinD(0.0) );
    p.parameter( v.episode_length_, "episode_length", 1000.0, pc::BiggerD(0.0) );
    p.parameter( v.psp_tau_rise_, "psp_tau_rise", 2.0, pc::BiggerD(0.0) );
    p.parameter( v.psp_tau_fall_, "psp_tau_fall", 20.0, pc::BiggerD(0.0) );
    p.parameter( v.temperature_, "temperature", 0.1, pc::MinD(0.0) );
    p.parameter( v.gradient_noise_, "gradient_noise", 0.0, pc::MinD(0.0) );
    p.parameter( v.max_param_, "max_param", 5.0 );
    p.parameter( v.min_param_, "min_param", -2.0 );
    p.parameter( v.max_param_change_, "max_param_change", 40.0, pc::MinD(0.0) );
    p.parameter( v.integration_time_, "integration_time", 50000.0, pc::BiggerD(0.0) );
    p.parameter( v.direct_gradient_rate_, "direct_gradient_rate", 0.0 );
    p.parameter( v.parameter_mapping_offset_, "parameter_mapping_offset", 3.0 );
    p.parameter( v.weight_scale_, "weight_scale", 1.0 );
    p.parameter( v.weight_update_interval_, "weight_update_interval", 100.0, pc::BiggerD(0.0) );
    p.parameter( v.gradient_scale_, "gradient_scale", 1.0 );
    p.parameter( v.bap_trace_id_, "bap_trace_id", 0l, pc::MinL(0) );
    p.parameter( v.dopa_trace_id_, "dopa_trace_id", 0l, pc::MinL(0) );
    p.parameter( v.psp_cutoff_amplitude_, "psp_cutoff_amplitude", 0.0001, pc::MinD(0) );
    p.parameter( v.simulate_retracted_synapses_, "simulate_retracted_synapses", false );
    p.parameter( v.delete_retracted_synapses_, "delete_retracted_synapses", false );
    p.parameter( v.verbose_, "verbose", false );
}

/**
 * Default constructor.
 */
SynapticSamplingRewardGradientCommonProperties::SynapticSamplingRewardGradientCommonProperties()
: nest::CommonSynapseProperties(),
  reward_transmitter_(0),
  resolution_unit_(-1.0),
  reward_gradient_update_(0.0),
  eligibility_trace_update_(0.0),
  psp_faciliation_update_(0.0),
  psp_depression_update_(0.0),
  psp_scale_factor_(0.0),
  weight_update_steps_(0),
  std_wiener_(0.0),
  std_gradient_(0.0)
{
    SetDefault p;
    define_parameters < SetDefault > ( p, *this );
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

    GetStatus p( d );
    define_parameters < GetStatus > ( p, *this );

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

    CheckParameters p_check( d );
    define_parameters < CheckParameters > ( p_check, *this );

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

    SetStatus p_set( d );
    define_parameters < SetStatus > ( p_set, *this );
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

    weight_update_steps_ = std::ceil(weight_update_interval_ / resolution_unit_);

    const double l_rate = weight_update_interval_*learning_rate_;
    std_wiener_ = std::sqrt(2.0 * temperature_ * l_rate);
    std_gradient_ = std::sqrt(2.0 * gradient_noise_ * l_rate);

    psp_faciliation_update_ = std::exp(-resolution_unit_ / psp_tau_fall_);
    psp_depression_update_ = std::exp(-resolution_unit_ / psp_tau_rise_);
    psp_scale_factor_ = (psp_tau_fall_ / (psp_tau_fall_ - psp_tau_rise_));
    reward_gradient_update_ = (integration_time_ == 0.0) ? 0.0 : std::exp(-resolution_unit_ / integration_time_);
    eligibility_trace_update_ = (episode_length_ == 0.0) ? 0.0 : std::exp(-resolution_unit_ / episode_length_);
}

}
