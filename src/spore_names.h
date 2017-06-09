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
 * File:   spore_names.h
 * Author: Kappel
 *
 * Created on June 8, 2017, 5:51 PM
 */

#ifndef SPORE_NAMES_H
#define SPORE_NAMES_H

#include "name.h"

namespace spore
{
/**
 * This namespace contains global Name objects. These can be used in
 * Node::get_status and Node::set_status to make data exchange
 * more efficient and consistent. Creating a Name from a std::string
 * is in O(log n), for n the number of Names already created. Using
 * predefined names should make data exchange much more efficient.
 */
namespace names
{
extern const Name trace;
extern const Name delay;
extern const Name reward_in_proxy;

extern const Name weight_update_time;
extern const Name bap_trace_id;
extern const Name recorder_times;
extern const Name recorder_interval;
extern const Name recorder_values;
extern const Name reset_recorder;
extern const Name test_name;
extern const Name test_time;

extern const Name reward_transmitter;
extern const Name learning_rate;
extern const Name episode_length;
extern const Name psp_tau_rise;
extern const Name psp_tau_fall;
extern const Name temperature;
extern const Name gradient_noise;
extern const Name max_param;
extern const Name min_param;
extern const Name max_param_change;
extern const Name integration_time;
extern const Name direct_gradient_rate;
extern const Name parameter_mapping_offset;
extern const Name weight_scale;
extern const Name weight_update_interval;
extern const Name gradient_scale;
extern const Name dopa_trace_id;
extern const Name psp_cutoff_amplitude;
extern const Name simulate_retracted_synapses;
extern const Name delete_retracted_synapses;

extern const Name synaptic_parameter;
extern const Name eligibility_trace;
extern const Name reward_gradient;
extern const Name prior_mean;
extern const Name prior_precision;

extern const Name eligibility_trace_values;
extern const Name psp_values;
extern const Name weight_values;
extern const Name synaptic_parameter_values;
extern const Name reward_gradient_values;

extern const Name tau_rise_exc;
extern const Name tau_fall_exc;
extern const Name tau_rise_inh;
extern const Name tau_fall_inh;
extern const Name input_conductance;
extern const Name target_rate;
extern const Name target_adaptation_speed;
extern const Name adaptive_threshold;
}

}

#endif
