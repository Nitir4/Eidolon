#pragma once
#include "primal_simplex.hpp"
namespace tora::lp {
class AllSlackSession final:public PrimalSimplexSession {
public:
    explicit AllSlackSession(LpProblem problem):PrimalSimplexSession(std::move(problem),PrimalMethod::AllSlack,0){}
};
} // namespace tora::lp
