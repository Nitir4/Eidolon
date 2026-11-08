#include <tora/lp/dual_revised_model.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>
using namespace tora::lp;
namespace {
void check(bool ok){if(!ok)throw std::runtime_error("Revised dual model regression failed");}
void near(double a,double b){check(std::abs(a-b)<1e-7*(1+std::abs(b)));}
void optimal(const RevisedDualLpResult& r,double objective){check(r.canonical.status==RevisedDualStatus::Optimal && r.objective);near(*r.objective,objective);}
void certify(const RevisedDualModel& model,const RevisedDualLpResult& r) {
    const auto& a=model.matrix;const auto& x=r.canonical.values;
    if(r.canonical.status==RevisedDualStatus::Infeasible) {
        const auto& w=r.canonical.infeasibilityWitness;double wb=0;
        for(std::size_t i=0;i<a.rows();++i)wb+=w[i]*model.rhs[i];
        check(wb<-1e-8);
        for(std::size_t j=0;j<a.columns();++j){double wa=0;for(std::size_t i=0;i<a.rows();++i)wa+=w[i]*a(i,j);check(wa>=-1e-8);}return;
    }
    check(r.canonical.status==RevisedDualStatus::Optimal);double primal=0,dual=0;
    const auto& y=r.canonical.trace.back().dual;
    for(std::size_t j=0;j<x.size();++j){check(x[j]>=-1e-8);primal+=x[j]*model.costs[j];double price=0;for(std::size_t i=0;i<a.rows();++i)price+=a(i,j)*y[i];check(price<=model.costs[j]+1e-7);}
    for(std::size_t i=0;i<a.rows();++i){double ax=0;for(std::size_t j=0;j<x.size();++j)ax+=a(i,j)*x[j];near(ax,model.rhs[i]);dual+=y[i]*model.rhs[i];}near(primal,dual);
    double restored=0;
    for(std::size_t j=0;j<r.values.size();++j){const auto& v=model.original.variables[j];if(!v.unrestricted)check(r.values[j]>=v.lowerBound-1e-7 && r.values[j]<=v.upperBound+1e-7);restored+=model.original.objective[j]*r.values[j];}
    near(restored,*r.objective);
    for(const auto& row:model.original.constraints){double lhs=0;for(std::size_t j=0;j<r.values.size();++j)lhs+=row.coefficients[j]*r.values[j];if(row.relation==Relation::Equal)near(lhs,row.rhs);else check(row.relation==Relation::LessEqual?lhs<=row.rhs+1e-7:lhs>=row.rhs-1e-7);}
}
// Enumerate boundary intersections independently of canonicalization/simplex.
std::optional<double> vertices(const LpProblem& p) {
    struct Line {double a,b,c;};std::vector<Line> lines;
    for(const auto& r:p.constraints)lines.push_back({r.coefficients[0],r.coefficients[1],r.rhs});
    for(std::size_t j=0;j<2;++j) {
        lines.push_back({j==0?1.:0.,j==1?1.:0.,p.variables[j].lowerBound});
        lines.push_back({j==0?1.:0.,j==1?1.:0.,p.variables[j].upperBound});
    }
    std::optional<double> best;
    for(std::size_t i=0;i<lines.size();++i)for(std::size_t k=0;k<i;++k) {
        const auto u=lines[i],v=lines[k];const double det=u.a*v.b-u.b*v.a;if(det==0)continue;
        const double x=(u.c*v.b-u.b*v.c)/det,y=(u.a*v.c-u.c*v.a)/det;
        if(x<p.variables[0].lowerBound-1e-8 || x>p.variables[0].upperBound+1e-8 || y<p.variables[1].lowerBound-1e-8 || y>p.variables[1].upperBound+1e-8)continue;
        bool feasible=true;
        for(const auto& row:p.constraints){const auto lhs=row.coefficients[0]*x+row.coefficients[1]*y;if(row.relation==Relation::LessEqual?lhs>row.rhs+1e-8:lhs<row.rhs-1e-8)feasible=false;}
        if(feasible){double z=p.objective[0]*x+p.objective[1]*y;if(!best || (p.sense==ObjectiveSense::Maximize?z>*best:z<*best))best=z;}
    }
    return best;
}
}
int main(){try {
    auto p=LpProblem::blank(2,3);p.objective={3,2};p.constraints={{{1,1},Relation::LessEqual,4},{{1,0},Relation::LessEqual,2},{{0,1},Relation::LessEqual,3}};
    const auto base=makeDualRevisedModel(p);const auto basis=prepareDualRevisedBasis(base);
    auto changed=p;changed.constraints[0].rhs=6;auto model=makeDualRevisedModel(changed);
    check(model.matrix.data()==base.matrix.data());auto r=solveDualRevisedModel(model,basis);optimal(r,12);certify(model,r);check(r.canonical.trace.size()>1);check(r.canonical.trace.front().basis==basis);
    check(*std::min_element(r.canonical.trace.front().basicValues.begin(),r.canonical.trace.front().basicValues.end())<0);
    optimal(reoptimizeDualRevisedRhs(p,{6,2,3}),12);
    changed.constraints[0].rhs=-1;model=makeDualRevisedModel(changed);check(model.matrix.data()==base.matrix.data());r=solveDualRevisedModel(model,basis);check(r.canonical.status==RevisedDualStatus::Infeasible);certify(model,r);check(!r.objective && r.values.empty());
    auto shifted=LpProblem::blank(1,1);shifted.objective={1};shifted.variables[0].lowerBound=-2;shifted.variables[0].upperBound=3;shifted.constraints={{{1},Relation::LessEqual,10}};
    model=makeDualRevisedModel(shifted);r=solveDualRevisedModel(model,prepareDualRevisedBasis(model));optimal(r,3);near(r.values[0],3);certify(model,r);
    shifted.variables[0].upperBound=-2;model=makeDualRevisedModel(shifted);r=solveDualRevisedModel(model,prepareDualRevisedBasis(model));optimal(r,-2);certify(model,r);
    auto free=LpProblem::blank(1,2);free.objective={1};free.variables[0].unrestricted=true;free.constraints={{{1},Relation::LessEqual,2},{{1},Relation::GreaterEqual,-3}};
    model=makeDualRevisedModel(free);r=solveDualRevisedModel(model,prepareDualRevisedBasis(model));optimal(r,2);certify(model,r);r=reoptimizeDualRevisedRhs(free,{-1,-3});optimal(r,-1);
    auto eq=LpProblem::blank(2,2);eq.objective={1,2};eq.constraints={{{1,1},Relation::Equal,4},{{1,0},Relation::LessEqual,3}};
    r=reoptimizeDualRevisedRhs(eq,{5,3});optimal(r,10);changed=eq;changed.constraints[0].rhs=5;certify(makeDualRevisedModel(changed),r);
    bool rejected=false;try{solveDualRevisedModel(makeDualRevisedModel(eq));}catch(const std::invalid_argument&){rejected=true;}check(rejected);
    rejected=false;try{reoptimizeDualRevisedRhs(p,{1});}catch(const std::invalid_argument&){rejected=true;}check(rejected);
    rejected=false;try{solveDualRevisedModel(base);}catch(const std::runtime_error&){rejected=true;}check(rejected);
    auto dependent=LpProblem::blank(1,2);dependent.objective={1};dependent.constraints={{{1},Relation::Equal,1},{{1},Relation::Equal,1}};
    rejected=false;try{prepareDualRevisedBasis(makeDualRevisedModel(dependent));}catch(const std::invalid_argument&){rejected=true;}check(rejected);
    std::mt19937 random(20261017);
    for(int sample=0;sample<150;++sample) {
        auto q=LpProblem::blank(2,3);q.sense=sample%2?ObjectiveSense::Minimize:ObjectiveSense::Maximize;
        std::vector<double> point(2),rhs;
        for(std::size_t j=0;j<2;++j){q.variables[j].lowerBound=sample%3==0?-2:0;q.variables[j].upperBound=3+random()%5;point[j]=(q.variables[j].lowerBound+q.variables[j].upperBound)/2;q.objective[j]=double(int(random()%9)-4);}
        for(auto& row:q.constraints){row.coefficients={double(int(random()%6)-2),double(int(random()%6)-2)};row.relation=random()%2?Relation::LessEqual:Relation::GreaterEqual;row.rhs=row.coefficients[0]*point[0]+row.coefficients[1]*point[1]+(row.relation==Relation::LessEqual?1:-1)*double(1+random()%3);rhs.push_back(row.rhs+double(int(random()%11)-5));}
        auto next=q;for(std::size_t i=0;i<rhs.size();++i)next.constraints[i].rhs=rhs[i];const auto expected=vertices(next);
        const auto result=reoptimizeDualRevisedRhs(q,rhs);certify(makeDualRevisedModel(next),result);
        check(bool(result.objective)==bool(expected));if(expected)near(*result.objective,*expected);
    }
    std::cout<<"Revised dual model: shifts/free/equality/bounds, true warm pivots, 150 independent RHS models passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
