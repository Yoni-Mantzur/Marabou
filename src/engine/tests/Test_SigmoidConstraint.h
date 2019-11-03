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

class SigmoidConstraintTestSuite : public CxxTest::TestSuite
{
public:
    MockForSigmoidConstraint *mock;

    void setUp() {
        TS_ASSERT(mock = new MockForSigmoidConstraint);
    }

    void tearDown() {
        TS_ASSERT_THROWS_NOTHING(delete mock);
    }

    void test_sigmoid_constraint()
    {
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
                MarabouError::PARTICIPATING_VARIABLES_ABSENT );

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
};

//    void test_sigmoid_fixes() {
//        TS_ASSERT(true);

//        unsigned b = 1;
//        unsigned f = 4;
//
//        SigmoidConstraint sigmoid( b, f );
//
//        List<PiecewiseLinearConstraint::Fix> fixes;
//        List<PiecewiseLinearConstraint::Fix>::iterator it;
//
//        sigmoid.notifyVariableValue( b, -1 );
//        sigmoid.notifyVariableValue( f, 1 );
//
//        fixes = sigmoid.getPossibleFixes();
//        it = fixes.begin();
//        TS_ASSERT_EQUALS( it->_variable, b );
//        TS_ASSERT_EQUALS( it->_value, 1 );
//        ++it;
//        TS_ASSERT_EQUALS( it->_variable, f );
//        TS_ASSERT_EQUALS( it->_value, 0 );
//
//        sigmoid.notifyVariableValue( b, 2 );
//        sigmoid.notifyVariableValue( f, 1 );
//
//        fixes = sigmoid.getPossibleFixes();
//        it = fixes.begin();
//        TS_ASSERT_EQUALS( it->_variable, b );
//        TS_ASSERT_EQUALS( it->_value, 1 );
//        ++it;
//        TS_ASSERT_EQUALS( it->_variable, f );
//        TS_ASSERT_EQUALS( it->_value, 2 );
//
//        sigmoid.notifyVariableValue( b, 11 );
//        sigmoid.notifyVariableValue( f, 0 );
//
//        fixes = sigmoid.getPossibleFixes();
//        it = fixes.begin();
//        TS_ASSERT_EQUALS( it->_variable, b );
//        TS_ASSERT_EQUALS( it->_value, 0 );
//        ++it;
//        TS_ASSERT_EQUALS( it->_variable, f );
//        TS_ASSERT_EQUALS( it->_value, 11 );
//    }
//
//    void test_sigmoid_case_splits()
//    {
//        unsigned b = 1;
//        unsigned f = 4;
//
//        SigmoidConstraint sigmoid( b, f );
//
//        List<PiecewiseLinearConstraint::Fix> fixes;
//        List<PiecewiseLinearConstraint::Fix>::iterator it;
//
//        List<PiecewiseLinearCaseSplit> splits = sigmoid.getCaseSplits();
//
//        Equation activeEquation, inactiveEquation;
//
//        TS_ASSERT_EQUALS( splits.size(), 2U );
//
//        List<PiecewiseLinearCaseSplit>::iterator split1 = splits.begin();
//        List<PiecewiseLinearCaseSplit>::iterator split2 = split1;
//        ++split2;
//
//        TS_ASSERT( isActiveSplit( b, f, split1 ) || isActiveSplit( b, f, split2 ) );
//        TS_ASSERT( isInactiveSplit( b, f, split1 ) || isInactiveSplit( b, f, split2 ) );
//    }
//
//    bool isActiveSplit( unsigned b, unsigned f, List<PiecewiseLinearCaseSplit>::iterator &split )
//    {
//        List<Tightening> bounds = split->getBoundTightenings();
//
//        auto bound = bounds.begin();
//        Tightening bound1 = *bound;
//
//        TS_ASSERT_EQUALS( bound1._variable, b );
//        TS_ASSERT_EQUALS( bound1._value, 0.0 );
//
//        if ( bound1._type != Tightening::LB )
//            return false;
//
//        TS_ASSERT_EQUALS( bounds.size(), 1U );
//
//        Equation activeEquation;
//        auto equations = split->getEquations();
//        TS_ASSERT_EQUALS( equations.size(), 1U );
//        activeEquation = split->getEquations().front();
//        TS_ASSERT_EQUALS( activeEquation._addends.size(), 2U );
//        TS_ASSERT_EQUALS( activeEquation._scalar, 0.0 );
//
//        auto addend = activeEquation._addends.begin();
//        TS_ASSERT_EQUALS( addend->_coefficient, 1.0 );
//        TS_ASSERT_EQUALS( addend->_variable, b );
//
//        ++addend;
//        TS_ASSERT_EQUALS( addend->_coefficient, -1.0 );
//        TS_ASSERT_EQUALS( addend->_variable, f );
//
//        TS_ASSERT_EQUALS( activeEquation._type, Equation::EQ );
//
//        return true;
//    }
//
//    bool isInactiveSplit( unsigned b, unsigned f, List<PiecewiseLinearCaseSplit>::iterator &split )
//    {
//        List<Tightening> bounds = split->getBoundTightenings();
//
//        auto bound = bounds.begin();
//        Tightening bound1 = *bound;
//
//        TS_ASSERT_EQUALS( bound1._variable, b );
//        TS_ASSERT_EQUALS( bound1._value, 0.0 );
//
//        if ( bound1._type != Tightening::UB )
//            return false;
//
//        TS_ASSERT_EQUALS( bounds.size(), 2U );
//
//        ++bound;
//        Tightening bound2 = *bound;
//
//        TS_ASSERT_EQUALS( bound2._variable, f );
//        TS_ASSERT_EQUALS( bound2._value, 0.0 );
//        TS_ASSERT_EQUALS( bound2._type, Tightening::UB );
//
//        auto equations = split->getEquations();
//        TS_ASSERT( equations.empty() );
//
//        return true;
//    }
//
//    void test_register_as_watcher()
//    {
//        unsigned b = 1;
//        unsigned f = 4;
//
//        MockTableau tableau;
//
//        SigmoidConstraint sigmoid( b, f );
//
//        TS_ASSERT_THROWS_NOTHING( sigmoid.registerAsWatcher( &tableau ) );
//
//        TS_ASSERT_EQUALS( tableau.lastRegisteredVariableToWatcher.size(), 2U );
//        TS_ASSERT( tableau.lastUnregisteredVariableToWatcher.empty() );
//        TS_ASSERT_EQUALS( tableau.lastRegisteredVariableToWatcher[b].size(), 1U );
//        TS_ASSERT( tableau.lastRegisteredVariableToWatcher[b].exists( &sigmoid ) );
//        TS_ASSERT_EQUALS( tableau.lastRegisteredVariableToWatcher[f].size(), 1U );
//        TS_ASSERT( tableau.lastRegisteredVariableToWatcher[f].exists( &sigmoid ) );
//
//        tableau.lastRegisteredVariableToWatcher.clear();
//
//        TS_ASSERT_THROWS_NOTHING( sigmoid.unregisterAsWatcher( &tableau ) );
//
//        TS_ASSERT( tableau.lastRegisteredVariableToWatcher.empty() );
//        TS_ASSERT_EQUALS( tableau.lastUnregisteredVariableToWatcher.size(), 2U );
//        TS_ASSERT_EQUALS( tableau.lastUnregisteredVariableToWatcher[b].size(), 1U );
//        TS_ASSERT( tableau.lastUnregisteredVariableToWatcher[b].exists( &sigmoid ) );
//        TS_ASSERT_EQUALS( tableau.lastUnregisteredVariableToWatcher[f].size(), 1U );
//        TS_ASSERT( tableau.lastUnregisteredVariableToWatcher[f].exists( &sigmoid ) );
//    }
//
//    void test_fix_active()
//    {
//        unsigned b = 1;
//        unsigned f = 4;
//
//        MockTableau tableau;
//
//        SigmoidConstraint sigmoid( b, f );
//
//        sigmoid.registerAsWatcher( &tableau );
//
//        List<PiecewiseLinearCaseSplit> splits = sigmoid.getCaseSplits();
//        TS_ASSERT_EQUALS( splits.size(), 2U );
//
//        sigmoid.notifyLowerBound( 1, 1.0 );
//        TS_ASSERT_THROWS_EQUALS( splits = sigmoid.getCaseSplits(),
//        const MarabouError &e,
//        e.getCode(),
//                MarabouError::REQUESTED_CASE_SPLITS_FROM_FIXED_CONSTRAINT );
//
//        sigmoid.unregisterAsWatcher( &tableau );
//
//        sigmoid = SigmoidConstraint( b, f );
//
//        sigmoid.registerAsWatcher( &tableau );
//
//        splits = sigmoid.getCaseSplits();
//        TS_ASSERT_EQUALS( splits.size(), 2U );
//
//        sigmoid.notifyLowerBound( 4, 1.0 );
//        TS_ASSERT_THROWS_EQUALS( splits = sigmoid.getCaseSplits(),
//        const MarabouError &e,
//        e.getCode(),
//                MarabouError::REQUESTED_CASE_SPLITS_FROM_FIXED_CONSTRAINT );
//
//        sigmoid.unregisterAsWatcher( &tableau );
//    }
//
//    void test_fix_inactive()
//    {
//        unsigned b = 1;
//        unsigned f = 4;
//
//        MockTableau tableau;
//
//        SigmoidConstraint sigmoid( b, f );
//
//        sigmoid.registerAsWatcher( &tableau );
//
//        List<PiecewiseLinearCaseSplit> splits = sigmoid.getCaseSplits();
//        TS_ASSERT_EQUALS( splits.size(), 2U );
//
//        sigmoid.notifyUpperBound( 4, -1.0 );
//        TS_ASSERT_THROWS_EQUALS( splits = sigmoid.getCaseSplits(),
//        const MarabouError &e,
//        e.getCode(),
//                MarabouError::REQUESTED_CASE_SPLITS_FROM_FIXED_CONSTRAINT );
//
//        sigmoid.unregisterAsWatcher( &tableau );
//    }
//
//    void test_constraint_phase_gets_fixed()
//    {
//        unsigned b = 1;
//        unsigned f = 4;
//
//        MockTableau tableau;
//
//        // Upper bounds
//        {
//            SigmoidConstraint sigmoid( b, f );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyUpperBound( b, -1.0 );
//            TS_ASSERT( sigmoid.phaseFixed() );
//        }
//
//        {
//            SigmoidConstraint sigmoid( b, f );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyUpperBound( b, 0.0 );
//            TS_ASSERT( sigmoid.phaseFixed() );
//        }
//
//        {
//            SigmoidConstraint sigmoid( b, f );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyUpperBound( f, 0.0 );
//            TS_ASSERT( sigmoid.phaseFixed() );
//        }
//
//        {
//            SigmoidConstraint sigmoid( b, f );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyUpperBound( b, 3.0 );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//        }
//
//        {
//            SigmoidConstraint sigmoid( b, f );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyUpperBound( b, 5.0 );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//        }
//
//        // Lower bounds
//        {
//            SigmoidConstraint sigmoid( b, f );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyLowerBound( b, 3.0 );
//            TS_ASSERT( sigmoid.phaseFixed() );
//        }
//
//        {
//            SigmoidConstraint sigmoid( b, f );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyLowerBound( b, 0.0 );
//            TS_ASSERT( sigmoid.phaseFixed() );
//        }
//
//        {
//            SigmoidConstraint sigmoid( b, f );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyLowerBound( f, 6.0 );
//            TS_ASSERT( sigmoid.phaseFixed() );
//        }
//
//        {
//            SigmoidConstraint sigmoid( b, f );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyLowerBound( f, 0.0 );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//        }
//
//        {
//            SigmoidConstraint sigmoid( b, f );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyLowerBound( b, -2.0 );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//        }
//
//        // Aux variables: upper bound
//        {
//            SigmoidConstraint sigmoid( b, f );
//
//            sigmoid.notifyLowerBound( b, -5 );
//            InputQuery dontCare;
//            unsigned aux = 300;
//            dontCare.setNumberOfVariables( aux );
//            sigmoid.addAuxiliaryEquations( dontCare );
//
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyUpperBound( aux, 3.0 );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyUpperBound( aux, 0.0 );
//            TS_ASSERT( sigmoid.phaseFixed() );
//        }
//
//        // Aux variables: lower bound
//        {
//            SigmoidConstraint sigmoid( b, f );
//
//            sigmoid.notifyLowerBound( b, -5 );
//            InputQuery dontCare;
//            unsigned aux = 300;
//            dontCare.setNumberOfVariables( aux );
//            sigmoid.addAuxiliaryEquations( dontCare );
//
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyLowerBound( aux, 0.0 );
//            TS_ASSERT( !sigmoid.phaseFixed() );
//            sigmoid.notifyLowerBound( aux, 1.0 );
//            TS_ASSERT( sigmoid.phaseFixed() );
//        }
//    }
//
//    void test_valid_split_sigmoid_phase_fixed_to_active()
//    {
//        unsigned b = 1;
//        unsigned f = 4;
//
//        SigmoidConstraint sigmoid( b, f );
//
//        List<PiecewiseLinearConstraint::Fix> fixes;
//        List<PiecewiseLinearConstraint::Fix>::iterator it;
//
//        TS_ASSERT( !sigmoid.phaseFixed() );
//        TS_ASSERT_THROWS_NOTHING( sigmoid.notifyLowerBound( b, 5 ) );
//        TS_ASSERT( sigmoid.phaseFixed() );
//
//        PiecewiseLinearCaseSplit split;
//        TS_ASSERT_THROWS_NOTHING( split = sigmoid.getValidCaseSplit() );
//
//        Equation activeEquation;
//
//        List<Tightening> bounds = split.getBoundTightenings();
//
//        TS_ASSERT_EQUALS( bounds.size(), 1U );
//        auto bound = bounds.begin();
//        Tightening bound1 = *bound;
//
//        TS_ASSERT_EQUALS( bound1._variable, b );
//        TS_ASSERT_EQUALS( bound1._type, Tightening::LB );
//        TS_ASSERT_EQUALS( bound1._value, 0.0 );
//
//        auto equations = split.getEquations();
//        TS_ASSERT_EQUALS( equations.size(), 1U );
//        activeEquation = split.getEquations().front();
//        TS_ASSERT_EQUALS( activeEquation._addends.size(), 2U );
//        TS_ASSERT_EQUALS( activeEquation._scalar, 0.0 );
//
//        auto addend = activeEquation._addends.begin();
//        TS_ASSERT_EQUALS( addend->_coefficient, 1.0 );
//        TS_ASSERT_EQUALS( addend->_variable, b );
//
//        ++addend;
//        TS_ASSERT_EQUALS( addend->_coefficient, -1.0 );
//        TS_ASSERT_EQUALS( addend->_variable, f );
//
//        TS_ASSERT_EQUALS( activeEquation._type, Equation::EQ );
//    }
//
//    void test_valid_split_sigmoid_phase_fixed_to_inactive()
//    {
//        unsigned b = 1;
//        unsigned f = 4;
//
//        SigmoidConstraint sigmoid( b, f );
//
//        List<PiecewiseLinearConstraint::Fix> fixes;
//        List<PiecewiseLinearConstraint::Fix>::iterator it;
//
//        TS_ASSERT( !sigmoid.phaseFixed() );
//        TS_ASSERT_THROWS_NOTHING( sigmoid.notifyUpperBound( b, -2 ) );
//        TS_ASSERT( sigmoid.phaseFixed() );
//
//        PiecewiseLinearCaseSplit split;
//        TS_ASSERT_THROWS_NOTHING( split = sigmoid.getValidCaseSplit() );
//
//        Equation activeEquation;
//
//        List<Tightening> bounds = split.getBoundTightenings();
//
//        TS_ASSERT_EQUALS( bounds.size(), 2U );
//        auto bound = bounds.begin();
//        Tightening bound1 = *bound;
//
//        TS_ASSERT_EQUALS( bound1._variable, b );
//        TS_ASSERT_EQUALS( bound1._type, Tightening::UB );
//        TS_ASSERT_EQUALS( bound1._value, 0.0 );
//
//        ++bound;
//        Tightening bound2 = *bound;
//
//        TS_ASSERT_EQUALS( bound2._variable, f );
//        TS_ASSERT_EQUALS( bound2._value, 0.0 );
//        TS_ASSERT_EQUALS( bound2._type, Tightening::UB );
//
//        auto equations = split.getEquations();
//        TS_ASSERT( equations.empty() );
//    }
//
//    void test_sigmoid_entailed_tightenings()
//    {
//        unsigned b = 1;
//        unsigned f = 4;
//
//        InputQuery dontCare;
//        dontCare.setNumberOfVariables( 500 );
//        unsigned aux = 500;
//
//        SigmoidConstraint sigmoid( b, f );
//
//        sigmoid.notifyUpperBound( b, 7 );
//        sigmoid.notifyUpperBound( f, 7 );
//
//        sigmoid.notifyLowerBound( b, -1 );
//        sigmoid.notifyLowerBound( f, 0 );
//
//        sigmoid.addAuxiliaryEquations( dontCare );
//
//        sigmoid.notifyLowerBound( aux, 0 );
//        sigmoid.notifyUpperBound( aux, 1 );
//
//        List<Tightening> entailedTightenings;
//        sigmoid.getEntailedTightenings( entailedTightenings );
//
//        // The phase is not fixed: upper bounds propagated between b
//        // and f, b lower bound propagated to aux and vice versa, f
//        // and aux both non-negative
//        TS_ASSERT_EQUALS( entailedTightenings.size(), 6U );
//        TS_ASSERT( entailedTightenings.exists( Tightening( f, 7, Tightening::UB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( b, 7, Tightening::UB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( f, 0, Tightening::LB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( aux, 0, Tightening::LB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( b, -1, Tightening::LB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( aux, 1, Tightening::UB ) ) );
//
//        entailedTightenings.clear();
//
//        // Positive lower bounds for b and f: active case. All bounds
//        // propagated between f and b, and aux is set to 0. F and b are
//        // non-negative.
//        sigmoid.notifyLowerBound( b, 1 );
//        sigmoid.notifyLowerBound( f, 2 );
//
//        sigmoid.getEntailedTightenings( entailedTightenings );
//
//        TS_ASSERT_EQUALS( entailedTightenings.size(), 8U );
//        TS_ASSERT( entailedTightenings.exists( Tightening( f, 1, Tightening::LB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( b, 2, Tightening::LB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( f, 7, Tightening::UB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( b, 7, Tightening::UB ) ) );
//
//        TS_ASSERT( entailedTightenings.exists( Tightening( aux, 0, Tightening::LB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( aux, 0, Tightening::UB ) ) );
//
//        TS_ASSERT( entailedTightenings.exists( Tightening( b, 0, Tightening::LB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( f, 0, Tightening::LB ) ) );
//
//        entailedTightenings.clear();
//
//        // Negative upper bound for b: inactive case. F is 0, b =
//        // -aux. B is non-positive, aux is non-negative.
//
//        dontCare.setNumberOfVariables( 500 );
//        SigmoidConstraint sigmoid2( b, f );
//
//        sigmoid2.notifyUpperBound( b, -1 );
//        sigmoid2.notifyUpperBound( f, 7 );
//
//        sigmoid2.notifyLowerBound( b, -2 );
//        sigmoid2.notifyLowerBound( f, 0 );
//
//        sigmoid2.addAuxiliaryEquations( dontCare );
//
//        sigmoid2.notifyLowerBound( aux, 0 );
//        sigmoid2.notifyUpperBound( aux, 2 );
//
//        sigmoid2.getEntailedTightenings( entailedTightenings );
//
//        TS_ASSERT_EQUALS( entailedTightenings.size(), 8U );
//
//        TS_ASSERT( entailedTightenings.exists( Tightening( b, 0, Tightening::UB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( aux, 0, Tightening::LB ) ) );
//
//        TS_ASSERT( entailedTightenings.exists( Tightening( b, -2, Tightening::LB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( b, 0, Tightening::UB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( aux, 1, Tightening::LB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( aux, 2, Tightening::UB ) ) );
//
//        TS_ASSERT( entailedTightenings.exists( Tightening( f, 0, Tightening::LB ) ) );
//        TS_ASSERT( entailedTightenings.exists( Tightening( f, 0, Tightening::UB ) ) );
//    }
//
//    void test_sigmoid_duplicate_and_restore()
//    {
//        SigmoidConstraint *sigmoid1 = new SigmoidConstraint( 4, 6 );
//        sigmoid1->setActiveConstraint( false );
//        sigmoid1->notifyVariableValue( 4, 1.0 );
//        sigmoid1->notifyVariableValue( 6, 1.0 );
//
//        sigmoid1->notifyLowerBound( 4, -8.0 );
//        sigmoid1->notifyUpperBound( 4, 8.0 );
//
//        sigmoid1->notifyLowerBound( 6, 0.0 );
//        sigmoid1->notifyUpperBound( 6, 8.0 );
//
//        PiecewiseLinearConstraint *sigmoid2 = sigmoid1->duplicateConstraint();
//
//        sigmoid1->notifyVariableValue( 4, -2 );
//        TS_ASSERT( !sigmoid1->satisfied() );
//
//        TS_ASSERT( !sigmoid2->isActive() );
//        TS_ASSERT( sigmoid2->satisfied() );
//
//        sigmoid2->restoreState( sigmoid1 );
//        TS_ASSERT( !sigmoid2->satisfied() );
//
//        TS_ASSERT_THROWS_NOTHING( delete sigmoid1 );
//        TS_ASSERT_THROWS_NOTHING( delete sigmoid2 );
//    }
//
//    void test_eliminate_variable_active()
//    {
//        unsigned b = 1;
//        unsigned f = 4;
//
//        MockTableau tableau;
//
//        SigmoidConstraint sigmoid( b, f );
//
//        sigmoid.registerAsWatcher( &tableau );
//
//        TS_ASSERT( !sigmoid.constraintObsolete() );
//        TS_ASSERT_THROWS_NOTHING( sigmoid.eliminateVariable( b, 5 ) );
//        TS_ASSERT( sigmoid.constraintObsolete() );
//    }
//
//    void test_serialize_and_unserialize()
//    {
//        unsigned b = 42;
//        unsigned f = 7;
//
//        SigmoidConstraint originalRelu( b, f );
//        originalRelu.notifyLowerBound( b, -10 );
//        originalRelu.notifyUpperBound( f, 5 );
//        originalRelu.notifyUpperBound( f, 5 );
//
//        String originalSerialized = originalRelu.serializeToString();
//        SigmoidConstraint recoveredRelu( originalSerialized );
//
//        TS_ASSERT_EQUALS( originalRelu.serializeToString(),
//                          recoveredRelu.serializeToString() );
//
//        TS_ASSERT( !originalRelu.auxVariableInUse() );
//        TS_ASSERT( !recoveredRelu.auxVariableInUse() );
//
//        InputQuery dontCare;
//        dontCare.setNumberOfVariables( 500 );
//
//        originalRelu.addAuxiliaryEquations( dontCare );
//
//        TS_ASSERT( originalRelu.auxVariableInUse() );
//
//        originalSerialized = originalRelu.serializeToString();
//        SigmoidConstraint recoveredRelu2( originalSerialized );
//
//        TS_ASSERT_EQUALS( originalRelu.serializeToString(),
//                          recoveredRelu2.serializeToString() );
//
//        TS_ASSERT( recoveredRelu2.auxVariableInUse() );
//        TS_ASSERT_EQUALS( originalRelu.getAux(), recoveredRelu2.getAux() );
//    }
//
//    bool haveFix( List<PiecewiseLinearConstraint::Fix> &fixes, unsigned var, double value )
//    {
//        PiecewiseLinearConstraint::Fix targetFix( var, value );
//        for ( const auto &fix : fixes )
//        {
//            if ( fix == targetFix )
//                return true;
//        }
//
//        return false;
//    }
//
//    void test_sigmoid_smart_fixes()
//    {
//        unsigned b = 1;
//        unsigned f = 4;
//
//        SigmoidConstraint sigmoid( b, f );
//
//        MockTableau tableau;
//
//        List<PiecewiseLinearConstraint::Fix> fixes;
//        List<PiecewiseLinearConstraint::Fix>::iterator it;
//
//        // b and f are not linearly dependent. Return non-smart fixes
//
//        tableau.nextLinearlyDependentResult = false;
//
//        sigmoid.notifyVariableValue( b, -1 );
//        sigmoid.notifyVariableValue( f, 1 );
//
//        TS_ASSERT_THROWS_NOTHING( fixes = sigmoid.getSmartFixes( &tableau ) );
//
//        TS_ASSERT_EQUALS( fixes.size(), 2U );
//        TS_ASSERT( haveFix( fixes, b, 1 ) );
//        TS_ASSERT( haveFix( fixes, f, 0 ) );
//
//        TS_ASSERT_EQUALS( tableau.lastLinearlyDependentX1, b );
//        TS_ASSERT_EQUALS( tableau.lastLinearlyDependentX2, f );
//
//        // From now on, assume b and f are linearly dependent
//        tableau.nextLinearlyDependentResult = true;
//
//        // First, assume f is basic, b is non basic
//        tableau.nextIsBasic.insert( f );
//
//        double bDeltaToFDelta = -2;
//        tableau.nextLinearlyDependentCoefficient = bDeltaToFDelta;
//
//        sigmoid.notifyVariableValue( b, 5 );
//        sigmoid.notifyVariableValue( f, 2 );
//
//        // We expect b to decrease by 1, which will cause f to increase by 2
//        fixes.clear();
//        TS_ASSERT_THROWS_NOTHING( fixes = sigmoid.getSmartFixes( &tableau ) );
//
//        TS_ASSERT_EQUALS( fixes.size(), 1U );
//        TS_ASSERT( haveFix( fixes, b, 4 ) );
//
//        sigmoid.notifyVariableValue( b, 5 );
//        sigmoid.notifyVariableValue( f, 1 );
//
//        bDeltaToFDelta = 3;
//        tableau.nextLinearlyDependentCoefficient = bDeltaToFDelta;
//
//        // We expect b to increase to 7, so that f will catch up
//        fixes.clear();
//        TS_ASSERT_THROWS_NOTHING( fixes = sigmoid.getSmartFixes( &tableau ) );
//
//        TS_ASSERT_EQUALS( fixes.size(), 1U );
//        TS_ASSERT( haveFix( fixes, b, 7 ) );
//
//        // If the coefficient is 1, no fix is possible
//        bDeltaToFDelta = 1;
//        tableau.nextLinearlyDependentCoefficient = bDeltaToFDelta;
//
//        fixes.clear();
//        TS_ASSERT_THROWS_NOTHING( fixes = sigmoid.getSmartFixes( &tableau ) );
//
//        TS_ASSERT( fixes.empty() );
//
//        // Now a case where both fixes are possible
//        sigmoid.notifyVariableValue( b, -1 );
//        sigmoid.notifyVariableValue( f, 1 );
//
//        bDeltaToFDelta = 1.0 / 3;
//        tableau.nextLinearlyDependentCoefficient = bDeltaToFDelta;
//
//        // Option 1: decrease b by 3, so that f decreases by 1
//        // Option 2: increase b by 3, so that f increases by 1
//        fixes.clear();
//        TS_ASSERT_THROWS_NOTHING( fixes = sigmoid.getSmartFixes( &tableau ) );
//
//        TS_ASSERT_EQUALS( fixes.size(), 2U );
//        TS_ASSERT( haveFix( fixes, b, -4 ) );
//        TS_ASSERT( haveFix( fixes, b, 2 ) );
//
//        // Now, assume b is basic, f is non basic
//        tableau.nextIsBasic.clear();
//        tableau.nextIsBasic.insert( b );
//
//        double fDeltaToBDelta = -1.0 / 2;
//        tableau.nextLinearlyDependentCoefficient = 1.0 / fDeltaToBDelta;
//
//        sigmoid.notifyVariableValue( b, 5 );
//        sigmoid.notifyVariableValue( f, 2 );
//
//        // We expect f to increase by 2, which will cause b to decrease by 1
//        fixes.clear();
//        TS_ASSERT_THROWS_NOTHING( fixes = sigmoid.getSmartFixes( &tableau ) );
//
//        TS_ASSERT_EQUALS( fixes.size(), 1U );
//        TS_ASSERT( haveFix( fixes, f, 4 ) );
//
//        sigmoid.notifyVariableValue( b, 5 );
//        sigmoid.notifyVariableValue( f, 1 );
//
//        fDeltaToBDelta = 1.0 / 3;
//        tableau.nextLinearlyDependentCoefficient = 1.0 / fDeltaToBDelta;
//
//        // We expect f to increase to 7, so that f will catch up
//        fixes.clear();
//        TS_ASSERT_THROWS_NOTHING( fixes = sigmoid.getSmartFixes( &tableau ) );
//
//        TS_ASSERT_EQUALS( fixes.size(), 1U );
//        TS_ASSERT( haveFix( fixes, f, 7 ) );
//
//        // If the coefficient is 1, no fix is possible
//        fDeltaToBDelta = 1;
//        tableau.nextLinearlyDependentCoefficient = 1.0 / fDeltaToBDelta;
//
//        fixes.clear();
//        TS_ASSERT_THROWS_NOTHING( fixes = sigmoid.getSmartFixes( &tableau ) );
//
//        TS_ASSERT( fixes.empty() );
//
//        // Now a case where both fixes are possible
//        sigmoid.notifyVariableValue( b, -1 );
//        sigmoid.notifyVariableValue( f, 1 );
//
//        fDeltaToBDelta = 3;
//        tableau.nextLinearlyDependentCoefficient = 1 / fDeltaToBDelta;
//
//        // Option 1: f decreases by 1, so that b decreases by 3
//        // Option 2: f increases by 1, so that b increases by 3
//        fixes.clear();
//        TS_ASSERT_THROWS_NOTHING( fixes = sigmoid.getSmartFixes( &tableau ) );
//
//        TS_ASSERT_EQUALS( fixes.size(), 2U );
//        TS_ASSERT( haveFix( fixes, f, 0 ) );
//        TS_ASSERT( haveFix( fixes, f, 2 ) );
//    }
//
//    void test_add_auxiliary_equations()
//    {
//        SigmoidConstraint sigmoid( 4, 6 );
//        InputQuery query;
//
//        query.setNumberOfVariables( 9 );
//
//        sigmoid.notifyLowerBound( 4, -10 );
//        sigmoid.notifyLowerBound( 6, 0 );
//
//        sigmoid.notifyUpperBound( 4, 15 );
//        sigmoid.notifyUpperBound( 6, 15 );
//
//        TS_ASSERT_THROWS_NOTHING( sigmoid.addAuxiliaryEquations( query ) );
//
//        const List<Equation> &equations( query.getEquations() );
//
//        TS_ASSERT_EQUALS( equations.size(), 1U );
//        TS_ASSERT_EQUALS( query.getNumberOfVariables(), 10U );
//
//        unsigned aux = 9;
//        TS_ASSERT_EQUALS( query.getLowerBound( aux ), 0 );
//        TS_ASSERT_EQUALS( query.getUpperBound( aux ), 10 );
//
//        Equation eq = *equations.begin();
//
//        TS_ASSERT_EQUALS( eq._addends.size(), 3U );
//
//        auto it = eq._addends.begin();
//        TS_ASSERT_EQUALS( *it, Equation::Addend( 1, 6 ) );
//        ++it;
//        TS_ASSERT_EQUALS( *it, Equation::Addend( -1, 4 ) );
//        ++it;
//        TS_ASSERT_EQUALS( *it, Equation::Addend( -1, aux ) );
//
//        TS_ASSERT_EQUALS( eq._scalar, 0 );
//    }
//
//
//    SigmoidConstraint prepareRelu( unsigned b, unsigned f, unsigned aux, IConstraintBoundTightener *tightener )
//    {
//        SigmoidConstraint sigmoid( b, f );
//
//        InputQuery dontCare;
//        dontCare.setNumberOfVariables( aux );
//
//        sigmoid.notifyLowerBound( b, -10 );
//        sigmoid.notifyLowerBound( f, 0 );
//
//        sigmoid.notifyUpperBound( b, 15 );
//        sigmoid.notifyUpperBound( f, 15 );
//
//        TS_ASSERT_THROWS_NOTHING( sigmoid.addAuxiliaryEquations( dontCare ) );
//
//        sigmoid.notifyLowerBound( aux, 0 );
//        sigmoid.notifyUpperBound( aux, 10 );
//
//        sigmoid.registerConstraintBoundTightener( tightener );
//
//        return sigmoid;
//    }
//
//    void test_notify_bounds()
//    {
//        unsigned b = 1;
//        unsigned f = 4;
//        unsigned aux = 10;
//        MockConstraintBoundTightener tightener;
//        List<Tightening> tightenings;
//
//        tightener.getConstraintTightenings( tightenings );
//
//        // Initial state: b in [-10, 15], f in [0, 15], aux in [0, 10]
//
//        {
//            SigmoidConstraint sigmoid = prepareRelu( b, f, aux, &tightener );
//
//            sigmoid.notifyLowerBound( b, -20 );
//            tightener.getConstraintTightenings( tightenings );
//            TS_ASSERT( tightenings.empty() );
//
//            sigmoid.notifyLowerBound( f, -3 );
//            tightener.getConstraintTightenings( tightenings );
//            TS_ASSERT( tightenings.empty() );
//
//            sigmoid.notifyLowerBound( aux, -5 );
//            tightener.getConstraintTightenings( tightenings );
//            TS_ASSERT( tightenings.empty() );
//
//            sigmoid.notifyUpperBound( b, 20 );
//            tightener.getConstraintTightenings( tightenings );
//            TS_ASSERT( tightenings.empty() );
//
//            sigmoid.notifyUpperBound( f, 23 );
//            tightener.getConstraintTightenings( tightenings );
//            TS_ASSERT( tightenings.empty() );
//
//            sigmoid.notifyUpperBound( aux, 35 );
//            tightener.getConstraintTightenings( tightenings );
//            TS_ASSERT( tightenings.empty() );
//        }
//
//        {
//            // Tighter lower bound for b that is negative
//            SigmoidConstraint sigmoid = prepareRelu( b, f, aux, &tightener );
//            sigmoid.notifyLowerBound( b, -8 );
//            tightener.getConstraintTightenings( tightenings );
//            TS_ASSERT( tightenings.exists( Tightening( aux, 8, Tightening::UB ) ) );
//        }
//
//        {
//            // Tighter upper bound for aux that is positive
//            SigmoidConstraint sigmoid = prepareRelu( b, f, aux, &tightener );
//            sigmoid.notifyUpperBound( aux, 7 );
//            tightener.getConstraintTightenings( tightenings );
//            TS_ASSERT( tightenings.exists( Tightening( b, -7, Tightening::LB ) ) );
//        }
//
//        {
//            // Tighter upper bound for b/f that is positive
//            SigmoidConstraint sigmoid = prepareRelu( b, f, aux, &tightener );
//            sigmoid.notifyUpperBound( b, 13 );
//            tightener.getConstraintTightenings( tightenings );
//            TS_ASSERT( tightenings.exists( Tightening( f, 13, Tightening::UB ) ) );
//
//            sigmoid.notifyUpperBound( f, 12 );
//            tightener.getConstraintTightenings( tightenings );
//            TS_ASSERT( tightenings.exists( Tightening( b, 12, Tightening::UB ) ) );
//        }
//
//        {
//            // Tighter upper bound 0 for f
//            SigmoidConstraint sigmoid = prepareRelu( b, f, aux, &tightener );
//            sigmoid.notifyUpperBound( f, 0 );
//            tightener.getConstraintTightenings( tightenings );
//
//            TS_ASSERT( tightenings.exists( Tightening( b, 0, Tightening::UB ) ) );
//        }
//
//        {
//            // Tighter negative upper bound for b
//            SigmoidConstraint sigmoid = prepareRelu( b, f, aux, &tightener );
//            sigmoid.notifyUpperBound( b, -1 );
//            tightener.getConstraintTightenings( tightenings );
//
//            TS_ASSERT( tightenings.exists( Tightening( f, 0, Tightening::UB ) ) );
//            TS_ASSERT( tightenings.exists( Tightening( aux, 1, Tightening::LB ) ) );
//        }
//
//        {
//            // Tighter positive lower bound for aux
//            SigmoidConstraint sigmoid = prepareRelu( b, f, aux, &tightener );
//            sigmoid.notifyLowerBound( aux, 1 );
//            tightener.getConstraintTightenings( tightenings );
//
//            TS_ASSERT( tightenings.exists( Tightening( f, 0, Tightening::UB ) ) );
//            TS_ASSERT( tightenings.exists( Tightening( b, -1, Tightening::UB ) ) );
//        }
//    }

//
// Local Variables:
// compile-command: "make -C ../../.. "
// tags-file-name: "../../../TAGS"
// c-basic-offset: 4
// End:
//

