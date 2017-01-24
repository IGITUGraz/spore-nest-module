/* 
 * File:   test_circular_buffer.h
 * Author: kappel
 *
 * Created on November 9, 2016, 3:16 PM
 */

#ifndef TEST_CIRCULAR_BUFFER_H
#define	TEST_CIRCULAR_BUFFER_H

#include "spore_test_base.h"

namespace spore
{
/**
 * Test class for circular buffer container.
 */
class TestCircularBuffer : public SambaTestBase
{
public:
    TestCircularBuffer();
    virtual void init();
};

}

#endif	/* TEST_CIRCULAR_BUFFER_H */

