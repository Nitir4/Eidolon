#include <tora/core/tableau.hpp>

#include <cmath>
#include <set>
#include <stdexcept>
#include <utility>

namespace tora::core {
Tableau::Tableau(Matrix values, std::vector<std::string> columns, std::vector<std::size_t> basis)
    : values_(std::move(values)), columns_(std::move(columns)), basis_(std::move(basis)), blocked_(columns_.size(), false)
{
    if (columns_.empty() || values_.columns() != columns_.size() + 1 || values_.rows() != basis_.size() + 1)
        throw std::invalid_argument("Tableau dimensions do not match ordered columns/basis");
    std::set<std::string> names;
    for (const auto& name : columns_)
        if (name.empty() || !names.insert(name).second) throw std::invalid_argument("Tableau column labels must be unique and nonempty");
    std::set<std::size_t> indices;
    for (const auto column : basis_)
        if (column >= columns_.size() || !indices.insert(column).second) throw std::invalid_argument("Tableau basis indices must be unique and in range");
    for (const auto value : values_.data())
        if (!std::isfinite(value)) throw std::domain_error("Numerical tableau cells must be finite");
}

const std::string& Tableau::basicLabel(std::size_t row) const
{
    if (row == 0) throw std::invalid_argument("The objective row has no basic variable");
    return columns_.at(basis_.at(row - 1));
}

PivotResult Tableau::pivot(std::size_t row, std::size_t column, const Tolerance& tolerance)
{
    if (row == 0) throw std::invalid_argument("Cannot pivot on the objective row");
    if (column >= columns_.size() || row >= values_.rows()) throw std::out_of_range("Pivot outside numerical tableau");
    for (std::size_t i = 0; i < basis_.size(); ++i)
        if (i != row - 1 && basis_[i] == column) throw std::invalid_argument("Entering column is already basic in another row");
    const Scalar pivot = values_(row, column);
    if (tolerance.zero(pivot)) throw std::domain_error("Pivot is zero at the caller's tolerance");
    Matrix next = values_;
    for (std::size_t j = 0; j < next.columns(); ++j) next(row, j) /= pivot;
    for (std::size_t i = 0; i < next.rows(); ++i) {
        if (i == row) continue;
        const Scalar multiplier = values_(i, column);
        for (std::size_t j = 0; j < next.columns(); ++j)
            next(i, j) = values_(i, j) - multiplier * next(row, j);
    }
    for (const auto value : next.data())
        if (!std::isfinite(value)) throw std::overflow_error("Requested pivot produced a nonfinite cell");
    const PivotResult result{row, column, basis_[row - 1], pivot};
    values_ = std::move(next);
    basis_[row - 1] = column;
    return result;
}
} // namespace tora::core
