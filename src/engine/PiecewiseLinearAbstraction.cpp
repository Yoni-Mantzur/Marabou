//
// Created by yoni_mantzur on 11/3/19.
//

#include <boost/concept_check.hpp>
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

        Equation lowerEquation = getLinearEquation(p1, p2);
        lowerEquation.setType(Equation::GE);
        split.addEquation(lowerEquation);
        for (Tightening tightening: boundVars(p1, p2))
            split.storeBoundTightening(tightening);

        splits.append(split);
        p1 = p2;
    }

    return splits;
}

List<Equation> PiecewiseLinearAbstraction::getRefinedUpperAbstraction(List<GuidedPoint> &guidedPoints) const
{
    List<Equation> refinements;
    auto guidedPointsIter = guidedPoints.begin();
//    auto guidedPointsIterReverse = guidedPoints.rbegin();

    GuidedPoint p1  = *guidedPointsIter;
//    GuidedPoint lowerBound = p1, upperBound = *guidedPointsIterReverse;
    while (guidedPointsIter != guidedPoints.end())
    {
        p1 = *guidedPointsIter++;
//
//        // We're w/o guided points, choosing one randomly
//        if (p1 == lowerBound && p2 == upperBound)
//        {
//            double x = (p1.x + p2.x) / 2;
//            double y = evaluateConciseFunction(x);
//            p2 = GuidedPoint(x, y);
//            guidedPointsIter--;
//        }
//
//        // Invalid guided points due to bounds were changed
//        if (p2 > upperBound || p2 < lowerBound)
//            continue;
//
//        // Same assignment as guided point
//        if ( p1 == p2 )
//        {
//            guidedPointsIter++;
//            double x = ((*guidedPointsIter).x + p2.x) / 2;
//            double y = evaluateConciseFunction(x);
//            p2 = GuidedPoint(x, y);
//            guidedPointsIter--;
//        }
        std::cout << ("p1: ");
        std::cout << ("(" + std::to_string(p1.x) + ", " + std::to_string(p1.y) + ")\n");
//        std::cout << ("p2: ");
//        std::cout << ("(" + std::to_string(p2.x) + ", " + std::to_string(p2.y) + ")\n");



        double slope = evaluateDerivativeOfConciseFunction(p1.x);
        Equation upperEquation = getLinearEquation(p1, slope);
        upperEquation.setType(Equation::LE);
        refinements.append(upperEquation);

        //TODO: at the moment we don't splits on above
//        for (Tightening tightening: boundVars(p1, p2))
//            split.storeBoundTightening(tightening);
//
//        splits.append(split);
//        p1 = *guidedPointsIter;
    }

    return refinements;
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


Equation PiecewiseLinearAbstraction::getLinearEquation(GuidedPoint p, double slope) const
{
    unsigned b = getB(), f = getF();
    double x0 = p.x, y0 = p.y;

    Equation equation;
    equation.addAddend(1, f);
    equation.addAddend(-slope, b);
    equation.setScalar(y0 - slope * x0);
    return equation;
}

Equation PiecewiseLinearAbstraction::getLinearEquation(GuidedPoint p1, GuidedPoint p2) const
{
    double x0 = p1.x, y0 = p1.y, x1 = p2.x, y1 = p2.y;
    double slope = (y1 - y0) / (x1 - x0);

    return getLinearEquation(GuidedPoint(x0, y0), slope);
}

