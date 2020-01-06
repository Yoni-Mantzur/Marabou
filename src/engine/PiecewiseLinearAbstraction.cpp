//
// Created by yoni_mantzur on 11/3/19.
//

#include <boost/concept_check.hpp>
#include <Debug.h>
#include "PiecewiseLinearAbstraction.h"


List<PiecewiseLinearCaseSplit> PiecewiseLinearAbstraction::getSplitsAbstraction() const
{
    List<Point> guidedPoints;
    List<PiecewiseLinearCaseSplit> splits;

    List<Point> spuriousPoints = _pointsForSplits;
    Point lowerBound = getLowerParticipantVariablesBounds(), upperBound = getUpperParticipantVariablesBounds();

    // Guided point will be from lower bound to upper bound
    guidedPoints.append(lowerBound);

    // If no spurious points, pick one in the middle
    if (spuriousPoints.empty()){
        std::cout << "EMPTY" << std::endl;
        guidedPoints.append(extractPointInSegment(lowerBound.x, upperBound.x));
    }

    guidedPoints.append(spuriousPoints);
    guidedPoints.append(upperBound);
//    spuriousPoints.sort();

    auto guidedPointsIter = guidedPoints.begin();

    Point p1  = *guidedPointsIter;
    std::cout << "split++:" << std::endl;
    while (++guidedPointsIter != guidedPoints.end())
    {
        Point p2 = *guidedPointsIter;

        // Invalid guided points due to bounds were changed
        if (p2 > upperBound || p2 < lowerBound)  {
            p2 = extractPointInSegment(p1.x, upperBound.x);
            std::cout << "NOT IN SEGMENT" << std::endl;
        }

        PiecewiseLinearCaseSplit split;

        p1 = {p1.x, evaluateConciseFunction(p1.x)};
        p2 = {p2.x, evaluateConciseFunction(p2.x)};

        std::cout << "p1:" << std::endl;
        std::cout << std::to_string(p1.x) << std::endl;
        std::cout << std::to_string(p1.y) << std::endl;
        std::cout << "p2:" << std::endl;
        std::cout << std::to_string(p2.x) << std::endl;
        std::cout << std::to_string(p2.y) << std::endl;

        Equation abstractedEquation = getLinearEquation(p1, p2);
        Equation::EquationType equationType = getConvexType() == CONVEX ? Equation::LE : Equation::GE;
        abstractedEquation.setType(equationType);
        split.addEquation(abstractedEquation);
        for (Tightening tightening: boundVars(p1, p2))
            split.storeBoundTightening(tightening);

        splits.append(split);
        p1 = p2;
    }
    return splits;
}


List<Equation> PiecewiseLinearAbstraction::getEquationsAbstraction() const
{
    ASSERT(getConvexType() != UNKNOWN )
    List<Point> guidedPoints = _pointsForAbstractedBounds;
    List<Equation> refinements;

    Point lowerBound = getLowerParticipantVariablesBounds(), upperBound = getUpperParticipantVariablesBounds();
    for (Point p : guidedPoints)
    {
        if (p >= lowerBound && p <= upperBound)
        {
            std::cout << "p:" << std::endl;
            std::cout << std::to_string(p.x) << std::endl;
            std::cout << std::to_string(p.y) << std::endl;
            double slope = evaluateDerivativeOfConciseFunction(p.x);
            std::cout << "slope:" << std::endl;
            std::cout << std::to_string(slope) << std::endl;
            Equation abstractedEquation = getLinearEquation(p, slope);
            Equation::EquationType equationType = getConvexType() == CONVEX ? Equation::GE : Equation::LE;
            abstractedEquation.setType(equationType);
//            refinements.append(abstractedEquation);
        }
    }

    // The problematic equation (getConvexType? fromAbove : fromBeneath)
    // TODO: need to think how to remove old equations as well (and not call after c.s)
//    Equation equation = getLinearEquation(lowerBound, upperBound);
//    Equation::EquationType equationType = getConvexType()? Equation::LE : Equation::GE;
//    equation.setType(equationType);
//    refinements.append(equation);

    return refinements;
}

void PiecewiseLinearAbstraction::addSpuriousPoint(Point p)
{
    double fixed_point = evaluateConciseFunction(p.x);
    _pointsForSplits.clear();
    _pointsForAbstractedBounds.clear();
    Point concisePoint = Point(p.x, fixed_point);
    ConvexType convexType = getConvexType();
    ASSERT(convexType != UNKNOWN)
    if (convexType == UNKNOWN)
        return;

    if (((p.y >=  fixed_point) && (convexType == CONVEX)) || ((p.y <= fixed_point) && (convexType == CONCAVE)))
        _pointsForSplits.append(concisePoint);
//    else
//        _pointsForAbstractedBounds.append(concisePoint);
}

List<Tightening> PiecewiseLinearAbstraction::boundVars(Point p1, Point p2) const{
    List<Tightening> bounds;
    unsigned b = getB(), f = getF();

    // Bound b
    ASSERT(FloatUtils::lte(p1.x, p2.x))
    bounds.append(Tightening (b, p1.x, Tightening::LB));
    bounds.append(Tightening(b, p2.x, Tightening::UB));

    // Bound f
    ASSERT(FloatUtils::lte(p1.y, p2.y))
    bounds.append(Tightening(f, p1.y, Tightening::LB));
    bounds.append(Tightening(f, p2.y, Tightening::UB));

    std::cout << "b bound on split:" << std::endl;
    std::cout << std::to_string(p1.x) << std::endl;
    std::cout << std::to_string(p2.x) << std::endl;

    std::cout << "f bound on split:" << std::endl;
    std::cout << std::to_string(p1.y) << std::endl;
    std::cout << std::to_string(p2.y) << std::endl;

    return bounds;
}


Equation PiecewiseLinearAbstraction::getLinearEquation(Point p, double slope) const
{
    unsigned b = getB(), f = getF();
    double x0 = p.x, y0 = p.y;

    Equation equation;
    equation.addAddend(1, f);
    equation.addAddend(-slope, b);
    equation.setScalar(y0 - slope * x0);
    return equation;
}

Equation PiecewiseLinearAbstraction::getLinearEquation(Point p1, Point p2) const
{
    double x0 = p1.x, y0 = p1.y, x1 = p2.x, y1 = p2.y;
    double slope = (y1 - y0) / (x1 - x0);

    return getLinearEquation(Point(x0, y0), slope);
}

PiecewiseLinearAbstraction::Point PiecewiseLinearAbstraction::extractPointInSegment(double x1, double x2) const {
    double x = (x1 + x2) / 2;
    double y = evaluateConciseFunction(x);
    return {x, y};
}