#include <tora/lp/problem.hpp>
#include <tora/lp/validation.hpp>
#include <stdexcept>

namespace tora::lp {
LpProblem LpProblem::blank(std::size_t variableCount, std::size_t constraintCount) {
    const auto report = LpValidator::dimensions(variableCount, constraintCount);
    if (!report.valid()) {
        throw std::invalid_argument(report.issues.front().message);
    }
    LpProblem problem;
    problem.variables.resize(variableCount);
    problem.objective.resize(variableCount, 0);
    problem.constraints.resize(constraintCount);
    for (auto& row : problem.constraints) {
        row.coefficients.resize(variableCount, 0);
    }
    return problem;
}

std::string LpProblem::variableLabel(std::size_t index) const {
    if (index >= variables.size()) throw std::out_of_range("Variable label index");
    return "x" + std::to_string(index + 1);
}

std::string LpProblem::constraintLabel(std::size_t index) const {
    if (index >= constraints.size()) throw std::out_of_range("Constraint label index");
    return "Constr " + std::to_string(index + 1);
}
} // namespace tora::lp
