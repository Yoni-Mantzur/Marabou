//
// Created by yoni_mantzur on 11/3/19.
//

#include <boost/concept_check.hpp>
#include <Debug.h>
#include "PiecewiseLinearAbstraction.h"


List<PiecewiseLinearCaseSplit> PiecewiseLinearAbstraction::getSplitsAbstraction() {
    List<double> guidedPoints;
    List<PiecewiseLinearCaseSplit> splits;

    Point lowerBound = getLowerParticipantVariablesBounds(), upperBound = getUpperParticipantVariablesBounds();
    ASSERT(!FloatUtils::areEqual(lowerBound.x, upperBound.x,
                                 GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))

    // Guided point will be from lower bound to upper bound
    _pointsForSplits.sort();

    if (_pointsForSplits.exists(upperBound.x))
        _pointsForSplits.popBack();

    if (_pointsForSplits.exists(lowerBound.x))
        _pointsForSplits.popFront();

    guidedPoints.append(lowerBound.x);

    if (_pointsForSplits.empty()) {
        if (getConvexTypeInSegment(lowerBound.x, upperBound.x) == UNKNOWN)
            _pointsForSplits.append(0);
        else
            _pointsForSplits.append((lowerBound.x + upperBound.x) / 2);
    }

    guidedPoints.append(_pointsForSplits);

    guidedPoints.append(upperBound.x);

    auto guidedPointsIter = guidedPoints.begin();

    double x1 = *guidedPointsIter;
    while (++guidedPointsIter != guidedPoints.end()) {
        double x2 = *guidedPointsIter;

        // Invalid guided points due to bounds were changed
        if (FloatUtils::gt(x2, upperBound.x, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE) ||
            FloatUtils::lt(x2, lowerBound.x, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE) ||
            FloatUtils::areEqual(x1, x2, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE)) {
            x2 = (x1 + *(++guidedPointsIter)) / 2;
            guidedPointsIter--;
        }

        PiecewiseLinearCaseSplit split;

        Point p1 = {x1, evaluateConciseFunction(x1)};
        Point p2 = {x2, evaluateConciseFunction(x2)};

        _registeredPointsForCurrentSplit.append({x1, x2});

        Equation abstractedEquation = getLinearEquation(p1, p2);
        setEquationTypeForSplitAbstraction(&abstractedEquation, p1.x, p2.x);
        split.addEquation(abstractedEquation);
        for (Tightening tightening: boundVars(p1, p2))
            split.storeBoundTightening(tightening);

        splits.append(split);

        x1 = x2;
    }
    _pointsForSplits.clear();
    return splits;
}

List<Equation> PiecewiseLinearAbstraction::getEquationsAbstraction() {

    List<double> guidedPoints = _pointsForAbstractedBounds;
    List<Equation> refinements;

    Point lowerBound = getLowerParticipantVariablesBounds(), upperBound = getUpperParticipantVariablesBounds();

    if (getConvexTypeInSegment(lowerBound.x, upperBound.x) != UNKNOWN && guidedPoints.empty()) {
        guidedPoints.append((lowerBound.x / 3) + ((2 * upperBound.x) / 3));
    }
    for (double x : guidedPoints) {
        // Invalid guided points due to bounds were changed
        if (FloatUtils::gt(x, upperBound.x, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE) ||
            FloatUtils::lt(x, lowerBound.x, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE) ||
            _registeredPointsForAbstraction.exists(x))
            continue;

        for (double x0 : _registeredPointsForAbstraction) {
            if (FloatUtils::areEqual(x, x0, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
                continue;
        }
        _registeredPointsForAbstraction.append(x);

        double slope = evaluateDerivativeOfConciseFunction(x);
        Point p = {x, evaluateConciseFunction(x)};
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
        throw 0;

    _registeredPointsForCurrentSplit.append(p);

    if (getConvexTypeInSegment(lowerBound.x, upperBound.x) == UNKNOWN) {
        // TODO: add equation for the general case
        throw 0;
    }

    Equation equation = getLinearEquation(lowerBound, upperBound);
    setEquationTypeForSplitAbstraction(&equation, lowerBound.x, upperBound.x);
    return equation;

}

void PiecewiseLinearAbstraction::addSpuriousPoint(Point p) {
    double fixed_point = evaluateConciseFunction(p.x);

    Point lowerBound = getLowerParticipantVariablesBounds(), upperBound = getUpperParticipantVariablesBounds();
    ConvexType convexType = getConvexTypeInSegment(lowerBound.x, upperBound.x);

    if ((convexType == UNKNOWN) ||
        (FloatUtils::gte(p.y, fixed_point, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE) &&
         (convexType == CONVEX))
        || (FloatUtils::lte(p.y, fixed_point, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE) &&
            (convexType == CONCAVE))) {
        if (!_pointsForSplits.exists(p.x)) {
            if (_pointsForSplits.size() == GlobalConfiguration::GUIDED_POINTS_FOR_SPLIT_SEGMENTS_TH)
                _pointsForSplits.popFront();

            _pointsForSplits.append(p.x);
        }
    } else {
        if (!_pointsForAbstractedBounds.exists(p.x)) {
            if (_pointsForAbstractedBounds.size() == GlobalConfiguration::GUIDED_POINTS_FOR_ABSTRACTION_EQUATIONS_TH)
                _pointsForAbstractedBounds.popFront();

            _pointsForAbstractedBounds.append(p.x);
        }
    }
}

List<Tightening> PiecewiseLinearAbstraction::boundVars(Point p1, Point p2) const {
    List<Tightening> bounds;
    unsigned b = getB(), f = getF();

    // Bound b
    ASSERT(FloatUtils::lt(p1.x, p2.x, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
    bounds.append(Tightening(b, p1.x, Tightening::LB));
    bounds.append(Tightening(b, p2.x, Tightening::UB));

    // Bound f
    ASSERT(FloatUtils::lt(p1.y, p2.y, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
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

