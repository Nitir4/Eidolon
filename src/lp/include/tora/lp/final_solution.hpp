#pragma once
#include "bounded.hpp"

namespace tora::lp {
struct SummarySection {
    std::vector<std::string> columns;
    std::vector<SimplexDisplayRow> rows;
    std::string heading={};
};
struct SummaryDisplay {
    std::string heading="LINEAR PROGRAMMING OUTPUT SUMMARY";
    std::string title;
    std::size_t iteration=0;
    std::string objectiveLabel;
    std::optional<std::string> objectiveValue;
    std::string message;
    std::vector<SummarySection> sections;
};

// Automatic final solving shares verified pivot rules with Bounded simplex.
// Its display count also includes TORA's automatic Phase 2 initialization.
class FinalSolutionSession {
public:
    explicit FinalSolutionSession(LpProblem problem,std::size_t iterationLimit=1024);
    const PrimalSimplexSession& solver() const {return solver_;}
    const LpProblem& problem() const {return problem_;}
    const std::vector<Scalar>& values() const {return values_;}
    SummaryDisplay display(core::NumberFormat format={},bool includeSensitivity=false) const;
private:
    LpProblem problem_;
    BoundedSession solver_;
    std::vector<Scalar> values_;
};
} // namespace tora::lp
