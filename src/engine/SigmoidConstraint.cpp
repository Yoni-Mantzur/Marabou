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

SigmoidConstraint::SigmoidConstraint(unsigned b, unsigned f)
        : _b(b), _f(f), _isBoundWereChanged(false), _haveEliminatedVariables(false), _isFixed(false) {
}

SigmoidConstraint::SigmoidConstraint(const String &serializedSigmoid)
        : _isBoundWereChanged(false), _haveEliminatedVariables(false), _isFixed(false) {
    const String &sigmoid = String("sigmoid");
    String constraintType = serializedSigmoid.substring(0, sigmoid.length());
    ASSERT(constraintType == sigmoid);

    // remove the constraint type in serialized form
    String serializedValues = serializedSigmoid.substring(sigmoid.length() + 1, serializedSigmoid.length() - 5);
    List<String> values = serializedValues.tokenize(",");

    ASSERT(values.size() == 2);

    auto var = values.begin();
    _f = atoi(var->ascii());
    ++var;
    _b = atoi(var->ascii());

    ASSERT(_b < _f)
    printf("b is: %d and _f is %d\n", _b, _f);

}

PiecewiseLinearConstraint *SigmoidConstraint::duplicateConstraint() const {
    SigmoidConstraint *clone = new SigmoidConstraint(_b, _f);
    *clone = *this;
    return clone;
}

void SigmoidConstraint::restoreState(const PiecewiseLinearConstraint *state) {
    const auto *sigmoid = dynamic_cast<const SigmoidConstraint *>( state );
    *this = *sigmoid;
}

void SigmoidConstraint::registerAsWatcher(ITableau *tableau) {
    tableau->registerToWatchVariable(this, _b);
    tableau->registerToWatchVariable(this, _f);
}

void SigmoidConstraint::unregisterAsWatcher(ITableau *tableau) {
    tableau->unregisterToWatchVariable(this, _b);
    tableau->unregisterToWatchVariable(this, _f);
}

void SigmoidConstraint::notifyVariableValue(unsigned variable, double value) {
    _assignment[variable] = value;
}

void SigmoidConstraint::notifyLowerBound(unsigned variable, double bound) {
    if (_statistics)
        _statistics->incNumBoundNotificationsPlConstraints();

    _lowerBounds[variable] = bound;
    _isBoundWereChanged = true;

    if (FloatUtils::areEqual(_lowerBounds[variable], _upperBounds[variable], 0.00001))
        _isFixed = true;

    if (_constraintBoundTightener) {
        if (variable == _b) {
            double sigmoidBound = FloatUtils::sigmoid(bound);
            _constraintBoundTightener->registerTighterLowerBound(_f, sigmoidBound);
        } else if (variable == _f) {
            double sigmoidInverseBound = FloatUtils::sigmoidInverse(bound);
            _constraintBoundTightener->registerTighterLowerBound(_b, sigmoidInverseBound);
        }
    }
}

void SigmoidConstraint::notifyUpperBound(unsigned variable, double bound) {
    if (_statistics)
        _statistics->incNumBoundNotificationsPlConstraints();

    _upperBounds[variable] = bound;
    _isBoundWereChanged = true;

    if (FloatUtils::areEqual(_lowerBounds[variable], _upperBounds[variable], 0.00001))
        _isFixed = true;

    if (_constraintBoundTightener) {
        if (variable == _b) {
            double sigmoidBound = FloatUtils::sigmoid(bound);
            _constraintBoundTightener->registerTighterUpperBound(_f, sigmoidBound);

        } else if (variable == _f) {
            double sigmoidInverseBound = FloatUtils::sigmoidInverse(bound);
            _constraintBoundTightener->registerTighterUpperBound(_b, sigmoidInverseBound);
        }
    }
}

void SigmoidConstraint::notifyBrokenAssignment() {
    ASSERT(_assignment.exists(_b) && _assignment.exists(_f))

    if (getConvexTypeInSegment(_lowerBounds[_b], _upperBounds[_b]))
        addSpuriousPoint({0.0, 0.5});
    else
        addSpuriousPoint({_assignment[_b], _assignment[_f]});
}

bool SigmoidConstraint::participatingVariable(unsigned variable) const {
    return (variable == _b || variable == _f);
}

List<unsigned> SigmoidConstraint::getParticipatingVariables() const {
    List<unsigned> participatingVariables;

    participatingVariables.append(_b);
    participatingVariables.append(_f);

    return participatingVariables;
}

bool SigmoidConstraint::satisfied() const {
    if (!(_assignment.exists(_b) && _assignment.exists(_f)))
        throw MarabouError(MarabouError::PARTICIPATING_VARIABLES_ABSENT);

    double bValue = _assignment.get(_b);
    double fValue = _assignment.get(_f);

    return FloatUtils::areEqual(FloatUtils::sigmoid(bValue), fValue,
                                GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE) ||
           FloatUtils::areEqual(bValue, FloatUtils::sigmoidInverse(fValue),
                                GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE);

}

List<PiecewiseLinearConstraint::Fix> SigmoidConstraint::getPossibleFixes() const {
    ASSERT(_assignment.exists(_b));
    ASSERT(_assignment.exists(_f));

    double bValue = _assignment.get(_b);
    double fValue = _assignment.get(_f);
    double sigmoidValue = FloatUtils::sigmoid(bValue);

    List<PiecewiseLinearConstraint::Fix> fixes;

    fixes.append(Fix(_f, sigmoidValue));

    double sigmoidInverseValue = FloatUtils::sigmoidInverse(fValue);
    fixes.append(Fix(_b, sigmoidInverseValue));

    return fixes;
}

PiecewiseLinearCaseSplit SigmoidConstraint::getValidCaseSplit() const {
    List<Equation> refinements;

    if (_isFixed) {
        std::cout << Stringf("Constraint %d is fixed, adding the const equations", sigmoid_num) << std::endl;
        PiecewiseLinearCaseSplit constraintToConst;

        Equation constEqForB(Equation::EQ);
        constEqForB.addAddend(1, _b);
        constEqForB.setScalar(_lowerBounds[_b]);

        ASSERT(FloatUtils::areEqual(_lowerBounds[_f], FloatUtils::sigmoid(_lowerBounds[_b]),
                                    GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
        Equation constEqForF(Equation::EQ);
        constEqForF.addAddend(1, _f);
        constEqForF.setScalar(_lowerBounds[_f]);

        constraintToConst.addEquation(constEqForB);
        constraintToConst.addEquation(constEqForF);

        constraintToConst.storeBoundTightening(Tightening(_b, _lowerBounds[_b], Tightening::LB));
        constraintToConst.storeBoundTightening(Tightening(_b, _lowerBounds[_b], Tightening::UB));

        constraintToConst.storeBoundTightening(Tightening(_f, _lowerBounds[_f], Tightening::LB));
        constraintToConst.storeBoundTightening(Tightening(_f, _lowerBounds[_f], Tightening::UB));

        return constraintToConst;
    }

    refinements.append(const_cast<SigmoidConstraint *>(this)->getEquationsAbstraction());

    if (GlobalConfiguration::REFINE_CURRENT_SPLIT_EQUATION) {
        if (_isBoundWereChanged) {

            try {
                refinements.append(const_cast<SigmoidConstraint *>(this)->refineCurrentSplit());
            } catch (...) {
                printf("Equation already was inserted\n");
            }
            const_cast<SigmoidConstraint *>(this)->_isBoundWereChanged = false;
        }
    }

    PiecewiseLinearCaseSplit serializedEquations;
    if (refinements.empty())
        throw 0;

    for (auto equation : refinements)
        serializedEquations.addEquation(equation);

    return serializedEquations;
}

List<PiecewiseLinearConstraint::Fix> SigmoidConstraint::getSmartFixes(__attribute__((unused)) ITableau *tableau) const {
    // TODO: write something smarter
    return getPossibleFixes();
}

List<PiecewiseLinearCaseSplit> SigmoidConstraint::getCaseSplits() const {
    ASSERT(_assignment.exists(_b));
    ASSERT(_assignment.exists(_f));
    ASSERT(_lowerBounds.exists(_b) && _lowerBounds.exists(_f));
    ASSERT(_upperBounds.exists(_b) && _upperBounds.exists(_f));

    return const_cast<SigmoidConstraint *>(this)->getSplitsAbstraction();
}

void SigmoidConstraint::dump(String &output) const {
    output = Stringf("SigmoidConstraint: x%u = sigmoid( x%u ).\n", _f, _b);

    output += Stringf("b in [%s, %s], ",
                      _lowerBounds.exists(_b) ? Stringf("%lf", _lowerBounds[_b]).ascii() : "-inf",
                      _upperBounds.exists(_b) ? Stringf("%lf", _upperBounds[_b]).ascii() : "inf");

    output += Stringf("f in [%s, %s]",
                      _lowerBounds.exists(_f) ? Stringf("%lf", _lowerBounds[_f]).ascii() : "-inf",
                      _upperBounds.exists(_f) ? Stringf("%lf", _upperBounds[_f]).ascii() : "inf");
}

void SigmoidConstraint::addAuxiliaryEquations(InputQuery &inputQuery) {
    try {
        inputQuery.addEquation((refineCurrentSplit()));
    } catch (...) {
        printf("Equation already was inserted\n");
    }
}

void SigmoidConstraint::eliminateVariable(unsigned /* variable */, double /* fixedValue */) {
    _haveEliminatedVariables = true;
}

bool SigmoidConstraint::constraintObsolete() const {
    return _haveEliminatedVariables;
}

void SigmoidConstraint::updateVariableIndex(unsigned oldIndex, unsigned newIndex) {
    ASSERT(oldIndex == _b || oldIndex == _f);
    ASSERT(!_assignment.exists(newIndex) &&
           !_lowerBounds.exists(newIndex) &&
           !_upperBounds.exists(newIndex) &&
           newIndex != _b && newIndex != _f);

    if (_assignment.exists(oldIndex)) {
        _assignment[newIndex] = _assignment.get(oldIndex);
        _assignment.erase(oldIndex);
    }

    if (_lowerBounds.exists(oldIndex)) {
        _lowerBounds[newIndex] = _lowerBounds.get(oldIndex);
        _lowerBounds.erase(oldIndex);
    }

    if (_upperBounds.exists(oldIndex)) {
        _upperBounds[newIndex] = _upperBounds.get(oldIndex);
        _upperBounds.erase(oldIndex);
    }

    if (oldIndex == _b)
        _b = newIndex;
    else
        _f = newIndex;
}

void SigmoidConstraint::getEntailedTightenings(List<Tightening> &tightenings) const {
    ASSERT(_lowerBounds.exists(_b) && _lowerBounds.exists(_f) &&
           _upperBounds.exists(_b) && _upperBounds.exists(_f));

    double bLowerBound = _lowerBounds[_b], sigmoidbLowerBound = FloatUtils::sigmoid(bLowerBound);
    double fLowerBound = _lowerBounds[_f], sigmoidInversefLowerBound = FloatUtils::sigmoidInverse(fLowerBound);

    double bUpperBound = _upperBounds[_b], sigmoidbUpperBound = FloatUtils::sigmoid(bUpperBound);
    double fUpperBound = _upperBounds[_f], sigmoidInversefUpperBound = FloatUtils::sigmoidInverse(fUpperBound);

    tightenings.append(Tightening(_b, sigmoidInversefLowerBound, Tightening::LB));
    tightenings.append(Tightening(_b, sigmoidInversefUpperBound, Tightening::UB));
    tightenings.append(Tightening(_f, sigmoidbLowerBound, Tightening::LB));
    tightenings.append(Tightening(_f, sigmoidbUpperBound, Tightening::UB));
}

void SigmoidConstraint::getCostFunctionComponent(__attribute__((unused)) Map<unsigned, double> &cost) const {
    //TODO: LEARN ABOUT THIS
}

String SigmoidConstraint::serializeToString() const {
    return Stringf("sigmoid,%u,%u", _f, _b);
}

double SigmoidConstraint::evaluateDerivativeOfConciseFunction(double x) const {
    return FloatUtils::sigmoidDerivative(x);
}


SigmoidConstraint::Point SigmoidConstraint::getLowerParticipantVariablesBounds() const {
    ASSERT(_lowerBounds.exists(_b) && _lowerBounds.exists(_f))

    return {_lowerBounds[_b], _lowerBounds[_f]};
}

SigmoidConstraint::Point SigmoidConstraint::getUpperParticipantVariablesBounds() const {
    ASSERT(_upperBounds.exists(_b) && _upperBounds.exists(_f))

    return {_upperBounds[_b], _upperBounds[_f]};
}

SigmoidConstraint::ConvexType SigmoidConstraint::getConvexTypeInSegment(double x0, double x1) const {
    if (FloatUtils::lte(x0, 0.0, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE) &&
        FloatUtils::lte(x1, 0.0, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
        return CONVEX;

    if (FloatUtils::gte(x0, 0.0, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE) &&
        FloatUtils::gte(x1, 0.0, GlobalConfiguration::SIGMOID_CONSTRAINT_COMPARISON_TOLERANCE))
        return CONCAVE;

    return UNKNOWN;
}

void SigmoidConstraint::setActiveConstraint( bool active )
{
    _constraintActive = active;

    // Sigmoid constraint should be always active if it's not fixed
    if ( !_isFixed )
        _constraintActive = true;
}


/** Debuging **/

void SigmoidConstraint::writePoint(double x, double y, bool isFix) {
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

void SigmoidConstraint::writeLimit(double lower, double upper, bool isB) {
    _logFile->open(IFile::MODE_WRITE_APPEND);
    _logFile->write("L,");
    isB ? _logFile->write("b," + std::to_string(_b) + ",") :
    _logFile->write("f," + std::to_string(_f) + ",");
    _logFile->write(std::to_string(lower));
    _logFile->write(",");
    _logFile->write(std::to_string(upper));
    _logFile->write("\n");
    _logFile->close();
}


void SigmoidConstraint::writeEquations(List<Equation> eqs) {
    _logFile->open(IFile::MODE_WRITE_APPEND);
    for (auto eq : eqs) {
        _logFile->write("E,");
        for (Equation::Addend addend : eq._addends) {
            _logFile->write(std::to_string(addend._coefficient));
            addend._variable == _b ?
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
                } else if (bound._variable == _f) {
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
        if (fValue < 1 && fValue > 0)
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