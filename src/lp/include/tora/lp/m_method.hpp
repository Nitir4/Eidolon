#pragma once
#include "primal_simplex.hpp"
namespace tora::lp {
// Finite numerical penalty, as observed in TORA. This is not symbolic Big-M.
class MMethodSession final:public PrimalSimplexSession {
public:
    explicit MMethodSession(LpProblem problem,Scalar penalty=100):PrimalSimplexSession(std::move(problem),PrimalMethod::MMethod,penalty){}
    static const char* allSlackWarning(){return "Problem has all <= constraints. All-Slack option will be used";}
};
} // namespace tora::lp
