#ifndef REWARD_IN_PROXY_H
#define REWARD_IN_PROXY_H

#include "nest.h"
#include "event.h"
#include "ring_buffer.h"
#include "spikecounter.h"
#include "connection.h"

#include "namedatum.h"
#include "universal_data_logger.h"

#include "tracing_node.h"

namespace spore {

    class RewardInProxy : public TracingNode {
    public:

        RewardInProxy();
        RewardInProxy(const RewardInProxy&);

        bool has_proxies() const {
            return false;
        }

        bool one_node_per_process() const {
            return true;
        }

        using nest::Node::handle;
        using nest::Node::handles_test_event;
        nest::port send_test_event(nest::Node&, nest::port, nest::synindex, bool);

        // TODO can we eliminate the spike handlers?

        void handle(nest::SpikeEvent &); //! accept spikes
        void handle(nest::DataLoggingRequest &); //! allow recording with multimeter

        nest::port handles_test_event(nest::SpikeEvent&, nest::port);
        nest::port handles_test_event(nest::DataLoggingRequest&, nest::port);

        virtual void get_status(DictionaryDatum& d) const;
        virtual void set_status(const DictionaryDatum& d);

    protected:

        virtual void init_buffers_();
        virtual void init_state_(const Node&);
        virtual void calibrate();

        virtual void update(nest::Time const&, const nest::long_t, const nest::long_t);

        // The next two classes need to be friends to access the State_ class/member
        friend class nest::RecordablesMap<RewardInProxy>;
        friend class nest::UniversalDataLogger<RewardInProxy>;

        // Access functions for UniversalDataLoggel

        double get_reward() const {
            return B_.data_[0]; // TODO read from trace?
        }

        // ------------------------------------------------------------

        struct State_;

        struct Parameters_ {
            Parameters_(); //!< Sets default parameter values
            Parameters_(const Parameters_&); //!< Recalibrate all times

            void get(DictionaryDatum&) const; //!< Store current values in dictionary
            void set(const DictionaryDatum&, State_&); //!< Set values from dicitonary

            std::string port_name_; //!< the name of MUSIC port to connect to
        };

        // ------------------------------------------------------------

        struct State_ {
            State_(); //!< Sets default state value

            void get(DictionaryDatum&) const; //!< Store current values in dictionary
            void set(const DictionaryDatum&, const Parameters_&); //!< Set values from dicitonary

            bool published_; //!< indicates whether this node has been published already with MUSIC
            int port_width_; //!< the width of the MUSIC port

        };

        // ------------------------------------------------------------

        struct Buffers_ {
            Buffers_(RewardInProxy &);
            Buffers_(const Buffers_ &, RewardInProxy &);

            void get(DictionaryDatum&) const;
            void set(const DictionaryDatum&);

            std::vector< double > data_; //!< The buffer for incoming data

            //! Logger for all analog data
            nest::UniversalDataLogger<RewardInProxy> logger_;
        };

        // ------------------------------------------------------------

        struct Variables_ {
            MUSIC::ContInputPort* MP_; //!< The MUSIC cont port for input of data
        };

        // ------------------------------------------------------------

        Parameters_ P_;
        State_ S_;
        Buffers_ B_;
        Variables_ V_;

        //! Mapping of recordables names to access functions
        static nest::RecordablesMap<RewardInProxy> recordablesMap_;
    };

    inline
    nest::port RewardInProxy::send_test_event(nest::Node& target, nest::port receptor_type,
            nest::synindex, bool) {
        // You should usually not change the code in this function.
        // It confirms that the target of connection @c c accepts @c SpikeEvent on
        // the given @c receptor_type.
        nest::SpikeEvent e;
        e.set_sender(*this);
        return target.handles_test_event(e, receptor_type);
    }

    inline
    nest::port RewardInProxy::handles_test_event(nest::SpikeEvent&, nest::port receptor_type) {
        // You should usually not change the code in this function.
        // It confirms to the connection management system that we are able
        // to handle @c SpikeEvent on port 0. You need to extend the function
        // if you want to differentiate between input ports.
        if (receptor_type != 0)
            throw nest::UnknownReceptorType(receptor_type, get_name());
        return 0;
    }

    inline
    nest::port RewardInProxy::handles_test_event(nest::DataLoggingRequest& dlr,
            nest::port receptor_type) {
        // You should usually not change the code in this function.
        // It confirms to the connection management system that we are able
        // to handle @c DataLoggingRequest on port 0.
        // The function also tells the built-in UniversalDataLogger that this node
        // is recorded from and that it thus needs to collect data during simulation.
        if (receptor_type != 0)
            throw nest::UnknownReceptorType(receptor_type, get_name());

        return B_.logger_.connect_logging_device(dlr, recordablesMap_);
    }
}

#endif
