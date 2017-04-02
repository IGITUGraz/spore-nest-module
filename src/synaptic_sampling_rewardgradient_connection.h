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
 * File:   synaptic_sampling_rewardgradient_connection.h
 * Author: Kappel, Hsieh
 *
 * This file is based on stdp_dopa_connection.h which is part of NEST
 * (Copyright (C) 2004 The NEST Initiative).
 * See: http://nest-initiative.org/
 */

#ifndef SYNAPTIC_SAMPLING_REWARDGRADIENT_CONNECTIO
#define SYNAPTIC_SAMPLING_REWARDGRADIENT_CONNECTIO

#include <cmath>
#include "nest.h"
#include "connection.h"
#include "normal_randomdev.h"
#include "spikecounter.h"

#include "tracing_node.h"
#include "connection_updater.h"
#include "connection_data_logger.h"
#include "param_utils.h"


namespace spore
{

/**
 * Class containing the common properties for all synapses of type
 * SynapticSamplingRewardGradientConnection.
 */
class SynapticSamplingRewardGradientCommonProperties : public nest::CommonSynapseProperties
{
public:

    SynapticSamplingRewardGradientCommonProperties();
    ~SynapticSamplingRewardGradientCommonProperties();

    using CommonSynapseProperties::get_status;
    using CommonSynapseProperties::set_status;
    using CommonSynapseProperties::calibrate;

    void get_status(DictionaryDatum & d) const;
    void set_status(const DictionaryDatum & d, nest::ConnectorModel& cm);
    void calibrate(const nest::TimeConverter &tc);

    /**
     * Check spike event.
     */
    void check_event(nest::SpikeEvent&)
    {
    }

    /**
     * @return GID of the reward transmitter associated to the synapse type.
     */
    long get_vt_gid() const
    {
        if (reward_transmitter_ != 0)
            return reward_transmitter_->get_gid();
        else
            return -1;
    }

    /**
     * @return The reward transmitter associated to the synapse type.
     */
    nest::Node* get_node()
    {
        if (reward_transmitter_ == 0)
            return nest::CommonSynapseProperties::get_node();
        else
            return reward_transmitter_;
    }

    /**
     * Convenience function to get Gaussian random number for parameter noise.
     */
    double get_d_wiener(nest::thread thread) const
    {
        double result = 0.0;
        if (std_wiener_ > 0)
        {
            result = std_wiener_ * normal_dev_(nest::kernel().rng_manager.get_rng(thread));
        }
        return result;
    }

    /**
     * Convenience function to get Gaussian random number for gradient noise.
     */
    double get_gradient_noise(nest::thread thread) const
    {
        double result = 0.0;
        if (std_gradient_ > 0)
        {
            result = std_gradient_ * normal_dev_(nest::kernel().rng_manager.get_rng(thread));
        }
        return result;
    }

    /**
     * Convenience function to random number.
     */
    double drand(nest::thread thread) const
    {
        return nest::kernel().rng_manager.get_rng(thread)->drand();
    }

    /**
     * @brief Define default values and constraints for synaptic parameters.
     */
    template < typename T >
        void define_parameters( T & p )
    {
        p.parameter( learning_rate_, "learning_rate", 0.0001, pc::MinD(0.0) );
        p.parameter( episode_length_, "episode_length", 100.0, pc::BiggerD(0.0) );
        p.parameter( psp_facilitation_rate_, "psp_facilitation_rate", 20.0, pc::BiggerD(0.0) );
        p.parameter( psp_depression_rate_, "psp_depression_rate", 2.0, pc::BiggerD(0.0) );
        p.parameter( temperature_, "temperature", 0.01, pc::MinD(0.0) );
        p.parameter( gradient_noise_, "gradient_noise", 0.0, pc::MinD(0.0) );
        p.parameter( max_param_, "max_param", 100.0 );
        p.parameter( min_param_, "min_param", -100.0 );
        p.parameter( max_param_change_, "max_param_change", 100.0, pc::MinD(0.0) );
        p.parameter( integration_time_, "integration_time", 10000.0, pc::BiggerD(0.0) );
        p.parameter( direct_gradient_rate_, "direct_gradient_rate", 0.0 );
        p.parameter( parameter_mapping_offset_, "parameter_mapping_offset", 3.0 );
        p.parameter( weight_update_time_, "weight_update_time", 100.0, pc::BiggerD(0.0) );
        p.parameter( gradient_scale_, "gradient_scale", 1.0 );
        p.parameter( bap_trace_id_, "bap_trace_id", 0l, pc::MinL(0) );
        p.parameter( dopa_trace_id_, "dopa_trace_id", 0l, pc::MinL(0) );
        p.parameter( simulate_retracted_synapses_, "simulate_retracted_synapses", false );
        p.parameter( verbose_, "verbose", false );
    }

    // parameters
    double learning_rate_;
    double episode_length_;
    double psp_facilitation_rate_;
    double psp_depression_rate_;

    double temperature_;
    double gradient_noise_;
    double max_param_;
    double min_param_;
    double max_param_change_;
    double integration_time_;
    double direct_gradient_rate_;
    double parameter_mapping_offset_;
    double weight_update_time_;
    double gradient_scale_;

    long bap_trace_id_;
    long dopa_trace_id_;

    bool simulate_retracted_synapses_;
    bool verbose_;

    // state variables
    TracingNode* reward_transmitter_;

    double resolution_unit_;
    double gamma_;
    double lambda_;

    double psp_faciliation_update_;
    double psp_depression_update_;
    double psp_scale_factor_;
    double epsilon_;

    long weight_update_steps_;

private:
    double std_wiener_;
    double std_gradient_;
    librandom::NormalRandomDev normal_dev_;
};

/**
 * @brief Reward-based synaptic sampling connection class
 *
 * SynapticSamplingRewardgradientConnection - Synapse implementing reward-based
 * learning through synaptic sampling. This class implements the model in [1,2].
 * This connection is a diligent synapse model, therefore updates are triggered
 * on a regular interval which is ensured by the ConnectionUpdateManager.
 * The target node and the reward transmitter must be derived from the
 * TracingNode model.
 *
 * <b>Parameters</b>
 *
 * The following parameters can be set in the common properties dictionary:
 * <table>
 * <tr><th>name</th>                        <th>type</th>   <th>comment</th></tr>
 * <tr><td>learning_rate</td>               <td>double</td> <td>learning rate</td></tr>
 * <tr><td>temperature</td>                 <td>double</td> <td>(amplitude) of parameter noise</td></tr>
 * <tr><td>gradient_noise</td>              <td>double</td> <td>amplitude of gradient noise</td></tr>
 * <tr><td>psp_facilitation_rate</td>       <td>double</td> <td>double exponential PSP kernel rise [1/s]</td></tr>
 * <tr><td>psp_depression_rate</td>         <td>double</td> <td>double exponential PSP kernel decay [1/s]</td></tr>
 * <tr><td>integration_time</td>            <td>double</td> <td>time of gradient integration [ms]</td></tr>
 * <tr><td>episode_lengtd</td>              <td>double</td> <td>lengtd of eligibility trace [ms]</td></tr>
 * <tr><td>weight_update_time</td>          <td>double</td> <td>interval of synaptic weight updates [ms]</td></tr>
 * <tr><td>parameter_mapping_offset</td>    <td>double</td> <td>offset parameter for computing synaptic
 *                                                              weight</td></tr>
 * <tr><td>max_param</td>                   <td>double</td> <td>maximum synaptic parameter</td></tr>
 * <tr><td>min_param</td>                   <td>double</td> <td>minimum synaptic parameter</td></tr>
 * <tr><td>max_param_change</td>            <td>double</td> <td>maximum synaptic parameter change</td></tr>
 * <tr><td>direct_gradient_rate</td>        <td>double</td> <td>rate of directly applying changes to the
 *                                                              synaptic parameter</td></tr>
 * <tr><td>gradient_scale</td>              <td>double</td> <td>scaling parameter for tde gradient</td></tr>
 * <tr><td>reward_transmitter</td>          <td>int</td>    <td>GID of tde synapse's reward transmitter</td></tr>
 * <tr><td>bap_trace_id</td>                <td>int</td>    <td>ID of tde BAP trace (default 0)</td></tr>
 * <tr><td>dopa_trace_id</td>               <td>int</td>    <td>ID of tde dopamine trace (default 0)</td></tr>
 * <tr><td>simulate_retracted_synapses</td> <td>bool</td>   <td>continue simulating retracted synapses
 *                                                              (default false)</td></tr>
 * <tr><td>verbose</td>                     <td>bool</td>   <td>write status to std::out (for debugging,
 *                                                              default false)</td></tr>
 * </table>
 *
 * The following parameters can be set in the status dictionary:
 * <table>
 * <tr><th>name</th>                        <th>type</th>   <th>comment</th></tr>
 * <tr><td>synaptic_parameter</td>          <td>double</td> <td>initial synaptic parameter</td></tr>
 * <tr><td>weight</td>                      <td>double</td> <td>current synaptic weight</td></tr>
 * <tr><td>prior_mean</td>                  <td>double</td> <td>mean of tde prior</td></tr>
 * <tr><td>prior_std</td>                   <td>double</td> <td>STD of tde prior</td></tr>
 * <tr><td>is_consolidated</td>             <td>bool</td>   <td>returns true if tde synapse is consolidated</td></tr>
 * <tr><td>recorder_times</td>              <td>double</td> <td>time points of parameter recordings</td></tr>
 * <tr><td>weight_values</td>               <td>double</td> <td>array of recorded synaptic weight values</td></tr>
 * <tr><td>synaptic_parameter_values</td>   <td>double</td> <td>array of recorded synaptic parameter values</td></tr>
 * <tr><td>reward_gradient_values</td>      <td>double</td> <td>array of recorded reward gradient values</td></tr>
 * <tr><td>eligibility_trace_values</td>    <td>double</td> <td>array of recorded eligibility trace values</td></tr>
 * <tr><td>psp_values</td>                  <td>double</td> <td>array of recorded psp values</td></tr>
 * <tr><td>recorder_interval</td>           <td>double</td> <td>interval of synaptic recordings [ms]</td></tr>
 * <tr><td>reset_recorder</td>              <td>bool</td>   <td>clear all recorded values now (write only)</td></tr>
 * </table>
 *
 * <b>References</b>
 *
 * [1] David Kappel, Robert Legenstein, Stefan Habenschuss, Michael Hsieh and
 * Wolfgang Maass. <i>Reward-based self-configuration of neural circuits.</i>
 * In preparation stochastic (available on request the authors).
 *
 * [2] Zhaofei Yu, David Kappel, Robert Legenstein, Sen Song, Feng Chen and
 * Wolfgang Maass. CaMKII activation supports reward-based neural network
 * optimization through Hamiltonian sampling. 2016.
 * https://arxiv.org/abs/1606.00157
 *
 * @author David Kappel, Michael Hsieh
 *
 * @see TracingNode, DiligentConnectorModel
 */
template<typename targetidentifierT>
class SynapticSamplingRewardGradientConnection : public nest::Connection<targetidentifierT>
{
public:

    SynapticSamplingRewardGradientConnection();
    SynapticSamplingRewardGradientConnection(const SynapticSamplingRewardGradientConnection<targetidentifierT>& rhs);
    ~SynapticSamplingRewardGradientConnection();

    //! Type to use for representing common synapse properties
    typedef SynapticSamplingRewardGradientCommonProperties CommonPropertiesType;

    //! Shortcut for base class
    typedef nest::Connection<targetidentifierT> ConnectionBase;

    class ConnTestDummyNode : public nest::ConnTestDummyNodeBase
    {
    public:
        using nest::ConnTestDummyNodeBase::handles_test_event;

        nest::port handles_test_event(nest::SpikeEvent&, nest::rport)
        {
            return nest::invalid_port_;
        }

        nest::port handles_test_event(nest::DSSpikeEvent&, nest::rport)
        {
            return nest::invalid_port_;
        }
    };

    /**
     * Checks if the type of the postsynaptic node is supported. Throws an
     * IllegalConnection exception if the postsynaptic node is not derived
     * from TracingNode.
     */
    void check_connection(nest::Node & s, nest::Node & t,
                          nest::rport receptor_type, double t_lastspike, const CommonPropertiesType &cp)
    {
        if (!dynamic_cast<TracingNode*> (&t))
        {
            throw nest::IllegalConnection("This synapse only works with nodes exposing their firing"
                                          " probability trace (i.e. TracingNode-Subclass)!");
        }

        ConnTestDummyNode dummy_target;
        ConnectionBase::check_connection_(dummy_target, s, t, receptor_type);
    }

    void get_status(DictionaryDatum & d) const;
    void set_status(const DictionaryDatum & d, nest::ConnectorModel &cm);

    void send(nest::Event& e, nest::thread t, double t_lastspike, const CommonPropertiesType &cp);

    using ConnectionBase::get_delay_steps;
    using ConnectionBase::get_delay;
    using ConnectionBase::get_rport;
    using ConnectionBase::get_target;

    /**
     * Sets the synaptic weight to the given value.
     *
     * @note This value will be overwritten at the next time when the synapse
     * is updated. Set the synaptic_parameter instead for permanent weight
     * changes.
     */
    void set_weight(double w)
    {
        synaptic_parameter_ = w;
    }

    /**
     * Access function to the current value of the eligibility trace.
     */
    double get_eligibility_trace() const
    {
        return stdp_eligibility_trace_;
    }

    /**
     * Access function to the current value of the postsynaptic potential.
     */
    double get_psp() const
    {
        return psp_facilitation_ - psp_depression_;
    }

    /**
     * Access function to the current value of the synaptic weight.
     */
    double get_weight() const
    {
        return weight_;
    }

    /**
     * Access function to the current value of the synaptic parameter.
     */
    double get_synaptic_parameter() const
    {
        return synaptic_parameter_;
    }

    /**
     * Access function to the current value of the reward gradient.
     */
    double get_reward_gradient() const
    {
        return reward_gradient_;
    }

    static ConnectionDataLogger<SynapticSamplingRewardGradientConnection> *logger();

private:

    double weight_;
    double synaptic_parameter_;

    double psp_facilitation_;
    double psp_depression_;

    double stdp_eligibility_trace_;
    double reward_gradient_;

    double prior_mean_;
    double prior_std_;

    nest::index recorder_port_;

    static ConnectionDataLogger<SynapticSamplingRewardGradientConnection> *logger_;

    void update_synapse_state(double t_to,
                              double t_last_update,
                              TracingNode::const_iterator &bap_trace,
                              TracingNode::const_iterator &dopa_trace,
                              const CommonPropertiesType& cp);

    void update_synapic_weight(double time,
                               nest::thread thread,
                               TracingNode *target,
                               const CommonPropertiesType& cp);

    /**
     * Recompute synaptic weight for current synapse state.
     *
     * @param target node of the target neuron.
     * @param cp common properties.
     */
    inline
    void recompute_synapic_weight(TracingNode *target,
                                  const CommonPropertiesType& cp)
    {
        if (synaptic_parameter_ > 0.0)
        {
            weight_ = std::exp(synaptic_parameter_ - cp.parameter_mapping_offset_);
        }
        else if (weight_ > 0.0)
        {
            weight_ = 0.0;
        }
    };
};


//
// Implementation of SynapticSamplingRewardGradientConnection
//

//
// Object life cycle
//

/**
 * Default Constructor.
 */
template <typename targetidentifierT>
SynapticSamplingRewardGradientConnection<targetidentifierT>::SynapticSamplingRewardGradientConnection()
: ConnectionBase(),
weight_(0.0),
synaptic_parameter_(0.0),
psp_facilitation_(0.0),
psp_depression_(0.0),
stdp_eligibility_trace_(0.0),
reward_gradient_(0.0),
prior_mean_(0.0),
prior_std_(1.0),
recorder_port_(nest::invalid_index)
{
}

/**
 * Copy Constructor.
 */
template <typename targetidentifierT>
SynapticSamplingRewardGradientConnection<targetidentifierT>::
SynapticSamplingRewardGradientConnection(const SynapticSamplingRewardGradientConnection& rhs)
: ConnectionBase(rhs)
{
    weight_ = rhs.weight_;
    synaptic_parameter_ = rhs.synaptic_parameter_;
    psp_facilitation_ = rhs.psp_facilitation_;
    psp_depression_ = rhs.psp_depression_;
    stdp_eligibility_trace_ = rhs.stdp_eligibility_trace_;
    reward_gradient_ = rhs.reward_gradient_;
    prior_mean_ = rhs.prior_mean_;
    prior_std_ = rhs.prior_std_;

    // recorder must be set up directly using the set_status method.
    recorder_port_ = nest::invalid_index;
}

/**
 * Destructor.
 */
template <typename targetidentifierT>
SynapticSamplingRewardGradientConnection<targetidentifierT>::~SynapticSamplingRewardGradientConnection()
{
}

//
// Instance of global data logger singleton.
//

/**
 * Pointer to the global instance of the data logger.
 */
template <typename targetidentifierT>
ConnectionDataLogger< SynapticSamplingRewardGradientConnection<targetidentifierT> >
*SynapticSamplingRewardGradientConnection<targetidentifierT>::logger_ = 0;

/**
 * Get the data logger singleton.
 *
 * @return the instance of the data logger.
 */
template <typename targetidentifierT>
ConnectionDataLogger< SynapticSamplingRewardGradientConnection<targetidentifierT> >
*SynapticSamplingRewardGradientConnection<targetidentifierT>::logger()
{
    if (!logger_)
    {
        logger_ = new ConnectionDataLogger<SynapticSamplingRewardGradientConnection>();

        logger_->register_recordable_variable("eligibility_trace",
                                              &SynapticSamplingRewardGradientConnection::get_eligibility_trace);
        logger_->register_recordable_variable("psp",
                                              &SynapticSamplingRewardGradientConnection::get_psp);
        logger_->register_recordable_variable("weight",
                                              &SynapticSamplingRewardGradientConnection::get_weight);
        logger_->register_recordable_variable("synaptic_parameter",
                                              &SynapticSamplingRewardGradientConnection::get_synaptic_parameter);
        logger_->register_recordable_variable("reward_gradient",
                                              &SynapticSamplingRewardGradientConnection::get_reward_gradient);
    }

    return logger_;
}

//
// Parameter and state extractions and manipulation functions
//

/**
 * Status getter function.
 */
template <typename targetidentifierT>
void SynapticSamplingRewardGradientConnection<targetidentifierT>::get_status(DictionaryDatum & d) const
{
    ConnectionBase::get_status(d);
    def<double>(d, nest::names::weight, weight_);
    def<double>(d, "synaptic_parameter", synaptic_parameter_);
    def<double>(d, "prior_mean", prior_mean_);
    def<double>(d, "prior_std", prior_std_);

    def<long>(d, nest::names::size_of, sizeof (*this));

    logger()->get_status(d, recorder_port_);
}

/**
 * Status setter function.
 */
template <typename targetidentifierT>
void SynapticSamplingRewardGradientConnection<targetidentifierT>::set_status(const DictionaryDatum & d,
                                                                             nest::ConnectorModel &cm)
{
    ConnectionBase::set_status(d, cm);
    updateValue<double>(d, "synaptic_parameter", synaptic_parameter_);
    updateValue<double>(d, "prior_mean", prior_mean_);
    updateValue<double>(d, "prior_std", prior_std_);

    logger()->set_status(d, recorder_port_);
}

//
// Synapse event handling
//

/**
 * Send an event to the postsynaptic neuron. This will update the synapse state
 * and synaptic weights to the current slice origin and send the spike event.
 * This method is also triggered by the ConnectionUpdateManager to indicate
 * that the synapse is running out of data. In this case an invalid rport of -1
 * is passed and the spike is not delivered to the postsynaptic neuron.
 *
 * @param e the spike event.
 * @param thread the id of the connections thread.
 * @param t_last_spike the time of the last spike.
 * @param cp the synapse type common properties.
 */
template <typename targetidentifierT>
void SynapticSamplingRewardGradientConnection<targetidentifierT>::send(nest::Event& e,
                                                                       nest::thread thread,
                                                                       double t_last_spike,
                                                                       const CommonPropertiesType &cp)
{
    assert(cp.resolution_unit_ > 0.0);

    const double t_to = e.get_stamp().get_ms();
    double t_from = t_last_spike;

    if (t_to > t_last_spike)
    {
        // prepare the pointer to the target neuron. We can safely static_cast
        // since the connection is checked when established.
        TracingNode* target = static_cast<TracingNode*> (get_target(thread));

        TracingNode::const_iterator bap_trace =
                target->get_trace(nest::delay(t_from / cp.resolution_unit_), cp.bap_trace_id_ + get_rport());

        TracingNode::const_iterator dopa_trace =
                cp.reward_transmitter_->get_trace(nest::delay(t_from / cp.resolution_unit_), cp.dopa_trace_id_);

        const double t_last_weight_update = std::floor(t_last_spike / cp.weight_update_time_) * cp.weight_update_time_;

        for (double next_weight_time = t_last_weight_update + cp.weight_update_time_;
                next_weight_time <= t_to;
                next_weight_time += cp.weight_update_time_)
        {
            update_synapse_state(next_weight_time, t_from, bap_trace, dopa_trace, cp);
            update_synapic_weight(next_weight_time, thread, target, cp);
            t_from = next_weight_time;
        }

        if (t_to > t_from)
        {
            update_synapse_state(t_to, t_from, bap_trace, dopa_trace, cp);
        }
    }

    // Make sure that the event is not a SynapseUpdateEvent.
    if (e.get_rport() >= 0)
    {
        // Apply presynaptic spike
        psp_facilitation_ += 1.0;
        psp_depression_ += 1.0;

        if (weight_ > 0.0)
        {
            e.set_weight(weight_);

            e.set_delay(get_delay_steps());
            e.set_receiver(*get_target(thread));
            e.set_rport(get_rport());
            e();
        }
    }
}

/**
 * Updates the state of the synapse to the given time point. This method
 * expects the back propagating action potential BAP trace of the postsynaptic
 * neuron and the reward (dopamine) trace of the reward transmitter to be
 * passed. Iterators are expected to be positioned at time t_last_update and
 * will be advanced to t_to after the call.
 *
 * @param t_to time to advance to.
 * @param t_last_update time of last update.
 * @param bap_trace iterator pointing to the current value of the BAP trace.
 * @param dopa_trace iterator pointing to the current value of the dopamine trace.
 * @param cp synapse type common properties.
 */
template <typename targetidentifierT>
void SynapticSamplingRewardGradientConnection<targetidentifierT>::update_synapse_state(double t_to,
                                                                                       double t_last_update,
                                                                                       TracingNode::const_iterator
                                                                                       &bap_trace,
                                                                                       TracingNode::const_iterator
                                                                                       &dopa_trace,
                                                                                       const CommonPropertiesType& cp)
{
    if ((weight_ == 0.0) && not cp.simulate_retracted_synapses_)
    {
        // synapse is retracted. psps and eligibility traces are not going to be simulated.
        return;
    }

    t_to -= cp.resolution_unit_ / 2.0; // exclude the last time step.

    for (double time = t_last_update; time < t_to; time += cp.resolution_unit_)
    {
        // This loop will - considering every call - iterate through EVERY time step (in steps of resolution)

        // decay eligibility trace
        stdp_eligibility_trace_ *= cp.lambda_;

        // decay gradient variable
        reward_gradient_ *= cp.gamma_;

        // update postsynaptic spike potential
        if (psp_facilitation_ != 0.0)
        {
            psp_facilitation_ *= cp.psp_faciliation_update_;
            psp_depression_ *= cp.psp_depression_update_;

            const double postsynaptic_potential = cp.psp_scale_factor_ * (psp_facilitation_ - psp_depression_);
            stdp_eligibility_trace_ += weight_ * postsynaptic_potential * (*bap_trace);

            if (psp_facilitation_ < cp.epsilon_)
            {
                psp_facilitation_ = 0.0;
                psp_depression_ = 0.0;
            }
        }

        reward_gradient_ += (*dopa_trace) * stdp_eligibility_trace_;

        if (cp.direct_gradient_rate_ > 0.0)
        {
            synaptic_parameter_ += (*dopa_trace) * cp.learning_rate_ *
                    cp.direct_gradient_rate_ * stdp_eligibility_trace_;
        }

        ++bap_trace;
        ++dopa_trace;
    }
}

/**
 * Updates the synaptic parameter and weight of the synapse.
 *
 * @param time the time when the synapse is updated.
 * @param thread the thread of the synapse.
 * @param target the target node of the synapse.
 * @param cp the synapse type common properties.
 */
template < typename targetidentifierT >
void SynapticSamplingRewardGradientConnection< targetidentifierT >::
update_synapic_weight(double time, nest::thread thread, TracingNode *target, const CommonPropertiesType& cp)
{
    // update synaptic parameters
    const double l_rate = cp.weight_update_time_ * cp.learning_rate_;

    // compute prior
    const double prior = (1.0 / pow(prior_std_, 2)) * (prior_mean_ - synaptic_parameter_);

    const bool synapse_is_active = (weight_ != 0.0);

    reward_gradient_ += cp.get_gradient_noise(thread);

    const double d_lik = std::max(-cp.max_param_change_,
                                  std::min(cp.max_param_change_, cp.gradient_scale_ * reward_gradient_));

    const double d_param = l_rate * (prior + d_lik) + cp.get_d_wiener(thread);

    synaptic_parameter_ = std::max(cp.min_param_, std::min(cp.max_param_, synaptic_parameter_ + d_param));

    // update synaptic weight
    recompute_synapic_weight(target, cp);

    if (synapse_is_active && not cp.simulate_retracted_synapses_ && (weight_ == 0.0))
    {
        // synapse is entering the retracted state, eligibility trace is reset.
        psp_facilitation_ = 0.0;
        psp_depression_ = 0.0;
        stdp_eligibility_trace_ = 0.0;
        reward_gradient_ = 0.0;
    }

    // update synapse recorder
    logger()->record(time, *this, recorder_port_);
}

}

#endif
