#include <tora/transportation/north_west.hpp>
#include <tora/transportation/final_summary.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
using namespace tora::transportation;
namespace {
void check(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
bool near(double x,double y){return std::abs(x-y)<1e-8;}
const std::vector<StartMethod> methods{StartMethod::NorthWest,StartMethod::LeastCost,StartMethod::Vogel};
void verify(const Problem& input,StartMethod method,double optimum) {
    const auto original=input;const auto s=solveDetailed(input,method);const auto& p=s.problem;
    const auto m=p.supply.size(),n=p.demand.size();
    check(input.costs==original.costs && input.supply==original.supply && input.demand==original.demand &&
          input.dummySources==original.dummySources && input.dummyDestinations==original.dummyDestinations,"Input mutated");
    check(near(s.tableaux.back().objective,optimum),"Optimum differs from exhaustive enumeration");
    check(s.allocations.size()==m+n-1,"Initial allocation does not construct a tree");
    std::vector<std::vector<std::optional<double>>> initial(m,std::vector<std::optional<double>>(n));
    auto supply=p.supply,demand=p.demand;double objective=0;
    for(const auto& a:s.allocations) {
        const auto i=a.cell.source,j=a.cell.destination;
        check(!initial[i][j],"Repeated allocation cell");check(a.amount==std::min(supply[i],demand[j]),"Incorrect allocation amount");
        initial[i][j]=a.amount;supply[i]-=a.amount;demand[j]-=a.amount;objective+=a.amount*p.costs[i][j];
        check(supply==a.remainingSupply && demand==a.remainingDemand,"Incorrect residual history");
        check(near(objective,a.objective) && !a.explanation.empty(),"Missing allocation objective/explanation");
    }
    check(initial==s.tableaux.front().shipments,"Construction and initial MODI basis differ");
    double previous=s.tableaux.front().objective;
    for(const auto& t:s.tableaux) {
        std::vector<std::size_t> parent(m+n);std::iota(parent.begin(),parent.end(),0);
        const auto root=[&](std::size_t v){while(parent[v]!=v)v=parent[v];return v;};
        std::vector<double> rows(m),columns(n);std::size_t basics=0;double value=0;
        for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<n;++j)if(t.shipments[i][j]) {
            ++basics;const double x=*t.shipments[i][j];check(x>=0,"Negative shipment");rows[i]+=x;columns[j]+=x;value+=x*p.costs[i][j];
            const auto a=root(i),b=root(m+j);check(a!=b,"Basis cycle");parent[a]=b;
            check(near(t.u[i]+t.v[j],p.costs[i][j]),"Basic potentials violate cost identity");
        }
        check(basics==m+n-1 && rows==p.supply && columns==p.demand,"Basis or feasibility broken");
        check(near(value,t.objective),"Incorrect objective");
        check(p.maximize?t.objective>=previous-1e-8:t.objective<=previous+1e-8,"Objective worsened");previous=t.objective;
    }
    const auto& terminal=s.tableaux.back();check(!terminal.nextPivot,"Nonterminal final tableau");
    for(const auto& row:terminal.reducedCosts)for(double x:row)check(p.maximize?x>=-1e-8:x<=1e-8,"Invalid optimality certificate");
}
// Enumerate only real shipments, independently of dummy construction and MODI.
double enumerate(const Problem& p) {
    const int s0=int(p.supply[0]),s1=int(p.supply[1]),d0=int(p.demand[0]),d1=int(p.demand[1]);
    const int total=std::min(s0+s1,d0+d1);double best=p.maximize?-INFINITY:INFINITY;
    for(int a=0;a<=std::min(s0,d0);++a)for(int b=0;b<=std::min(s0-a,d1);++b)
    for(int c=0;c<=std::min(s1,d0-a);++c)for(int d=0;d<=std::min(s1-c,d1-b);++d) {
        if(a+b+c+d!=total)continue;
        const double v=a*p.costs[0][0]+b*p.costs[0][1]+c*p.costs[1][0]+d*p.costs[1][1];
        best=p.maximize?std::max(best,v):std::min(best,v);
    }
    return best;
}
}
int main() {
 try {
    std::mt19937 random(20261029);
    for(int k=0;k<100;++k) {
        auto p=Problem::blank(2,2);for(auto& x:p.supply)x=random()%5;for(auto& x:p.demand)x=random()%5;
        for(auto& row:p.costs)for(auto& x:row)x=int(random()%11)-5;
        for(bool maximize:{false,true}){p.maximize=maximize;for(auto method:methods)verify(p,method,enumerate(p));}
    }
    auto p=Problem::blank(2,3);p.costs={{2,3,1},{5,4,8}};p.supply={20,30};p.demand={10,25,10};
    for(auto method:methods){verify(p,method,130);const auto s=solveDetailed(p,method);check(s.problem.dummyDestinations==std::vector<std::size_t>{3} && !s.balancing.empty(),"Missing surplus explanation");}
    p.supply={20,25};p.demand={10,25,15};for(auto method:methods)verify(p,method,125);
    const auto summary=finalSummary(p);check(std::any_of(summary.rows.begin(),summary.rows.end(),[](const auto& r){return r[0].find("unmet demand")!=std::string::npos;}),"Summary hides shortage");
    p=Problem::blank(2,2);p.costs={{1,4},{3,2}};p.supply={5,5};p.demand={5,5};
    for(auto method:methods) {
        verify(p,method,15);const auto s=solveDetailed(p,method);
        check(std::any_of(s.allocations.begin(),s.allocations.end(),[](const auto& a){return a.amount==0;}),"Degenerate zero missing from construction history");
    }
    p=Problem::blank(2,3);p.costs={{2,3,1},{5,4,8}};p.supply={20,30};p.demand={10,25,15};
    const auto vogel=solveDetailed(p,StartMethod::Vogel);check(vogel.allocations[0].cell.source==0 && vogel.allocations[0].cell.destination==2 && vogel.allocations[0].amount==15,"Wrong Vogel first allocation");
    check(vogel.allocations[0].sourcePenalties==std::vector<std::optional<double>>{1,1} && vogel.allocations[0].destinationPenalties==std::vector<std::optional<double>>{3,1,7},"Incorrect recorded Vogel penalties");
    // Fractional dyadic quantities retain exact conservation after balancing.
    p=Problem::blank(1,1);p.costs={{2}};p.supply={0.5};p.demand={0.75};for(auto method:methods)verify(p,method,1);
    auto limit=Problem::blank(1,100);limit.supply={1};bool rejected=false;try{solveDetailed(limit,StartMethod::NorthWest);}catch(const std::invalid_argument&){rejected=true;}check(rejected,"Dummy exceeded dimension safeguard");
    std::cout<<"600 exhaustive min/max method comparisons plus degeneracy, balancing, penalties, conservation and summaries passed\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
