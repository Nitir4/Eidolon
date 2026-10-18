#pragma once
#include "problem.hpp"
#include <optional>

namespace tora::lp {
enum class RevisedDualStatus { Optimal, Infeasible, Limit };
struct RevisedDualStep {
    std::vector<std::size_t> basis;
    std::vector<Scalar> basicValues, dual, reducedCosts;
    std::optional<std::size_t> leavingRow, enteringColumn;
};
struct RevisedDualResult {
    RevisedDualStatus status=RevisedDualStatus::Limit;
    std::vector<RevisedDualStep> trace;
    std::vector<Scalar> values, infeasibilityWitness;
    Scalar objective=0;
};
// min c*x, A*x=b, x>=0. Supplied basis must be nonsingular and dual feasible.
// See docs/tora-analysis/LP_DUAL_REVISED.md for limits and certificates.
RevisedDualResult solveDualRevised(const core::Matrix& a,
    const std::vector<Scalar>& b,const std::vector<Scalar>& c,
    std::vector<std::size_t> basis,std::size_t iterationLimit=1024,
    Scalar tolerance=1e-9);
}
