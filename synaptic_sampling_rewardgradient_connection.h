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


namespace spore
{

/**
 * Class containing the common properties for all synapses of type
 * SynapticSamplingRewardGradientConnection.
 */
class SynapticSamplingRewardGradientCommonProperties : public nest::CommonSynapseProperties
{
public:

    /**
     * Default constructor.
     */
    SynapticSamplingRewardGradientCommonProperties()
    : nest::CommonSynapseProperties(),
      reward_transmitter_(0),
      learning_rate_(0.0001),
      episode_length_(100.0),
      psp_facilitation_rate_(20.0),
      psp_depression_rate_(2.0),
      temperature_(0.01),
      gradient_noise_(0.00),
      max_param_(100.0),
      min_param_(-100.0),
      max_param_change_(100.0),
      integration_time_(10000.0),
      direct_gradient_rate_(0.0),
      parameter_mapping_offset_(3.0),
      weight_update_time_(100.0),
      consolidation_time_(-1.0),
      consolidation_interval_(-1.0),
      consolidation_threshold_(1.0),
      consolidation_probability_(1.0),
      consolidation_prior_std_(1.0),
      gradient_scale_(1.0),
      resolution_unit_(-1.0),
      gamma_(0.0),
      lambda_(0.0),
      psp_faciliation_update_(0.0),
      psp_depression_update_(0.0),
      psp_scale_factor_(0.0),
      epsilon_(0.0001),
      weight_update_steps_(0),
      bap_trace_id_(0),
      dopa_trace_id_(0),
      verbose_(false),
      std_wiener_(0.0),
      std_gradient_(0.0)
    {}

    /**
     * Destructor.
     */
    ~SynapticSamplingRewardGradientCommonProperties()
    {}
    
    using CommonSynapseProperties::get_status;
    using CommonSynapseProperties::set_status;
    using CommonSynapseProperties::calibrate;

    /**
     * Status getter function.
     */
    void get_status(DictionaryDatum & d) const
    {
        nest::CommonSynapseProperties::get_status(d);

        if (reward_transmitter_ != 0)
            def<long>(d, "reward_transmitter", reward_transmitter_->get_gid());
        else
            def<long>(d, "reward_transmitter", -1);
    }

    /**
     * Status setter function.
     */
    void set_status(const DictionaryDatum & d, nest::ConnectorModel& cm)
    {
        nest::CommonSynapseProperties::set_status(d, cm);

        updateValue<double>(d, "max_param", max_param_);
        updateValue<double>(d, "min_param", min_param_);
        updateValue<double>(d, "max_param_change", max_param_change_);
        updateValue<double>(d, "parameter_mapping_offset", parameter_mapping_offset_);
        updateValue<double>(d, "direct_gradient_rate", direct_gradient_rate_);
        updateValue<double>(d, "consolidation_time", consolidation_time_);
        updateValue<double>(d, "consolidation_interval", consolidation_interval_);
        updateValue<double>(d, "consolidation_threshold", consolidation_threshold_);
        updateValue<double>(d, "consolidation_probability", consolidation_probability_);
        updateValue<double>(d, "consolidation_prior_std", consolidation_prior_std_);
        updateValue<double>(d, "gradient_scale", gradient_scale_);
        updateValue<long>(d, "bap_trace_id", bap_trace_id_);
        updateValue<long>(d, "dopa_trace_id", dopa_trace_id_);
      
        updateValue<double>(d, "learning_rate", learning_rate_);
        updateValue<double>(d, "temperature", temperature_);
        updateValue<double>(d, "gradient_noise", gradient_noise_);
        updateValue<double>(d, "psp_facilitation_rate", psp_facilitation_rate_);
        updateValue<double>(d, "psp_depression_rate", psp_depression_rate_);
        updateValue<double>(d, "integration_time", integration_time_);
        updateValue<double>(d, "episode_length", episode_length_);
        updateValue<long>(d, "weight_update_time", weight_update_time_);
        
        updateValue<bool>(d, "verbose", verbose_);
        
        if (weight_update_time_ <= 0.0)
        {
            throw nest::BadProperty("weight_update_time must strictly bigger than 0");
        }

        long rtgid;
        if (updateValue<long>(d, "reward_transmitter", rtgid))
        {
#if defined(__SPORE_WITH_NEST_2_10__)
            reward_transmitter_ = dynamic_cast<TracingNode*> (nest::NestModule::get_network().get_node(rtgid));
#elif defined(__SPORE_WITH_NEST_2_12__)
            reward_transmitter_ = dynamic_cast<TracingNode*> (nest::kernel().node_manager.get_node(rtgid));
#else
#error NEST version is not supported!
#endif
            if (reward_transmitter_ == 0)
                throw nest::BadProperty("Reward transmitter must be of model RewardTransmitter");
        }
    }
    
    /**
     * Calibrate all time objects, which are contained in this object.
     * This function is called when the time resolution changes and on
     * simulation startup.
     * 
     * @param tc time converter object.
     */
    void calibrate( const nest::TimeConverter &tc )
    {
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

        // initialize lookup tables for faster synapse updates. ( coming soon ;-) )
        //
        // long idx = 0;
        // 
        // const double tau_e_tr = (episode_length_ * integration_time_) / (integration_time_ - episode_length_);
        //
        // psp_facilitation_lookup_.resize(weight_update_steps_);
        // psp_depression_lookup_.resize(weight_update_steps_);
        // gamma_lookup_.resize(weight_update_steps_);
        // lamdba_lookup_.resize(weight_update_steps_);
        // e_tr_decay_lookup_.resize(weight_update_steps_);        
        //
        // for (double dt = resolution_unit_; dt < weight_update_time_; dt += resolution_unit_)
        // {
        //     psp_facilitation_lookup_[idx] = std::exp(-dt / psp_facilitation_rate_);
        //     psp_depression_lookup_[idx] = std::exp(-dt / psp_depression_rate_);
        //     gamma_lookup_[idx] = std::exp(-dt / integration_time_);
        //     lamdba_lookup_[idx] = std::exp(-dt / episode_length_);
        //     e_tr_decay_lookup_[idx] = tau_e_tr * std::expm1(-dt / tau_e_tr);
        //
        //     idx++;
        // }
    }
    

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
#if defined(__SPORE_WITH_NEST_2_10__)
            nest::Network *net = nest::Node::network();
            result = std_wiener_ * normal_dev_(net->get_rng(thread));
#elif defined(__SPORE_WITH_NEST_2_12__)
            result = std_wiener_ * normal_dev_(nest::kernel().rng_manager.get_rng(thread));
#else
#error NEST version is not supported!
#endif
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
#if defined(__SPORE_WITH_NEST_2_10__)
            nest::Network *net = nest::Node::network();
            result = std_gradient_ * normal_dev_(net->get_rng(thread));
#elif defined(__SPORE_WITH_NEST_2_12__)
            result = std_gradient_ * normal_dev_(nest::kernel().rng_manager.get_rng(thread));
#else
#error NEST version is not supported!
#endif
        }
        return result;
    }
    
   /**
     * Convenience function to random number.
     */
    double drand(nest::thread thread) const
    {
#if defined(__SPORE_WITH_NEST_2_10__)
        nest::Network *net = nest::Node::network();
        return net->get_rng(thread)->drand();
#elif defined(__SPORE_WITH_NEST_2_12__)
        return nest::kernel().rng_manager.get_rng(thread)->drand();
#else
#error NEST version is not supported!
#endif
    }

    TracingNode* reward_transmitter_;

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
    double consolidation_time_;
    double consolidation_interval_;
    double consolidation_threshold_;
    double consolidation_probability_;
    double consolidation_prior_std_;
    double gradient_scale_;

    double resolution_unit_;
    double gamma_;
    double lambda_;

    double psp_faciliation_update_;
    double psp_depression_update_;
    double psp_scale_factor_;

    double epsilon_;

    long weight_update_steps_;
    int bap_trace_id_;
    int dopa_trace_id_;
    
    bool verbose_;

    // lookup tables for faster synapse updates. ( coming soon ;-) ) 
    //std::vector<double> psp_facilitation_lookup_;
    //std::vector<double> psp_depression_lookup_;
    //std::vector<double> gamma_lookup_;
    //std::vector<double> lamdba_lookup_;
    //std::vector<double> e_tr_decay_lookup_;

private:
    double std_wiener_;
    double std_gradient_;
    librandom::NormalRandomDev normal_dev_;
};


/**
 * @brief Reward-based synaptic sampling connection class
 * 
 * SynapticSamplingRewardgradientConnection - Synapse implementing reward-based
 * learning through synaptic sampling. This class implements the model in [1].
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
 * <tr><td>parameter_mapping_offset</td>    <td>double</td> <td>offset parameter for computing synaptic weight</td></tr> 
 * <tr><td>max_param</td>                   <td>double</td> <td>maximum synaptic parameter</td></tr> 
 * <tr><td>min_param</td>                   <td>double</td> <td>minimum synaptic parameter</td></tr> 
 * <tr><td>max_param_change</td>            <td>double</td> <td>maximum synaptic parameter change</td></tr> 
 * <tr><td>direct_gradient_rate</td>        <td>double</td> <td>rate of directly applying changes to tde synaptic parameter</td></tr> 
 * <tr><td>consolidation_time</td>          <td>double</td> <td>time point when consolidation is triggered</td></tr> 
 * <tr><td>consolidation_interval</td>      <td>double</td> <td>interval after which synapses become consolidated</td></tr> 
 * <tr><td>consolidation_tdreshold</td>     <td>double</td> <td>tdreshold of consolidation</td></tr> 
 * <tr><td>consolidation_probability</td>   <td>double</td> <td>probability of consolidation</td></tr> 
 * <tr><td>consolidation_prior_std</td>     <td>double</td> <td>prior STD of consolidated synapses</td></tr> 
 * <tr><td>gradient_scale</td>              <td>double</td> <td>scaling parameter for tde gradient</td></tr> 
 * <tr><td>reward_transmitter</td>          <td>int</td>    <td>GID of tde synapse's reward transmitter</td></tr> 
 * <tr><td>bap_trace_id</td>                <td>int</td>    <td>ID of tde BAP trace (default 0)</td></tr> 
 * <tr><td>dopa_trace_id</td>               <td>int</td>    <td>ID of tde dopamine trace (default 0)</td></tr> 
 * <tr><td>verbose</td>                     <td>bool</td>   <td>write status to std::out (for debugging)</td></tr> 
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
 * Wolfgang Maass. <i>Reward-based self-configuration of neural circuits.</i> 2016.
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
            throw nest::IllegalConnection("This synapse only works with nodes exposing their firing probability trace (e.g. TracingNode-Subclass)!");
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
    double t_weight_;
    double synaptic_parameter_;    

    double psp_facilitation_;
    double psp_depression_;

    double stdp_eligibility_trace_;
    double reward_gradient_;

    double prior_mean_;
    double prior_std_;

    double t_consolidate_;
    
    nest::index recorder_port_;
    
    static ConnectionDataLogger<SynapticSamplingRewardGradientConnection> *logger_;
    

    void update_synapse_state( double t_to,
                               double t_last_update,
                               TracingNode::const_iterator &bap_trace,
                               TracingNode::const_iterator &dopa_trace,
                               const CommonPropertiesType& cp );
    
    void update_synapic_weight( double time,
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
    void recompute_synapic_weight( TracingNode *target,
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
  t_weight_(0.0),
  synaptic_parameter_(0.0),
  psp_facilitation_(0.0),
  psp_depression_(0.0),
  stdp_eligibility_trace_(0.0),
  reward_gradient_(0.0),
  prior_mean_(0.0),
  prior_std_(1.0),
  t_consolidate_(0.0),
  recorder_port_(nest::invalid_index)
{}


/**
 * Copy Constructor.
 */
template <typename targetidentifierT>
SynapticSamplingRewardGradientConnection<targetidentifierT>::SynapticSamplingRewardGradientConnection(const SynapticSamplingRewardGradientConnection& rhs) :
ConnectionBase(rhs)
{
    weight_ = rhs.weight_;
    t_weight_ = rhs.t_weight_;
    synaptic_parameter_ = rhs.synaptic_parameter_;
    psp_facilitation_ = rhs.psp_facilitation_;
    psp_depression_ = rhs.psp_depression_;
    stdp_eligibility_trace_ = rhs.stdp_eligibility_trace_;
    reward_gradient_ = rhs.reward_gradient_;
    prior_mean_ = rhs.prior_mean_;
    prior_std_ = rhs.prior_std_;
    t_consolidate_ = rhs.t_consolidate_;
    
    // recorder must be set up directly using the set_status method.
    recorder_port_ = nest::invalid_index;
}


/**
 * Destructor.
 */
template <typename targetidentifierT>
SynapticSamplingRewardGradientConnection<targetidentifierT>::~SynapticSamplingRewardGradientConnection()
{}

//
// Instance of global data logger singleton.
//

/**
 * Pointer to the global instance of the data logger.
 */
template <typename targetidentifierT>
ConnectionDataLogger< SynapticSamplingRewardGradientConnection<targetidentifierT> >
    *SynapticSamplingRewardGradientConnection<targetidentifierT>::logger_=0;


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
        logger_->register_recordable_variable("eligibility_trace", &SynapticSamplingRewardGradientConnection::get_eligibility_trace);
        logger_->register_recordable_variable("psp", &SynapticSamplingRewardGradientConnection::get_psp);
        logger_->register_recordable_variable("weight", &SynapticSamplingRewardGradientConnection::get_weight);
        logger_->register_recordable_variable("synaptic_parameter", &SynapticSamplingRewardGradientConnection::get_synaptic_parameter);
        logger_->register_recordable_variable("reward_gradient", &SynapticSamplingRewardGradientConnection::get_reward_gradient);
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

    const bool is_consolidated = (t_consolidate_ < 0.0);
    def<bool>(d, "is_consolidated", is_consolidated);

    def<long>(d, nest::names::size_of, sizeof (*this));

    logger()->get_status(d, recorder_port_);
}


/**
 * Status setter function.
 */
template <typename targetidentifierT>
void SynapticSamplingRewardGradientConnection<targetidentifierT>::set_status(const DictionaryDatum & d, nest::ConnectorModel &cm)
{
    ConnectionBase::set_status(d, cm);
    updateValue<double>(d, "synaptic_parameter", synaptic_parameter_);
    updateValue<double>(d, "prior_mean", prior_mean_);
    updateValue<double>(d, "prior_std", prior_std_);

    logger()->set_status(d, recorder_port_);

    bool clear_consolidation = false;
    updateValue<bool>(d, "clear_consolidation", clear_consolidation);

    if (clear_consolidation)
    {
        t_consolidate_ = 0.0;
    }

    bool consolidate = false;
    updateValue<bool>(d, "consolidate_now", consolidate);

    if (consolidate)
    {
        prior_mean_ = synaptic_parameter_;
    }
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
void SynapticSamplingRewardGradientConnection<targetidentifierT>::send( nest::Event& e,
                                                                        nest::thread thread,
                                                                        double t_last_spike,
                                                                        const CommonPropertiesType &cp )
{
    assert(cp.resolution_unit_>0.0);

    const double t_to = e.get_stamp().get_ms(); // ConnectionUpdateManager::instance()->get_origin().get_ms();
    double t_from = t_last_spike;

    if (t_to>t_last_spike)
    {
        // prepare the pointer to the target neuron. We can safely static_cast
        // since the connection is checked when established.
        TracingNode* target = static_cast<TracingNode*> (get_target(thread));

        TracingNode::const_iterator bap_trace = target->get_trace( nest::delay(t_from/cp.resolution_unit_), cp.bap_trace_id_ + get_rport() );
        TracingNode::const_iterator dopa_trace = cp.reward_transmitter_->get_trace( nest::delay(t_from/cp.resolution_unit_), cp.dopa_trace_id_ );
    
        for ( double next_weight_time = t_weight_ + cp.weight_update_time_;
              next_weight_time <= t_to;
              next_weight_time += cp.weight_update_time_ )
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
void SynapticSamplingRewardGradientConnection<targetidentifierT>::update_synapse_state( double t_to,
                                                                                        double t_last_update,
                                                                                        TracingNode::const_iterator &bap_trace,
                                                                                        TracingNode::const_iterator &dopa_trace,
                                                                                        const CommonPropertiesType& cp )
{
    t_to -= cp.resolution_unit_/2.0; // exclude the last time step.

    for (double time = t_last_update; time < t_to; time += cp.resolution_unit_)
    {
        // This loop will - considering every call - iterate through EVERY time step (in steps of resolution)

        // decay eligibility trace
        stdp_eligibility_trace_ *= cp.lambda_;

        // decay gradient variable
        reward_gradient_ *= cp.gamma_;

        // update postsynaptic spike potential
        if (psp_facilitation_ > 0.0)
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
            synaptic_parameter_ += (*dopa_trace) * cp.learning_rate_ * cp.direct_gradient_rate_*stdp_eligibility_trace_;
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
template <typename targetidentifierT>
void SynapticSamplingRewardGradientConnection<targetidentifierT>::update_synapic_weight(double time,
                                                                                        nest::thread thread,
                                                                                        TracingNode *target,
                                                                                        const CommonPropertiesType& cp)
{
    // update synaptic parameters
    const double l_rate = cp.weight_update_time_ * cp.learning_rate_;

    if ((time >= cp.consolidation_time_) && (t_weight_ < cp.consolidation_time_))
    {
        if (cp.drand(thread) <= cp.consolidation_probability_)
        {
            prior_mean_ = synaptic_parameter_;
            prior_std_ = cp.consolidation_prior_std_;
            t_consolidate_ = -1.0;

            if (cp.verbose_)
            {
                std::cout << "time: " << time
                        << ", consolidating synapse to parameter value :" << synaptic_parameter_ << std::endl;
            }
        }
        else if (cp.verbose_)
        {
            std::cout << "time: " << time << ", consolidation skipped." << std::endl;
        }
    }

    if ((cp.consolidation_interval_ > 0.0) && (t_consolidate_ >= 0.0))
    {
        if (synaptic_parameter_ > cp.consolidation_threshold_)
        {
            if (t_consolidate_ > cp.consolidation_interval_)
            {
                prior_mean_ = synaptic_parameter_;
                prior_std_ = cp.consolidation_prior_std_;
                t_consolidate_ = -1.0;
            }
            else
            {
                t_consolidate_ += cp.weight_update_time_;
            }
        }
        else
        {
            t_consolidate_ = 0.0;
        }
    }

    // compute prior
    double prior = (1.0 / pow(prior_std_, 2)) * (prior_mean_ - synaptic_parameter_);

    reward_gradient_ += cp.get_gradient_noise(thread);

    //const double d_param = l_rate * (prior + cp.gradient_scale_*reward_gradient_) + cp.get_d_wiener(thread);

    const double d_lik = std::max(-cp.max_param_change_, std::min(cp.max_param_change_, cp.gradient_scale_ * reward_gradient_));

    const double d_param = l_rate * (prior + d_lik) + cp.get_d_wiener(thread);

    synaptic_parameter_ = std::max(cp.min_param_, std::min(cp.max_param_, synaptic_parameter_ + d_param));

    // update synaptic weight
    recompute_synapic_weight(target, cp);

    // update synapse recorder
    logger()->record(time, *this, recorder_port_);

    t_weight_ = time;
}

} // namespace

#endif
