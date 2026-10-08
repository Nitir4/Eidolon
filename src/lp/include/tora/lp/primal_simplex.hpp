#pragma once
#include "problem.hpp"
#include <tora/core/number_formatter.hpp>
#include <tora/core/tableau.hpp>
#include <optional>
#include <utility>

namespace tora::lp {
enum class SimplexStatus { Running, Optimal, Alternative, Unbounded, Infeasible };
struct SimplexStep {
    core::Tableau tableau;
    std::optional<std::size_t> entering;
    std::optional<std::size_t> leaving; // numerical row; row zero is objective
    int phase=0; // zero for All-slack/M; observed Two-phase snapshots use 1/2
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
enum class PrimalMethod { AllSlack, MMethod, TwoPhase, Bounded }; // Bounded selects a captured shared method/profile.
class PrimalSimplexSession {
public:
    virtual ~PrimalSimplexSession()=default;
    PrimalMethod method() const noexcept {return method_;}
    static bool requiresArtificial(const LpProblem& problem);
    const std::vector<SimplexStep>& history() const noexcept {return history_;}
    const LpProblem& problem() const noexcept {return problem_;}
    SimplexStatus status() const noexcept {return status_;}
    bool phaseOneComplete() const noexcept {return phaseReady_;}
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
protected:
    PrimalSimplexSession(LpProblem problem,PrimalMethod method,Scalar penalty);
private:
    std::optional<SimplexChoice> choice(std::size_t column) const;
    bool basic(std::size_t column) const;
    void assess();
    ObjectiveSense objectiveSense() const;
    core::Tableau complemented(core::Tableau table,std::size_t column) const;
    core::Tableau reconstructed(const core::Tableau& table,bool restoreObjective=false) const;
    PrimalMethod method_;
    LpProblem problem_;
    std::vector<Scalar> ranges_;
    std::vector<Scalar> originalObjective_;
    std::vector<SimplexStep> history_;
    SimplexStatus status_=SimplexStatus::Running;
    bool phaseReady_=false;
    bool canonicalBasicCosts_=false;
};
} // namespace tora::lp
