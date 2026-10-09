#include <tora/lp/sensitivity.hpp>
#include <stdexcept>
#include <cmath>
void check(bool ok){if(!ok)throw std::runtime_error("Sensitivity safety check failed");}
int main(){using namespace tora::lp;
    auto p=LpProblem::blank(2,3);p.objective={3,2};p.constraints={{{1,1},Relation::LessEqual,4},{{1,0},Relation::LessEqual,2},{{0,1},Relation::LessEqual,3}};
    FinalSolutionSession session(p);const auto report=sensitivity(session);
    check(report.variables.size()==2 && report.constraints.size()==3);
    check(report.variables[0].range.minimum==2 && std::isinf(report.variables[0].range.maximum));
    check(report.constraints[0].range.minimum==2 && report.constraints[0].range.maximum==5 && report.constraints[0].price==2);
    auto changed=p;changed.constraints[0].rhs=4.5;FinalSolutionSession interior(changed);
    check(interior.values()[0]==2 && interior.values()[1]==2.5);
    check(interior.solver().history().back().tableau.basis()==session.solver().history().back().tableau.basis());
    changed.constraints[0].rhs=6;FinalSolutionSession exterior(changed);check(exterior.solver().history().back().tableau.basis()!=session.solver().history().back().tableau.basis());
    changed=p;changed.objective[0]=2.5;FinalSolutionSession costInterior(changed);check(costInterior.values()==session.values());
    changed.objective[0]=1.5;FinalSolutionSession costExterior(changed);check(costExterior.values()!=session.values());
    changed=p;changed.constraints[1].relation=Relation::GreaterEqual;changed.constraints[1].rhs=6;FinalSolutionSession infeasible(changed);check(sensitivity(infeasible).variables.empty());
}
