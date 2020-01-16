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

        bool operator==(Point other)
        {
            return FloatUtils::areEqual(x, other.x);
        }

        bool operator<(Point other)
        {
            return FloatUtils::lt(x, other.x);
        }

        bool operator>(Point other)
        {
            return FloatUtils::gt(x, other.x);
        }

        bool operator<=(Point other)
        {
            return FloatUtils::lte(x, other.x);
        }

        bool operator>=(Point other)
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
    List<PiecewiseLinearCaseSplit> getSplitsAbstraction() const;

    /*
     * Get equations abstraction
     */
    List<Equation> getEquationsAbstraction() const;

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
     * Check if the concise function is convex function
     */
    virtual ConvexType getConvexType() const = 0;

private:
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
     * List of spurious points from above and beneath
     */
    List<double> _pointsForSplits;
    List<double> _pointsForAbstractedBounds;

    const static unsigned _SPURIOUS_POINTS_TS = 1;
};


#endif //MARABOU_PIECEWISELINEARABSTRACTION_H
