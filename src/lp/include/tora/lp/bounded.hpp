#pragma once
#include "primal_simplex.hpp"

namespace tora::lp {
// Captured shared method, with exact basic reduced costs in the Bounded
// scientific profile. Constraint residuals and earlier methods stay intact.
class BoundedSession final : public PrimalSimplexSession {
public:
    explicit BoundedSession(LpProblem problem)
        : PrimalSimplexSession(std::move(problem), PrimalMethod::Bounded, 0) {}
};
} // namespace tora::lp
