/*********************                                                        */
/*! \file Test_SigmoidConstraint.h
 **
 ** [[ Add lengthier description here ]]

**/

#include <cxxtest/TestSuite.h>

#include "InputQuery.h"
#include "MockConstraintBoundTightener.h"
#include "MockErrno.h"
#include "MockTableau.h"
#include "PiecewiseLinearCaseSplit.h"
#include "SigmoidConstraint.h"
#include "MarabouError.h"
#include "FloatUtils.h"


#include <string.h>

class MockForSigmoidConstraint : public MockErrno
{
public:
};

class SigmoidConstraintTestSuite : public CxxTest::TestSuite {
public:
    MockForSigmoidConstraint *mock;

    void setUp() {
        TS_ASSERT(mock = new MockForSigmoidConstraint);
    }

    void tearDown() {
        TS_ASSERT_THROWS_NOTHING(delete mock);
    }

    void test_sigmoid_constraint() {
        unsigned b = 1;
        unsigned f = 4;

        SigmoidConstraint sigmoid(b, f);

        List<unsigned> participatingVariables;
        TS_ASSERT_THROWS_NOTHING(
                participatingVariables = sigmoid.getParticipatingVariables());
        TS_ASSERT_EQUALS(participatingVariables.size(), 2U);
        auto it = participatingVariables.begin();
        TS_ASSERT_EQUALS(*it, b);
        ++it;
        TS_ASSERT_EQUALS(*it, f);

        TS_ASSERT(sigmoid.participatingVariable(b));
        TS_ASSERT(sigmoid.participatingVariable(f));
        TS_ASSERT(!sigmoid.participatingVariable(2));
        TS_ASSERT(!sigmoid.participatingVariable(0));
        TS_ASSERT(!sigmoid.participatingVariable(5));

        TS_ASSERT_THROWS_EQUALS(sigmoid.satisfied(),
                                const MarabouError &e,
                                e.getCode(),
                                MarabouError::PARTICIPATING_VARIABLES_ABSENT);

        sigmoid.notifyVariableValue(b, 0);
        sigmoid.notifyVariableValue(f, 0.5);

        TS_ASSERT(sigmoid.satisfied());

        sigmoid.notifyVariableValue(f, FloatUtils::sigmoid(1));

        TS_ASSERT(!sigmoid.satisfied());

        sigmoid.notifyVariableValue(b, 1);

        TS_ASSERT(sigmoid.satisfied());

        sigmoid.notifyVariableValue(b, -2);

        TS_ASSERT(!sigmoid.satisfied());

        sigmoid.notifyVariableValue(f, FloatUtils::sigmoid(-2));

        TS_ASSERT(sigmoid.satisfied());

        sigmoid.notifyVariableValue(b, -3);

        TS_ASSERT(!sigmoid.satisfied());

        sigmoid.notifyVariableValue(b, 0);

        TS_ASSERT(!sigmoid.satisfied());

        sigmoid.notifyVariableValue(f, 0);
        sigmoid.notifyVariableValue(b, 11);

        TS_ASSERT(!sigmoid.satisfied());

        sigmoid.notifyVariableValue(b, 0);
        sigmoid.notifyVariableValue(f, 0.5);
        TS_ASSERT(sigmoid.satisfied());

        // Changing variable indices
        unsigned newB = 12;
        unsigned newF = 14;

        TS_ASSERT_THROWS_NOTHING(sigmoid.updateVariableIndex(b, newB));
        TS_ASSERT_THROWS_NOTHING(sigmoid.updateVariableIndex(f, newF));

        TS_ASSERT(sigmoid.satisfied());

        sigmoid.notifyVariableValue(newF, FloatUtils::sigmoid(1));

        TS_ASSERT(!sigmoid.satisfied());

        sigmoid.notifyVariableValue(newB, 1);

        TS_ASSERT(sigmoid.satisfied());
    }


    void test_sigmoid_fixes() {
        unsigned b = 1;
        unsigned f = 4;

        SigmoidConstraint sigmoid(b, f);

        List<PiecewiseLinearConstraint::Fix> fixes;
        List<PiecewiseLinearConstraint::Fix>::iterator it;


        sigmoid.notifyVariableValue(b, 0);
        sigmoid.notifyVariableValue(f, 2);

        TS_ASSERT(!sigmoid.satisfied());

        fixes = sigmoid.getPossibleFixes();
        it = fixes.begin();
        TS_ASSERT_EQUALS(it->_variable, f);
        TS_ASSERT_EQUALS(it->_value, 0.5);
    }


    void test_sigmoid_case_splits_after_assignments()
    {
        unsigned b = 2;
        unsigned f = 3;

        SigmoidConstraint sigmoid( b, f );

        List<PiecewiseLinearConstraint::Fix> fixes;

        sigmoid.notifyLowerBound(b, 0);
        sigmoid.notifyLowerBound(f, 0.5);

        sigmoid.notifyUpperBound(b, 1);
        sigmoid.notifyUpperBound(f, FloatUtils::sigmoid(1));

        sigmoid.notifyVariableValue(b, 0.5);
        sigmoid.notifyVariableValue(f, 2);

        List<PiecewiseLinearCaseSplit> splits = sigmoid.getCaseSplits();


        TS_ASSERT_EQUALS( splits.size(), 2U );

        PiecewiseLinearCaseSplit split1 = splits.front();
        PiecewiseLinearCaseSplit split2 = splits.back();


        Tightening boundsForSplit1[4] = {
                Tightening(b, 0, Tightening::BoundType::LB),
                Tightening(b, 0.5, Tightening::BoundType::UB),
                Tightening(f, 0.5, Tightening::BoundType::LB),
                Tightening(f, FloatUtils::sigmoid(0.5), Tightening::BoundType::UB)
        };

        Tightening boundsForSplit2[4] = {
                Tightening(b, 0.5, Tightening::BoundType::LB),
                Tightening(b, 1, Tightening::BoundType::UB),
                Tightening(f, FloatUtils::sigmoid(0.5), Tightening::BoundType::LB),
                Tightening(f, FloatUtils::sigmoid(1), Tightening::BoundType::UB)
        };

        Equation eq1 = split1.getEquations().front();
        TS_ASSERT_EQUALS(getEquationValue(eq1, 0, 0.5), 0);
        TS_ASSERT_EQUALS(getEquationValue(eq1, 0.5, FloatUtils::sigmoid(0.5)), 0);
        assertBounds(split1, boundsForSplit1);

        Equation eq2 = split2.getEquations().front();
        TS_ASSERT_EQUALS(getEquationValue(eq1, 0.5, FloatUtils::sigmoid(0.5)), 0);
        TS_ASSERT_EQUALS(getEquationValue(eq2, 1, FloatUtils::sigmoid(1)), 0);
        assertBounds(split2, boundsForSplit2);
    }

    double getEquationValue(Equation &equation, double x, double y) {
        double sum = 0;
        for (Equation::Addend addend : equation._addends)
            sum += (addend._variable == 2? addend._coefficient * x : addend._coefficient * y);
        return sum - equation._scalar;
    }

    void assertBounds( PiecewiseLinearCaseSplit &split, Tightening* bounds )
    {
        TS_ASSERT_EQUALS( split.getBoundTightenings().size(), 4U);
        for (int i=0; i<4; i++)
            TS_ASSERT(split.getBoundTightenings().exists(bounds[i]));
    }
};

//
// Local Variables:
// compile-command: "make -C ../../.. "
// tags-file-name: "../../../TAGS"
// c-basic-offset: 4
// End:
//

