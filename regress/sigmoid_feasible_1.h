//
// Created by yoni_mantzur on 11/11/19.
//

#ifndef MARABOU_SIGMOID_FEASIBLE_1_H
#define MARABOU_SIGMOID_FEASIBLE_1_H

#include "Engine.h"
#include "FloatUtils.h"
#include "InputQuery.h"
#include "SigmoidConstraint.h"
#include "RegressUtils.h"

class Sigmoid_Feasible_1 {
public:
    void run()
    {
        //   0.5   <= x0  <= 1
        //   -1  <= x1f  <= 1
        //   -1 <= x3 <= 1
        //
        //  x0 - x1b = 0        -->  x0 - x1b + x4 = 0
        //  x1f - x3 = 0  -->  x1f - x3 + x5 = 0
        //
        //  x1f = Sigmoid(x1b)
        //
        //   x0: x0
        //   x1: x1b
        //   x2: x1f
        //   x3: x3

        InputQuery inputQuery;
        inputQuery.setNumberOfVariables( 6 );

        inputQuery.setLowerBound( 0, 0.5 );
        inputQuery.setUpperBound( 0, 1 );

        inputQuery.setLowerBound( 1, -1 );
        inputQuery.setUpperBound( 1, 1 );

        inputQuery.setLowerBound( 2, -1 );
        inputQuery.setUpperBound( 2, 1 );

        inputQuery.setLowerBound( 3, -1 );
        inputQuery.setUpperBound( 3, 1 );


        inputQuery.setLowerBound( 4, 0 );
        inputQuery.setUpperBound( 4, 0 );
        inputQuery.setLowerBound( 5, 0 );
        inputQuery.setUpperBound( 5, 0 );

        // x0 - x1b + x4 = 0
        Equation equation1;
        equation1.addAddend( 1, 0 );
        equation1.addAddend( -1, 1 );
        equation1.addAddend( 1, 4 );
        equation1.setScalar( 0 );
        inputQuery.addEquation( equation1 );

        // x1f - x3 + x5 = 0
        Equation equation2;
        equation2.addAddend( 1, 2 );
        equation2.addAddend( -1, 3 );
        equation2.addAddend( 1, 5 );
        equation2.setScalar( 0 );
        inputQuery.addEquation( equation2 );

        SigmoidConstraint *sigmoid1 = new SigmoidConstraint( 1, 2 );

        inputQuery.addPiecewiseLinearConstraint( sigmoid1 );

        int outputStream = redirectOutputToFile( "logs/sigmoid_feasible_1.txt" );

        struct timespec start = TimeUtils::sampleMicro();

        Engine engine;
        if ( !engine.processInputQuery( inputQuery ) )
        {
            struct timespec end = TimeUtils::sampleMicro();
            restoreOutputStream( outputStream );
            printFailed( "sigmoid_feasible_1", start, end );
            return;
        }

        bool result = engine.solve();

        struct timespec end = TimeUtils::sampleMicro();

        restoreOutputStream( outputStream );

        if ( !result )
        {
            printFailed( "sigmoid_feasible_1", start, end );
            return;
        }

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

        if ( !correctSolution )
            printFailed( "sigmoid_feasible_1", start, end );
        else
            printPassed( "sigmoid", start, end );
    }
};


#endif //MARABOU_SIGMOID_FEASIBLE_1_H
