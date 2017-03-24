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

    /**
     * @brief A device which receives reward traces from MUSIC.
     */
    class RewardInProxy : public TracingNode {
    public:

        RewardInProxy();

        bool has_proxies() const {
            return false;
        }

        bool one_node_per_process() const {
            return true;
        }

        virtual void get_status(DictionaryDatum& d) const;
        virtual void set_status(const DictionaryDatum& d);

    protected:

        virtual void init_buffers_();
        virtual void init_state_(const Node&);

        virtual void calibrate();

        virtual void update(nest::Time const&, const long, const long);

        MUSIC::ContInputPort* reward_in_; //!< The MUSIC cont port for reward input
        std::vector< double > reward_in_buffer_; //!< The mapped buffer for receiving the reward values via MPI

        // ------------------------------------------------------------

        struct State_ {
            State_(); //!< Sets default state value

            void get(DictionaryDatum&) const; //!< Store current values in dictionary

            bool published_; //!< indicates whether this node has been published already with MUSIC
            int port_width_; //!< the width of the MUSIC port
        };

        struct Parameters_ {
            Parameters_(); //!< Sets default parameter values

            void get(DictionaryDatum&) const; //!< Store current values in dictionary
            void set(const DictionaryDatum&, State_&); //!< Set values from dicitonary

            std::string port_name_; //!< the name of MUSIC port to read from
            float delay_; //!< the accepted delay for the MUSIC connection
        };

        State_ S_;
        Parameters_ P_;
    };
}

#endif