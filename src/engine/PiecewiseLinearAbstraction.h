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

    class GuidedPoint
    {
    public:
        GuidedPoint(double x, double y) :  x(x), y(y) {}
        double x;
        double y;
    };

    List<PiecewiseLinearCaseSplit> getRefinedSplits(List<GuidedPoint> guidedPoints) const;

    virtual unsigned getB() const = 0;
    virtual unsigned getF() const = 0;


private:
    List<PiecewiseLinearCaseSplit> getRefinedUpperAbstraction(List<GuidedPoint> guidedPoints) const;
    List<PiecewiseLinearCaseSplit> getRefinedLowerAbstraction(List<GuidedPoint> guidedPoints) const;
    Equation buildLinearEquationGivenTwoPoints(GuidedPoint p1, GuidedPoint p2) const;

    List<Tightening> boundVars(GuidedPoint p1, GuidedPoint p2) const;
};


#endif //MARABOU_PIECEWISELINEARABSTRACTION_H
