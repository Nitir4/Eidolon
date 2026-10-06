#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace tora::core {

using Scalar = double;
static_assert(std::numeric_limits<Scalar>::is_iec559 && std::numeric_limits<Scalar>::digits == 53,
              "The default backend requires IEEE-754 binary64");

template<class Value> class DenseMatrix {
public:
    DenseMatrix() = default;
    DenseMatrix(std::size_t rows, std::size_t columns)
        : rows_(rows), columns_(columns), data_(checkedSize(rows, columns)) {}
    DenseMatrix(std::size_t rows, std::size_t columns, std::vector<Value> values)
        : rows_(rows), columns_(columns), data_(std::move(values)) {
        if (data_.size() != checkedSize(rows, columns))
            throw std::invalid_argument("Matrix data does not match its dimensions");
    }
    std::size_t rows() const noexcept { return rows_; }
    std::size_t columns() const noexcept { return columns_; }
    const std::vector<Value>& data() const noexcept { return data_; }
    Value& operator()(std::size_t row, std::size_t column) { return data_[index(row, column)]; }
    const Value& operator()(std::size_t row, std::size_t column) const { return data_[index(row, column)]; }

private:
    static std::size_t checkedSize(std::size_t rows, std::size_t columns) {
        const auto maximum = std::vector<Value>().max_size();
        if (columns && rows > maximum / columns) throw std::length_error("Matrix dimensions overflow storage");
        return rows * columns;
    }
    std::size_t index(std::size_t row, std::size_t column) const {
        if (row >= rows_ || column >= columns_) throw std::out_of_range("Matrix index outside dimensions");
        return row * columns_ + column;
    }
    std::size_t rows_ = 0;
    std::size_t columns_ = 0;
    std::vector<Value> data_;
};

using Matrix = DenseMatrix<Scalar>;
} // namespace tora::core
