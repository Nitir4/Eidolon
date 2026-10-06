#include <tora/lp/validation.hpp>
#include <algorithm>
#include <cmath>
#include <utility>

namespace tora::lp {
namespace {
void add(ValidationReport& report, IssueCode code, std::string message,
         std::optional<std::size_t> variable = {},
         std::optional<std::size_t> constraint = {},
         DiagnosticOrigin origin = DiagnosticOrigin::Engineering,
         std::string title = "Input Data Error") {
    report.issues.push_back({code, Severity::Error, origin, variable, constraint,
                             std::move(title), std::move(message)});
}
} // namespace

bool ValidationReport::valid() const noexcept {
    return std::none_of(issues.begin(), issues.end(), [](const auto& issue) {
        return issue.severity == Severity::Error;
    });
}

ValidationReport LpValidator::dimensions(std::size_t variables, std::size_t constraints) {
    ValidationReport report;
    if (variables == 0) {
        add(report, IssueCode::NoVariables, "Invalid problem size", {}, {},
            DiagnosticOrigin::ObservedTora, "Problem Size");
    }
    if (constraints == 0) {
        add(report, IssueCode::NoConstraints, "Invalid problem size", {}, {},
            DiagnosticOrigin::ObservedTora, "Problem Size");
    }
    if (variables > StudentSizeLimit || constraints > StudentSizeLimit) {
        add(report, IssueCode::StudentSizeLimit, "Student version limits maximum size to 100",
            {}, {}, DiagnosticOrigin::ObservedTora, "Problem size");
    }
    return report;
}

ValidationReport LpValidator::validate(const LpProblem& problem) {
    auto report = dimensions(problem.variables.size(), problem.constraints.size());
    if (problem.sense != ObjectiveSense::Maximize && problem.sense != ObjectiveSense::Minimize) {
        add(report, IssueCode::InvalidSense, "Objective direction must be Maximize or Minimize");
    }
    if (problem.objective.size() != problem.variables.size()) {
        add(report, IssueCode::ObjectiveSize, "Objective must contain one coefficient per variable");
    }
    for (std::size_t j = 0; j < problem.objective.size(); ++j) {
        if (!std::isfinite(problem.objective[j])) {
            add(report, IssueCode::NonfiniteObjective, "Objective coefficient must be finite", j);
        }
    }
    for (std::size_t j = 0; j < problem.variables.size(); ++j) {
        const auto& variable = problem.variables[j];
        const bool lowerValid = std::isfinite(variable.lowerBound);
        const bool upperValid = std::isfinite(variable.upperBound) ||
            (std::isinf(variable.upperBound) && variable.upperBound > 0);
        if (!lowerValid) {
            add(report, IssueCode::InvalidLowerBound, "Lower bound must be finite", j);
        }
        if (!upperValid) {
            add(report, IssueCode::InvalidUpperBound, "Upper bound must be finite or positive infinity", j);
        }
        if (lowerValid && upperValid && variable.lowerBound > variable.upperBound) {
            add(report, IssueCode::InconsistentBounds,
                "Lower bound exceeds upper bound for variable " + std::to_string(j + 1) +
                ".\nReenter correct bounds", j, {}, DiagnosticOrigin::ObservedTora);
        }
    }
    for (std::size_t i = 0; i < problem.constraints.size(); ++i) {
        const auto& row = problem.constraints[i];
        if (row.coefficients.size() != problem.variables.size()) {
            add(report, IssueCode::ConstraintSize,
                "Constraint must contain one coefficient per variable", {}, i);
        }
        if (row.relation != Relation::LessEqual && row.relation != Relation::GreaterEqual &&
            row.relation != Relation::Equal) {
            add(report, IssueCode::InvalidRelation, "Constraint relation must be <=, >= or =", {}, i);
        }
        for (std::size_t j = 0; j < row.coefficients.size(); ++j) {
            if (!std::isfinite(row.coefficients[j])) {
                add(report, IssueCode::NonfiniteCoefficient, "Constraint coefficient must be finite", j, i);
            }
        }
        if (!std::isfinite(row.rhs)) {
            add(report, IssueCode::NonfiniteRhs, "Constraint right-hand side must be finite", {}, i);
        }
    }
    return report;
}
} // namespace tora::lp
