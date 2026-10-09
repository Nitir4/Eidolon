#include <tora/lp/dual.hpp>
#include <stdexcept>
#include <iostream>
using namespace tora::lp;
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){try{
 auto p=LpProblem::blank(2,3);p.sense=ObjectiveSense::Minimize;p.objective={2,3};
 p.constraints[0].coefficients={1,1};p.constraints[0].rhs=4;
 p.constraints[1].coefficients={1,2};p.constraints[1].rhs=6;
 p.constraints[2].coefficients={2,1};p.constraints[2].rhs=5;
 for(auto& r:p.constraints)r.relation=Relation::GreaterEqual;
 DualSession s(p);require(s.method()==PrimalMethod::Dual,"direct Dual setup");
 require(s.recommendation()->row==2 && s.recommendation()->column==1,"initial Dual pivot");
 const auto initial=s.history().front().tableau.values().data();
 bool rejected=false;try{s.selectEntering(0);}catch(const std::invalid_argument&){rejected=true;}
 require(rejected && !s.history().front().entering,"invalid selection changed history");
 s.selectLeaving(2);require(s.history().back().leaving==2 && !s.history().back().entering,"row-only selection fabricated a column");
 s.selectEntering(1);require(s.history().back().leaving==2,"entering selection erased leaving highlight");
 bool capped=false;try{s.solveAll(1);}catch(const std::runtime_error& e){capped=std::string(e.what()).find("safety limit")!=std::string::npos;}
 require(capped && s.history().size()==2,"partial trace lost on cap");s.solveAll();
 require(s.history().size()==4 && s.history().back().tableau.rhs(0)==10,"Dual resume/final value");
 require(s.history().front().tableau.values().data()==initial,"initial numerical snapshot mutated");
 p.constraints[0].relation=Relation::Equal;bool equation=false;try{DualSession invalid(p);}catch(const std::invalid_argument&){equation=true;}
 require(equation,"Dual equality accepted");std::cout<<"Dual safety checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
