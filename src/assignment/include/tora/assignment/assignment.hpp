#pragma once
#include <tora/core/matrix.hpp>
#include <optional>

namespace tora::assignment {
enum class Sense { Minimize, Maximize };
struct Problem {
    core::Matrix costs;
    Sense sense=Sense::Minimize;
};
struct Result {
    core::Scalar objective=0;
    std::vector<std::optional<std::size_t>> rowToColumn;
    // Certificate for the square, zero-padded minimization problem. For
    // maximization its costs are negated. Indices are zero-based.
    std::vector<std::size_t> paddedAssignment;
    std::vector<core::Scalar> rowPotential,columnPotential;
};
// Exactly min(rows,columns) real pairs, remaining rows explicitly unassigned.
// Finite costs, 1..100 rows/columns. Overflow is diagnosed, never returned as
// an optimal answer. Equal candidates use ascending column order.
Result solve(const Problem& problem);
} // namespace tora::assignment
