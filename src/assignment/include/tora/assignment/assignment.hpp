#pragma once
#include <tora/core/matrix.hpp>
#include <optional>
#include <string>
#include <utility>

namespace tora::assignment {
enum class Sense { Minimize, Maximize };
struct Problem {
    core::Matrix costs;
    Sense sense=Sense::Minimize;
    std::vector<std::pair<std::size_t,std::size_t>> forbidden={};
};
enum class Status { Optimal, Infeasible };
struct Step {
    std::string event;
    std::size_t root=0,column=0; // One-based; column zero is the search sentinel.
    core::Scalar delta=0;
    std::vector<core::Scalar> rowPotential,columnPotential,distance;
    std::vector<std::size_t> matched,previous; // Column -> row, including sentinel.
    std::vector<bool> used;
};
struct Result {
    Status status=Status::Optimal;
    core::Scalar objective=0;
    std::vector<std::optional<std::size_t>> rowToColumn;
    // Certificate for the square, zero-padded minimization problem. For
    // maximization its costs are negated. Forbidden edges are excluded from
    // the dual inequalities. Indices are zero-based.
    std::vector<std::size_t> paddedAssignment;
    std::vector<core::Scalar> rowPotential,columnPotential;
    core::Matrix transformedCosts;
    std::vector<Step> history;
};
// Exactly min(rows,columns) real pairs, remaining rows explicitly unassigned.
// Finite costs, 1..100 rows/columns. Overflow is diagnosed, never returned as
// an optimal answer. Equal candidates use ascending column order.
Result solve(const Problem& problem);
} // namespace tora::assignment
