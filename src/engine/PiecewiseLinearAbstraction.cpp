//
// Created by yoni_mantzur on 11/3/19.
//

#include "PiecewiseLinearAbstraction.h"

void PiecewiseLinearAbstraction::refineLowerAbstraction()
{
    auto guidedPointsIter =_guidedPoints.begin();
    GuidedPoint p1  = *guidedPointsIter;
    while (++guidedPointsIter != _guidedPoints.end())
    {
        GuidedPoint p2 = *guidedPointsIter;
        PiecewiseLinearCaseSplit split;

        Equation lowerEquation = buildLinearEquationGivenTwoPoints(p1, p2);
        lowerEquation.setType(Equation::GE);
        split.addEquation(buildLinearEquationGivenTwoPoints(p1, p2));
        for (Tightening tightening: boundVars(p1, p2))
            split.storeBoundTightening(tightening);

        _abstractedLowerSplits.append(split);

        p1 = p2;
    }
}

void PiecewiseLinearAbstraction::refineUpperAbstraction()
{
}

List<Tightening> PiecewiseLinearAbstraction::boundVars(GuidedPoint p1, GuidedPoint p2) {
    List<Tightening> bounds;
    unsigned b = getB(), f = getF();

    // Bound b
    bounds.append(Tightening (b, p1.x, Tightening::LB));
    bounds.append(Tightening(b, p2.x, Tightening::UB));

    // Bound f
    bounds.append(Tightening(f, p1.y, Tightening::LB));
    bounds.append(Tightening(f, p2.y, Tightening::UB));

    return bounds;
}

Equation PiecewiseLinearAbstraction::buildLinearEquationGivenTwoPoints(GuidedPoint p1, GuidedPoint p2)
{
    unsigned b = getB(), f = getF();
    double x0 = p1.x, y0 = p1.y, x1 = p2.x, y1 = p2.y;
    double slot = (y1 - y0) / (x1 - x0);

    Equation equation;
    equation.addAddend(slot, b);
    equation.addAddend(-1, f);
    equation.setScalar(slot * x0 - y0);

    return equation;
}

void PiecewiseLinearAbstraction::addGuidedPoint(GuidedPoint p)
{
    _guidedPoints.append(p);
}

void PiecewiseLinearAbstraction::refine()
{
    refineLowerAbstraction();
    refineUpperAbstraction();
}

List<PiecewiseLinearCaseSplit> PiecewiseLinearAbstraction::getLowerSplits() const
{
    return _abstractedLowerSplits;
}

