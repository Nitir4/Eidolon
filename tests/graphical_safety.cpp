#include <tora/lp/graphical.hpp>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <stdexcept>
using namespace tora::lp;
int main(){
    auto p=LpProblem::blank(2,3);p.objective={3,2};p.constraints[0].coefficients={1,1};p.constraints[0].rhs=4;
    p.constraints[1].coefficients={1,0};p.constraints[1].rhs=2;p.constraints[2].coefficients={0,1};p.constraints[2].rhs=3;
    GraphicalSession graph(p);assert(!graph.objectiveVisible());assert(!graph.zoom(1));
    assert(!graph.drawObjective());assert(graph.notice()=="Must graph all constraints prior to objective function");
    graph.selectConstraint(0);graph.selectConstraint(0);assert(graph.visibleConstraints()[0]);assert(!graph.drawObjective());
    graph.selectConstraint(1);graph.selectConstraint(2);assert(graph.drawObjective());assert(graph.result().objective=="10.00");
    assert(graph.zoom(1));assert(graph.rangeMax()==4);assert(graph.zoom(-1));assert(graph.rangeMax()==5);
    graph.resetSignRestrictions();assert(!graph.objectiveVisible());assert(!graph.zoom(1));
    graph.drawAll();assert(graph.result().x1=="2.00");
    auto unbounded=LpProblem::blank(2,1);unbounded.objective={1,1};unbounded.constraints[0].coefficients={1,-1};unbounded.constraints[0].rhs=1;unbounded.constraints[0].relation=Relation::GreaterEqual;
    GraphicalSession terminal(unbounded);terminal.drawAll();assert(terminal.result().message=="Optimum solution is unbounded");assert(!terminal.objectiveSegment());
    auto infeasible=LpProblem::blank(2,2);infeasible.objective={1,1};infeasible.constraints[0].coefficients={1,0};infeasible.constraints[0].rhs=1;infeasible.constraints[1].coefficients={1,0};infeasible.constraints[1].rhs=3;infeasible.constraints[1].relation=Relation::GreaterEqual;
    GraphicalSession rejected(infeasible);rejected.drawAll();assert(rejected.result().message=="Problem has no feasible solution");assert(!rejected.objectiveSegment());
    bool bad=false;try{graph.selectConstraint(3);}catch(const std::out_of_range&){bad=true;}assert(bad);
    assert(!GraphicalSession::eligible(LpProblem::blank(3,1)));
    assert(!GraphicalSession::eligible(LpProblem::blank(1,1)));
    p.variables[0].upperBound=1;bad=false;try{GraphicalSession bounded(p);}catch(const std::invalid_argument& e){bad=std::string(e.what())==GraphicalSession::boundsError();}assert(bad);
    p.variables[0].upperBound=std::numeric_limits<Scalar>::infinity();p.variables[0].lowerBound=1;assert(!GraphicalSession::boundsSupported(p));
    p.variables[0].lowerBound=0;p.variables[0].unrestricted=true;assert(!GraphicalSession::boundsSupported(p));
}
