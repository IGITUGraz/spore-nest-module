/* 
 * File:   spore_test_module.h
 * Author: Kappel
 *
 * Created on November 9, 2016, 2:30 PM
 */

#ifndef SPORE_TEST_MODULE_H
#define	SPORE_TEST_MODULE_H

#include "dictutils.h"

#include "tracing_node.h"
#include "spore_test_base.h"

namespace spore
{
/**
 * Node to test spore module.
 */
class SporeTestNode : public TracingNode
{
public:
    SporeTestNode();
    ~SporeTestNode();
    
    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &);
    
    /**
     * Import sets of overloaded virtual functions.
     * @see Technical Issues / Virtual Functions: Overriding, Overloading, and Hiding
     */
    using nest::Node::handle;
    using nest::Node::handles_test_event;    
    
    void handle(nest::SpikeEvent &);
    nest::port handles_test_event(nest::SpikeEvent&, nest::rport);
    nest::port send_test_event(nest::Node&, nest::rport, nest::synindex, bool);

        
protected:
    void register_test(SporeTestBase *test);
    
private:
    void init_state_(const nest::Node& proto);
    void init_buffers_();
    void calibrate();

    void update(nest::Time const &, const long, const long);

    std::string test_name_;
    double test_time_;
    
    std::map<std::string, SporeTestBase*> tests_;
};


/**
 * PoissonDblExpNeuron test event.
 */
inline
nest::port SporeTestNode::send_test_event(nest::Node& target, nest::rport receptor_type, nest::synindex, bool)
{
    nest::SpikeEvent e;
    e.set_sender(*this);

    return target.handles_test_event(e, receptor_type);
}


/**
 * PoissonDblExpNeuron test event.
 */
inline
nest::port SporeTestNode::handles_test_event(nest::SpikeEvent&, nest::rport receptor_type)
{
    return 0;
}


/**
 * Status getter function.
 */
inline
void SporeTestNode::get_status(DictionaryDatum &d) const
{
    def<std::string>(d, "test_name", test_name_);
    def<double>(d, "test_time", test_time_);
}

/**
 * Status setter function.
 */
inline
void SporeTestNode::set_status(const DictionaryDatum &d)
{
    std::string test_name;
    if (updateValue<std::string>(d, "test_name", test_name))
    {
        if (tests_.find(test_name)==tests_.end())
            throw nest::BadParameter("test '"+test_name+"' does not exist!");
                    
        test_name_ = test_name;
    }
    
    updateValue<double>(d, "test_time", test_time_);
}

}

#endif	/* SPORE_TEST_MODULE_H */

