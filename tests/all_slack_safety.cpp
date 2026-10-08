#include <tora/lp/all_slack.hpp>
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace tora::lp;
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
LpProblem normal(){auto p=LpProblem::blank(2,3);p.objective={3,2};p.constraints[0].coefficients={1,1};p.constraints[0].rhs=4;p.constraints[1].coefficients={1,0};p.constraints[1].rhs=2;p.constraints[2].coefficients={0,1};p.constraints[2].rhs=3;return p;}
int main(){try {
    AllSlackSession s(normal());const auto initial=s.history()[0].tableau.values().data();
    for(const auto bad:{std::size_t(1),std::size_t(2),std::size_t(100)}){bool failed=false;try{s.selectEntering(bad);}catch(const std::invalid_argument&){failed=true;}require(failed,"Wrong entering column accepted");require(s.history()[0].tableau.values().data()==initial && !s.history()[0].entering,"Invalid manual choice changed the snapshot");}
    s.selectEntering(0);bool failed=false;try{s.selectLeaving(3);}catch(const std::invalid_argument&){failed=true;}require(failed && !s.history()[0].leaving,"Invalid leaving row retained");
    bool capped=false;try{s.solveAll(1);}catch(const std::runtime_error&){capped=true;}require(capped && s.history().size()==2 && s.status()==SimplexStatus::Running,"Safety cap lost resumable trace");
    require(s.history()[0].tableau.values().data()==initial,"Pivot mutated an earlier snapshot");
    s.solveAll();require(s.history().size()==3 && s.history().back().tableau.rhs(0)==10,"Resuming partial trace failed");
    const auto final=s.history().back().tableau.values().data();
    (void)s.display(2,{tora::core::Notation::Scientific,10,5});(void)s.display(2,{tora::core::Notation::Decimal,99,99});require(s.history().back().tableau.values().data()==final,"Formatting changed arithmetic");
    require(!s.next() && s.history().size()==3,"Optimal Next appended a duplicate table");
    auto p=normal();p.constraints[0].relation=Relation::Equal;require(AllSlackSession::requiresArtificial(p),"Equality did not redirect");
    p=normal();p.constraints[0].rhs=-1;p.constraints[0].relation=Relation::GreaterEqual;require(!AllSlackSession::requiresArtificial(p),"Negative RHS >= normalization misclassified");
    p=normal();p.variables[0].lowerBound=1e308;p.objective[0]=1e308;failed=false;try{AllSlackSession overflow(p);}catch(const std::exception&){failed=true;}require(failed,"Nonfinite initialization was accepted");
    std::cout<<"Manual-choice rollback, immutable snapshots, resumable cap, precision isolation, redirect normalization and overflow safety passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
