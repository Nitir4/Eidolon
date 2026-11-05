#pragma once
#include <tora/core/matrix.hpp>
#include <tora/core/tolerance.hpp>
#include <tora/lp/problem.hpp>
#include <utility>

namespace tora::games {
struct Result {
    std::vector<core::Scalar> rowStrategy,columnStrategy;
    core::Scalar value=0,lowerBound=0,upperBound=0,dualityGap=0;
    std::vector<std::pair<std::size_t,std::size_t>> saddlePoints;
};
// Payoff is to row player A (maximizer); column player B minimizes.
// Finite 1..100 strategy matrices. Mixed equilibria carry normalized
// probability and saddle-inequality checks; no strategy uniqueness claim.
Result solve(const core::Matrix& payoff,
    const core::Tolerance& tolerance=core::Tolerance(1e-9,1e-10),std::size_t iterationLimit=1024);
// Original row-player formulation: maximize free v, v-A^T*p<=0, sum(p)=1.
// This data model can have 101 rows/variables at the game-size limit.
lp::LpProblem rowPlayerLp(const core::Matrix& payoff);
} // namespace tora::games
