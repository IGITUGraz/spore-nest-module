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
 * File:   reward_in_proxy.h
 * Author: Hoff
 *
 * This file is based on music_cont_in_proxy.h which is part of NEST
 * (Copyright (C) 2004 The NEST Initiative).
 * See: http://nest-initiative.org/
 */

#ifndef REWARD_IN_PROXY_H
#define REWARD_IN_PROXY_H

#include "nest.h"
#include "event.h"
#include "ring_buffer.h"
#include "spikecounter.h"
#include "connection.h"

#include "namedatum.h"
#include "universal_data_logger.h"

#include <music.hh>

#include "tracing_node.h"

namespace spore
{

/**
 * @brief A device which receives reward traces from MUSIC.
 */
class RewardInProxy : public TracingNode
{
public:

    RewardInProxy();

    bool has_proxies() const
    {
        return false;
    }

    bool one_node_per_process() const
    {
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

    /**
     * @brief Class holding state variables of the proxy.
     */
    struct State_
    {
        State_(); //!< Sets default state value

        void get(DictionaryDatum&) const; //!< Store current values in dictionary

        bool published_; //!< indicates whether this node has been published already with MUSIC
        int port_width_; //!< the width of the MUSIC port
    };

    /**
     * @brief Class holding parameter variables of the proxy.
     */
    struct Parameters_
    {
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
