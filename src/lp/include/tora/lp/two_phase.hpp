#pragma once
#include "primal_simplex.hpp"

namespace tora::lp {
class TwoPhaseSession final:public PrimalSimplexSession {
public:
    explicit TwoPhaseSession(LpProblem problem):PrimalSimplexSession(std::move(problem),PrimalMethod::TwoPhase,0) {}
};
} // namespace tora::lp
