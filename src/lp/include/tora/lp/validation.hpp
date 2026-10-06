#pragma once
#include "problem.hpp"
#include <optional>
#include <string>
#include <vector>

namespace tora::lp {
enum class Severity { Error, Warning };
enum class DiagnosticOrigin { ObservedTora, Engineering };
enum class IssueCode {
    NoVariables, NoConstraints, StudentSizeLimit, InvalidSense,
    ObjectiveSize, ConstraintSize, InvalidRelation, NonfiniteObjective,
    NonfiniteCoefficient, NonfiniteRhs, InvalidLowerBound, InvalidUpperBound,
    InconsistentBounds
};
struct ValidationIssue {
    IssueCode code;
    Severity severity;
    DiagnosticOrigin origin;
    std::optional<std::size_t> variable;
    std::optional<std::size_t> constraint;
    std::string title;
    std::string message;
};
struct ValidationReport {
    std::vector<ValidationIssue> issues;
    bool valid() const noexcept;
};
class LpValidator {
public:
    static constexpr std::size_t StudentSizeLimit = 100;
    static ValidationReport dimensions(std::size_t variables, std::size_t constraints);
    static ValidationReport validate(const LpProblem& problem);
};
} // namespace tora::lp
