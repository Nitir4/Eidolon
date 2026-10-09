#pragma once
#include "primal_simplex.hpp"

namespace tora::lp {
class DualSession final:public PrimalSimplexSession {
public:
    explicit DualSession(LpProblem problem)
        :PrimalSimplexSession(std::move(problem),PrimalMethod::Dual,0) {}
};
} // namespace tora::lp
