//
// Created by yoni_mantzur on 11/3/19.
//

#ifndef MARABOU_PIECEWISELINEARABSTRACTION_H
#define MARABOU_PIECEWISELINEARABSTRACTION_H

#include "List.h"
#include "Pair.h"
#include "Equation.h"
#include "PiecewiseLinearConstraint.h"

class PiecewiseLinearAbstraction
{
public:

    class Point
    {
    public:
        Point(double x, double y) :  x(x), y(y) {}
        double x;
        double y;

        bool operator==(Point other) const
        {
            return FloatUtils::areEqual(x, other.x) && FloatUtils::areEqual(y, other.y);
        }

        bool operator<(Point other) const
        {
            return FloatUtils::lt(x, other.x);
        }

        bool operator>(Point other) const
        {
            return FloatUtils::gt(x, other.x);
        }

        bool operator<=(Point other) const
        {
            return FloatUtils::lte(x, other.x);
        }

        bool operator>=(Point other) const
        {
            return FloatUtils::gte(x, other.x);
        }
    };

    enum ConvexType{
        CONVEX,
        CONCAVE,
        UNKNOWN
    };
    /*
     * Get splits abstraction
     */
    List<PiecewiseLinearCaseSplit> getSplitsAbstraction(bool isGuidedByF);

    /*
     * Get equations abstraction
     */
    List<Equation> getEquationsAbstraction(bool isGuidedByF);

    /*
     * Refine the current split guided the new bounds are given
     */
    Equation refineCurrentSplit();

    /*
     * Add spurious example
     * NOTE: the example should no on the concise function by definition, o.w. undefined behavior
     */
    void addSpuriousPoint(Point p);

    /*
     * Get participant variables in the constraint
     */
    virtual unsigned getB() const = 0;
    virtual unsigned getF() const = 0;

    /*
     * Get the current bounds of the participant variables in the constraint
     */
    virtual Point getLowerParticipantVariablesBounds() const = 0;
    virtual Point getUpperParticipantVariablesBounds() const = 0;


    /*
     * Evaluate the concise function given point in the range
     */
    virtual double evaluateConciseFunction(double x) const = 0;

    /*
     * Evaluate the derivative of the concise function given point in the range
     */
    virtual double evaluateDerivativeOfConciseFunction(double x) const = 0;

    /*
     * Evaluate the Inverse of the concise function given point in the range
    */
    virtual double evaluateInverseOfConciseFunction(double y) const = 0;

    /*
     * Check if the concise function is convex function
     */
    virtual ConvexType getConvexTypeInSegment(double x0, double x1) const = 0;

private:
    /*
    * List of spurious points from above and beneath
    */
    List<Point> _pointsForSplits;
    List<Point> _pointsForAbstractedBounds;

    List<Point> _registeredPointsForAbstraction;
    List<Point> _registeredPointsForCurrentSplit;

    /*
     * Build linear equation
     */
    Equation getLinearEquation(Point p, double slope) const;
    Equation getLinearEquation(Point p1, Point p2) const;

    /*
     * Get list of bounds on b and f given bounding box (defined by two points)
     */
    List<Tightening> boundVars(Point p1, Point p2) const;

    /*
     * Get the equation type, given the convex type we're
     */
    void setEquationType(Equation *equation, double x0, double x1, bool forSplitAbstraction);
    void setEquationTypeForSplitAbstraction(Equation *equation, double x0, double x1);
    void setEquationTypeForAbstraction(Equation *equation, double x0, double x1);

    bool validatePoint(Point p, List<Point> registeredPoints, bool includeP = false, bool withBounds = false);

    Point getMiddlePoint(bool isGuidedByF, Point &lowerBound, Point &upperBound) const;
};


#endif //MARABOU_PIECEWISELINEARABSTRACTION_H
