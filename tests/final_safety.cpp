#include <tora/lp/final_solution.hpp>
#include <string>
#include <stdexcept>
void check(bool ok){if(!ok)throw std::runtime_error("Final safety check failed");}
int main(){using namespace tora::lp;
 auto p=LpProblem::blank(2,1);p.objective={3,2};p.constraints[0].coefficients={1,1};p.constraints[0].rhs=4;
 bool limited=false;try{FinalSolutionSession s(p,0);}catch(const std::runtime_error&){limited=true;}check(limited);
 FinalSolutionSession normal(p);check(normal.display().objectiveValue=="12.00");
 p.variables[0].upperBound=1;FinalSolutionSession upper(p);check(upper.values()[0]==1 && upper.values()[1]==3);
 p.variables[0].lowerBound=1;FinalSolutionSession fixed(p);check(fixed.values()[0]==1 && fixed.values()[1]==3);
 p.variables[0].unrestricted=true;p.variables[0].lowerBound=-std::numeric_limits<double>::infinity();p.variables[0].upperBound=std::numeric_limits<double>::infinity();p.constraints.push_back({{1,0},Relation::GreaterEqual,-2});p.objective={-3,2};FinalSolutionSession free(p);check(free.values()[0]==-2);
 bool invalid=false;try{FinalSolutionSession s(LpProblem{});}catch(const std::invalid_argument&){invalid=true;}check(invalid);
}
