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
    // For debugging
    dumpRestore(true);

    const auto *sigmoid = dynamic_cast<const SigmoidConstraint *>( state );
    *this = *sigmoid;

    // For debugging
    dumpRestore(false);
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
    if ( FloatUtils::isZero(value, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE ) )
        value = 0.0;

    _assignment[variable] = value;
}

void SigmoidConstraint::notifyLowerBound( unsigned variable, double bound )
{
    if (_statistics)
        _statistics->incNumBoundNotificationsPlConstraints();

    if (_lowerBounds.exists(variable) && !FloatUtils::gt(bound, _lowerBounds[variable]))
        return;

    if (variable == _f && !isValueInSigmoidBounds(bound))
        bound = GlobalConfiguration::SIGMOID_DEFAULT_LOWER_BOUND;

    _lowerBounds[variable] = bound;

    if (variable == _b && _constraintBoundTightener)
    {
        double sigmoidBound = FloatUtils::sigmoid(_lowerBounds[variable]);
        if (!_lowerBounds.exists(_f) || FloatUtils::gt(sigmoidBound, _lowerBounds[_f]))
            _constraintBoundTightener->registerTighterLowerBound(_f, sigmoidBound);

    }
    else if (variable == _f && _constraintBoundTightener)
    {
        double sigmoidInverseBound = FloatUtils::sigmoidInverse( _lowerBounds[variable]);
        if (!_lowerBounds.exists(_b) || FloatUtils::gt(sigmoidInverseBound, _lowerBounds[_b]))
            _constraintBoundTightener->registerTighterLowerBound(_b, sigmoidInverseBound);
    }
}

void SigmoidConstraint::notifyUpperBound( unsigned variable, double bound )
{
    if ( _statistics )
        _statistics->incNumBoundNotificationsPlConstraints();

    if ( _upperBounds.exists( variable ) && !FloatUtils::lt( bound, _upperBounds[variable] ) )
        return;

    if (variable == _f && !isValueInSigmoidBounds(bound))
        bound = GlobalConfiguration::SIGMOID_DEFAULT_UPPER_BOUND;

    _upperBounds[variable] = bound;

    if (variable == _b && _constraintBoundTightener)
    {
        double sigmoidBound = FloatUtils::sigmoid(_upperBounds[variable]);
        if (!_upperBounds.exists(_f) || FloatUtils::lt(sigmoidBound, _upperBounds[_f] ) )
            _constraintBoundTightener->registerTighterUpperBound(_f, sigmoidBound);

    }
    else if (variable == _f && _constraintBoundTightener)
    {
        double sigmoidInverseBound = FloatUtils::sigmoidInverse(_upperBounds[variable]);
        if (!_upperBounds.exists(_b) || FloatUtils::lt(sigmoidInverseBound, _upperBounds[_b]))
            _constraintBoundTightener->registerTighterUpperBound(_b, sigmoidInverseBound);
    }
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

    // Debugging
    dumpAssignment(bValue, fValue);

    return FloatUtils::areEqual(FloatUtils::sigmoid(bValue), fValue,
            GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE);

}

List<PiecewiseLinearConstraint::Fix> SigmoidConstraint::getPossibleFixes() const
{
    ASSERT(_assignment.exists(_b));
    ASSERT(_assignment.exists(_f));

    double bValue = _assignment.get(_b);
    double fValue = _assignment.get(_f);
    double sigmoidValue = FloatUtils::sigmoid(bValue);

    List <PiecewiseLinearConstraint::Fix> fixes;

    fixes.append(Fix(_f, sigmoidValue ));

    // If fValue is out of bounds, can fix it
    if (isValueInSigmoidBounds( fValue ) )
    {
        double sigmoidInverseValue = FloatUtils::sigmoidInverse(fValue);
        fixes.append(Fix(_b, sigmoidInverseValue ));
    }

    // Upper bound refinement
//    refineUpperBounds(bValue, sigmoidValue);

    // Debugging
    dumpFixes(bValue, fValue, sigmoidValue);

    return fixes;
}

void SigmoidConstraint::refineBounds() const {
    ASSERT(_assignment.exists(_b));
    double bValue = _assignment.get(_b);
    double sigmoidValue = FloatUtils::sigmoid(bValue);

    List<GuidedPoint> guidedPoints;
    guidedPoints.append(GuidedPoint(bValue, sigmoidValue));
    List<Equation> refinements = getRefinedUpperAbstraction(guidedPoints);

    for (Equation equation : refinements) {
        List<Tightening> auxBounds = _engine->addEquation(equation);
        _engine->tightenBounds(auxBounds);
    }

    // Debugging
    dumpUpperBound(refinements);
}

List<PiecewiseLinearConstraint::Fix> SigmoidConstraint::getSmartFixes(__attribute__((unused)) ITableau *tableau ) const
{
    // TODO: write something smarter
    return getPossibleFixes();
}

List<PiecewiseLinearCaseSplit> SigmoidConstraint::getCaseSplits() const
{
    ASSERT(_assignment.exists(_b));
    ASSERT(_assignment.exists(_f));
    ASSERT(_lowerBounds.exists(_b) && _lowerBounds.exists(_f));
    ASSERT(_upperBounds.exists(_b) && _upperBounds.exists(_f));

    double bValue = _assignment.get(_b);
    double sigmoidValue = FloatUtils::sigmoid(bValue);


    // TODO: guided points should be append in satisfied
    List<GuidedPoint> guidedPoints;
    guidedPoints.append(GuidedPoint(_lowerBounds[_b], _lowerBounds[_f]));
    guidedPoints.append(GuidedPoint(bValue, sigmoidValue));
    guidedPoints.append(GuidedPoint(_upperBounds[_b], _upperBounds[_f]));

    List<PiecewiseLinearCaseSplit> splits = getRefinedLowerAbstraction(guidedPoints);

    // Debugging
    dumpSplits(splits);

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

void SigmoidConstraint::getEntailedTightenings(List<Tightening> &tightenings ) const
{
    ASSERT( _lowerBounds.exists( _b ) && _lowerBounds.exists( _f ) &&
            _upperBounds.exists( _b ) && _upperBounds.exists( _f ) );
    ASSERT(isValueInSigmoidBounds(_lowerBounds[_f]) && isValueInSigmoidBounds(_upperBounds[_f]));

    double bLowerBound = _lowerBounds[_b], sigmoidbLowerBound = FloatUtils::sigmoid( bLowerBound );
    double fLowerBound = _lowerBounds[_f], sigmoidInversefLowerBound = FloatUtils::sigmoidInverse( fLowerBound );

    double bUpperBound = _upperBounds[_b], sigmoidbUpperBound = FloatUtils::sigmoid( bUpperBound );
    double fUpperBound = _upperBounds[_f], sigmoidInversefUpperBound = FloatUtils::sigmoidInverse( fUpperBound );

    if (FloatUtils::lt(bLowerBound, sigmoidInversefLowerBound))
        tightenings.append(Tightening(_b, sigmoidInversefLowerBound, Tightening::LB));

    if (FloatUtils::lt(fLowerBound, sigmoidbLowerBound))
        tightenings.append(Tightening(_f, sigmoidbLowerBound, Tightening::LB));

    if (FloatUtils::gt(bUpperBound, sigmoidInversefUpperBound))
        tightenings.append(Tightening(_b, sigmoidInversefUpperBound, Tightening::UB));

    if (FloatUtils::gt(fUpperBound, sigmoidbUpperBound))
        tightenings.append(Tightening(_f, sigmoidbUpperBound, Tightening::UB));
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

bool SigmoidConstraint::isValueInSigmoidBounds(double value) const
{
    return value < 1.0 && value > -1.0;
}

double SigmoidConstraint::evaluateDerivativeOfConciseFunction(double x) const
{
    double sigmoidValue = FloatUtils::sigmoid(x);
    return sigmoidValue * ( 1 - sigmoidValue );
}












/** Debuging **/

void SigmoidConstraint::writePoint(double x, double y, bool isFix)
{
    _logFile->open(IFile::MODE_WRITE_APPEND);
    if (isFix)
        _logFile->write("F,");
    else
        _logFile->write("P,");
    _logFile->write(std::to_string(x));
    _logFile->write(",");
    _logFile->write(std::to_string(y));
    _logFile->write("\n");
    _logFile->close();
}

void SigmoidConstraint::writeLimit(double lower, double upper, bool isB)
{
    _logFile->open(IFile::MODE_WRITE_APPEND);
    _logFile->write("L,");
    isB? _logFile->write("b," + std::to_string(_b) + ",") :
         _logFile->write("f," + std::to_string(_f) + ",");
    _logFile->write(std::to_string(lower));
    _logFile->write(",");
    _logFile->write(std::to_string(upper));
    _logFile->write("\n");
    _logFile->close();
}


void SigmoidConstraint::writeEquations(List<Equation> eqs)
{
    _logFile->open(IFile::MODE_WRITE_APPEND);
    for (auto eq : eqs) {
        _logFile->write("E,");
        for (Equation::Addend addend : eq._addends) {
            _logFile->write(std::to_string(addend._coefficient));
            addend._variable == _b?
            _logFile->write("b(" + std::to_string(_b) + ")") :
            _logFile->write("f(" + std::to_string(_f) + ")");
        }

        if (eq._type == Equation::EquationType::GE)
            _logFile->write(">=");

        else if (eq._type == Equation::EquationType::LE)
            _logFile->write("<=");

        else
            _logFile->write("=");

        _logFile->write(std::to_string(eq._scalar));
        _logFile->write("\n");
    }
    _logFile->close();
}


void SigmoidConstraint::dumpSplits(const List<PiecewiseLinearCaseSplit> &splits) const {
    if (_logFile != nullptr) {

        auto *s = const_cast<SigmoidConstraint *>(this);
        s->_iter_case_splits++;
        s->_logFile->open(IFile::MODE_WRITE_APPEND);
        s->_logFile->write("\nSigmoid " + std::__cxx11::to_string(
                sigmoid_num) + "\nIteration: "
                           + std::__cxx11::to_string(s->_iter_case_splits)
                           + " case_splits() was called\n");
        s->_logFile->close();
        for (PiecewiseLinearCaseSplit split : splits) {
            s->writeEquations(split.getEquations());
            List<Tightening> b = split.getBoundTightenings();
            double bounds[4] = {0};
            for (Tightening bound : b) {
                if (bound._variable == _b) {
                    if (bound._type == bound.LB)
                        bounds[0] = bound._value;
                    else
                        bounds[1] = bound._value;
                } else if (bound._variable == _f){
                    if (bound._type == bound.LB)
                        bounds[2] = bound._value;
                    else
                        bounds[3] = bound._value;
                }
            }
            s->writeLimit(bounds[0], bounds[1], true);
            s->writeLimit(bounds[2], bounds[3], false);
        }
    }
}

void SigmoidConstraint::dumpUpperBound(const List<Equation> &refinements) const {
    if (_logFile != nullptr) {

        auto *s = const_cast<SigmoidConstraint *>(this);

        s->_logFile->open(IFile::MODE_WRITE_APPEND);
        s->_logFile->write("\nSigmoid " + std::__cxx11::to_string(
                sigmoid_num) +
                           "\nUpper Bound\n");
        s->_logFile->close();
        s->writeLimit(_lowerBounds[_b], _upperBounds[_b], true);
        s->writeEquations(refinements);
    }
}

void SigmoidConstraint::dumpAssignment(double bValue, double fValue) const {
    if (_logFile != nullptr) {
        auto *s = const_cast<SigmoidConstraint *>(this);
        s->_iter_satisfied++;
        s->_logFile->open(IFile::MODE_WRITE_APPEND);
        s->_logFile->write("\nSigmoid " + std::__cxx11::to_string(sigmoid_num) +
                           "\nIteration: " + std::__cxx11::to_string(s->_iter_satisfied)
                           + " satisfied() was called\n");
        s->_logFile->close();
        s->writePoint(bValue, fValue);
        s->writeLimit(_lowerBounds[_b], _upperBounds[_b], true);
        s->writeLimit(_lowerBounds[_f], _upperBounds[_f]);
    }
}

void SigmoidConstraint::dumpFixes(double bValue, double fValue,
                                  double sigmoidValue) const {
    if (_logFile != nullptr) {

        auto *s = const_cast<SigmoidConstraint *>(this);
        // Equations before:
        s->_iter_fixes++;
        s->_logFile->open(IFile::MODE_WRITE_APPEND);
        s->_logFile->write("\nSigmoid " + std::__cxx11::to_string(sigmoid_num)
        + "\nIteration: " + std::__cxx11::to_string(s->_iter_fixes)
        + " fixes() was called\n");
        s->_logFile->close();
        s->writePoint(bValue, sigmoidValue, true);
        if (isValueInSigmoidBounds(fValue))
            s->writePoint(FloatUtils::sigmoidInverse(fValue), fValue, true);
    }
}

void SigmoidConstraint::dumpRestore(bool isBefore) const {
    if (_logFile != nullptr) {
        auto *s = const_cast<SigmoidConstraint *>(this);
        if (isBefore) {
            // Equations before:
            s->_restore++;
            s->_logFile->open(IFile::MODE_WRITE_APPEND);
            s->_logFile->write("\nSigmoid " + std::__cxx11::to_string(
                    sigmoid_num) +
                               "\nIteration: " +
                               std::__cxx11::to_string(s->_restore) +
                               " restore() was called\n");
            s->_logFile->close();
            s->_logFile->open(IFile::MODE_WRITE_APPEND);
            s->_logFile->write("Limits Before\n");
            s->_logFile->close();
            s->writeLimit(_lowerBounds[_b], _upperBounds[_b], true);
            s->writeLimit(_lowerBounds[_f], _upperBounds[_f]);
        } else {
            s->_logFile->open(IFile::MODE_WRITE_APPEND);
            s->_logFile->write("Limits after\n");
            s->_logFile->close();
            s->writeLimit(_lowerBounds[_b], _upperBounds[_b], true);
            s->writeLimit(_lowerBounds[_f], _upperBounds[_f]);
        }
    }
}