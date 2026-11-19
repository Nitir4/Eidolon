#include "sensitivity_cases.hpp"
#include <tora/lp/sensitivity.hpp>
#include <tora/lp/postoptimal.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <optional>
#include <stdexcept>

using namespace tora::lp;
namespace {
void check(bool ok,const char* message) {if(!ok)throw std::runtime_error(message);}
bool near(double a,double b) {return a==b || std::abs(a-b)<1e-9;}
bool contains(const SensitivityRange& r,double value) {return r.minimum<=value && value<=r.maximum;}
double value(const LpProblem& p,const std::vector<double>& x) {return p.objective[0]*x[0]+p.objective[1]*x[1];}

// Independent intersections of the entered constraints and variable bounds.
std::vector<std::vector<double>> vertices(const LpProblem& p)
{
    struct Line {double x,y,b;};
    std::vector<Line> lines;
    for(const auto& r:p.constraints)lines.push_back({r.coefficients[0],r.coefficients[1],r.rhs});
    for(std::size_t j=0;j<2;++j)if(!p.variables[j].unrestricted) {
        const auto& v=p.variables[j];
        lines.push_back({j==0?1.:0.,j==1?1.:0.,v.lowerBound});
        if(std::isfinite(v.upperBound))lines.push_back({j==0?1.:0.,j==1?1.:0.,v.upperBound});
    }
    std::vector<std::vector<double>> result;
    for(std::size_t i=0;i<lines.size();++i)for(std::size_t j=i+1;j<lines.size();++j) {
        const auto a=lines[i],b=lines[j];const auto det=a.x*b.y-a.y*b.x;
        if(det==0)continue;
        std::vector<double> x{(a.b*b.y-a.y*b.b)/det,(a.x*b.b-a.b*b.x)/det};
        bool feasible=true;
        for(std::size_t k=0;k<2;++k)if(!p.variables[k].unrestricted)
            feasible=feasible && x[k]>=p.variables[k].lowerBound-1e-9 && x[k]<=p.variables[k].upperBound+1e-9;
        for(const auto& r:p.constraints) {
            const auto lhs=r.coefficients[0]*x[0]+r.coefficients[1]*x[1];
            if(r.relation==Relation::Equal)feasible=feasible && near(lhs,r.rhs);
            else feasible=feasible && (r.relation==Relation::LessEqual?lhs<=r.rhs+1e-9:lhs>=r.rhs-1e-9);
        }
        if(feasible)result.push_back(x);
    }
    return result;
}
double optimum(const LpProblem& p)
{
    const auto points=vertices(p);check(!points.empty(),"No vertices for the independent optimum");
    auto best=value(p,points.front());
    for(const auto& x:points)best=p.sense==ObjectiveSense::Maximize?std::max(best,value(p,x)):std::min(best,value(p,x));
    return best;
}
SensitivityRange preservingPoint(const LpProblem& p,const std::vector<double>& point,std::size_t variable)
{
    SensitivityRange result;const auto sign=p.sense==ObjectiveSense::Maximize?1.:-1.;
    for(const auto& x:vertices(p)) {
        const auto margin=sign*(value(p,point)-value(p,x));
        const auto slope=sign*(point[variable]-x[variable]);
        if(slope>0)result.minimum=std::max(result.minimum,p.objective[variable]-margin/slope);
        if(slope<0)result.maximum=std::min(result.maximum,p.objective[variable]-margin/slope);
    }
    return result;
}
void solved(const LpProblem& p)
{
    FinalSolutionSession session(p);const auto display=session.display();
    check(display.objectiveValue.has_value(),"Expected a finite optimum");
    check(near(value(p,session.values()),optimum(p)),"Decision vector disagrees with independent vertices");
    check(near(std::stod(*display.objectiveValue),optimum(p)),"Objective disagrees with independent vertices");
}
void rhsCoordinates(char id)
{
    auto p=sensitivityCase(id);FinalSolutionSession base(p);solved(p);
    const auto row=sensitivity(base).constraints[0];
    const auto& a=p.constraints[0].coefficients;
    const auto lhs=a[0]*base.values()[0]+a[1]*base.values()[1];
    check(near(row.range.minimum,lhs) && row.range.maximum==INFINITY,"Original-RHS interval has the wrong orientation");
    check(near(row.price,0),"Inactive row must have zero price");
    p.constraints[0].rhs=id=='A'?-5:-4;
    check(!contains(row.range,p.constraints[0].rhs),"Invalid RHS perturbation was included");
    if(id=='A') {
        check(vertices(p).empty(),"Contradictory sum bounds should be infeasible");
        check(FinalSolutionSession(p).solver().status()==SimplexStatus::Infeasible,"Infeasibility was lost");
    }else solved(p);
}
void freeCosts(char id)
{
    auto p=sensitivityCase(id);FinalSolutionSession base(p);solved(p);
    const auto range=sensitivity(base).variables[0].range;
    const auto expected=preservingPoint(p,base.values(),0);
    check(near(range.minimum,expected.minimum) && near(range.maximum,expected.maximum),"Free-variable cost interval disagrees with vertex inequalities");
    auto costs=p.objective;costs[0]=id=='C'?-3:3;
    check(!contains(range,costs[0]),"Non-optimal free-variable coefficient was included");
    const auto changed=analyzeChanges(p,costs);
    check(!changed.retained.dualFeasible,"Changed costs must invalidate the original dual basis");
    for(const auto& term:changed.mappings[0].terms)
        check(changed.canonicalProblem.objective[term.column]==costs[0]*term.coefficient,"Split free-variable costs must change together");
    p.objective=costs;solved(p);
    check(near(*changed.after.objective,optimum(p)),"Changed-model solve was altered");
}
void equalityPrice()
{
    auto p=sensitivityCase('D');FinalSolutionSession base(p);const auto row=sensitivity(base).constraints[0];
    auto above=p,below=p;const double h=0.125;
    above.constraints[0].rhs+=h;below.constraints[0].rhs-=h;
    const auto derivative=(optimum(above)-optimum(below))/(2*h);
    check(near(row.price,derivative),"Equality price is not the original-RHS derivative");
    p.constraints[0].rhs=-0.9;solved(p);
    check(near(optimum(p),optimum(base.problem())+row.price*0.1),"Allowed RHS change has the wrong objective prediction");
    p.constraints[0].rhs=1;solved(p);
    check(!contains(row.range,1),"The retained negative free-column orientation cannot cross zero");
}
void basisIsNotPointRange()
{
    auto p=LpProblem::blank(2,1);p.objective={-1,-2};p.constraints={{{1,1},Relation::LessEqual,0}};
    FinalSolutionSession base(p);const auto basis=sensitivity(base).variables[0].range;
    const auto point=preservingPoint(p,base.values(),0);
    check(point.minimum==-INFINITY && point.maximum==INFINITY,"The singleton feasible point is always optimal");
    check(basis.maximum<point.maximum && !contains(basis,1),"Retained-basis and preserving-point intervals were conflated");
    p.objective[0]=1;solved(p);
    check(FinalSolutionSession(p).values()==base.values(),"Another basis should preserve the degenerate optimum");
}
void nonbasicFreeDirections()
{
    auto p=LpProblem::blank(2,1);p.variables[0].unrestricted=true;p.objective={0,1};
    p.constraints={{{0,1},Relation::LessEqual,1}};
    const auto range=sensitivity(FinalSolutionSession(p)).variables[0].range;
    check(contains(range,0) && !contains(range,-1) && !contains(range,1),"Both directions of a nonbasic free variable must be considered");
    for(double cost:{-1.,1.}){p.objective[0]=cost;check(FinalSolutionSession(p).solver().status()==SimplexStatus::Unbounded,"Any nonzero free cost is unbounded in this model");}
}
}
int main()
{
    int failures=0;
    const auto run=[&](const char* name,auto test){try{test();std::cout<<name<<" PASS\n";}catch(const std::exception& e){++failures;std::cerr<<name<<" FAIL: "<<e.what()<<'\n';}};
    run("A original RHS",[]{rhsCoordinates('A');});run("B original RHS",[]{rhsCoordinates('B');});
    run("C free cost",[]{freeCosts('C');});run("D free cost",[]{freeCosts('D');});
    run("D equality price",equalityPrice);run("Basis versus point",basisIsNotPointRange);
    run("Nonbasic free directions",nonbasicFreeDirections);
    return failures?1:0;
}
