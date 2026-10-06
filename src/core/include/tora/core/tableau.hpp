#pragma once

#include "matrix.hpp"
#include "tolerance.hpp"
#include <string>
#include <vector>

namespace tora::core {
struct PivotResult {
    std::size_t row;
    std::size_t column;
    std::size_t previousBasicColumn;
    Scalar value;
};

// Objective row is 0, RHS is the final matrix column. Column/basis ordering
// is caller supplied. This performs a requested pivot, never chooses one.
class Tableau {
public:
    Tableau(Matrix values, std::vector<std::string> columns, std::vector<std::size_t> basis);
    const Matrix& values() const noexcept { return values_; }
    const std::vector<std::string>& columns() const noexcept { return columns_; }
    const std::vector<std::size_t>& basis() const noexcept { return basis_; }
    const std::string& basicLabel(std::size_t row) const;
    Scalar rhs(std::size_t row) const { return values_(row, columns_.size()); }
    bool objectiveBlocked(std::size_t column) const { return blocked_.at(column); }
    void setObjectiveBlocked(std::size_t column, bool blocked) { blocked_.at(column) = blocked; }
    PivotResult pivot(std::size_t row, std::size_t column, const Tolerance& tolerance);
private:
    Matrix values_;
    std::vector<std::string> columns_;
    std::vector<std::size_t> basis_;
    std::vector<bool> blocked_;
};
} // namespace tora::core
