/* 
 * File:   spore_test_base.h
 * Author: Kappel
 *
 * Created on November 9, 2016, 3:13 PM
 */

#ifndef SAMBA_TEST_BASE_H
#define	SAMBA_TEST_BASE_H

#include <string>

#include "nest.h"

#include "tracing_node.h"


namespace spore
{
/**
 * Base class to all tests.
 */
class SporeTestBase
{
public:
    SporeTestBase(const std::string &name, nest::delay t_max=1)
    :name_(name), t_max_(t_max)
    {};
    
    ~SporeTestBase()
    {};
    
    const std::string &get_name() const
    {
        return name_;
    };

    nest::delay get_t_max() const
    {
        return t_max_;
    };
    
    virtual void init() {};
    virtual void check(nest::delay time_step, TracingNode *node) {};
    virtual size_t get_num_traces() const {return 0;};
    virtual double get_trace(nest::delay time_step, TracingNode::trace_id id) { assert(false); };
    
protected:
    void test_assert(bool condition, const std::string &msg)
    {
        if (!condition)
            throw std::runtime_error(name_+" failed: "+msg);
    };
    
private:
    std::string name_;
    nest::delay t_max_;
};

}

#endif	/* SAMBA_TEST_BASE_H */

