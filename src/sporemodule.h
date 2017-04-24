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
 * File:   sporemodule.h
 * Author: Kappel, Hsieh
 *
 * Created on March 30, 2015, 3:19 AM
 */


#ifndef SPOREMODULE_H
#define SPOREMODULE_H

#include "slimodule.h"
#include "slifunction.h"


namespace spore
{

/**
 * @brief The class defining the SPORE module.
 */
class SporeModule : public SLIModule
{
public:

    SporeModule();
    ~SporeModule();

    void init(SLIInterpreter*);

    const std::string name(void) const;
    const std::string commandstring(void) const;

protected:

    /**
     * @brief \a InitSynapseUpdater SLI function.
     *
     * This SLI command takes two arguments \a interval and \a max_delay.
     * \a interval determines the update interval of diligent synapses and
     * must be strictly larger than 0. \a max_delay determines the maximum
     * acceptable delay for diligent synapses and must be larger or equal
     * to 0.
     *
     * @see ConnectionUpdateManager
     */
    class InitSynapseUpdater_i_i_Function : public SLIFunction
    {
    public:
        InitSynapseUpdater_i_i_Function();
        void execute(SLIInterpreter*) const;
    }
    init_synapse_updater_i_i_function_;

};

}

#endif
