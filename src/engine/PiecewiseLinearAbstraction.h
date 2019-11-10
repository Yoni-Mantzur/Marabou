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


    List<PiecewiseLinearCaseSplit> getUpperSplits() const;

    List<PiecewiseLinearCaseSplit> getLowerSplits() const;

    void addGuidedPoint(GuidedPoint p);

    void refine();

    virtual unsigned getB() const = 0;
    virtual unsigned getF() const = 0;


private:
    List<GuidedPoint> _guidedPoints;
    List<PiecewiseLinearCaseSplit> _abstractedUpperSplits;
    List<PiecewiseLinearCaseSplit> _abstractedLowerSplits;

    void refineUpperAbstraction();
    void refineLowerAbstraction();
    Equation buildLinearEquationGivenTwoPoints(GuidedPoint p1, GuidedPoint p2);

    List<Tightening> boundVars(GuidedPoint p1, GuidedPoint p2);

};


#endif //MARABOU_PIECEWISELINEARABSTRACTION_H
