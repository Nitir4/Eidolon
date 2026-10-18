#pragma once
#include "final_solution.hpp"

namespace tora::lp {
struct VariableTerm {std::size_t column;Scalar coefficient;};
struct VariableMapping {
    Scalar shift=0;
    std::vector<VariableTerm> terms;
};
struct PostOptimalOutcome {
    SimplexStatus status=SimplexStatus::Running;
    std::vector<Scalar> values;
    std::optional<Scalar> objective;
    std::vector<SimplexStep> trace; // nonnegative canonical variables, before restoration
};
struct RetainedBasis {
    std::vector<std::size_t> columns;
    std::vector<Scalar> basisValues;
    std::vector<Scalar> reducedCosts; // z_j-c_j, including slack/artificial columns
    std::vector<Scalar> values; // restored original variables, even if infeasible
    Scalar objective=0;
    bool primalFeasible=false;
    bool dualFeasible=false;
};
struct PostOptimalResult {
    LpProblem changedProblem;
    LpProblem canonicalProblem; // changed model, with finite upper bounds as rows
    std::vector<VariableMapping> mappings;
    std::vector<std::string> canonicalVariables;
    PostOptimalOutcome before,after;
    RetainedBasis retained;
};
// Apply full replacement Cj/RHS vectors; empty means unchanged. The baseline
// must have a finite optimum. Bounds/free variables are transformed explicitly;
// a fresh changed-model solve is returned alongside the retained-basis analysis.
// Canonical size obeys the solver's existing 100-variable/100-row limit.
PostOptimalResult analyzeChanges(const LpProblem& problem,
    const std::vector<Scalar>& objective={},const std::vector<Scalar>& rhs={},
    const core::Tolerance& tolerance=core::Tolerance{},std::size_t iterationLimit=1024);
} // namespace tora::lp
