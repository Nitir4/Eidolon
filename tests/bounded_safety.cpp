#include <tora/lp/bounded.hpp>
#include <cassert>
#include <stdexcept>
using namespace tora::lp;
int main() {
    auto p=LpProblem::blank(2,3);p.objective={4,3};
    p.constraints[0].coefficients={2,1};p.constraints[0].rhs=4;
    p.constraints[1].coefficients={1,0};p.constraints[1].rhs=5;
    p.constraints[2].coefficients={0,1};p.constraints[2].rhs=6;
    p.variables[0].upperBound=1;
    BoundedSession s(p);assert(s.method()==PrimalMethod::AllSlack);
    const auto initial=s.history().front().tableau.values().data();
    assert(s.recommendation()->upperBound && !s.recommendation()->row);
    s.selectEntering(0);bool rejected=false;
    try{s.selectLeaving(1);}catch(const std::invalid_argument&){rejected=true;}
    assert(rejected && s.history().size()==1 && !s.history().back().leaving);
    bool paused=false;try{s.solveAll(1);}catch(const std::runtime_error&){paused=true;}
    assert(paused && s.history().size()==2 && s.history().back().tableau.columns()[0]=="Ux1");
    s.solveAll();assert(s.history().size()==4 && s.history().back().tableau.columns()[0]=="x1");
    assert(s.history().back().tableau.rhs(0)==12 && s.history().front().tableau.values().data()==initial);
    p.constraints[0].coefficients={1,1};p.constraints[0].rhs=10;
    p.constraints[1].rhs=6;p.constraints[2].rhs=7;p.variables[0].upperBound=6;
    BoundedSession tie(p);assert(tie.recommendation()->row==2 && !tie.recommendation()->upperBound);
    tie.next();assert(tie.history().back().tableau.columns()[0]=="x1" && tie.history().back().tableau.basis()[1]==0);
    p.variables[0].lowerBound=7;p.variables[0].upperBound=9;
    BoundedSession shifted(p);assert(shifted.method()==PrimalMethod::TwoPhase);
    shifted.solveAll();assert(shifted.status()==SimplexStatus::Infeasible);
    assert(!shifted.next() && shifted.history().back().phase==1);
}
