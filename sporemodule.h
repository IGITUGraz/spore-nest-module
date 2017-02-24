/**
 * File:   spore_module.h
 * Author: Kappel, Hsieh
 */


#ifndef SPOREMODULE_H
#define SPOREMODULE_H

#include "slimodule.h"
#include "slifunction.h"


namespace spore
{

/**
 * Class defining the SPORE module.
 */
class SporteModule : public SLIModule
{
public:

    SporeModule();
    ~SporeModule();

    void init(SLIInterpreter*);

    const std::string name(void) const;
    const std::string commandstring(void) const;

private:

    class InitSynapseUpdater_i_i_Function : public SLIFunction
    {
    public:
        InitSynapseUpdater_i_i_Function();
        void execute(SLIInterpreter *) const;
    }
    init_synapse_updater_i_i_function_;

};

}

#endif
