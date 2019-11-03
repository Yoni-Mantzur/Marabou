//
// Created by yoni_mantzur on 11/3/19.
//

#include "PiecewiseLinearAbstraction.h"

List<Equation> PiecewiseLinearAbstraction::extractNewLowerEquations()
{
    List<Equation> refinedEquations;

    auto guidedPointsIter =_guidedPoints.begin();
    double x0 = guidedPointsIter->first(), y0 = guidedPointsIter->second();
    double x1, y1;
    while (++guidedPointsIter != _guidedPoints.end())
    {
        x1 = guidedPointsIter->first(), y1 = guidedPointsIter->second();

        Equation newLowerEquation = buildLinearEquationGivenTwoPoints(x0, y0, x1, y1);
        refinedEquations.append(newLowerEquation);

        x0 = x1, y0 = y1;
    }
    _abstractedUpperEquations.append(refinedEquations);
    return refinedEquations;
}

Equation PiecewiseLinearAbstraction::buildLinearEquationGivenTwoPoints(double x0, double y0,
                                                                       double x1, double y1)
{
    List<unsigned>  vars = getConstraint().getParticipatingVariables();

    unsigned b = vars.back(), f = vars.front();
    double slot = (y1 - y0) / (x1 - x0);

    Equation equation;
    equation.addAddend(1, f);
    equation.addAddend(-slot, b);
    equation.setScalar(y0 - slot * x0);

    return equation;
}


void PiecewiseLinearAbstraction::addGuidedPoint(double b, double f)
{
    _guidedPoints.append(Pair<double, double>(b, f));
}

