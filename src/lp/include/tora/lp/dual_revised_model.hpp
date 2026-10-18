#pragma once
#include "dual_revised.hpp"

namespace tora::lp {
struct RevisedDualVariable {
    Scalar shift=0;
    std::size_t positiveColumn=0;
    std::optional<std::size_t> negativeColumn;
};
struct RevisedDualModel {
    LpProblem original, nonnegative;
    core::Matrix matrix;
    std::vector<Scalar> rhs, costs;
    std::vector<RevisedDualVariable> variables;
    std::vector<std::optional<std::size_t>> rowSlacks;
    Scalar objectiveSign=1, objectiveOffset=0;
};
struct RevisedDualLpResult {
    RevisedDualResult canonical;
    std::vector<Scalar> values; // original variables; only populated for Optimal
    std::optional<Scalar> objective;
};
// Stable standard form: shifts finite lower bounds, splits free variables,
// appends upper-bound rows, negates >= rows; never changes A based on RHS sign.
RevisedDualModel makeDualRevisedModel(const LpProblem& problem);
// Empty basis uses inequality slacks; equalities require a supplied basis.
RevisedDualLpResult solveDualRevisedModel(const RevisedDualModel& model,
    std::vector<std::size_t> basis={},std::size_t iterationLimit=1024,Scalar tolerance=1e-9);
// Explicit bootstrap: existing primal/two-phase solver finds a baseline optimum.
// Rejects rank-dependent rows if zero artificial basics cannot be removed.
std::vector<std::size_t> prepareDualRevisedBasis(const RevisedDualModel& baseline,
    std::size_t iterationLimit=1024,Scalar tolerance=1e-9);
// Bootstrap baseline once, then true revised-dual RHS reoptimization.
// Empty replacement RHS means unchanged; no fresh changed-model primal solve.
RevisedDualLpResult reoptimizeDualRevisedRhs(const LpProblem& baseline,
    const std::vector<Scalar>& rhs,std::size_t iterationLimit=1024,Scalar tolerance=1e-9);
}
