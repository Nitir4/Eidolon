#pragma once
#include <tora/core/matrix.hpp>
#include <string>
#include <optional>
#include <vector>
namespace tora::equations {
enum class Status { Unique, Infinite, Inconsistent };
enum class Operation { Scale, Swap, Eliminate };
struct Step {
    Operation operation;
    std::size_t row = 0, otherRow = 0, column = 0;
    double factor = 0;
    core::Matrix augmented;
};
struct Options {
    double relativeTolerance = 1e-12;
    bool recordTrace = false;
    std::size_t maximumTraceSteps = 1000;
};
struct Result {
    Status status = Status::Inconsistent;
    std::size_t rank = 0, augmentedRank = 0;
    std::vector<std::size_t> pivotColumns, freeColumns;
    std::vector<double> solution;
    // Each vector z satisfies A z = 0; x = solution + sum(t_i z_i).
    std::vector<std::vector<double>> nullspace;
    core::Matrix reducedAugmented;
    std::vector<Step> trace;
    bool traceTruncated = false;
    double maximumResidual = 0, relativeResidual = 0;
    // For inconsistent systems, y^T A ~= 0 and y^T b != 0.
    std::vector<double> inconsistencyWitness;
    double witnessRhs = 0, witnessCoefficientResidual = 0;
};
// Nonempty finite matrices up to 100 x 100. Numerical rank is tolerance dependent.
Result solve(const core::Matrix& coefficients, const std::vector<double>& rhs,
             const Options& options = {});
struct InverseResult {
    bool invertible = false;
    core::Matrix inverse;
    std::size_t rank = 0;
    double maximumIdentityResidual = 0;
    // Row-operation snapshots contain [A | I], ending in [I | inverse].
    std::vector<Step> trace;
    bool traceTruncated = false;
    int determinantSign = 0;
    long double logAbsDeterminant = 0;
    std::optional<double> determinant;
};
InverseResult invert(const core::Matrix& coefficients, const Options& options = {});
} // namespace tora::equations
