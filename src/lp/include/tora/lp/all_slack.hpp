#pragma once
#include "problem.hpp"
#include <tora/core/number_formatter.hpp>
#include <tora/core/tableau.hpp>
#include <optional>

namespace tora::lp {
enum class SimplexStatus { Running, Optimal, Alternative, Unbounded };
struct SimplexStep {
    core::Tableau tableau;
    std::optional<std::size_t> entering;
    std::optional<std::size_t> leaving; // numerical row; row zero is objective
};
struct SimplexDisplayRow {std::string label;std::vector<std::string> values;};
struct SimplexDisplay {
    std::string heading;
    std::vector<std::string> columns;
    std::vector<SimplexDisplayRow> rows;
    std::optional<std::string> entering,leaving;
};
struct SimplexChoice {
    std::size_t column;
    std::optional<std::size_t> row;
    bool upperBound=false; // entering variable reaches its upper bound
    bool leavingUpperBound=false;
    Scalar step=0;
};
// No GUI/file dependency. Each numerical snapshot remains independently owned.
// Strict binary64 comparisons match the captured policy; hidden TORA tolerances
// are not claimed. Presentation precision never changes arithmetic.
class AllSlackSession {
public:
    explicit AllSlackSession(LpProblem problem);
    static bool requiresArtificial(const LpProblem& problem);
    const std::vector<SimplexStep>& history() const noexcept {return history_;}
    const LpProblem& problem() const noexcept {return problem_;}
    SimplexStatus status() const noexcept {return status_;}
    std::optional<SimplexChoice> recommendation() const;
    void selectEntering(std::size_t column);
    void selectLeaving(std::size_t row);
    bool next();
    void solveAll(std::size_t iterationLimit=1024);
    SimplexDisplay display(std::size_t iteration,core::NumberFormat format={}) const;
    std::string terminalMessage() const;
    static const char* enteringError();
    static const char* leavingError();
    static const char* redirectWarning();
private:
    std::optional<SimplexChoice> choice(std::size_t column) const;
    bool basic(std::size_t column) const;
    void assess();
    core::Tableau complemented(core::Tableau table,std::size_t column) const;
    core::Tableau reconstructed(const core::Tableau& table) const;
    LpProblem problem_;
    std::vector<Scalar> ranges_;
    std::vector<SimplexStep> history_;
    SimplexStatus status_=SimplexStatus::Running;
};
} // namespace tora::lp
