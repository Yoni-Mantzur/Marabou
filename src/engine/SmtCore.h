/*********************                                                        */
/*! \file SmtCore.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Guy Katz, Parth Shah
 ** This file is part of the Marabou project.
 ** Copyright (c) 2017-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved. See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** [[ Add lengthier description here ]]

**/

#ifndef __SmtCore_h__
#define __SmtCore_h__

#include <cmath>
#include "PiecewiseLinearCaseSplit.h"
#include "PiecewiseLinearConstraint.h"
#include "Stack.h"
#include "Statistics.h"
#include "File.h"
#include "MStringf.h"

class EngineState;
class IEngine;
class String;

class SmtCore
{
public:
    class SigmoidStats{
    public:
        int id = 0;
        bool visited = false;
        double seg_low = 0;
        double seg_upper = 0;
        int number_states = 0;
        int total_depth = 0;
        int vistied_states = 0;

        SigmoidStats *left = nullptr;
        SigmoidStats *right = nullptr;


        void dump() {
            File *_logFile = new File("sigmoid_stats.log");
            _logFile->open(IFile::MODE_WRITE_APPEND);
            Stringf s("");
            _logFile->write(s.ascii());
            _logFile->close();
        }

        SigmoidStats * get(int id)
        {
            if (this->id == id)
                return this;

            if (left == NULL || right == NULL) {
                return nullptr;
            }

            auto l = left->get(id);
            if (l != NULL)
                return l;

            auto r = right->get(id);
            return r;
        }

        void append(int id) {
            SigmoidStats *split = get(id);
            static int u_id = 0;
            if (split->left == NULL) {
                split->left = new SigmoidStats();
                split->left->id = (++u_id);
            }

            if (split->right == NULL) {
                split->right = new SigmoidStats();
                split->right->id = (++u_id);

            }
        }

    };
    SmtCore( IEngine *engine );
    ~SmtCore();

    /*
      Clear the stack.
    */
    void freeMemory();

    /*
      Inform the SMT core that a PL constraint is violated.
    */
    void reportViolatedConstraint( PiecewiseLinearConstraint *constraint );

    /*
      Get the number of times a specific PL constraint has been reported as
      violated.
    */
    unsigned getViolationCounts( PiecewiseLinearConstraint* constraint ) const;

    /*
      Reset all reported violation counts.
    */
    void resetReportedViolations();

    /*
      Returns true iff the SMT core wants to perform a case split.
    */
    bool needToSplit() const;

    /*
      Perform the split according to the constraint marked for
      splitting. Update bounds, add equations and update the stack.
    */
    void performSplit();

    /*
      Pop an old split from the stack, and perform a new split as
      needed. Return true if successful, false if the stack is empty.
    */
    bool popSplit();

    /*
      The current stack depth.
    */
    unsigned getStackDepth() const;

    /*
      Let the smt core know of an implied valid case split that was discovered.
    */
    void recordImpliedValidSplit( PiecewiseLinearCaseSplit &validSplit );

    /*
      Return a list of all splits performed so far, both SMT-originating and valid ones,
      in the correct order.
    */
    void allSplitsSoFar( List<PiecewiseLinearCaseSplit> &result ) const;

    /*
      Have the SMT core start reporting statistics.
    */
    void setStatistics( Statistics *statistics );

    /*
      Have the SMT core choose, among a set of violated PL constraints, which
      constraint should be repaired (without splitting)
    */
    PiecewiseLinearConstraint *chooseViolatedConstraintForFixing( List<PiecewiseLinearConstraint *> &_violatedPlConstraints ) const;

    /*
      For debugging purposes only - store a correct possible solution
    */
    void storeDebuggingSolution( const Map<unsigned, double> &debuggingSolution );
    bool checkSkewFromDebuggingSolution();
    bool splitAllowsStoredSolution( const PiecewiseLinearCaseSplit &split, String &error ) const;

private:
    /*
      A stack entry consists of the engine state before the split,
      the active split, the alternative splits (in case of backtrack),
      and also any implied splits that were discovered subsequently.
    */
    struct StackEntry
    {
    public:
        PiecewiseLinearCaseSplit _activeSplit;
        List<PiecewiseLinearCaseSplit> _impliedValidSplits;
        List<PiecewiseLinearCaseSplit> _alternativeSplits;
        EngineState *_engineState;
        int sig_num;
    };

    /*
      Valid splits that were implied by level 0 of the stack.
    */
    List<PiecewiseLinearCaseSplit> _impliedValidSplitsAtRoot;

    /*
      Collect and print various statistics.
    */
    Statistics *_statistics;

    /*
      The case-split stack.
    */
    List<StackEntry *> _stack;

    /*
      The engine.
    */
    IEngine *_engine;

    /*
      Do we need to perform a split and on which constraint.
    */
    bool _needToSplit;
    PiecewiseLinearConstraint *_constraintForSplitting;

    /*
      Count how many times each constraint has been violated.
    */
    Map<PiecewiseLinearConstraint *, unsigned> _constraintToViolationCount;

    static void log( const String &message );

    /*
      For debugging purposes only
    */
    Map<unsigned, double> _debuggingSolution;

    /*
      A unique ID allocated to every state that is stored, for
      debugging purposes.
    */
    unsigned _stateId;

    Map<int, SigmoidStats> _sigmoids;
    Map<int, Stack<int>> ids;
};

#endif // __SmtCore_h__

//
// Local Variables:
// compile-command: "make -C ../.. "
// tags-file-name: "../../TAGS"
// c-basic-offset: 4
// End:
//
