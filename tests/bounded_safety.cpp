#include <tora/lp/bounded.hpp>
#include <iostream>
#include <stdexcept>
using namespace tora::lp;
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main() {try {
    auto p=LpProblem::blank(2,3);p.objective={4,3};
    p.constraints[0].coefficients={2,1};p.constraints[0].rhs=4;
    p.constraints[1].coefficients={1,0};p.constraints[1].rhs=5;
    p.constraints[2].coefficients={0,1};p.constraints[2].rhs=6;
    p.variables[0].upperBound=1;
    BoundedSession s(p);require(s.method()==PrimalMethod::AllSlack,"inequality start method");
    const auto initial=s.history().front().tableau.values().data();
    require(s.recommendation()->upperBound && !s.recommendation()->row,"entering bound must win before a row");
    s.selectEntering(0);bool rejected=false;
    try{s.selectLeaving(1);}catch(const std::invalid_argument&){rejected=true;}
    require(rejected && s.history().size()==1 && !s.history().back().leaving,"invalid row changed partial trace");
    bool paused=false;try{s.solveAll(1);}catch(const std::runtime_error& e){paused=true;require(std::string(e.what()).find("safety limit")!=std::string::npos,"unexpected cap error");}
    require(paused && s.history().size()==2 && s.history().back().tableau.columns()[0]=="Ux1","bound complement/cap state");
    s.solveAll();require(s.history().size()==4 && s.history().back().tableau.columns()[0]=="x1","complement return after resume");
    require(s.history().back().tableau.rhs(0)==12 && s.history().front().tableau.values().data()==initial,"objective or immutable initial trace");
    p.constraints[0].coefficients={1,1};p.constraints[0].rhs=10;
    p.constraints[1].rhs=6;p.constraints[2].rhs=7;p.variables[0].upperBound=6;
    BoundedSession tie(p);require(tie.recommendation()->row==2 && !tie.recommendation()->upperBound,"exact bound/row tie must choose the row");
    tie.next();require(tie.history().back().tableau.columns()[0]=="x1" && tie.history().back().tableau.basis()[1]==0,"tied row pivot basis");
    p.variables[0].lowerBound=7;p.variables[0].upperBound=9;
    BoundedSession shifted(p);require(shifted.method()==PrimalMethod::TwoPhase,"dispatch must follow lower-bound shift");
    shifted.solveAll();require(shifted.status()==SimplexStatus::Infeasible,"shifted infeasibility");
    require(!shifted.next() && shifted.history().back().phase==1,"infeasible Phase 1 must not transition");
    std::cout<<"Bounded safety checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
