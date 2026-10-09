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
// One-at-a-time retained-basis intervals, expressed in the captured TORA
// summary convention. Terminal infeasible/unbounded sessions return no rows.
SensitivityReport sensitivity(const FinalSolutionSession& session);
} // namespace tora::lp
