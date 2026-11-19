#pragma once
#include "final_solution.hpp"
#include <limits>

namespace tora::lp {
struct SensitivityRange {
    Scalar minimum=-std::numeric_limits<Scalar>::infinity();
    Scalar maximum=std::numeric_limits<Scalar>::infinity();
};
struct SensitivityValue {
    Scalar current=0;
    SensitivityRange range;
    Scalar price=0; // reduced cost for a variable; dual price for a row
};
struct SensitivityReport {
    std::vector<SensitivityValue> variables;
    std::vector<SensitivityValue> constraints;
};
// One-at-a-time intervals in original model coordinates, retaining the final
// basis and column orientations. Other bases may preserve an optimal point
// outside these intervals. Infeasible/unbounded sessions return no rows.
SensitivityReport sensitivity(const FinalSolutionSession& session);
} // namespace tora::lp
