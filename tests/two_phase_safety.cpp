#include <tora/lp/two_phase.hpp>
#include <iostream>
#include <stdexcept>

using namespace tora::lp;
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
LpProblem example(){
    auto p=LpProblem::blank(2,3);p.objective={3,2};
    p.constraints[0]={ {1,1},Relation::Equal,4};
    p.constraints[1]={ {1,0},Relation::GreaterEqual,1};
    p.constraints[2]={ {0,1},Relation::GreaterEqual,1};return p;
}
int main(){try{
    auto p=example();TwoPhaseSession s(p);
    const auto initial=s.history().front().tableau.values().data();
    require(s.display(0).rows[0].label=="z (min)","phase one must minimize feasibility");
    auto before=s.history().back().tableau.values().data();
    try{s.selectEntering(1);throw std::runtime_error("wrong tied column accepted");}catch(const std::invalid_argument&){}
    require(s.history().back().tableau.values().data()==before,"failed manual choice changed trace");
    s.next();s.next();s.next();require(s.history().size()==4,"phase one pivot count");
    require(s.status()==SimplexStatus::Running,"transition must remain a separate Next step");
    s.next();require(s.history().size()==5,"phase two transition must append a table");
    require(s.display(4).rows[0].label=="z (max)","phase two restores original objective sense");
    for(std::size_t j=4;j<7;++j){require(s.history().back().tableau.objectiveBlocked(j),"artificial objective unblocked");require(s.display(4).rows[0].values[j]=="blocked","blocked display missing");}
    try{s.selectEntering(4);throw std::runtime_error("blocked artificial accepted");}catch(const std::invalid_argument&){}
    require(s.history().front().tableau.values().data()==initial,"initial trace mutated");
    TwoPhaseSession limited(p);try{limited.solveAll(2);throw std::runtime_error("safety cap ignored");}catch(const std::runtime_error& e){require(std::string(e.what()).find("safety limit")!=std::string::npos,"unexpected cap error");}
    limited.solveAll();require(limited.terminalMessage()==s.terminalMessage(),"resumed trace differs");
    const auto snapshot=limited.history().back().tableau.values().data();limited.display(4,{tora::core::Notation::Scientific,2,2});require(snapshot==limited.history().back().tableau.values().data(),"format changed arithmetic");
    auto redundant=example();redundant.constraints[1]={{2,2},Relation::Equal,8};TwoPhaseSession kept(redundant);kept.solveAll();require(kept.history().size()==4,"redundant equality transition count");
    require(kept.history().back().tableau.basicLabel(2)=="Rx5","zero artificial basis row removed");require(kept.history().back().tableau.rhs(2)==0,"redundant artificial not zero");require(kept.history().back().tableau.objectiveBlocked(4),"basic artificial objective not blocked");
    p.constraints[0]={ {1,0},Relation::LessEqual,1};p.constraints[1]={ {1,0},Relation::GreaterEqual,3};p.constraints[2]={ {0,1},Relation::LessEqual,2};
    TwoPhaseSession infeasible(p);infeasible.solveAll();require(infeasible.status()==SimplexStatus::Infeasible,"phase one infeasibility lost");require(infeasible.history().size()==2,"infeasible transition appended");require(!infeasible.next(),"infeasible Next progressed");
    std::cout<<"Two-phase snapshots, transition, blocked columns, infeasibility, manual rollback and resumable cap pass\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
