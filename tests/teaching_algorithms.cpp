#include <tora/assignment/assignment.hpp>
#include <tora/games/games.hpp>
#include <tora/games/graphical.hpp>
#include <tora/transportation/north_west.hpp>
#include <algorithm>
#include <numeric>
#include <random>
#include <cmath>
#include <stdexcept>
#include <iostream>
using tora::core::Matrix;
namespace {
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
bool near(double x,double y){return std::abs(x-y)<=1e-8*(1+std::abs(y));}
bool allowed(const tora::assignment::Problem& p,std::size_t i,std::size_t j){return std::find(p.forbidden.begin(),p.forbidden.end(),std::make_pair(i,j))==p.forbidden.end();}
void assignment(tora::assignment::Problem p) {
    using namespace tora::assignment;
    const auto saved=p.costs.data();const auto n=std::max(p.costs.rows(),p.costs.columns());
    std::vector<std::size_t> perm(n);std::iota(perm.begin(),perm.end(),0);
    const auto sign=p.sense==Sense::Minimize?1.:-1.;double best=INFINITY;
    do{double sum=0;bool admissible=true;for(std::size_t i=0;i<n;++i){admissible=admissible&&allowed(p,i,perm[i]);if(i<p.costs.rows()&&perm[i]<p.costs.columns())sum+=p.costs(i,perm[i]);}if(admissible)best=std::min(best,sign*sum);}while(std::next_permutation(perm.begin(),perm.end()));
    const auto r=solve(p);check(p.costs.data()==saved,"Assignment solve mutated costs");
    check(!r.history.empty(),"Missing assignment history");
    check((r.status==Status::Optimal)==std::isfinite(best),"Forbidden-pair feasibility disagrees with exhaustive permutations");
    for(std::size_t k=0;k<r.history.size();++k){const auto& s=r.history[k];
        for(std::size_t j=1;j<s.matched.size();++j)if(s.matched[j])check(allowed(p,s.matched[j]-1,j-1),"Trace matching contains a forbidden edge");
        if(s.event=="Update potentials and remaining distances"){
            check(k>0,"Missing pre-update state");const auto& before=r.history[k-1];
            check(before.event=="Relax admissible edges; select minimum distance","Potential update has no recorded search");
            for(std::size_t j=0;j<s.used.size();++j){if(s.used[j]){check(near(s.rowPotential[s.matched[j]],before.rowPotential[s.matched[j]]+s.delta),"Incorrect row-potential transition");check(near(s.columnPotential[j],before.columnPotential[j]-s.delta),"Incorrect column-potential transition");}else if(j&&std::isfinite(before.distance[j]))check(near(s.distance[j],before.distance[j]-s.delta),"Incorrect distance transition");}
        }
    }
    if(!std::isfinite(best)){check(r.history.back().event.find("Infeasible")==0,"Missing infeasible terminal event");return;}
    check(near(sign*r.objective,best),"Assignment optimum disagrees with exhaustive permutations");
    const auto& last=r.history.back();double primal=0,dual=0;
    for(std::size_t i=0;i<n;++i){const auto j=r.paddedAssignment[i];check(last.matched[j+1]==i+1,"Final trace/result matching differs");primal+=r.transformedCosts(i,j);dual+=r.rowPotential[i]+r.columnPotential[i];for(std::size_t col=0;col<n;++col)if(allowed(p,i,col))check(r.rowPotential[i]+r.columnPotential[col]<=r.transformedCosts(i,col)+1e-8,"Invalid admissible dual certificate");}
    check(near(primal,dual),"Primal/dual certificate mismatch");
}
void game(const Matrix& a) {
    const auto saved=a.data();const auto r=tora::games::solve(a);check(a.data()==saved,"Game solve mutated input");
    check(!r.history.empty(),"Missing game history");
    std::optional<tora::core::Tableau> before;bool initial=false;std::size_t pivots=0;
    for(const auto& step:r.history){if(!step.tableau)continue;const auto& table=*step.tableau;
        if(step.pivot){check(before.has_value(),"Pivot has no recorded initial tableau");++pivots;const auto p=*step.pivot;const auto& old=before->values();const auto& now=table.values();
            check(before->basis()[p.row-1]==p.previousBasicColumn&&table.basis()[p.row-1]==p.column,"Pivot labels do not match basis transition");
            for(std::size_t i=0;i<old.rows();++i)for(std::size_t j=0;j<old.columns();++j){const auto expected=i==p.row?old(i,j)/old(p.row,p.column):old(i,j)-old(i,p.column)*old(p.row,j)/old(p.row,p.column);check(near(now(i,j),expected),"Recorded pivot violates Gauss-Jordan row operations");}
        }else if(!before){initial=true;for(std::size_t i=0;i<a.rows();++i){check(table.basis()[i]==a.columns()+i,"Game LP did not start in its actual slack basis");for(std::size_t j=0;j<a.columns();++j)check(near(table.values()(i+1,j),a(i,j)/r.scale+2),"Incorrect game-to-LP transformation");}}
        before=table;
    }
    if(r.saddlePoints.empty())check(initial&&pivots>0,"Mixed game is missing actual simplex pivots");else check(!initial&&pivots==0,"Pure game fabricated simplex iterations");
    check(near(std::accumulate(r.rowStrategy.begin(),r.rowStrategy.end(),0.),1)&&near(std::accumulate(r.columnStrategy.begin(),r.columnStrategy.end(),0.),1),"Strategy probabilities do not sum to one");
    double lower=INFINITY,upper=-INFINITY;
    for(std::size_t j=0;j<a.columns();++j){double v=0;for(std::size_t i=0;i<a.rows();++i)v+=r.rowStrategy[i]*a(i,j);lower=std::min(lower,v);}
    for(std::size_t i=0;i<a.rows();++i){double v=0;for(std::size_t j=0;j<a.columns();++j)v+=r.columnStrategy[j]*a(i,j);upper=std::max(upper,v);}
    check(near(lower,r.value)&&near(upper,r.value),"Independent saddle inequalities fail");
    check(r.history.back().rowValues==r.rowStrategy&&r.history.back().columnValues==r.columnStrategy,"Final game trace/result differs");
}
}
int main(){
    std::mt19937 random(6128);
    for(std::size_t m=1;m<=5;++m)for(std::size_t n=1;n<=5;++n)for(int repeat=0;repeat<10;++repeat){tora::assignment::Problem p;p.costs=Matrix(m,n);for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<n;++j){p.costs(i,j)=int(random()%21)-10;if(random()%4==0)p.forbidden.emplace_back(i,j);}assignment(p);p.sense=tora::assignment::Sense::Maximize;assignment(p);}
    tora::assignment::Problem penalty;penalty.costs=Matrix(2,2,{1000,1000,1000,1000});check(tora::assignment::solve(penalty).objective==2000,"Finite penalty became forbidden");assignment(penalty);
    for(const auto& a:{Matrix(2,2,{1,-1,-1,1}),Matrix(2,2,{-4,-3,-2,-1}),Matrix(2,2,{4,3,2,1}),Matrix(2,3,{5,2,0,1,3,4}),Matrix(3,2,{5,1,2,3,0,4}),Matrix(3,3,{0,-1,1,1,0,-1,-1,1,0}),Matrix(3,3)})game(a);
    for(int k=0;k<80;++k){Matrix a(2+random()%3,2+random()%3);for(std::size_t i=0;i<a.rows();++i)for(std::size_t j=0;j<a.columns();++j)a(i,j)=int(random()%21)-10;game(a);}
    const auto graph=tora::games::solveGraphical(Matrix(2,3,{5,2,0,1,3,4}));check(!graph.equilibrium.history.empty()&&near(graph.equilibrium.value,2.5),"Graphical calculation history/result missing");
    // A 2x2 transportation model has one independent shipment t in [0,4].
    for(int sample=0;sample<40;++sample){auto p=tora::transportation::Problem::blank(2,2);p.supply={4,6};p.demand={5,5};for(auto& row:p.costs)for(auto& c:row)c=int(random()%21)-10;
        for(bool maximize:{false,true}){p.maximize=maximize;double best=maximize?-INFINITY:INFINITY;for(int t=0;t<=4;++t){const auto value=t*p.costs[0][0]+(4-t)*p.costs[0][1]+(5-t)*p.costs[1][0]+(1+t)*p.costs[1][1];best=maximize?std::max(best,value):std::min(best,value);}for(const auto method:{tora::transportation::StartMethod::NorthWest,tora::transportation::StartMethod::LeastCost,tora::transportation::StartMethod::Vogel}){const auto trace=tora::transportation::solve(p,method);check(near(trace.back().objective,best),"Transportation direction conversion disagrees with independent shipment enumeration");}}
    }
    std::cout<<"500 restricted assignment models, finite penalties, 87 game certificates and recorded transitions PASS\n";
}
