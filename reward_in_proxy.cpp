#include "reward_in_proxy.h"

#include "config.h"
#include "exceptions.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "arraydatum.h"
#include "spikecounter.h"
#include "universal_data_logger_impl.h"
#include "connection_updater.h"
#include "music.hh"
#include "dictutils.h"
#include "music.hh"
#include "tracing_node.h"

#include <numeric>
#include <iterator>

#include <stdio.h>

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap<spore::RewardInProxy> spore::RewardInProxy::recordablesMap_;

namespace nest {

    template<>
    void RecordablesMap<spore::RewardInProxy>::create() {
        Name reward("reward");
        insert_(reward, &spore::RewardInProxy::get_reward);
    }
}

namespace spore {

    /* ----------------------------------------------------------------
     * Default constructors defining default parameters, state and buffer
     * ---------------------------------------------------------------- */

    RewardInProxy::Parameters_::Parameters_()
    : port_name_("reward_in") {
    }

    RewardInProxy::Parameters_::Parameters_(const Parameters_& op)
    : port_name_(op.port_name_) {
    }

    RewardInProxy::State_::State_()
    : published_(false)
    , port_width_(-1) {
    }

    RewardInProxy::Buffers_::Buffers_(RewardInProxy &n)
    : logger_(n) {
    }

    RewardInProxy::Buffers_::Buffers_(const Buffers_ &src, RewardInProxy &n)
    : logger_(n) {
        // TODO use src
    }

    /* ----------------------------------------------------------------
     * Parameter extraction and manipulation functions
     * ---------------------------------------------------------------- */

    void
    RewardInProxy::Parameters_::get(DictionaryDatum& d) const {
        (*d)[ nest::names::port_name ] = port_name_;
    }

    void
    RewardInProxy::Parameters_::set(const DictionaryDatum& d, State_& s) {
        // TODO: This is not possible, as P_ does not know about get_name()
        //  if(d->known(names::port_name) && s.published_)
        //    throw MUSICPortAlreadyPublished(get_name(), P_.port_name_);

        if (!s.published_)
            updateValue< string >(d, nest::names::port_name, port_name_);
    }

    void
    RewardInProxy::State_::get(DictionaryDatum& d) const {
        (*d)[ nest::names::published ] = published_;
        (*d)[ nest::names::port_width ] = port_width_;
    }

    void
    RewardInProxy::State_::set(const DictionaryDatum&, const Parameters_&) {
    }

    void
    RewardInProxy::Buffers_::get(DictionaryDatum & d) const {
        (*d)[ "reward" ] = DoubleVectorDatum(new std::vector< double >(data_));
    }

    void
    RewardInProxy::Buffers_::set(const DictionaryDatum & d) {
    }

    /* ----------------------------------------------------------------
     * Default and copy constructor for node
     * ---------------------------------------------------------------- */

    RewardInProxy::RewardInProxy()
    : TracingNode()
    , S_()
    , P_()
    , B_(*this) {
        // TODO this class allows multiple instances -> static object wrong?
        recordablesMap_.create();
    }

    RewardInProxy::RewardInProxy(const RewardInProxy & n)
    : TracingNode(n)
    , S_()
    , P_(n.P_)
    , B_(n.B_, *this) {
    }

    /* ----------------------------------------------------------------
     * Node initialization functions
     * ---------------------------------------------------------------- */

    void
    RewardInProxy::init_state_(const Node& n) {
        const RewardInProxy& pr = downcast< RewardInProxy >(n);

        S_ = pr.S_;
    }

    void
    RewardInProxy::init_buffers_() {
        B_.logger_.reset(); // includes resize
        init_traces(1);
    }

    void
    RewardInProxy::calibrate() {
        B_.logger_.init();

        // only publish the port once
        if (!S_.published_) {
            MUSIC::Setup* s = nest::Communicator::get_music_setup();
            if (s == 0)
                throw nest::MUSICSimulationHasRun(get_name());

            V_.MP_ = s->publishContInput(P_.port_name_);

            if (V_.MP_->isConnected()) {
				if (!V_.MP_->hasWidth())
					throw nest::MUSICPortHasNoWidth(get_name(), P_.port_name_);

				S_.port_width_ = V_.MP_->width();

				B_.data_ = std::vector< double >(S_.port_width_);
				MUSIC::ArrayData data_map(
						static_cast<void*> (&(B_.data_[ 0 ])), MPI::DOUBLE, 0, S_.port_width_);

				V_.MP_->map(&data_map, 0.0, true);
				S_.published_ = true;

				std::string msg = String::compose(
						"Mapping MUSIC input port '%1' with width=%2.", P_.port_name_, S_.port_width_);
				net_->message(SLIInterpreter::M_INFO, "reward_in_proxy::calibrate()", msg.c_str());
			} else {
				// throw nest::MUSICPortUnconnected(get_name(), P_.port_name_);
				std::string msg = String::compose("MUSIC port '%1' is unconnected.", P_.port_name_);
				net_->message(SLIInterpreter::M_WARNING, "reward_in_proxy::calibrate()", msg.c_str());
			}
        }
    }

    void
    RewardInProxy::get_status(DictionaryDatum& d) const
    {
        P_.get(d);
        S_.get(d);
        B_.get(d);

        (*d)[nest::names::element_type] = LiteralDatum(nest::names::other);
        (*d)[nest::names::recordables] = recordablesMap_.get_list();
    }

    void
    RewardInProxy::set_status(const DictionaryDatum& d)
    {
        Parameters_ ptmp = P_; // temporary copy in case of errors
        ptmp.set(d, S_); // throws if BadProperty

        State_ stmp = S_;
        stmp.set(d, P_); // throws if BadProperty

        // if we get here, temporaries contain consistent set of properties
        P_ = ptmp;
        S_ = stmp;

        B_.set(d);
    }

    void RewardInProxy::handle(nest::SpikeEvent& e)
    {
    }

    void RewardInProxy::handle(nest::DataLoggingRequest& e)
    {
        B_.logger_.handle(e);
    }

    void
    RewardInProxy::update(const nest::Time& origin, const long from, const long to)
    {
        int n_channels = S_.port_width_;
        std::vector<double> data = B_.data_;
		
        if (n_channels == -1)
        {
            return;
        }

        for (long lag = from; lag < to; ++lag)
        {
            if (n_channels != 1)
            {
                std::cerr << "RewardInProxy::update ~ Port width 1 required!" << std::endl;
            }
            nest::Time time = nest::Time::step(origin.get_steps() + lag);
            set_trace(time.get_steps(), data[0]);
        }

        // debug
        /*
        for (int i = 0; i < n_channels; i++) {
            std::cout << "trace #" << i << ":";
            TracingNode::const_iterator trace = get_trace(0, i);
            size_t steps = ConnectionUpdateManager::instance()->get_max_latency();
            while (steps--) {
                std::cout << " " << *trace;
                ++trace;
            }
            std::cout << std::endl;
        }
        */
    }
}
