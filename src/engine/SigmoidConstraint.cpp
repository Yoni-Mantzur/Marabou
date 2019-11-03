//
// Created by yoni_mantzur on 5/18/19.
//

#include "SigmoidConstraint.h"
#include "ConstraintBoundTightener.h"
#include "Debug.h"
#include "FloatUtils.h"
#include "ITableau.h"
#include "InputQuery.h"
#include "MStringf.h"
#include "PiecewiseLinearCaseSplit.h"
#include "MarabouError.h"
#include "Statistics.h"
#include "TableauRow.h"

#ifdef _WIN32
#define __attribute__(x)
#endif

SigmoidConstraint::SigmoidConstraint( unsigned b, unsigned f )
        : _b ( b )
        , _f( f )
{
}

SigmoidConstraint::SigmoidConstraint( const String &serializedSigmoid )
{
    String constraintType = serializedSigmoid.substring(0, 7);
    ASSERT(constraintType == String("sigmoid"));

    // remove the constraint type in serialized form
    String serializedValues = serializedSigmoid.substring(5, serializedSigmoid.length()-7);
    List<String> values = serializedValues.tokenize( "," );
    _b = atoi( values.back().ascii() );
    _f = atoi( values.front().ascii() );
}

PiecewiseLinearConstraint *SigmoidConstraint::duplicateConstraint() const
{
    SigmoidConstraint *clone = new SigmoidConstraint( _b, _f );
    *clone = *this;
    return clone;
}


void SigmoidConstraint::restoreState( const PiecewiseLinearConstraint *state )
{
    const auto *sigmoid = dynamic_cast<const SigmoidConstraint *>( state );
    *this = *sigmoid;
}

void SigmoidConstraint::registerAsWatcher( ITableau *tableau )
{
    tableau->registerToWatchVariable( this, _b );
    tableau->registerToWatchVariable( this, _f );
}

void SigmoidConstraint::unregisterAsWatcher( ITableau *tableau )
{
    tableau->unregisterToWatchVariable( this, _b );
    tableau->unregisterToWatchVariable( this, _f );
}

void SigmoidConstraint::notifyVariableValue( unsigned variable, double value )
{
    //  TODO:ADD ASSIGNMENT TOLERANCE
    _assignment[variable] = value;
}

void SigmoidConstraint::notifyLowerBound( unsigned variable, double bound )
{
    if ( _statistics )
        _statistics->incNumBoundNotificationsPlConstraints();

    if ( _lowerBounds.exists( variable ) && !FloatUtils::gt( bound, _lowerBounds[variable] ) )
        return;

    _lowerBounds[variable] = bound;
    //  TODO:ADD TIGHTER BOUND
}

void SigmoidConstraint::notifyUpperBound( unsigned variable, double bound )
{
    if ( _statistics )
        _statistics->incNumBoundNotificationsPlConstraints();

    if ( _upperBounds.exists( variable ) && !FloatUtils::lt( bound, _upperBounds[variable] ) )
        return;

    _upperBounds[variable] = bound;
    //  TODO: ADD TIGHTER BOUND
}

bool SigmoidConstraint::participatingVariable( unsigned variable ) const
{
    return ( variable == _b ||  variable == _f );
}

List<unsigned> SigmoidConstraint::getParticipatingVariables() const
{
    List<unsigned> participatingVariables;

    participatingVariables.append( _b );
    participatingVariables.append( _f );

    return participatingVariables;
}

bool SigmoidConstraint::satisfied() const
{
    if ( !( _assignment.exists( _b ) && _assignment.exists( _f ) ) )
        throw MarabouError( MarabouError::PARTICIPATING_VARIABLES_ABSENT );

    double bValue = _assignment.get( _b );
    double fValue = _assignment.get( _f );

    return FloatUtils::areEqual( FloatUtils::sigmoid(bValue), fValue,
            GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE );
}

List<PiecewiseLinearConstraint::Fix> SigmoidConstraint::getPossibleFixes() const
{
    ASSERT(!satisfied());
    ASSERT(_assignment.exists(_b));
    ASSERT(_assignment.exists(_f));

    double bValue = _assignment.get(_b);
    double sigmoidValue = FloatUtils::sigmoid(bValue);

    List <PiecewiseLinearConstraint::Fix> fixes;

    // TODO: Refactor this casting
    const_cast<SigmoidConstraint&>(*this).addGuidedPoint(bValue, sigmoidValue);
    // TODO: Add all unsatisfied pts

    return fixes;
}

List<PiecewiseLinearConstraint::Fix> SigmoidConstraint::getSmartFixes(__attribute__((unused)) ITableau *tableau ) const
{
    // TODO: write something smarter
    return getPossibleFixes();
}

List<PiecewiseLinearCaseSplit> SigmoidConstraint::getCaseSplits() const
{
    // TODO: Refactor this casting
    List<Equation> refinedAbstractionEquations = const_cast<SigmoidConstraint&>(*this).extractNewLowerEquations();
    // TODO: ADD UPPER EQUATIONS
    
    List<PiecewiseLinearCaseSplit> splits;

    for (Equation equation : refinedAbstractionEquations)
    {
        PiecewiseLinearCaseSplit caseSplit;
        caseSplit.addEquation(equation);
        // TODO: storeBoundTightening for the current region
        splits.append(caseSplit);
    }
    return splits;
}

void SigmoidConstraint::dump( String &output ) const
{
    output = Stringf( "SigmoidConstraint: x%u = sigmoid( x%u ).\n", _f, _b);

    output += Stringf( "b in [%s, %s], ",
                       _lowerBounds.exists( _b ) ? Stringf( "%lf", _lowerBounds[_b] ).ascii() : "-inf",
                       _upperBounds.exists( _b ) ? Stringf( "%lf", _upperBounds[_b] ).ascii() : "inf" );

    output += Stringf( "f in [%s, %s]",
                       _lowerBounds.exists( _f ) ? Stringf( "%lf", _lowerBounds[_f] ).ascii() : "-inf",
                       _upperBounds.exists( _f ) ? Stringf( "%lf", _upperBounds[_f] ).ascii() : "inf" );
}

void SigmoidConstraint::updateVariableIndex( unsigned oldIndex, unsigned newIndex )
{
    ASSERT( oldIndex == _b || oldIndex == _f );
    ASSERT( !_assignment.exists( newIndex ) &&
            !_lowerBounds.exists( newIndex ) &&
            !_upperBounds.exists( newIndex ) &&
            newIndex != _b && newIndex != _f );

    if ( _assignment.exists( oldIndex ) )
    {
        _assignment[newIndex] = _assignment.get( oldIndex );
        _assignment.erase( oldIndex );
    }

    if ( _lowerBounds.exists( oldIndex ) )
    {
        _lowerBounds[newIndex] = _lowerBounds.get( oldIndex );
        _lowerBounds.erase( oldIndex );
    }

    if ( _upperBounds.exists( oldIndex ) )
    {
        _upperBounds[newIndex] = _upperBounds.get( oldIndex );
        _upperBounds.erase( oldIndex );
    }

    if ( oldIndex == _b )
        _b = newIndex;
    else
        _f = newIndex;
}

void SigmoidConstraint::eliminateVariable( __attribute__((unused)) unsigned variable,
                                        __attribute__((unused)) double fixedValue )
{
    //TODO: implement this
}

bool SigmoidConstraint::constraintObsolete() const
{
    //TODO: implement this
    return false;
}

void SigmoidConstraint::getEntailedTightenings(__attribute__((unused)) List<Tightening> &tightenings ) const
{
    //TODO: implement this
}

void SigmoidConstraint::addAuxiliaryEquations(__attribute__((unused)) InputQuery &inputQuery )
{
    //TODO: implement this
}

void SigmoidConstraint::getCostFunctionComponent(__attribute__((unused)) Map<unsigned, double> &cost ) const
{
    //TODO: implement this
}

String SigmoidConstraint::serializeToString() const
{
    // Output format is: sigmoid,f,b
    return Stringf( "sigmoid,%u,%u", _f, _b );
}

bool SigmoidConstraint::supportsSymbolicBoundTightening() const
{
    //TODO: implement this
    return false;
}

PiecewiseLinearConstraint& SigmoidConstraint::getConstraint()
{
    return *this;
}
