//
// Created by yoni_mantzur on 11/11/19.
//

#include <cxxtest/TestSuite.h>

#include "Engine.h"
#include "FloatUtils.h"
#include "InputQuery.h"
#include "SigmoidConstraint.h"


class SigmoidTestSuite : public CxxTest::TestSuite
{
public:
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void test_sigmoid_1()
    {
        //   0.5   <= x0  <= 1
        //   -1  <= x1f  <= 1
        //   -1 <= x3 <= 1
        //
        //  x0 - x1b = 0        -->  x0 - x1b  = 0
        //  x1f - x3 = 0  -->  x1f - x3  = 0
        //
        //  x1f = Sigmoid(x1b)
        //
        //   x0: x0
        //   x1: x1b
        //   x2: x1f
        //   x3: x3

        double sigLowerBound = GlobalConfiguration::SIGMOID_DEFAULT_LOWER_BOUND;
        double sigUpperBound = GlobalConfiguration::SIGMOID_DEFAULT_UPPER_BOUND;
        double large = 1000;

        InputQuery inputQuery;
        inputQuery.setNumberOfVariables( 4 );

        inputQuery.setLowerBound( 0, 0.5 );
        inputQuery.setUpperBound( 0, 1 );

        inputQuery.setLowerBound( 1, -large );
        inputQuery.setUpperBound( 1, large );

        inputQuery.setLowerBound( 2, sigLowerBound );
        inputQuery.setUpperBound( 2, sigUpperBound );

        inputQuery.setLowerBound( 3, sigLowerBound );
        inputQuery.setUpperBound( 3, sigUpperBound );


        // x0 - x1b = 0
        Equation equation1;
        equation1.addAddend( 1, 0 );
        equation1.addAddend( -1, 1 );
        equation1.setScalar( 0 );
        inputQuery.addEquation( equation1 );

        // x1f - x3 = 0
        Equation equation2;
        equation2.addAddend( 1, 2 );
        equation2.addAddend( -1, 3 );
        equation2.setScalar( 0 );
        inputQuery.addEquation( equation2 );

        SigmoidConstraint *sigmoid1 = new SigmoidConstraint( 1, 2 );

        inputQuery.addPiecewiseLinearConstraint( sigmoid1 );

        Engine engine;
        TS_ASSERT_THROWS_NOTHING ( engine.processInputQuery( inputQuery ) );
        TS_ASSERT_THROWS_NOTHING ( engine.solve() );

        engine.extractSolution( inputQuery );

        bool correctSolution = true;
        // Sanity test

        double value_x0 = inputQuery.getSolutionValue( 0 );
        double value_x1b = inputQuery.getSolutionValue( 1 );
        double value_x1f = inputQuery.getSolutionValue( 2 );
        double value_x3 = inputQuery.getSolutionValue( 3 );

        if ( !FloatUtils::areEqual( value_x0, value_x1b ) )
            correctSolution = false;


        if ( !FloatUtils::areEqual( value_x3, value_x1f ) )
            correctSolution = false;

        if ( FloatUtils::lt( value_x0, 0.5 ) || FloatUtils::gt( value_x0, 1 ) ||
             FloatUtils::lt( value_x1f, 0.62245 ) || FloatUtils::lt( value_x3, 0.62245)
             || FloatUtils::gt( value_x3, 1 ) )
        {
            correctSolution = false;
        }

        if ( !FloatUtils::areEqual( FloatUtils::sigmoid(value_x1b), value_x1f ) )
        {
            correctSolution = false;
        }

        TS_ASSERT ( correctSolution );
    }
};

