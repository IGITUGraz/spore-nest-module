/*
 * This file is part of SPORE.
 *
 * Copyright (c) 2016, the SPORE team (see AUTHORS).
 * 
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
 * File:   poisson_dbl_exp_neuron.cpp
 *
 * This file is based on pp_psc_delta.cpp which is part of NEST
 * (Copyright (C) 2004 The NEST Initiative).
 * See: http://nest-initiative.org/
 */

#include "exceptions.h"
#include "poisson_dbl_exp_neuron.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"
#include "universal_data_logger_impl.h"
#include "compose.hpp"

#include <limits>

namespace nest
{
/* 
 * Recordables map of PoissonDblExpNeuron.
 */
template <>
void nest::RecordablesMap<spore::PoissonDblExpNeuron>::create()
{
    // use standard names whereever you can for consistency!
    insert_(names::V_m, &spore::PoissonDblExpNeuron::get_V_m_);
    insert_(names::E_sfa, &spore::PoissonDblExpNeuron::get_E_sfa_);
}
}

namespace spore
{
/* 
 * Recordables map instance.
 */
nest::RecordablesMap<PoissonDblExpNeuron> PoissonDblExpNeuron::recordablesMap_;

//
// PoissonDblExpNeuron::Parameters_ implementation.
//

/**
 * Default constructor defining default parameters.
 */
PoissonDblExpNeuron::Parameters_::Parameters_()
: tau_rise_exc_(2.0), // ms
  tau_fall_exc_(20.0), // ms
  tau_rise_inh_(1.0), // ms
  tau_fall_inh_(10.0), // ms
  input_conductance_(1.0), // S
  dead_time_(1.0), // ms
  dead_time_random_(0), // bool
  dead_time_shape_(1), // int
  with_reset_(1), // bool
  c_1_(0.0), // Hz / mV
  c_2_(1.238), // Hz / mV
  c_3_(0.25), // 1.0 / mV
  I_e_(0.0), // pA
  t_ref_remaining_(0.0), // ms
  target_rate_(10.0), // Hz
  target_adaptation_speed_(0.0) //
{}


/**
 * Default constructor.
 */
PoissonDblExpNeuron::State_::State_()
: u_rise_exc_(0.0),
  u_fall_exc_(0.0),
  u_rise_inh_(0.0),
  u_fall_inh_(0.0),
  u_membrane_(0.0),
  input_current_(0.0),
  adaptative_threshold_(0.0),
  lost_p_values_(0),
  r_(0)
{}


/**
 * Parameter getter function.
 */
void PoissonDblExpNeuron::Parameters_::get(DictionaryDatum &d) const
{
    def<double>(d, nest::names::I_e, I_e_);
    def<double>(d, nest::names::dead_time, dead_time_);
    def<bool>(d, nest::names::dead_time_random, dead_time_random_);
    def<long>(d, nest::names::dead_time_shape, dead_time_shape_);
    def<bool>(d, nest::names::with_reset, with_reset_);

    def<double>(d, nest::names::c_1, c_1_);
    def<double>(d, nest::names::c_2, c_2_);
    def<double>(d, nest::names::c_3, c_3_);
    def<double>(d, nest::names::t_ref_remaining, t_ref_remaining_);

    def<double>(d, "tau_rise_exc", tau_rise_exc_);
    def<double>(d, "tau_fall_exc", tau_fall_exc_);
    def<double>(d, "tau_rise_inh", tau_rise_inh_);
    def<double>(d, "tau_fall_inh", tau_fall_inh_);
    def<double>(d, "input_conductance", input_conductance_);
    def<double>(d, "target_rate", target_rate_);
    def<double>(d, "target_adaptation_speed", target_adaptation_speed_);
}

/**
 * Parameter setter function.
 */
void PoissonDblExpNeuron::Parameters_::set(const DictionaryDatum& d)
{
    updateValue<double>(d, nest::names::I_e, I_e_);
    updateValue<double>(d, nest::names::dead_time, dead_time_);
    updateValue<bool>(d, nest::names::dead_time_random, dead_time_random_);
    updateValue<long>(d, nest::names::dead_time_shape, dead_time_shape_);
    updateValue<bool>(d, nest::names::with_reset, with_reset_);
    updateValue<double>(d, nest::names::c_1, c_1_);
    updateValue<double>(d, nest::names::c_2, c_2_);
    updateValue<double>(d, nest::names::c_3, c_3_);
    updateValue<double>(d, nest::names::t_ref_remaining, t_ref_remaining_);

    updateValue<double>(d, "tau_rise_exc", tau_rise_exc_);
    updateValue<double>(d, "tau_fall_exc", tau_fall_exc_);
    updateValue<double>(d, "tau_rise_inh", tau_rise_inh_);
    updateValue<double>(d, "tau_fall_inh", tau_fall_inh_);
    updateValue<double>(d, "input_conductance", input_conductance_);
    updateValue<double>(d, "target_rate", target_rate_);
    updateValue<double>(d, "target_adaptation_speed", target_adaptation_speed_);

    if (dead_time_ < 0)
        throw nest::BadProperty("Absolute refractory time must not be negative.");

    if (dead_time_shape_ < 1)
        throw nest::BadProperty("Shape of the dead time gamma distribution must not be smaller than 1.");

    if ((tau_rise_exc_ <= 0) || (tau_fall_exc_ <= 0) || (tau_rise_inh_ <= 0) || (tau_fall_inh_ <= 0))
        throw nest::BadProperty("All time constants must be strictly positive.");

    if (t_ref_remaining_ < 0)
        throw nest::BadProperty("Remaining refractory time can not be negative.");

    if (c_3_ < 0)
        throw nest::BadProperty("C_3 must be positive.");
}

//
// PoissonDblExpNeuron::State_ implementation.
//

/**
 * State getter function.
 */
void PoissonDblExpNeuron::State_::get(DictionaryDatum &d, const Parameters_&) const
{
    def<double>(d, nest::names::V_m, u_membrane_); // Membrane potential
    def<double>(d, "adaptive_threshold", adaptative_threshold_);
    def<long>(d, "lost_p_values", lost_p_values_);
}


/**
 * Sate setter function.
 */
void PoissonDblExpNeuron::State_::set(const DictionaryDatum& d, const Parameters_&)
{
    updateValue<double>(d, nest::names::V_m, u_membrane_);
    updateValue<double>(d, "adaptive_threshold", adaptative_threshold_);

    long ne;
    if (updateValue<long>(d, "lost_p_values", ne))
    {
        if (ne == 0)
            lost_p_values_ = 0;
        else
            throw nest::BadProperty("lost_p_values can only be set to 0.");
    }
}

//
// PoissonDblExpNeuron::Buffers_ implementation.
//

/**
 * Constructor.
 */
PoissonDblExpNeuron::Buffers_::Buffers_(PoissonDblExpNeuron &n)
: logger_(n)
{}


/**
 * Constructor.
 */
PoissonDblExpNeuron::Buffers_::Buffers_(const Buffers_ &, PoissonDblExpNeuron &n)
: logger_(n)
{}

//
// PoissonDblExpNeuron implementation.
//

/**
 * Default Constructor.
 */
PoissonDblExpNeuron::PoissonDblExpNeuron()
: TracingNode(),
  P_(),
  S_(),
  B_(*this)
{
    recordablesMap_.create();
}

/**
 * Copy Constructor.
 */
PoissonDblExpNeuron::PoissonDblExpNeuron(const PoissonDblExpNeuron& n)
: TracingNode(n),
  P_(n.P_),
  S_(n.S_),
  B_(n.B_, *this)
{
}


/**
 * Node state initialization.
 */ 
void PoissonDblExpNeuron::init_state_(const nest::Node& proto)
{
    const PoissonDblExpNeuron& pr = downcast<PoissonDblExpNeuron>(proto);
    S_ = pr.S_;
    S_.r_ = nest::Time(nest::Time::ms(P_.t_ref_remaining_)).get_steps();
}


/**
 * Initialize the node's spike and current buffers.
 */
void PoissonDblExpNeuron::init_buffers_()
{
    B_.exc_spikes_.clear(); //!< includes resize
    B_.inh_spikes_.clear(); //!< includes resize
    B_.currents_.clear(); //!< includes resize
    B_.logger_.reset(); //!< includes resize

    init_traces(1);
}


/**
 * Calibrate the node.
 */
void PoissonDblExpNeuron::calibrate()
{

    B_.logger_.init();

    V_.h_ = nest::Time::get_resolution().get_ms();
    V_.rng_ = net_->get_rng(get_thread());

    V_.decay_rise_exc_ = std::exp(-V_.h_ / P_.tau_rise_exc_);
    V_.decay_fall_exc_ = std::exp(-V_.h_ / P_.tau_fall_exc_);
    V_.decay_rise_inh_ = std::exp(-V_.h_ / P_.tau_rise_inh_);
    V_.decay_fall_inh_ = std::exp(-V_.h_ / P_.tau_fall_inh_);
    V_.norm_exc_ = (P_.tau_fall_exc_ / (P_.tau_fall_exc_ - P_.tau_rise_exc_));
    V_.norm_inh_ = (P_.tau_fall_inh_ / (P_.tau_fall_inh_ - P_.tau_rise_inh_));

    if (P_.dead_time_ != 0 && P_.dead_time_ < V_.h_)
        P_.dead_time_ = V_.h_;

    // TauR specifies the length of the absolute refractory period as
    // a double in ms. The grid based iaf_psp_delta can only handle refractory
    // periods that are integer multiples of the computation step size (h).
    // To ensure consistency with the overall simulation scheme such conversion
    // should be carried out via objects of class nest::Time. The conversion
    // requires 2 steps:
    //
    //     1. A time object r is constructed defining the representation of
    //        TauR in tics. This representation is then converted to computation time
    //        steps again by a strategy defined by class nest::Time.
    //     2. The refractory time in units of steps is read out by get_steps(), a member
    //        function of class nest::Time.
    //
    // The definition of the refractory period of the PoissonDblExpNeuron is consistent
    // with the one of iaf_neuron_ps.
    //
    // Choosing a TauR that is not an integer multiple of the computation time
    // step h will lead to accurate (up to the resolution h) and self-consistent
    // results. However, a neuron model capable of operating with real valued spike
    // time may exhibit a different effective refractory time.
    if (P_.dead_time_random_)
    {
        V_.dt_rate_ = P_.dead_time_shape_ / P_.dead_time_; // Choose dead time rate parameter such that mean equals dead_time
        V_.gamma_dev_.set_order(P_.dead_time_shape_);
    }
    else
    {
        V_.DeadTimeCounts_ = nest::Time(nest::Time::ms(P_.dead_time_)).get_steps();
        assert(V_.DeadTimeCounts_ >= 0); // Since t_ref_ >= 0, this can only fail in error
    }
}


/**
 * Update the node to the given time point.
 */
void PoissonDblExpNeuron::update(nest::Time const & origin, const long from, const long to)
{

    assert(to >= 0 && from < nest::Scheduler::get_min_delay());
    assert(from < to);

    for (long lag = from; lag < to; ++lag)
    {
        double psp_amplitude_exc = B_.exc_spikes_.get_value(lag);
        double psp_amplitude_inh = B_.inh_spikes_.get_value(lag);

        S_.u_rise_exc_ *= V_.decay_rise_exc_;
        S_.u_fall_exc_ *= V_.decay_fall_exc_;
        S_.u_rise_inh_ *= V_.decay_rise_inh_;
        S_.u_fall_inh_ *= V_.decay_fall_inh_;

        if (psp_amplitude_exc != 0.0)
        {
            S_.u_rise_exc_ += psp_amplitude_exc;
            S_.u_fall_exc_ += psp_amplitude_exc;
        }
        
        if (psp_amplitude_inh != 0.0)
        {
            S_.u_rise_inh_ += psp_amplitude_inh;
            S_.u_fall_inh_ += psp_amplitude_inh;
        }

        S_.u_membrane_ = V_.norm_exc_ * (S_.u_fall_exc_ - S_.u_rise_exc_) +
                V_.norm_inh_ * (S_.u_fall_inh_ - S_.u_rise_inh_) +
                P_.input_conductance_ * (S_.input_current_ + P_.I_e_);

        nest::Time time = nest::Time::step(origin.get_steps() + lag);

        S_.adaptative_threshold_ -= 1e-3 * V_.h_ * P_.target_rate_ * P_.target_adaptation_speed_;

        if (S_.r_ == 0)
        {
            // Neuron not refractory

            // Calculate instantaneous rate from transfer function:
            //     rate = c1 * u' + c2 * exp(c3 * u')
            
            double V_eff = S_.u_membrane_ - S_.adaptative_threshold_;

            double rate = (P_.c_1_ * V_eff + P_.c_2_ * std::exp(P_.c_3_ * V_eff));
            double spike_probability = -numerics::expm1(-rate * V_.h_ * 1e-3);
            long n_spikes = 0;

            if (rate > 0.0)
            {
                if (P_.dead_time_ > 0.0)
                {
                    // Draw random number and compare to probability to have a spike
                    if (V_.rng_->drand() <= spike_probability)
                        n_spikes = 1;
                }
                else
                {
                    // Draw Poisson random number of spikes
                    V_.poisson_dev_.set_lambda(rate);
                    n_spikes = V_.poisson_dev_.ldev(V_.rng_);
                }

                if (n_spikes > 0) // Is there a spike? Then set the new dead time.
                {
                    // Set dead time interval according to parameters
                    if (P_.dead_time_random_)
                    {
                        S_.r_ = nest::Time(nest::Time::ms(V_.gamma_dev_(V_.rng_) / V_.dt_rate_)).get_steps();
                    }
                    else
                        S_.r_ = V_.DeadTimeCounts_;

                    // And send the spike event
                    nest::SpikeEvent se;
                    se.set_multiplicity(n_spikes);
                    network()->send(*this, se, lag);

                    // Reset the potential if applicable
                    if (P_.with_reset_)
                    {
                        S_.u_membrane_ = 0.0;
                        S_.u_rise_exc_ = 0.0;
                        S_.u_fall_exc_ = 0.0;
                        S_.u_rise_inh_ = 0.0;
                        S_.u_fall_inh_ = 0.0;
                    }

                    S_.adaptative_threshold_ += P_.target_adaptation_speed_;
                } // S_.u_membrane_ = P_.V_reset_;
            } // if (rate > 0.0)

            set_trace(time.get_steps(), double(n_spikes) - spike_probability);
        }
        else // Neuron is within dead time
        {
            set_trace(time.get_steps(), 0.0);
            --S_.r_;
        }

        // Set new input current
        S_.input_current_ = B_.currents_.get_value(lag);

        // Voltage logging
        B_.logger_.record_data(origin.get_steps() + lag);
    }
}


/**
 * SpikeEvent handling.
 * @param e the event.
 */
void PoissonDblExpNeuron::handle(nest::SpikeEvent & e)
{
    assert(e.get_delay() > 0);

    // EX: We must compute the arrival time of the incoming spike
    //     explicitly, since it depends on delay and offset within
    //     the update cycle.  The way it is done here works, but
    //     is clumsy and should be improved.
    if (e.get_rport()==0)
    {
        B_.exc_spikes_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
                             e.get_weight() * e.get_multiplicity());
    }
    else if (e.get_rport()==1)
    {
        B_.inh_spikes_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
                             e.get_weight() * e.get_multiplicity());
    }
    else
        throw nest::BadProperty( "Unexpected rport id: "+std::to_string(e.get_rport()) );
}


/**
 * CurrentEvent handling.
 * @param e the event.
 */
void PoissonDblExpNeuron::handle(nest::CurrentEvent& e)
{
    assert(e.get_delay() > 0);

    const double c = e.get_current();
    const double w = e.get_weight();

    B_.currents_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
                           w * c);
}


/**
 * DataLoggingRequest handling.
 * @param e the event.
 */
void PoissonDblExpNeuron::handle(nest::DataLoggingRequest &e)
{
    B_.logger_.handle(e);
}

} // namespace
