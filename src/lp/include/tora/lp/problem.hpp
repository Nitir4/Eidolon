#pragma once
#include <tora/core/matrix.hpp>
#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace tora::lp {
using Scalar = core::Scalar;
enum class ObjectiveSense { Maximize, Minimize };
enum class Relation { LessEqual, GreaterEqual, Equal };
struct Variable {
    // User-entered name is distinct from the ordered x1/x2 column identifier.
    std::string name;
    Scalar lowerBound = 0;
    Scalar upperBound = std::numeric_limits<Scalar>::infinity();
    bool unrestricted = false;
};
struct Constraint {
    std::vector<Scalar> coefficients;
    Relation relation = Relation::LessEqual;
    Scalar rhs = 0;
};
struct LpProblem {
    std::string title;
    ObjectiveSense sense = ObjectiveSense::Maximize;
    std::vector<Variable> variables;
    std::vector<Scalar> objective;
    std::vector<Constraint> constraints;

    static LpProblem blank(std::size_t variableCount, std::size_t constraintCount);
    std::string variableLabel(std::size_t index) const;
    std::string constraintLabel(std::size_t index) const;
};
} // namespace tora::lp
