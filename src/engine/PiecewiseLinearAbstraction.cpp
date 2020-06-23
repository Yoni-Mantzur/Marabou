//
// Created by yoni_mantzur on 11/3/19.
//

#include <boost/concept_check.hpp>
#include <Debug.h>
#include "PiecewiseLinearAbstraction.h"


List<PiecewiseLinearCaseSplit> PiecewiseLinearAbstraction::getSplitsAbstraction(bool isGuidedByF) {
    List<Point> guidedPoints;
    List<PiecewiseLinearCaseSplit> splits;

    Point lowerBound = getLowerParticipantVariablesBounds(), upperBound = getUpperParticipantVariablesBounds();
    ASSERT(!FloatUtils::areEqual(lowerBound.x, upperBound.x,
                                 GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))


    // TODO: Generalize to several guided points
    ASSERT(_pointsForSplits.size() <= 1)

    guidedPoints.append({ lowerBound.x, evaluateConciseFunction(lowerBound.x) } );

    if (_pointsForSplits.empty())
        guidedPoints.append(getMiddlePoint(isGuidedByF, lowerBound, upperBound));

    else
    {
        bool getMiddle = false;
        for (Point p : _pointsForSplits)
        {
            if (!validatePoint(p, _pointsForSplits, true))
            {
                getMiddle = true;
                break;
            }
        }

        if (getMiddle)
            guidedPoints.append(getMiddlePoint(isGuidedByF, lowerBound, upperBound));

        else
            guidedPoints.append(_pointsForSplits);
    }


    guidedPoints.append({upperBound.x, evaluateConciseFunction(upperBound.x)} );

    auto guidedPointsIter = guidedPoints.begin();

    Point p1 = *guidedPointsIter;
    _registeredPointsForCurrentSplit.append(p1);
    while (++guidedPointsIter != guidedPoints.end())
    {

        ASSERT((validatePoint(p1, guidedPoints, true, true)))
        Point p2 = *guidedPointsIter;

        PiecewiseLinearCaseSplit split;

        _registeredPointsForCurrentSplit.append(p2);


        Equation abstractedEquation = getLinearEquation(p1, p2);
        setEquationTypeForSplitAbstraction(&abstractedEquation, p1.x, p2.x);
        split.addEquation(abstractedEquation);
        for (Tightening tightening: boundVars(p1, p2))
            split.storeBoundTightening(tightening);

        splits.append(split);

        p1 = p2;
    }
    _pointsForSplits.clear();
    return splits;
}

List<Equation> PiecewiseLinearAbstraction::getEquationsAbstraction(bool isGuidedByF) {

    List<Point> guidedPoints = _pointsForAbstractedBounds;
    List<Equation> refinements;

    Point lowerBound = getLowerParticipantVariablesBounds(), upperBound = getUpperParticipantVariablesBounds();

    if (getConvexTypeInSegment(lowerBound.x, upperBound.x) != UNKNOWN && guidedPoints.empty())
        guidedPoints.append(getMiddlePoint(isGuidedByF, lowerBound, upperBound));



    for (Point p : guidedPoints) {

        if (!(validatePoint(p, guidedPoints, true) && validatePoint(p, _registeredPointsForAbstraction)))
        {
            p = getMiddlePoint(isGuidedByF, lowerBound, upperBound);
            if (!(validatePoint(p, guidedPoints) && validatePoint(p, _registeredPointsForAbstraction)))
                continue;
        }


        ASSERT(p.x > lowerBound.x && p.y > lowerBound.y && p.x < upperBound.x && p.y < upperBound.y)
        _registeredPointsForAbstraction.append(p);

        double slope = evaluateDerivativeOfConciseFunction(p.x);
        Equation abstractedEquation = getLinearEquation(p, slope);
        setEquationTypeForAbstraction(&abstractedEquation, lowerBound.x, upperBound.x);
        refinements.append(abstractedEquation);

    }
    _pointsForAbstractedBounds.clear();
    return refinements;
}


Equation PiecewiseLinearAbstraction::refineCurrentSplit() {
    Point lowerBound = getLowerParticipantVariablesBounds(), upperBound = getUpperParticipantVariablesBounds();
    Point p = {lowerBound.x, upperBound.x};

    if (_registeredPointsForCurrentSplit.exists(p))
        return Equation();

    _registeredPointsForCurrentSplit.append(p);

    if (getConvexTypeInSegment(lowerBound.x, upperBound.x) == UNKNOWN) {
        // TODO: add equation for the general case
        return Equation();
    }

    Equation equation = getLinearEquation(lowerBound, upperBound);
    setEquationTypeForSplitAbstraction(&equation, lowerBound.x, upperBound.x);
    return equation;

}

void PiecewiseLinearAbstraction::addSpuriousPoint(Point p) {
    double fixed_point = evaluateConciseFunction(p.x);

    Point lowerBound = getLowerParticipantVariablesBounds(), upperBound = getUpperParticipantVariablesBounds();
    ConvexType convexType = getConvexTypeInSegment(lowerBound.x, upperBound.x);
    Point fixedPoint = {p.x, fixed_point};

    if (_pointsForSplits.exists({p.x, fixed_point}) || _pointsForAbstractedBounds.exists({p.x, fixed_point}))
        return;


    if ((convexType == UNKNOWN) ||
        (FloatUtils::gte(p.y, fixed_point, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE) && (convexType == CONVEX))
        || (FloatUtils::lte(p.y, fixed_point, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE) && (convexType == CONCAVE)))
    {
        if (validatePoint(fixedPoint, _pointsForSplits))
            _pointsForSplits.append(fixedPoint);

        if (_pointsForSplits.size() > GlobalConfiguration::GUIDED_POINTS_FOR_SPLIT_SEGMENTS_TH)
            _pointsForSplits.popFront();

    } else {

        if (validatePoint(fixedPoint, _pointsForAbstractedBounds))
            _pointsForAbstractedBounds.append(fixedPoint);

        if (_pointsForAbstractedBounds.size() > GlobalConfiguration::GUIDED_POINTS_FOR_ABSTRACTION_EQUATIONS_TH)
            _pointsForAbstractedBounds.popFront();

    }
}

List<Tightening> PiecewiseLinearAbstraction::boundVars(Point p1, Point p2) const {
    List<Tightening> bounds;
    unsigned b = getB(), f = getF();

    // Bound b
    if (!FloatUtils::lte(p1.x, p2.x, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
        printf("here");

    ASSERT(FloatUtils::lte(p1.x, p2.x, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))

    bounds.append(Tightening(b, p1.x, Tightening::LB));
    bounds.append(Tightening(b, p2.x, Tightening::UB));

    // Bound f
    ASSERT(FloatUtils::lte(p1.y, p2.y, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
    bounds.append(Tightening(f, p1.y, Tightening::LB));
    bounds.append(Tightening(f, p2.y, Tightening::UB));

    return bounds;
}

Equation PiecewiseLinearAbstraction::getLinearEquation(Point p, double slope) const {
    unsigned b = getB(), f = getF();
    double x0 = p.x, y0 = p.y;

    Equation equation;
    equation.addAddend(1, f);
    equation.addAddend(-slope, b);
    equation.setScalar(y0 - slope * x0);
    return equation;
}

Equation PiecewiseLinearAbstraction::getLinearEquation(Point p1, Point p2) const {
    double x0 = p1.x, y0 = p1.y, x1 = p2.x, y1 = p2.y;
    ASSERT(!FloatUtils::areEqual(x1, x0, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
    double slope = (y1 - y0) / (x1 - x0);

    return getLinearEquation(Point(x0, y0), slope);
}

void PiecewiseLinearAbstraction::setEquationType(Equation *equation, double x0, double x1, bool isForSplitCase) {
    ConvexType convexType = getConvexTypeInSegment(x0, x1);
    Equation::EquationType equationType;

    switch (convexType) {
        case CONCAVE:
            equationType = isForSplitCase ? Equation::GE : Equation::LE;
            break;

        case CONVEX:
            equationType = isForSplitCase ? Equation::LE : Equation::GE;
            break;

        default:
            throw;
    }
    equation->setType(equationType);
}

void PiecewiseLinearAbstraction::setEquationTypeForSplitAbstraction(Equation *equation, double x0, double x1) {
    setEquationType(equation, x0, x1, true);
}

void PiecewiseLinearAbstraction::setEquationTypeForAbstraction(Equation *equation, double x0, double x1) {
    setEquationType(equation, x0, x1, false);
}

bool PiecewiseLinearAbstraction::validatePoint(Point p, List<PiecewiseLinearAbstraction::Point> registeredPoints, bool includeP, bool withBounds)
{
    Point lowerBound = getLowerParticipantVariablesBounds(), upperBound = getUpperParticipantVariablesBounds();
    bool visitP = false;

    if (!withBounds &&
        (FloatUtils::lte(p.x, lowerBound.x, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE)
         || FloatUtils::lte(p.y, lowerBound.y, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE)
         || FloatUtils::gte(p.x, upperBound.x, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE)
         || FloatUtils::gte(p.y, upperBound.y, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE)))
        return false;

    for (Point registeredP : registeredPoints)
    {
        if (FloatUtils::areEqual(registeredP.x, p.x, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE)
            || FloatUtils::areEqual(registeredP.y, p.y, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
        {
            if (visitP || !includeP)
                return false;

            visitP = true;
        }
    }
    return true;
}


PiecewiseLinearAbstraction::Point PiecewiseLinearAbstraction::getMiddlePoint(bool isGuidedByF, Point &lowerBound, Point &upperBound) const
{
    if (isGuidedByF)
    {
        double middleF = (lowerBound.y + upperBound.y) / 2;
        return {evaluateInverseOfConciseFunction(middleF), middleF };

    }

    else
    {
        double middle = (lowerBound.x + upperBound.x) / 2;
        return { middle, evaluateConciseFunction(middle) };
    }
}
