//
// Created by yoni_mantzur on 11/3/19.
//

#include <boost/concept_check.hpp>
#include <Debug.h>
#include "PiecewiseLinearAbstraction.h"


List<PiecewiseLinearCaseSplit> PiecewiseLinearAbstraction::getSplitsAbstraction() const
{
    List<double> guidedPoints;
    List<double > insertedPoints;
    List<PiecewiseLinearCaseSplit> splits;

    List<double> spuriousPoints = _pointsForSplits;
    Point lowerBound = getLowerParticipantVariablesBounds(), upperBound = getUpperParticipantVariablesBounds();

    // Guided point will be from lower bound to upper bound
    guidedPoints.append(lowerBound.x);

    if (spuriousPoints.empty())
        spuriousPoints.append((lowerBound.x + upperBound.x) / 2);
    guidedPoints.append(spuriousPoints);

    guidedPoints.append(upperBound.x);

    auto guidedPointsIter = guidedPoints.begin();

    double x1  = *guidedPointsIter;
    while (++guidedPointsIter != guidedPoints.end())
    {
        double x2 = *guidedPointsIter;

        // Invalid guided points due to bounds were changed
        if (FloatUtils::gt(x2, upperBound.x) || FloatUtils::lt(x2, lowerBound.x))
        {
            x2 = (x1 + *(++guidedPointsIter)) / 2;
            guidedPointsIter--;
        }

        PiecewiseLinearCaseSplit split;

        Point p1 = {x1, evaluateConciseFunction(x1)};
        Point p2 = {x2, evaluateConciseFunction(x2)};

        if (!insertedPoints.exists(x1))
        {
            insertedPoints.append(x1);
            Equation abstractedEquation = getLinearEquation(p1, p2);
            Equation::EquationType equationType = getConvexType() == CONVEX ? Equation::LE : Equation::GE;
            abstractedEquation.setType(equationType);
            split.addEquation(abstractedEquation);

            for (Tightening tightening: boundVars(p1, p2))
                split.storeBoundTightening(tightening);

            splits.append(split);
        }
        x1 = x2;
    }
    return splits;
}


List<Equation> PiecewiseLinearAbstraction::getEquationsAbstraction() const
{

    List<double> guidedPoints = _pointsForAbstractedBounds;
    List<Equation> refinements;

    Point lowerBound = getLowerParticipantVariablesBounds(), upperBound = getUpperParticipantVariablesBounds();
    for (double x : guidedPoints)
    {
        // Invalid guided points due to bounds were changed
        if (FloatUtils::gte(x, upperBound.x) || FloatUtils::lte(x, lowerBound.x))
            continue;

        double slope = evaluateDerivativeOfConciseFunction(x);
        Point p = {x, evaluateConciseFunction(x)};
        Equation abstractedEquation = getLinearEquation(p, slope);
        Equation::EquationType equationType = getConvexType() == CONVEX ? Equation::GE : Equation::LE;
        abstractedEquation.setType(equationType);
        refinements.append(abstractedEquation);

    }
    return refinements;
}

Equation PiecewiseLinearAbstraction::refineCurrentSplit() const
{
    // TODO: need to think how to remove old equations as well (and not call after c.s)

    Point lowerBound = getLowerParticipantVariablesBounds(), upperBound = getUpperParticipantVariablesBounds();
    Equation equation = getLinearEquation(lowerBound, upperBound);
    Equation::EquationType equationType = getConvexType() == CONVEX ? Equation::LE : Equation::GE;
    equation.setType(equationType);
    return equation;
}

void PiecewiseLinearAbstraction::addSpuriousPoint(Point p)
{
    double fixed_point = evaluateConciseFunction(p.x);

    // TODO: has a bug when list is more then one
    _pointsForSplits.clear();
    _pointsForAbstractedBounds.clear();

    ConvexType convexType = getConvexType();

    if ((convexType == UNKNOWN) || (FloatUtils::gte(p.y, fixed_point) && (convexType == CONVEX))
        || (FloatUtils::lte(p.y, fixed_point) && (convexType == CONCAVE)))
    {
        _pointsForSplits.append(p.x);
    } else {
        _pointsForAbstractedBounds.append(p.x);
    }
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
