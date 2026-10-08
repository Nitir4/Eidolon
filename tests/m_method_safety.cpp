#include <tora/lp/m_method.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace tora::lp;
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
LpProblem model(){auto p=LpProblem::blank(2,3);p.objective={3,2};p.constraints[0].coefficients={1,1};p.constraints[0].relation=Relation::Equal;p.constraints[0].rhs=4;p.constraints[1].coefficients={1,0};p.constraints[1].relation=Relation::GreaterEqual;p.constraints[1].rhs=1;p.constraints[2].coefficients={0,1};p.constraints[2].relation=Relation::GreaterEqual;p.constraints[2].rhs=1;return p;}
int main(){try{
    MMethodSession s(model());auto original=s.history().front().tableau.values().data();
    bool failed=false;try{s.selectEntering(1);}catch(const std::invalid_argument&){failed=true;}require(failed && !s.history().front().entering,"Invalid M selection retained");
    s.selectLeaving(2);require(s.history().front().entering==std::optional<std::size_t>(0),"Automatic M entering selection failed");
    bool capped=false;try{s.solveAll(1);}catch(const std::runtime_error&){capped=true;}require(capped && s.history().size()==2,"M cap did not preserve partial trace");s.solveAll();
    require(s.history().size()==4 && s.history().back().tableau.rhs(0)==11,"M resume failed");require(s.history().front().tableau.values().data()==original,"Earlier M snapshot changed");
    auto before=s.history().back().tableau.values().data();(void)s.display(3,{tora::core::Notation::Scientific,12,4});require(s.history().back().tableau.values().data()==before,"M display changed arithmetic");
    require(!s.next(),"M optimal Next appended duplicate");
    for(const auto penalty:{std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}){failed=false;try{MMethodSession bad(model(),penalty);}catch(const std::invalid_argument&){failed=true;}require(failed,"Nonfinite M accepted");}
    failed=false;try{MMethodSession overflow(model(),1e308);}catch(const std::exception&){failed=true;}require(failed,"Finite-M multiplication overflow accepted");
    MMethodSession zero(model(),0);zero.solveAll();require(zero.history().size()==3 && zero.status()==SimplexStatus::Alternative,"Zero M legacy behavior lost");
    std::cout<<"M selection rollback, immutable snapshots, resumable cap, precision isolation, finite penalty and overflow guard passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
