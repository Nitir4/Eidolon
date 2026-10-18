#include <tora/lp/dual_revised.hpp>
#include <tora/lp/dual.hpp>
#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>
using namespace tora::lp;
namespace core=tora::core;
void check(bool ok){if(!ok)throw std::runtime_error("revised dual check failed");}
void near(double a,double b){check(std::abs(a-b)<1e-7*(1+std::abs(b)));}
void certificate(const core::Matrix& a,const std::vector<double>& b,const std::vector<double>& c,const RevisedDualResult& r){
 check(r.status==RevisedDualStatus::Optimal);
 const auto& y=r.trace.back().dual;
 double primal=0,dual=0;
 for(std::size_t j=0;j<c.size();++j){check(r.values[j]>=-1e-8);primal+=c[j]*r.values[j];double price=0;for(std::size_t i=0;i<b.size();++i)price+=a(i,j)*y[i];check(price<=c[j]+1e-7);}
 for(std::size_t i=0;i<b.size();++i){double ax=0;for(std::size_t j=0;j<c.size();++j)ax+=a(i,j)*r.values[j];near(ax,b[i]);dual+=b[i]*y[i];}
 near(primal,dual);near(primal,r.objective);
}
int main(){try {
 core::Matrix a(3,5,{-1,-1,1,0,0,-1,-2,0,1,0,-2,-1,0,0,1});
 std::vector<double> b{-4,-6,-5}, c{2,3,0,0,0};
 auto r=solveDualRevised(a,b,c,{2,3,4});certificate(a,b,c,r);near(r.objective,10);
 // Reuse authenticated TORA dual.json cases 10_dual and du-decimal.
 core::Matrix decimal(3,5,{-0.3,-0.7,1,0,0,-1,-2,0,1,0,-2,-1,0,0,1});
 auto dec=solveDualRevised(decimal,{-1.9,-6,-5},{0.7,1.1,0,0,0},{2,3,4});
 certificate(decimal,{-1.9,-6,-5},{0.7,1.1,0,0,0},dec);near(dec.objective,3.5);near(dec.values[0],4.0/3);near(dec.values[1],7.0/3);
 auto warm=solveDualRevised(a,b,c,r.trace.back().basis);certificate(a,b,c,warm);check(warm.trace.size()==1);
 auto capped=solveDualRevised(a,b,c,{2,3,4},0);check(capped.status==RevisedDualStatus::Limit && capped.trace.size()==1);
 auto infeasible=solveDualRevised(core::Matrix(1,2,{1,1}),{-1},{0,1},{0});
 check(infeasible.status==RevisedDualStatus::Infeasible);check(infeasible.infeasibilityWitness[0]>0);
 bool rejected=false;try{solveDualRevised(a,b,c,{2,2,4});}catch(const std::invalid_argument&){rejected=true;}check(rejected);
 rejected=false;try{solveDualRevised(core::Matrix(1,2,{1,1}),{1},{1,0},{0});}catch(const std::runtime_error&){rejected=true;}check(rejected);
 rejected=false;try{solveDualRevised(core::Matrix(1,1,{0}),{1},{0},{0});}catch(const std::runtime_error&){rejected=true;}check(rejected);
 // Covering LPs: enumerate all axis/constraint intersections independently.
 std::mt19937 gen(20261016);std::uniform_int_distribution<int> coef(1,9), rhs(1,20);
 for(int k=0;k<300;++k){
  const std::size_t m=2+static_cast<std::size_t>(k%5);
  core::Matrix matrix(m,m+2);std::vector<double> bounds(m),cost(m+2,0);cost[0]=coef(gen);cost[1]=coef(gen);
  std::vector<std::size_t> basis(m);
  auto p=LpProblem::blank(2,m);p.sense=ObjectiveSense::Minimize;p.objective={cost[0],cost[1]};
  for(std::size_t i=0;i<m;++i){matrix(i,0)=-coef(gen);matrix(i,1)=-coef(gen);matrix(i,i+2)=1;bounds[i]=-rhs(gen);basis[i]=i+2;p.constraints[i].coefficients={-matrix(i,0),-matrix(i,1)};p.constraints[i].rhs=-bounds[i];p.constraints[i].relation=Relation::GreaterEqual;}
  double expected=1e100;
  auto candidate=[&](double x,double y){if(x<-1e-9 || y<-1e-9)return;for(std::size_t i=0;i<m;++i)if(matrix(i,0)*x+matrix(i,1)*y>bounds[i]+1e-8)return;expected=std::min(expected,cost[0]*x+cost[1]*y);};
  for(std::size_t i=0;i<m;++i){candidate(bounds[i]/matrix(i,0),0);candidate(0,bounds[i]/matrix(i,1));for(std::size_t j=0;j<i;++j){double d=matrix(i,0)*matrix(j,1)-matrix(i,1)*matrix(j,0);if(d!=0)candidate((bounds[i]*matrix(j,1)-matrix(i,1)*bounds[j])/d,(matrix(i,0)*bounds[j]-bounds[i]*matrix(j,0))/d);}}
  auto solved=solveDualRevised(matrix,bounds,cost,basis);certificate(matrix,bounds,cost,solved);near(solved.objective,expected);
  DualSession tableau(p);tableau.solveAll();near(tableau.history().back().tableau.rhs(0),expected);
  for(const auto& step:solved.trace)for(auto reduced:step.reducedCosts)check(reduced>=-1e-8);
 }
 std::cout<<"Revised dual: 300 independent vertex models, tableau comparisons and certificates passed\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
