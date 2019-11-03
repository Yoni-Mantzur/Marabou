//
// Created by yoni_mantzur on 11/3/19.
//

#ifndef MARABOU_TEST_PIECEWISELINEARABSTRACTION_H
#define MARABOU_TEST_PIECEWISELINEARABSTRACTION_H

//
// Created by yoni_mantzur on 11/3/19.
//

#ifndef MARABOU_TEST_SIGMOIDCONSTRAINT_H
#define MARABOU_TEST_SIGMOIDCONSTRAINT_H


#include <cxxtest/TestSuite.h>

#include "InputQuery.h"
#include "MockConstraintBoundTightener.h"
#include "MockErrno.h"
#include "MockTableau.h"
#include "PiecewiseLinearCaseSplit.h"
#include "SigmoidConstraint.h"
#include "MarabouError.h"

#include <string.h>

class MockForPiecewiseLinearAbstraction : public MockErrno
{
public:
};

class PiecewiseLinearAbstractionTestSuite : public CxxTest::TestSuite
{
public:
    MockForPiecewiseLinearAbstraction; *mock;

    void setUp()
    {
        TS_ASSERT( mock = new MockForPiecewiseLinearAbstraction );
    }

    void tearDown()
    {
        TS_ASSERT_THROWS_NOTHING( delete mock );
    }

#endif //MARABOU_TEST_PIECEWISELINEARABSTRACTION_H
