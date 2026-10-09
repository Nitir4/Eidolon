#pragma once
#include "problem.hpp"
#include <tora/core/number_formatter.hpp>
#include <array>
#include <optional>

namespace tora::lp {
struct GraphPoint {Scalar x=0,y=0;};
struct GraphSegment {GraphPoint first,second;};
struct GraphResult {
    std::string objective,x1,x2,message,markerLabel;
    GraphPoint point;
    Scalar objectiveValue=0;
};
// Numerical graph state is independent of Qt. Visible constraints are added
// idempotently; sign selection resets the observed teaching sequence.
class GraphicalSession {
public:
    explicit GraphicalSession(LpProblem problem,core::NumberFormat format={});
    static bool eligible(const LpProblem& problem);
    static bool boundsSupported(const LpProblem& problem);
    static const char* boundsError();
    const LpProblem& problem() const {return problem_;}
    core::NumberFormat format() const {return format_;}
    Scalar rangeMax() const {return maximum_;}
    Scalar rangeMin() const {return -axisUnit_;}
    Scalar gridStep() const {return gridStep_;}
    Scalar tickStep() const;
    bool objectiveVisible() const {return objectiveVisible_;}
    const std::vector<bool>& visibleConstraints() const {return visible_;}
    const GraphResult& result() const {return result_;}
    const std::string& notice() const {return notice_;}
    void selectConstraint(std::size_t index);
    bool drawObjective();
    void drawAll();
    void resetSignRestrictions();
    bool zoom(int direction); // +1 = Zoom In, -1 = Zoom Out
    bool contains(GraphPoint point) const;
    std::optional<GraphSegment> constraintSegment(std::size_t index) const;
    std::optional<GraphSegment> objectiveSegment() const;
    std::vector<std::string> expressionLines() const;
private:
    std::optional<GraphSegment> segment(Scalar a,Scalar b,Scalar rhs) const;
    void solve();
    LpProblem problem_;
    core::NumberFormat format_;
    std::vector<bool> visible_;
    Scalar initialMaximum_=6,maximum_=6,axisUnit_=1,gridStep_=0.15;
    bool objectiveVisible_=false;
    GraphResult result_;
    std::string notice_;
};
} // namespace tora::lp
