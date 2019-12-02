//
// Created by yoni_mantzur on 11/3/19.
//

#ifndef MARABOU_SIGMOIDCONSTRAINT_H
#define MARABOU_SIGMOIDCONSTRAINT_H

#include <File.h>
#include "Map.h"
#include "PiecewiseLinearConstraint.h"
#include "PiecewiseLinearAbstraction.h"

class SigmoidConstraint : public PiecewiseLinearConstraint, PiecewiseLinearAbstraction
{
public:
    SigmoidConstraint( unsigned b, unsigned f );
    SigmoidConstraint( const String &serializedRelu );

    /*
      Return a clone of the constraint.
    */
    PiecewiseLinearConstraint *duplicateConstraint() const;

    /*
      Restore the state of this constraint from the given one.
    */
    void restoreState( const PiecewiseLinearConstraint *state );

    /*
      Register/unregister the constraint with a talbeau.
     */
    void registerAsWatcher( ITableau *tableau );
    void unregisterAsWatcher( ITableau *tableau );

    /*
      These callbacks are invoked when a watched variable's value
      changes, or when its bounds change.
    */
    void notifyVariableValue( unsigned variable, double value );
    void notifyLowerBound( unsigned variable, double bound );
    void notifyUpperBound( unsigned variable, double bound );

    /*
      Returns true iff the variable participates in this piecewise
      linear constraint
    */
    bool participatingVariable( unsigned variable ) const;

    /*
      Get the list of variables participating in this constraint.
    */
    List<unsigned> getParticipatingVariables() const;

    /*
      Returns true iff the assignment satisfies the constraint
    */
    bool satisfied() const;

    /*
      Returns a list of possible fixes for the violated constraint.
    */
    List<PiecewiseLinearConstraint::Fix> getPossibleFixes() const;

    /*
      Return a list of smart fixes for violated constraint.
    */
    List<PiecewiseLinearConstraint::Fix> getSmartFixes( ITableau *tableau ) const;

    /*
      Returns the list of case splits that this piecewise linear
      constraint breaks into. These splits need to complementary,
      i.e. if the list is {l1, l2, ..., ln-1, ln},
      then ~l1 /\ ~l2 /\ ... /\ ~ln-1 --> ln.
     */
    List<PiecewiseLinearCaseSplit> getCaseSplits() const;

    /*
      Check if the constraint's phase has been fixed.
    */
    bool phaseFixed() const {
        return false;
    }

    /*
      If the constraint's phase has been fixed, get the (valid) case split.
    */
    PiecewiseLinearCaseSplit getValidCaseSplit() const {
        return PiecewiseLinearCaseSplit();  // Not implemented
    }

    /*
      Preprocessing related functions, to inform that a variable has been eliminated completely
      because it was fixed to some value, or that a variable's index has changed (e.g., x4 is now
      called x2). constraintObsolete() returns true iff and the constraint has become obsolote
      as a result of variable eliminations.
    */
    void eliminateVariable( unsigned variable, double fixedValue );
    void updateVariableIndex( unsigned oldIndex, unsigned newIndex );
    bool constraintObsolete() const;

    /*
      Get the tightenings entailed by the constraint.
    */
    void getEntailedTightenings( List<Tightening> &tightenings ) const;

    /*
      Dump the current state of the constraint.
    */
    void dump( String &output ) const;

    /*
      For preprocessing: get any auxiliary equations that this
      constraint would like to add to the equation pool. In the ReLU
      case, this is an equation of the form aux = f - b, where aux is
      non-negative.
    */
    void addAuxiliaryEquations( InputQuery &inputQuery );

    /*
      Ask the piecewise linear constraint to contribute a component to the cost
      function. If implemented, this component should be empty when the constraint is
      satisfied or inactive, and should be non-empty otherwise. Minimizing the returned
      equation should then lead to the constraint being "closer to satisfied".
    */
    virtual void getCostFunctionComponent( Map<unsigned, double> &cost ) const;

    /*
      Returns string with shape: relu, _f, _b
    */
    String serializeToString() const;

    /*
      Get the index of the B variable.
    */
    unsigned getB() const { return _b; }

    /*
        Get the index of the B variable.
    */
    unsigned getF() const { return _f; }


    double evaluateConciseFunction(double x) { return FloatUtils::sigmoid(x); }

    /*
      Check if the aux variable is in use and retrieve it
    */
    bool auxVariableInUse() const;
    unsigned getAux() const;

    /*
      Return true if and only if this piecewise linear constraint supports
      symbolic bound tightening.
    */
    bool supportsSymbolicBoundTightening() const;

    /*
     * The constraint is active even if splited
     */
    virtual bool isActive() const
    {
        return true;
    }

    void setLogFile(File *file = nullptr);

private:

    unsigned _b, _f;

    bool _haveEliminatedVariables;

    /*
      Return true iff b or f are out of bounds.
    */
    bool haveOutOfBoundVariables() const;
    bool isValueInSigmoidBounds(double value) const;

    /* For debugging propose */
    IFile *_logFile = nullptr;
    int _iter_case_splits = 0;
    int _iter_fixes = 0;
    int _iter_satisfied = 0;
    void writePoint(double x, double y, bool isFix = false);
    void writeLimit(double lower, double upper, bool isB = false);
    void writeEquations(List<Equation> eqs);
};


#endif //MARABOU_SIGMOIDCONSTRAINT_H
