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
 * File:   spore_names.cpp
 * Author: Kappel
 *
 * Created on June 8, 2017, 5:51 PM
 */

#include "spore_names.h"

namespace spore
{

namespace names
{
const Name trace("trace");
const Name delay("delay");
const Name reward_in_proxy("reward_in_proxy");

const Name weight_update_time("weight_update_time");
const Name bap_trace_id("bap_trace_id");
const Name recorder_times("recorder_times");
const Name recorder_interval("recorder_interval");
const Name recorder_values("recorder_values");
const Name reset_recorder("reset_recorder");
const Name test_name("test_name");
const Name test_time("test_time");

const Name reward_transmitter("reward_transmitter");
const Name learning_rate("learning_rate");
const Name episode_length("episode_length");
const Name psp_tau_rise("psp_tau_rise");
const Name psp_tau_fall("psp_tau_fall");
const Name temperature("temperature");
const Name gradient_noise("gradient_noise");
const Name max_param("max_param");
const Name min_param("min_param");
const Name max_param_change("max_param_change");
const Name integration_time("integration_time");
const Name direct_gradient_rate("direct_gradient_rate");
const Name parameter_mapping_offset("parameter_mapping_offset");
const Name weight_scale("weight_scale");
const Name weight_update_interval("weight_update_interval");
const Name gradient_scale("gradient_scale");
const Name dopa_trace_id("dopa_trace_id");
const Name psp_cutoff_amplitude("psp_cutoff_amplitude");
const Name simulate_retracted_synapses("simulate_retracted_synapses");
const Name delete_retracted_synapses("delete_retracted_synapses");

const Name synaptic_parameter("synaptic_parameter");
const Name eligibility_trace("eligibility_trace");
const Name reward_gradient("reward_gradient");
const Name prior_mean("prior_mean");
const Name prior_precision("prior_precision");

const Name eligibility_trace_values("eligibility_trace_values");
const Name psp_values("psp_values");
const Name weight_values("weight_values");
const Name synaptic_parameter_values("synaptic_parameter_values");
const Name reward_gradient_values("reward_gradient_values");

const Name tau_rise_exc("tau_rise_exc");
const Name tau_fall_exc("tau_fall_exc");
const Name tau_rise_inh("tau_rise_inh");
const Name tau_fall_inh("tau_fall_inh");
const Name input_conductance("input_conductance");
const Name target_rate("target_rate");
const Name target_adaptation_speed("target_adaptation_speed");
const Name adaptive_threshold("adaptive_threshold");
}

}
