#include <tora/assignment/assignment.hpp>
#include <tora/equations/equations.hpp>
#include <tora/games/games.hpp>
#include <tora/games/graphical.hpp>
#include <tora/integer/integer.hpp>
#include <tora/lp/postoptimal.hpp>
#include <tora/lp/dual_revised_model.hpp>
#include <tora/networks/networks.hpp>
#include <tora/planning/planning.hpp>
#include <tora/queuing/queuing.hpp>
#include <tora/queuing/transient.hpp>
#include <tora/transportation/north_west.hpp>
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>

// These checks use analytic identities and transformations of a model, not
// another implementation of its optimization algorithm or TORA output values.
namespace {
using namespace tora;
std::size_t assertions=0;
void require(bool value,int line){++assertions;if(!value)throw std::runtime_error("Contract failed at line "+std::to_string(line));}
#define CHECK(value) require(static_cast<bool>(value),__LINE__)
void near(double a,double b,double relative=1e-8){++assertions;if(!std::isfinite(a)||!std::isfinite(b)||std::abs(a-b)>relative*std::max({1e-290,std::abs(a),std::abs(b)}))throw std::runtime_error("Numeric contract "+std::to_string(a)+" != "+std::to_string(b));}
void zero(double a){CHECK(std::isfinite(a)&&std::abs(a)<1e-10);}
lp::LpProblem baseline(){auto p=lp::LpProblem::blank(2,3);p.objective={3,2};p.constraints={{{1,1},lp::Relation::LessEqual,4},{{1,0},lp::Relation::LessEqual,2},{{0,1},lp::Relation::LessEqual,3}};return p;}
void lpContracts(){
 auto p=baseline();
 for(double scale:{.001,.5,8.,1000.}){
  auto q=p;for(auto& row:q.constraints){for(auto& x:row.coefficients)x*=scale;row.rhs*=scale;}
  auto r=lp::analyzeChanges(q);CHECK(r.after.status==lp::SimplexStatus::Optimal);near(*r.after.objective,10);near(r.after.values[0],2);near(r.after.values[1],2);
 }
 for(double scale:{.125,16.}){auto r=lp::analyzeChanges(p,{3*scale,2*scale});near(*r.after.objective,10*scale);near(r.after.values[0],2);near(r.after.values[1],2);}
 // Redundant, zero and duplicate rows preserve a tied optimal face.
 auto tied=lp::LpProblem::blank(2,4);tied.objective={1,1};tied.constraints={{{1,1},lp::Relation::LessEqual,4},{{2,2},lp::Relation::LessEqual,8},{{0,0},lp::Relation::Equal,0},{{1,1},lp::Relation::GreaterEqual,0}};
 auto r=lp::analyzeChanges(tied);near(*r.after.objective,4);near(r.after.values[0]+r.after.values[1],4);
}
void changeContracts(){
 auto p=baseline();
 // Revised RHS is checked against the analytic box/diagonal intersection,
 // including both a still-feasible retained basis and a required repair.
 for(double rhs:{0.,1.,3.,4.,4.5,5.,6.,8.}){
  auto r=lp::reoptimizeDualRevisedRhs(p,{rhs,2,3});CHECK(r.canonical.status==lp::RevisedDualStatus::Optimal);
  const double x=std::min(2.,rhs),y=std::min(3.,rhs-x);near(*r.objective,3*x+2*y);CHECK(r.values[0]+r.values[1]<=rhs+1e-9);
 }
 auto bad=lp::reoptimizeDualRevisedRhs(p,{-1,2,3});CHECK(bad.canonical.status==lp::RevisedDualStatus::Infeasible&&!bad.objective);
 auto shifted=p;shifted.variables[0].lowerBound=-2;shifted.variables[0].upperBound=2;
 auto r=lp::analyzeChanges(shifted,{-3,2});near(*r.after.objective,12);near(r.after.values[0],-2);near(r.after.values[1],3);
}
void assignmentContracts(){
 for(auto sense:{assignment::Sense::Minimize,assignment::Sense::Maximize}){
  assignment::Problem p{core::Matrix(3,2,{-4,3,2,-1,5,0}),sense};const auto base=assignment::solve(p);
  for(double scale:{.125,8.}){auto q=p;for(std::size_t i=0;i<3;++i)for(std::size_t j=0;j<2;++j)q.costs(i,j)=scale*p.costs(i,j)-7;auto r=assignment::solve(q);near(r.objective,scale*base.objective-14);CHECK(std::count_if(r.rowToColumn.begin(),r.rowToColumn.end(),[](auto x){return x.has_value();})==2);}
  auto q=p;for(std::size_t i=0;i<3;++i)for(std::size_t j=0;j<2;++j)q.costs(i,j)=p.costs(2-i,1-j);near(assignment::solve(q).objective,base.objective);
 }
 assignment::Problem tied{core::Matrix(4,2)};auto r=assignment::solve(tied);zero(r.objective);CHECK(r.rowToColumn.size()==4);CHECK(std::count_if(r.rowToColumn.begin(),r.rowToColumn.end(),[](auto x){return x.has_value();})==2);
}
void transportContracts(){
 transportation::Problem p=transportation::Problem::blank(3,3);p.costs={{-2,1,3},{4,-1,2},{8,9,10}};p.supply={2,3,0};p.demand={1,2,2};
 for(auto method:{transportation::StartMethod::NorthWest,transportation::StartMethod::LeastCost,transportation::StartMethod::Vogel}){
  const auto t=transportation::solve(p,method);near(t.back().objective,1); // x11=1,x13=1,x22=2,x23=1
  for(double quantityScale:{.5,2.}){
   auto q=p;for(auto& x:q.supply)x*=quantityScale;for(auto& x:q.demand)x*=quantityScale;for(auto& row:q.costs)for(auto& x:row)x=2*x+3;
   const auto result=transportation::solve(q,method);near(result.back().objective,17*quantityScale);
   for(std::size_t i=0;i<3;++i){double total=0;for(auto x:result.back().shipments[i])total+=x.value_or(0);if(q.supply[i]==0)zero(total);else near(total,q.supply[i]);}
  }
 }
}
void gameContracts(){
 core::Matrix a(2,3,{4,1,-1,0,2,3});const auto base=games::solve(a);near(base.value,1.5);
 for(double s:{.125,8.}){core::Matrix q(2,4);for(std::size_t j=0;j<4;++j){const auto old=j==3?1:j;q(0,j)=s*a(1,old)-5;q(1,j)=s*a(0,old)-5;}auto r=games::solve(q);near(r.value,s*base.value-5);near(games::solveGraphical(q).equilibrium.value,r.value);for(std::size_t j=0;j<4;++j)CHECK(r.rowStrategy[0]*q(0,j)+r.rowStrategy[1]*q(1,j)>=r.value-1e-8);}
 core::Matrix equal(2,5);for(std::size_t j=0;j<5;++j)equal(0,j)=equal(1,j)=-7;auto r=games::solveGraphical(equal);near(r.equilibrium.value,-7);zero(r.optimalInterval[0]);near(r.optimalInterval[1],1);
}
void equationContracts(){
 const core::Matrix a(2,2,{3,1,1,2});const std::vector<double>b{5,0};
 for(double scale:{1e-150,1e-20,1.,1e20,1e150}){
  core::Matrix q(2,2);std::vector<double>rhs=b;for(std::size_t i=0;i<2;++i){rhs[i]*=scale;for(std::size_t j=0;j<2;++j)q(i,j)=a(i,j)*scale;}
  auto r=equations::solve(q,rhs);CHECK(r.status==equations::Status::Unique);near(r.solution[0],2);near(r.solution[1],-1);auto inv=equations::invert(q);CHECK(inv.invertible);near(inv.inverse(0,0)*scale,.4);
 }
 // Numerical rank must follow the stated tolerance, not decimal formatting.
 core::Matrix close(2,2,{1,1,1,1+1e-10});auto r=equations::solve(close,{2,2+1e-10});CHECK(r.status==equations::Status::Unique);equations::Options options;options.relativeTolerance=1e-8;r=equations::solve(close,{2,2+1e-10},options);CHECK(r.status==equations::Status::Infinite);
}
void integerContracts(){
 auto p=baseline();p.constraints[0].rhs=4.5;p.variables[0].upperBound=4;p.variables[1].upperBound=4;
 for(bool mixed:{false,true}){
  const std::vector<bool>integral{true,!mixed};auto base=integer::solve({p,integral});CHECK(base.status==integer::Status::Optimal);
  for(const std::vector<double>& shift: {std::vector<double>{-7,3},std::vector<double>{4,-6}}){
   auto q=p;for(std::size_t j=0;j<2;++j){q.variables[j].lowerBound+=shift[j];q.variables[j].upperBound+=shift[j];}for(auto& row:q.constraints)for(std::size_t j=0;j<2;++j)row.rhs+=row.coefficients[j]*shift[j];
   const auto r=integer::solve({q,integral});CHECK(r.status==integer::Status::Optimal);near(*r.objective,*base.objective+3*shift[0]+2*shift[1]);for(std::size_t j=0;j<2;++j)near(r.values[j]-shift[j],base.values[j]);
  }
 }
}
void networkContracts(){
 networks::Graph g{4,true,{{0,1,2},{0,2,5},{1,2,1},{1,3,4},{2,3,3}}};const std::vector<std::size_t>perm{3,1,0,2};
 for(double s:{1e-100,1.,1e100}){
  auto q=g;for(auto& e:q.edges){e.from=perm[e.from];e.to=perm[e.to];e.value*=s;}
  near(*networks::dijkstra(q,perm[0]).routes[perm[3]].distance,6*s);near(*networks::floydWarshall(q).sources[perm[0]].routes[perm[3]].distance,6*s);near(*networks::shortestRoutes(q,perm[0]).routes[perm[3]].distance,6*s);
  auto f=networks::maximumFlow(q,perm[0],perm[3]);near(f.value,5*s);near(f.cutCapacity,f.value);q.directed=false;near(networks::minimumSpanningForestPrim(q,perm[0]).totalWeight,6*s);
 }
 // A cycle reachable from the source but unable to reach the target must
 // not poison that target's independent finite shortest route.
 g={5,true,{{0,1,1},{1,2,-2},{2,1,1},{0,3,5},{3,4,2}}};auto r=networks::shortestRoutes(g,0);CHECK(r.routes[1].status==networks::RouteStatus::NegativeCycle);near(*r.routes[4].distance,7);
}
void planningContracts(){
 planning::Problem p;p.method=planning::Method::Pert;p.activities={{1,2,"A",0,1,2,3},{1,3,"B",0,0,1,2},{2,4,"C",0,2,3,4},{3,4,"D",0,1,2,3}};const auto base=planning::solve(p);near(base.duration,5);
 for(double s:{.001,4.,1000.}){auto q=p;for(auto& a:q.activities){a.optimistic*=s;a.mostLikely*=s;a.pessimistic*=s;}auto r=planning::solve(q);near(r.duration,5*s);near(r.pathVariance,base.pathVariance*s*s);near(planning::approximateDeadlineProbability(r,6*s),planning::approximateDeadlineProbability(base,6));for(std::size_t i=0;i<4;++i){if(base.activities[i].totalFloat==0)zero(r.activities[i].totalFloat);else near(r.activities[i].totalFloat,base.activities[i].totalFloat*s);}}
 planning::Problem zeros;zeros.activities={{9,2,"zero",0},{2,1,"zero",0}};auto r=planning::solve(zeros);zero(r.duration);CHECK(r.activities[0].critical&&r.activities[1].critical);near(planning::approximateDeadlineProbability(r,0),1);
}
void queueContracts(){
 // K=1, N=3, lambda=2, mu=3: p=(1/3,2/3), attempted=14/3,
 // rejected=8/3, admitted=2. There is no waiting room: Wq=0,W=1/3.
 auto r=queuing::solve({2,3,1,1,3});near(r.probabilities[0],1./3);near(r.probabilities[1],2./3);near(r.metrics->attemptedArrivalRate,14./3);near(r.metrics->rejectedArrivalRate,8./3);near(r.metrics->throughput,2);zero(r.metrics->meanQueueTime);near(r.metrics->meanSystemTime,1./3);
 // When K>=N, capacity ceases to alter the finite-source model.
 for(auto cap:{std::size_t{3},std::size_t{4},std::size_t{100}}){auto a=queuing::solve({2,3,7,cap,3});CHECK(a.capacity==3);near(a.metrics->meanSystem,1.2);near(a.metrics->throughput,3.6);zero(a.metrics->rejectedArrivalRate);near(a.metrics->meanSystemTime,1./3);}
 for(double s:{1e-100,1e100}){auto a=queuing::solve({2*s,3*s,1,1,3});near(a.probabilities[0],1./3);near(a.metrics->throughput,2*s);near(a.metrics->meanSystemTime*s,1./3);}
 // Pure death at N=1 is an exponential survival probability, including t=0.
 for(double t:{0.,.125,2.,32.}){auto a=queuing::pureDeath(2,1,t);near(a.mean,std::exp(-2*t));near(queuing::probability(a,1),std::exp(-2*t));zero(a.mean-queuing::probability(a,1));}
}
}
int main(int argc,char**argv){
 const std::map<std::string,std::function<void()>> suites{{"lp",lpContracts},{"changes",changeContracts},{"transportation",transportContracts},{"assignment",assignmentContracts},{"games",gameContracts},{"equations",equationContracts},{"integer",integerContracts},{"networks",networkContracts},{"planning",planningContracts},{"queuing",queueContracts}};
 try{if(argc!=2||!suites.count(argv[1]))throw std::invalid_argument("Select one mathematical contract suite");suites.at(argv[1])();std::cout<<argv[1]<<": "<<assertions<<" analytic/transformation assertions passed\n";return 0;}catch(const std::exception&e){std::cerr<<(argc>1?argv[1]:"contracts")<<": "<<e.what()<<'\n';return 1;}
}
