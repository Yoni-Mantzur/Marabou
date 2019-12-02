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

//    void test_sigmoid_1()
//    {
//        //   0.5   <= x0  <= 1
//        //   -large <= x1b <= large
//        //   -1  < x1f  < 1
//        //   -large < x3 < large
//        //
//        //  x0 - x1b = 0        -->  x0 - x1b  = 0
//        //  x1f - x3 = 0  -->  x1f - x3  = 0
//        //
//        //  x1f = Sigmoid(x1b)
//        //
//        //   x0: x0
//        //   x1: x1b
//        //   x2: x1f
//        //   x3: x3
//
//        double sigLowerBound = GlobalConfiguration::SIGMOID_DEFAULT_LOWER_BOUND;
//        double sigUpperBound = GlobalConfiguration::SIGMOID_DEFAULT_UPPER_BOUND;
//        double large = 1000;
//
//        InputQuery inputQuery;
//        inputQuery.setNumberOfVariables( 4 );
//
//        inputQuery.setLowerBound( 0, 0.5 );
//        inputQuery.setUpperBound( 0, 1 );
//
//        inputQuery.setLowerBound( 1, -large );
//        inputQuery.setUpperBound( 1, large );
//
//        inputQuery.setLowerBound( 2, sigLowerBound );
//        inputQuery.setUpperBound( 2, sigUpperBound );
//
//        inputQuery.setLowerBound( 3, sigLowerBound );
//        inputQuery.setUpperBound( 3, sigUpperBound );
//
//
//        // x0 - x1b = 0
//        Equation equation1;
//        equation1.addAddend( 1, 0 );
//        equation1.addAddend( -1, 1 );
//        equation1.setScalar( 0 );
//        inputQuery.addEquation( equation1 );
//
//        // x1f - x3 = 0
//        Equation equation2;
//        equation2.addAddend( 1, 2 );
//        equation2.addAddend( -1, 3 );
//        equation2.setScalar( 0 );
//        inputQuery.addEquation( equation2 );
//
//        SigmoidConstraint *sigmoid1 = new SigmoidConstraint( 1, 2 );
//
//        inputQuery.addPiecewiseLinearConstraint( sigmoid1 );
//
//        Engine engine;
//        TS_ASSERT_THROWS_NOTHING ( engine.processInputQuery( inputQuery ) );
//        TS_ASSERT_THROWS_NOTHING ( engine.solve() );
//
//        engine.extractSolution( inputQuery );
//
//        bool correctSolution = true;
//        // Sanity test
//
//        double value_x0 = inputQuery.getSolutionValue( 0 );
//        double value_x1b = inputQuery.getSolutionValue( 1 );
//        double value_x1f = inputQuery.getSolutionValue( 2 );
//        double value_x3 = inputQuery.getSolutionValue( 3 );
//
//        if ( !FloatUtils::areEqual( value_x0, value_x1b ) )
//            correctSolution = false;
//
//
//        if ( !FloatUtils::areEqual( value_x3, value_x1f ) )
//            correctSolution = false;
//
//        if ( FloatUtils::lt( value_x0, 0.5 ) || FloatUtils::gt( value_x0, 1 ) ||
//             FloatUtils::lt(value_x1b, 0.5 ) || FloatUtils::gt(value_x1b, 1) ||
//             FloatUtils::lt( value_x1f, FloatUtils::sigmoid(0.5) ) ||
//             FloatUtils::gt( value_x1f, FloatUtils::sigmoid(1) ) ||
//                FloatUtils::lt( value_x3, FloatUtils::sigmoid(0.5) ) ||
//                FloatUtils::gt( value_x3, FloatUtils::sigmoid(1) ) )
//        {
//            correctSolution = false;
//        }
//
//        if ( !FloatUtils::areEqual( FloatUtils::sigmoid(value_x1b), value_x1f ) )
//        {
//            correctSolution = false;
//        }
//
//        TS_ASSERT ( correctSolution );
//    }
//
//    void test_sigmoid_2()
//    {
//        //   0.5   <= x0  <= 1
//        //   0.5   <= x1  <= 1
//        //   -large <= x2b <= large
//        //   -1  < x2f  < 1
//        //   -large < x3 < large
//        //
//        //  x0 + x1 - x1b = 0
//        //  x1f - x3 = 0
//        //
//        //  x1f = Sigmoid(x1b)
//        //
//        //   x0: x0
//        //   x1: x1
//        //   x2: x2b
//        //   x3: x2f
//        //   x4: x3
//        double sigLowerBound = GlobalConfiguration::SIGMOID_DEFAULT_LOWER_BOUND;
//        double sigUpperBound = GlobalConfiguration::SIGMOID_DEFAULT_UPPER_BOUND;
//        double large = 1000;
//
//        InputQuery inputQuery;
//        inputQuery.setNumberOfVariables( 5 );
//
//        inputQuery.setLowerBound( 0, 0.5 );
//        inputQuery.setUpperBound( 0, 1 );
//
//        inputQuery.setLowerBound( 1, 0.5 );
//        inputQuery.setUpperBound( 1, 1 );
//
//        inputQuery.setLowerBound( 2, -large );
//        inputQuery.setUpperBound( 2, large );
//
//        inputQuery.setLowerBound( 3, sigLowerBound );
//        inputQuery.setUpperBound( 3, sigUpperBound );
//
//        inputQuery.setLowerBound( 4, -large );
//        inputQuery.setUpperBound( 4, large );
//
//        Equation equation1;
//        equation1.addAddend( 1, 0 );
//        equation1.addAddend( 1, 1 );
//        equation1.addAddend( -1, 2 );
//        equation1.setScalar( 0 );
//        inputQuery.addEquation( equation1 );
//
//        Equation equation2;
//        equation2.addAddend( 1, 3 );
//        equation2.addAddend( -1, 4 );
//        equation2.setScalar( 0 );
//        inputQuery.addEquation( equation2 );
//
//
//        auto *sigmoid = new SigmoidConstraint( 2, 3 );
//
//        inputQuery.addPiecewiseLinearConstraint( sigmoid );
//
//        Engine engine;
//        TS_ASSERT_THROWS_NOTHING( engine.processInputQuery( inputQuery ) );
//
//        TS_ASSERT_THROWS_NOTHING ( engine.solve() );
//
//
//        engine.extractSolution( inputQuery );
//
//        bool correctSolution = true;
//        // Sanity test
//
//        double value_x0 = inputQuery.getSolutionValue( 0 );
//        double value_x1 = inputQuery.getSolutionValue( 1 );
//        double value_x2b = inputQuery.getSolutionValue( 2 );
//        double value_x2f = inputQuery.getSolutionValue( 3 );
//        double value_x3 = inputQuery.getSolutionValue( 4 );
//
//        if ( !FloatUtils::areEqual( value_x0 + value_x1, value_x2b ) )
//            correctSolution = false;
//
//        if ( !FloatUtils::areEqual( value_x2f, value_x3 ) )
//            correctSolution = false;
//
//        if ( FloatUtils::lt( value_x0, 0.5) || FloatUtils::gt( value_x0, 1 ) ||
//             FloatUtils::lt( value_x1, 0.5) || FloatUtils::gt( value_x1, 1 ) ||
//             FloatUtils::lt( value_x2b, 1 ) || FloatUtils::gt( value_x2b, 2 ) ||
//             FloatUtils::lt( value_x2f, FloatUtils::sigmoid(1) ) ||
//             FloatUtils::gt( value_x2f, FloatUtils::sigmoid(2) ) ||
//             FloatUtils::lt( value_x3, FloatUtils::sigmoid(1) ) ||
//             FloatUtils::gt( value_x3, FloatUtils::sigmoid(2) ) )
//        {
//            correctSolution = false;
//        }
//
//        if ( !FloatUtils::areEqual(value_x2f, FloatUtils::sigmoid(value_x2b)) )
//        {
//            correctSolution = false;
//        }
//
//
//        TS_ASSERT ( correctSolution );
//    }
//
//    void test_sigmoid_3()
//    {
//        //   0.5   <= x0  <= 1
//        //   0.5   <= x1  <= 1
//        //   -large <= x2b <= large
//        //   -1  < x2f  < 1
//        //   -large <= x3b <= large
//        //   -1  < x3f  < 1
//        //   -large < x3 < large
//        //
//        //  x0 - x2b = 0
//        //  x1 - x3b = 0
//        //  x2f + x3f - x4 = 0
//        //
//        //  x2f = Sigmoid(x2b)
//        //  x3f = Sigmoid(x3b)
//        //
//        //   x0: x0
//        //   x1: x1
//        //   x2: x2b
//        //   x3: x2f
//        //   x4: x3b
//        //   x5: x3f
//        //   x6: x4
//        double sigLowerBound = GlobalConfiguration::SIGMOID_DEFAULT_LOWER_BOUND;
//        double sigUpperBound = GlobalConfiguration::SIGMOID_DEFAULT_UPPER_BOUND;
//        double large = 1000;
//
//        InputQuery inputQuery;
//        inputQuery.setNumberOfVariables( 7 );
//
//        inputQuery.setLowerBound( 0, 0.5 );
//        inputQuery.setUpperBound( 0, 1 );
//
//        inputQuery.setLowerBound( 1, 0.5 );
//        inputQuery.setUpperBound( 1, 1 );
//
//        inputQuery.setLowerBound( 2, -large );
//        inputQuery.setUpperBound( 2, large );
//
//        inputQuery.setLowerBound( 3, -sigLowerBound );
//        inputQuery.setUpperBound( 3, sigUpperBound );
//
//        inputQuery.setLowerBound( 4, -large );
//        inputQuery.setUpperBound( 4, large );
//
//        inputQuery.setLowerBound( 5, sigLowerBound );
//        inputQuery.setUpperBound( 5, sigUpperBound );
//
//        inputQuery.setLowerBound( 6, -large );
//        inputQuery.setUpperBound( 6, large );
//
//        Equation equation1;
//        equation1.addAddend( 1, 0 );
//        equation1.addAddend( -1, 2 );
//        equation1.setScalar( 0 );
//        inputQuery.addEquation( equation1 );
//
//        Equation equation2;
//        equation2.addAddend( 1, 1 );
//        equation2.addAddend( -1, 4 );
//        equation2.setScalar( 0 );
//        inputQuery.addEquation( equation2 );
//
//        Equation equation3;
//        equation3.addAddend( 1, 3 );
//        equation3.addAddend( 1, 5 );
//        equation3.addAddend( -1, 6 );
//        equation3.setScalar( 0 );
//        inputQuery.addEquation( equation3 );
//
//
//        auto *sigmoid1 = new SigmoidConstraint( 2, 3 );
//        auto *sigmoid2 = new SigmoidConstraint( 4, 5 );
//
//        inputQuery.addPiecewiseLinearConstraint( sigmoid1 );
//        inputQuery.addPiecewiseLinearConstraint( sigmoid2 );
//
//        Engine engine;
//        TS_ASSERT_THROWS_NOTHING( engine.processInputQuery( inputQuery ) );
//
//        TS_ASSERT_THROWS_NOTHING ( engine.solve() );
//
//
//        engine.extractSolution( inputQuery );
//
//        bool correctSolution = true;
//        // Sanity test
//
//        double value_x0 = inputQuery.getSolutionValue( 0 );
//        double value_x1 = inputQuery.getSolutionValue( 1 );
//        double value_x2b = inputQuery.getSolutionValue( 2 );
//        double value_x2f = inputQuery.getSolutionValue( 3 );
//        double value_x3b = inputQuery.getSolutionValue( 4 );
//        double value_x3f = inputQuery.getSolutionValue( 5 );
//        double value_x4 = inputQuery.getSolutionValue( 6 );
//
//        if ( !FloatUtils::areEqual( value_x0, value_x2b ) )
//            correctSolution = false;
//
//        if ( !FloatUtils::areEqual( value_x1, value_x3b ) )
//            correctSolution = false;
//
//        if ( !FloatUtils::areEqual( value_x2f + value_x3f, value_x4 ) )
//            correctSolution = false;
//
//        if ( FloatUtils::lt( value_x0, 0.5) || FloatUtils::gt( value_x0, 1 ) ||
//             FloatUtils::lt( value_x1, 0.5) || FloatUtils::gt( value_x1, 1 ) ||
//             FloatUtils::lt( value_x2b, 0.5 ) || FloatUtils::gt( value_x2b, 1 ) ||
//             FloatUtils::lt( value_x3b, 0.5 ) || FloatUtils::gt( value_x3b, 1 ) ||
//             FloatUtils::lt( value_x2f, FloatUtils::sigmoid(0.5) ) ||
//             FloatUtils::gt( value_x2f, FloatUtils::sigmoid(1) )  ||
//             FloatUtils::lt( value_x3f, FloatUtils::sigmoid(0.5) ) ||
//             FloatUtils::gt( value_x3f, FloatUtils::sigmoid(1) ) ||
//             FloatUtils::lt( value_x4, FloatUtils::sigmoid(1) ) ||
//             FloatUtils::gt( value_x4, FloatUtils::sigmoid(2) ) )
//        {
//            correctSolution = false;
//        }
//
//        if ( !FloatUtils::areEqual(value_x2f, FloatUtils::sigmoid(value_x2b)) )
//        {
//            correctSolution = false;
//        }
//
//        if ( !FloatUtils::areEqual(value_x3f, FloatUtils::sigmoid(value_x3b)) )
//        {
//            correctSolution = false;
//        }
//
//
//        TS_ASSERT ( correctSolution );
//    }

//    void test_sigmoid_4()
//    {
//        //   0.5   <= x0  <= 1
//        //   0.5   <= x1  <= 1
//        //   -large <= x2b <= large
//        //   -1  < x2f  < 1
//        //   -large <= x3b <= large
//        //   -1  < x3f  < 1
//        //   -large < x4b < large
//        //   -large < x4f < large
//        //
//        //   query:
//        //   -large < x5 < large
//        //
//        //  x0 - x2b = 0
//        //  x1 - x3b = 0
//        //  x2f + x3f - x4b = 0
//        //  x4f - x5 = 0
//        //
//        //  x2f = Sigmoid(x2b)
//        //  x3f = Sigmoid(x3b)
//        //  x4f = Sigmoid(x3b)
//        //
//        //   x0: x0
//        //   x1: x1
//        //   x2: x2b
//        //   x3: x2f
//        //   x4: x3b
//        //   x5: x3f
//        //   x6: x4b
//        //   x7: x4f
//        //   x8: x5
//
//        double output_lower = FloatUtils::sigmoid(1.31);
//        double output_upper = FloatUtils::sigmoid(1.35);
//
//        InputQuery inputQuery;
//        inputQuery.setNumberOfVariables( 9 );
//
//        inputQuery.setLowerBound( 0, 0.5 );
//        inputQuery.setUpperBound( 0, 1.0 );
//
//        inputQuery.setLowerBound( 1, 0.5 );
//        inputQuery.setUpperBound( 1, 1.0 );
//
//        inputQuery.setLowerBound( 8, output_lower);
//        inputQuery.setUpperBound( 8,  output_upper);
//
////        inputQuery.markInputVariable(0, 0);
////        inputQuery.markInputVariable(1, 1);
////        inputQuery.markOutputVariable(8, 0);
//
//        Equation equation1;
//        equation1.addAddend( 1, 0 );
//        equation1.addAddend( -1, 2 );
//        equation1.setScalar( 0 );
//        inputQuery.addEquation( equation1 );
//
//        Equation equation2;
//        equation2.addAddend( 1, 1 );
//        equation2.addAddend( -1, 4 );
//        equation2.setScalar( 0 );
//        inputQuery.addEquation( equation2 );
//
//        Equation equation3;
//        equation3.addAddend( 1, 7 );
//        equation3.addAddend( -1, 8 );
//        equation3.setScalar( 0 );
//        inputQuery.addEquation( equation3 );
//
//        Equation equation4;
//        equation4.addAddend( 1, 3 );
//        equation4.addAddend( 1, 5 );
//        equation4.addAddend( -1, 6 );
//        equation4.setScalar( 0 );
//        inputQuery.addEquation( equation4 );
//
//
//        auto *sigmoid1 = new SigmoidConstraint( 2, 3 );
//        auto *sigmoid2 = new SigmoidConstraint( 4, 5 );
//        auto *sigmoid3 = new SigmoidConstraint( 6, 7 );
//
//        sigmoid1->setLogFile(new File("log/log_test_4"));
//        sigmoid2->setLogFile(new File("log/log_test_4"));
//        sigmoid3->setLogFile(new File("log/log_test_4"));
//
//        inputQuery.addPiecewiseLinearConstraint( sigmoid1 );
//        inputQuery.addPiecewiseLinearConstraint( sigmoid2 );
//        inputQuery.addPiecewiseLinearConstraint( sigmoid3 );
//
//        Engine engine;
//        TS_ASSERT_THROWS_NOTHING( engine.processInputQuery( inputQuery ) );
//
//        TS_ASSERT_THROWS_NOTHING ( engine.solve() );
//
//
//        engine.extractSolution( inputQuery );
//
//        bool correctSolution = true;
//        // Sanity test
//
//        double value_x0 = inputQuery.getSolutionValue( 0 );
//        double value_x1 = inputQuery.getSolutionValue( 1 );
//        double value_x2b = inputQuery.getSolutionValue( 2 );
//        double value_x2f = inputQuery.getSolutionValue( 3 );
//        double value_x3b = inputQuery.getSolutionValue( 4 );
//        double value_x3f = inputQuery.getSolutionValue( 5 );
//        double value_x4b = inputQuery.getSolutionValue( 6 );
//        double value_x4f = inputQuery.getSolutionValue( 7 );
//        double value_x5 = inputQuery.getSolutionValue( 8 );
//
//
//        if ( !FloatUtils::areEqual( value_x0, value_x2b ) )
//            correctSolution = false;
//
//        if ( !FloatUtils::areEqual( value_x1, value_x3b ) )
//            correctSolution = false;
//
//        if ( !FloatUtils::areEqual( value_x3f + value_x2f, value_x4b ) )
//            correctSolution = false;
//
//        if ( !FloatUtils::areEqual( value_x4f, value_x5 ) )
//            correctSolution = false;
//
//        if ( FloatUtils::lt( value_x0, 0.5) || FloatUtils::gt( value_x0, 1 ) ||
//             FloatUtils::lt( value_x1, 0.5) || FloatUtils::gt( value_x1, 1 ) ||
//             FloatUtils::lt( value_x2b, 0.5 ) || FloatUtils::gt( value_x2b, 1 ) ||
//             FloatUtils::lt( value_x3b, 0.5 ) || FloatUtils::gt( value_x3b, 1 ) ||
//             FloatUtils::lt( value_x2f, FloatUtils::sigmoid(0.5) ) ||
//             FloatUtils::gt( value_x2f, FloatUtils::sigmoid(1) )  ||
//             FloatUtils::lt( value_x3f, FloatUtils::sigmoid(0.5) ) ||
//             FloatUtils::gt( value_x3f, FloatUtils::sigmoid(1) ) ||
//             FloatUtils::lt( value_x4b, 2*FloatUtils::sigmoid(0.5) ) ||
//             FloatUtils::gt( value_x4b, 2*FloatUtils::sigmoid(1) )   ||
//             FloatUtils::lt( value_x4f, output_lower ) ||
//             FloatUtils::gt( value_x4f, output_upper ) )
//        {
//            correctSolution = false;
//        }
//
//        if ( !FloatUtils::areEqual(value_x2f, FloatUtils::sigmoid(value_x2b), GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
//        {
//            correctSolution = false;
//        }
//
//        if ( !FloatUtils::areEqual(value_x3f, FloatUtils::sigmoid(value_x3b), GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
//        {
//            correctSolution = false;
//        }
//
//        if ( !FloatUtils::areEqual(value_x4f, FloatUtils::sigmoid(value_x4b), GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE) )
//        {
//            correctSolution = false;
//        }
//
//        TS_ASSERT ( correctSolution );
//    }

    void test_sigmoid_5()
    {

        double output_lower = 0.833;
        double output_upper = 0.844;

        InputQuery inputQuery;
        inputQuery.setNumberOfVariables( 8 );

        inputQuery.setLowerBound( 0, 0.5 );
        inputQuery.setUpperBound( 0, 1.0 );

        inputQuery.setLowerBound( 1, 0.5 );
        inputQuery.setUpperBound( 1, 1.0 );

        inputQuery.setLowerBound( 7, output_lower);
        inputQuery.setUpperBound( 7,  output_upper);

        // X0+X1-X2=0
        Equation equation1;
        equation1.addAddend( 1, 0 );
        equation1.addAddend( 1, 1 );
        equation1.addAddend( -1, 2 );
        equation1.setScalar( 0 );
        inputQuery.addEquation( equation1 );

        // X0+X1-X3=0
        Equation equation2;
        equation2.addAddend( 1, 0 );
        equation2.addAddend( 1, 1 );
        equation2.addAddend( -1, 3 );
        equation2.setScalar( 0 );
        inputQuery.addEquation( equation2 );


        // X4+X5-X6=0
        Equation equation6;
        equation6.addAddend( 1, 4 );
        equation6.addAddend( 1, 5 );
        equation6.addAddend( -1, 6 );
        equation6.setScalar( 0 );
        inputQuery.addEquation( equation6 );



        auto *sigmoid1 = new SigmoidConstraint( 2, 4 );
        auto *sigmoid2 = new SigmoidConstraint( 3, 5 );
        auto *sigmoid3 = new SigmoidConstraint( 6, 7 );

        sigmoid1->setLogFile(new File("log/log_test_5"));
        sigmoid2->setLogFile(new File("log/log_test_5"));
        sigmoid3->setLogFile(new File("log/log_test_5"));

        inputQuery.addPiecewiseLinearConstraint( sigmoid1 );
        inputQuery.addPiecewiseLinearConstraint( sigmoid2 );
        inputQuery.addPiecewiseLinearConstraint( sigmoid3 );


        Engine engine;
        TS_ASSERT_THROWS_NOTHING( engine.processInputQuery( inputQuery ) );

        TS_ASSERT_THROWS_NOTHING ( engine.solve() );


        engine.extractSolution( inputQuery );

        bool correctSolution = true;
        // Sanity test

        double value_x0 = inputQuery.getSolutionValue( 0 );
        double value_x1 = inputQuery.getSolutionValue( 1 );
        double value_x2b = inputQuery.getSolutionValue( 2 );
        double value_x3b = inputQuery.getSolutionValue( 3 );
        double value_x2f = inputQuery.getSolutionValue( 4 );
        double value_x3f = inputQuery.getSolutionValue( 5 );
        double value_x4b = inputQuery.getSolutionValue( 6 );
        double value_x4f = inputQuery.getSolutionValue( 7 );


        if ( !FloatUtils::areEqual( value_x0 + value_x1, value_x2b ) )
            correctSolution = false;

        if ( !FloatUtils::areEqual( value_x0 + value_x1, value_x3b ) )
            correctSolution = false;

        if ( !FloatUtils::areEqual( value_x3f + value_x2f, value_x4b ) )
            correctSolution = false;


        if ( FloatUtils::lt( value_x0, 0.5) || FloatUtils::gt( value_x0, 1 ) ||
             FloatUtils::lt( value_x1, 0.5) || FloatUtils::gt( value_x1, 1 ) ||
             FloatUtils::lt( value_x2b, 1 ) || FloatUtils::gt( value_x2b, 2 ) ||
             FloatUtils::lt( value_x3b, 1 ) || FloatUtils::gt( value_x3b, 2 ) ||
             FloatUtils::lt( value_x2f, FloatUtils::sigmoid(1) ) ||
             FloatUtils::gt( value_x2f, FloatUtils::sigmoid(2) )  ||
             FloatUtils::lt( value_x3f, FloatUtils::sigmoid(1) ) ||
             FloatUtils::gt( value_x3f, FloatUtils::sigmoid(2) ) ||
             FloatUtils::lt( value_x4b, 2*FloatUtils::sigmoid(1) ) ||
             FloatUtils::gt( value_x4b, 2*FloatUtils::sigmoid(2) )   ||
             FloatUtils::lt( value_x4f, output_lower ) ||
             FloatUtils::gt( value_x4f, output_upper ) )
        {
            correctSolution = false;
        }

        if ( !FloatUtils::areEqual(value_x2f, FloatUtils::sigmoid(value_x2b), GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
        {
            correctSolution = false;
        }

        if ( !FloatUtils::areEqual(value_x3f, FloatUtils::sigmoid(value_x3b), GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
        {
            correctSolution = false;
        }

        if ( !FloatUtils::areEqual(value_x4f, FloatUtils::sigmoid(value_x4b), GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE) )
        {
            correctSolution = false;
        }

        TS_ASSERT ( correctSolution );
    }
};

