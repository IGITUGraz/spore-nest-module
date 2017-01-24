/*
 * File: poisson_dbl_exp_neuron.h
 * Author: Hsieh, Kappel
 * 
 * This file is mainly based on pp_psc_delta.h which is part of NEST.
 */

#ifndef POISSON_DBL_EXP_NEURON_H
#define POISSON_DBL_EXP_NEURON_H

#include "nest.h"
#include "event.h"
#include "tracing_node.h"
#include "ring_buffer.h"
#include "connection.h"
#include "poisson_randomdev.h"
#include "gamma_randomdev.h"
#include "universal_data_logger.h"
#include "archiving_node.h"


namespace spore
{

/* BeginDocumentation
 * Name: PoissonDblExpNeuron - Point process neuron with double-exponential
 * PSPs.
 *
 * Description:
 *
 * PoissonDblExpNeuron is a variant of the spike response model model with
 * double exponential PSP shapes an absolute refractory mechanism.
 *
 * Spikes are generated randomly according to the current value of the
 * transfer function which operates on the membrane potential. Spike
 * generation is followed by an optional dead time (refractory state).
 * Setting with_reset to true will reset the membrane potential after
 * each spike.
 *
 * The transfer function can be chosen to be linear, exponential or a sum of 
 * both by adjusting three parameters:
 *
 *     rate = Rect[ c1 * V' + c2 * exp(c3 * V') ],
 *
 * where the effective potential V' = V_m - E_sfa and E_sfa is called 
 * the adaptive threshold.
 *
 * By setting c3 = 0, c2 can be used as an offset spike rate for an otherwise
 * linear rate model.
 *
 * The dead time enables to include refractoriness. If dead time is 0, the
 * number of spikes in one time step might exceed one and is drawn from the
 * Poisson distribution accordingly. Otherwise, the probability for a spike
 * is given by 1 - exp(-rate*h), where h is the simulation time step. If dead_time
 * is smaller than the simulation resolution (time step), it is internally 
 * set to the time step.
 *
 * Note that, even if non-refractory neurons are to be modeled, a small value 
 * of dead_time, like dead_time=1e-8, might be the value of choice since it 
 * uses faster uniform random numbers than dead_time=0, which draws Poisson 
 * numbers. Only for very large spike rates (> 1 spike/h) this will cause errors.
 *
 * The model implements an adaptive threshold. If the neuron spikes, the 
 * threshold increases and the membrane potential will take longer to reach it.
 * If the neuron does not spike, the threshold linearly decays over time,
 * increasing the firing probability of the neuron (parameters: target_rate,
 * target_adaptation_speed).
 *   
 * This model has been adapted from iaf_psc_delta. The default parameters are
 * set to the mean values in [2], which have were matched to spike-train 
 * recordings.
 *   
 * References:
 *
 * [1] Multiplicatively interacting point processes and applications to neural 
 * modeling (2010) Stefano Cardanobile and Stefan Rotter, Journal of 
 * Computational Neuroscience.
 *   
 * [2] Predicting spike timing of neocortical pyramidal neurons by simple
 * threshold models (2006) Jolivet R, Rauch A, Luescher H-R, Gerstner W. 
 * J Comput Neurosci 21:35-49.
 *
 * [3] Pozzorini C, Naud R, Mensi S, Gerstner W (2013) Temporal whitening by 
 * power-law adaptation in neocortical neurons. Nat Neurosci 16: 942-948.
 * (uses a similar model of multi-timescale adaptation).
 *
 * [4] Grytskyy D, Tetzlaff T, Diesmann M and Helias M (2013) A unified view 
 * on weakly correlated recurrent networks. Front. Comput. Neurosci. 7:131. 
 *   
 * [5] Deger M, Schwalger T, Naud R, Gerstner W (2014) Fluctuations and 
 * information filtering in coupled populations of spiking neurons with
 * adaptation. Physical Review E 90:6, 062704. 
 *
 *
 * Parameters:
 *
 * The following parameters can be set in the status dictionary.
 *
 * V_m                      double - Membrane potential in mV.
 * tau_rise_exc             double - Fall time constant of excitatory PSP.
 * tau_fall_exc             double - Rise time constant of excitatory PSP.
 * tau_rise_inh             double - Fall time constant of inhibitory PSP.
 * tau_fall_inh             double - Rise time constant of inhibitory PSP.
 * q_sfa                    double - Adaptive threshold jump in mV.
 * dead_time                double - Duration of the dead time in ms.
 * dead_time_random         bool   - Should a random dead time be drawn after each spike?
 * dead_time_shape          int    - Shape parameter of dead time gamma distribution.
 * t_ref_remaining          double   Remaining dead time at simulation start.
 * with_reset               bool     Should the membrane potential be reset after a spike?
 * I_e                      double - Constant input current [pA].
 * input_conductance        double - Conductance of input currents [S]. 
 * c_1                      double - Slope of linear part of transfer function in Hz/mV.
 * c_2                      double - Prefactor of exponential part of transfer function in Hz.
 * c_3                      double - Coefficient of exponential non-linearity of transfer function in 1/mV.
 * target_rate              double - Target rate of neuron for adaptation mechanism [Hz].
 * target_adaptation_speed  double - Speed of rate adaptation.
 * 
 * Sends: SpikeEvent
 *
 * Receives: SpikeEvent, CurrentEvent, DataLoggingRequest
 *
 * Author:  Kappel, Hsieh; (of pp_psc_delta) July 2009, Deger, Helias; January 2011, Zaytsev; May 2014, Setareh
 * SeeAlso: TracingNode, pp_psc_delta
 */

/**
 * Point process neuron with leaky integration of delta-shaped PSCs.
 */
class PoissonDblExpNeuron : public TracingNode
{
public:

    PoissonDblExpNeuron();
    PoissonDblExpNeuron(const PoissonDblExpNeuron&);

    /**
     * Import sets of overloaded virtual functions.
     * @see Technical Issues / Virtual Functions: Overriding, Overloading, and Hiding
     */
    using nest::Node::handle;
    using nest::Node::handles_test_event;

    nest::port send_test_event(nest::Node&, nest::rport, nest::synindex, bool);

    void handle(nest::SpikeEvent &);
    void handle(nest::CurrentEvent &);
    void handle(nest::DataLoggingRequest &);

    nest::port handles_test_event(nest::SpikeEvent&, nest::rport);
    nest::port handles_test_event(nest::CurrentEvent&, nest::rport);
    nest::port handles_test_event(nest::DataLoggingRequest&, nest::rport);

    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &);

private:

    void init_state_(const nest::Node& proto);
    void init_buffers_();
    void calibrate();

    void update(nest::Time const &, const nest::long_t, const nest::long_t);

    // The next two classes need to be friends to access the State_ class/member
    friend class nest::RecordablesMap<PoissonDblExpNeuron>;
    friend class nest::UniversalDataLogger<PoissonDblExpNeuron>;

    // ----------------------------------------------------------------

    /**
     * Independent parameters of the model.
     */
    struct Parameters_
    {
        /** Rise time constant of excitatory PSP. */
        double tau_rise_exc_;

        /** Fall time constant of excitatory PSP. */
        double tau_fall_exc_;

        /** Rise time constant of inhibitory PSP. */
        double tau_rise_inh_;

        /** Fall time constant of inhibitory PSP. */
        double tau_fall_inh_;

        /** Conductance for piecewise constant input currents. */
        double input_conductance_;

        /** Dead time in ms. */
        double dead_time_;

        /** Do we use random dead time? */
        bool dead_time_random_;

        /** Shape parameter of random dead time gamma distribution. */
        unsigned long dead_time_shape_;

        /** Do we reset the membrane potential after each spike? */
        bool with_reset_;

        /** Slope of the linear part of transfer function. */
        double c_1_;

        /** Prefactor of exponential part of transfer function. */
        double c_2_;

        /** Coefficient of exponential non-linearity of transfer function. */
        double c_3_;

        /** External DC current. */
        double I_e_;

        /** Dead time from simulation start. */
        double t_ref_remaining_;

        /** Target average output rate for homeostatic adaptation. */
        double target_rate_;

        /** Rate with which the homeostatic adaptation current is updated. */
        double target_adaptation_speed_;

        Parameters_(); //!< Sets default parameter values

        void get(DictionaryDatum&) const; //!< Store current values in dictionary
        void set(const DictionaryDatum&); //!< Set values from dictionary
    };

    /**
     * State variables of the model.
     */
    struct State_
    {
        double u_rise_exc_; //!< Rising part of excitatory potential
        double u_fall_exc_; //!< Falling part of excitatory potential
        double u_rise_inh_; //!< Rising part of inhibitory potential
        double u_fall_inh_; //!< Falling part of inhibitory potential
        double u_membrane_; //!< The membrane potential
        double input_current_; //!< The piecewise linear input currents
        double adaptative_threshold_; //!< adaptive threshold to maintain average output rate
        long lost_p_values_; //!< Counter of lost spike prob values
        int r_; //!< Number of refractory steps remaining

        State_(); //!< Default initialization

        void get(DictionaryDatum&, const Parameters_&) const;
        void set(const DictionaryDatum&, const Parameters_&);
    };

    /**
     * Buffers of the model.
     */
    struct Buffers_
    {
        Buffers_(PoissonDblExpNeuron &);
        Buffers_(const Buffers_ &, PoissonDblExpNeuron &);

        /** buffers and sums up incoming spikes/currents */
        nest::RingBuffer exc_spikes_;
        nest::RingBuffer inh_spikes_;
        nest::RingBuffer currents_;

        //! Logger for all analog data
        nest::UniversalDataLogger<PoissonDblExpNeuron> logger_;
    };

    /**
     * Internal variables of the model.
     */
    struct Variables_
    {
        double decay_rise_exc_;
        double decay_fall_exc_;
        double decay_rise_inh_;
        double decay_fall_inh_;
        double norm_exc_;
        double norm_inh_;
        double h_; //!< simulation time step in ms
        double dt_rate_; //!< rate parameter of dead time distribution

        librandom::RngPtr rng_; // random number generator of my own thread
        librandom::PoissonRandomDev poisson_dev_; // random deviate generator
        librandom::GammaRandomDev gamma_dev_; // random deviate generator

        int DeadTimeCounts_;

    };

    // Access functions for UniversalDataLogger -----------------------

    //! Read out the real membrane potential

    double get_V_m_() const
    {
        return S_.u_membrane_;
    }

    //! Read out the adaptive threshold potential

    double get_E_sfa_() const
    {
        return S_.adaptative_threshold_;
    }

    /**
     * @defgroup iaf_psc_alpha_data
     * Instances of private data structures for the different types
     * of data pertaining to the model.
     * @note The order of definitions is important for speed.
     * @{
     */
    Parameters_ P_;
    State_ S_;
    Variables_ V_;
    Buffers_ B_;
    /** @} */

    //! Mapping of recordables names to access functions
    static nest::RecordablesMap<PoissonDblExpNeuron> recordablesMap_;
};


/**
 * PoissonDblExpNeuron test event.
 */
inline
nest::port PoissonDblExpNeuron::send_test_event(nest::Node& target, nest::rport receptor_type, nest::synindex, bool)
{
    nest::SpikeEvent e;
    e.set_sender(*this);

    return target.handles_test_event(e, receptor_type);
}

/**
 * PoissonDblExpNeuron test event.
 */
inline
nest::port PoissonDblExpNeuron::handles_test_event(nest::SpikeEvent&, nest::rport receptor_type)
{
    if ((receptor_type != 0) && (receptor_type != 1))
        throw nest::UnknownReceptorType(receptor_type, get_name());
    return 0;
}

/**
 * PoissonDblExpNeuron test event.
 */
inline
nest::port PoissonDblExpNeuron::handles_test_event(nest::CurrentEvent&, nest::rport receptor_type)
{
    if (receptor_type != 0)
        throw nest::UnknownReceptorType(receptor_type, get_name());
    return 0;
}

/**
 * PoissonDblExpNeuron test event.
 */
inline
nest::port PoissonDblExpNeuron::handles_test_event(nest::DataLoggingRequest &dlr,
                                                   nest::rport receptor_type)
{
    if (receptor_type != 0)
        throw nest::UnknownReceptorType(receptor_type, get_name());
    return B_.logger_.connect_logging_device(dlr, recordablesMap_);
}

/**
 * Status getter function.
 */
inline
void PoissonDblExpNeuron::get_status(DictionaryDatum &d) const
{
    P_.get(d);
    S_.get(d, P_);

    (*d)[nest::names::recordables] = recordablesMap_.get_list();
}

/**
 * Status setter function.
 */
inline
void PoissonDblExpNeuron::set_status(const DictionaryDatum &d)
{
    Parameters_ ptmp = P_; // temporary copy in case of errors
    ptmp.set(d); // throws if BadProperty
    State_ stmp = S_; // temporary copy in case of errors
    stmp.set(d, ptmp); // throws if BadProperty

    // We now know that (ptmp, stmp) are consistent. We do not
    // write them back to (P_, S_) before we are also sure that
    // the properties to be set in the parent class are internally
    // consistent.
    //Archiving_Node::set_status(d);

    // if we get here, temporaries contain consistent set of properties
    P_ = ptmp;
    S_ = stmp;
}

} // namespace

#endif /* #ifndef POISSON_DBL_EXP_NEURON_H */
