//
// Created by yoni_mantzur on 11/3/19.
//

#include "PiecewiseLinearAbstraction.h"


List<PiecewiseLinearCaseSplit> PiecewiseLinearAbstraction::getRefinedLowerAbstraction(List<GuidedPoint> &guidedPoints) const
{
    List<PiecewiseLinearCaseSplit> splits;
    auto guidedPointsIter = guidedPoints.begin();
    auto guidedPointsIterReverse = guidedPoints.rbegin();

    GuidedPoint p1  = *guidedPointsIter;
    GuidedPoint lowerBound = p1, upperBound = *guidedPointsIterReverse;
    while (++guidedPointsIter != guidedPoints.end())
    {
        GuidedPoint p2 = *guidedPointsIter;

        // We're w/o guided points, choosing one randomly
        if (p1 == lowerBound && p2 == upperBound)
        {
            double x = (p1.x + p2.x) / 2;
            double y = evaluateConciseFunction(x);
            p2 = GuidedPoint(x, y);
            guidedPointsIter--;
        }

        // Invalid guided points due to bounds were changed
        if (p2 > upperBound || p2 < lowerBound)
            continue;

        // Same assignment as guided point
        if ( p1 == p2 )
        {
            guidedPointsIter++;
            double x = ((*guidedPointsIter).x + p2.x) / 2;
            double y = evaluateConciseFunction(x);
            p2 = GuidedPoint(x, y);
            guidedPointsIter--;
        }
        std::cout << ("p1: ");
        std::cout << ("(" + std::to_string(p1.x) + ", " + std::to_string(p1.y) + ")\n");
        std::cout << ("p2: ");
        std::cout << ("(" + std::to_string(p2.x) + ", " + std::to_string(p2.y) + ")\n");


        PiecewiseLinearCaseSplit split;

        Equation lowerEquation = buildLinearEquationGivenTwoPoints(p1, p2);
        lowerEquation.setType(Equation::GE);
        split.addEquation(lowerEquation);
        for (Tightening tightening: boundVars(p1, p2))
            split.storeBoundTightening(tightening);

        splits.append(split);
        p1 = p2;
    }

    return splits;
}

List<PiecewiseLinearCaseSplit> PiecewiseLinearAbstraction::getRefinedUpperAbstraction(__attribute__((unused))List<GuidedPoint> &guidedPoints) const
{
    return List<PiecewiseLinearCaseSplit>();
}

List<Tightening> PiecewiseLinearAbstraction::boundVars(GuidedPoint p1, GuidedPoint p2) const{
    List<Tightening> bounds;
    unsigned b = getB(), f = getF();

    // Bound b
    bounds.append(Tightening (b, p1.x, Tightening::LB));
    bounds.append(Tightening(b, p2.x, Tightening::UB));

    // Bound f
    bounds.append(Tightening(f, p1.y, Tightening::LB));
    bounds.append(Tightening(f, p2.y, Tightening::UB));

    std::cout << ("b bound: ");
    std::cout << ("(" + std::to_string(p1.x) + ", " + std::to_string(p2.x) + ")\n");
    std::cout << ("f bound: ");
    std::cout << ("(" + std::to_string(p1.y) + ", " + std::to_string(p2.y) + ")\n");

    return bounds;
}

Equation PiecewiseLinearAbstraction::buildLinearEquationGivenTwoPoints(GuidedPoint p1, GuidedPoint p2) const
{
    unsigned b = getB(), f = getF();
    double x0 = p1.x, y0 = p1.y, x1 = p2.x, y1 = p2.y;
    double slot = (y1 - y0) / (x1 - x0);

    Equation equation;
    equation.addAddend(1, f);
    equation.addAddend(-slot, b);
    equation.setScalar(y0 - slot * x0);

    return equation;
}


List<PiecewiseLinearCaseSplit> PiecewiseLinearAbstraction::getRefinedSplits(List<GuidedPoint> guidedPoints) const
{
    List<PiecewiseLinearCaseSplit> splits;
    splits.append(getRefinedLowerAbstraction(guidedPoints));
//    splits.append(getRefinedUpperAbstraction(guidedPoints));

    return splits;
}


