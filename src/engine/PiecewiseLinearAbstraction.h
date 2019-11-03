//
// Created by yoni_mantzur on 11/3/19.
//

#ifndef MARABOU_PIECEWISELINEARABSTRACTION_H
#define MARABOU_PIECEWISELINEARABSTRACTION_H

#include "List.h"
#include "Pair.h"
#include "Equation.h"
#include "PiecewiseLinearConstraint.h"

class PiecewiseLinearAbstraction {
public:
    /*
     * Refine the abstraction guided _listViolations
     */
    List<Equation> extractNewUpperEquations();

    List<Equation> extractNewLowerEquations();

    void addGuidedPoint(double b, double f);

    virtual PiecewiseLinearConstraint& getConstraint() = 0;

private:
    List<Pair<double, double>> _guidedPoints;
    List<Equation> _abstractedUpperEquations;
    List<Equation> _abstractedLowerEquations;

    Equation buildLinearEquationGivenTwoPoints(double x0, double y0, double x1, double y1);

};


#endif //MARABOU_PIECEWISELINEARABSTRACTION_H
