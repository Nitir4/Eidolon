#include <tora/lp/postoptimal.hpp>
#include <cmath>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <optional>
#include <random>
#include <limits>
using namespace tora::lp;
namespace {
void check(bool ok){if(!ok)throw std::runtime_error("Post-optimal regression failed");}
void near(double a,double b){check(std::abs(a-b)<1e-9);}
LpProblem example() {
    auto p=LpProblem::blank(2,3);p.objective={3,2};
    p.constraints={{{1,1},Relation::LessEqual,4},{{1,0},Relation::LessEqual,2},{{0,1},Relation::LessEqual,3}};return p;
}
// Independent 2D vertex enumeration, unrelated to simplex/tableau operations.
std::optional<double> vertices(const LpProblem& p) {
    struct Line {double a,b,c;};std::vector<Line> lines;
    for(const auto& r:p.constraints)lines.push_back({r.coefficients[0],r.coefficients[1],r.rhs});
    for(std::size_t j=0;j<2;++j)if(!p.variables[j].unrestricted) {
        lines.push_back({j==0?1.:0.,j==1?1.:0.,p.variables[j].lowerBound});
        if(std::isfinite(p.variables[j].upperBound))lines.push_back({j==0?1.:0.,j==1?1.:0.,p.variables[j].upperBound});
    }
    std::optional<double> best;
    for(std::size_t i=0;i<lines.size();++i)for(std::size_t k=i+1;k<lines.size();++k) {
        const auto& u=lines[i];const auto& v=lines[k];const auto det=u.a*v.b-u.b*v.a;if(det==0)continue;
        const std::vector<double> x{(u.c*v.b-u.b*v.c)/det,(u.a*v.c-u.c*v.a)/det};bool valid=true;
        for(std::size_t j=0;j<2;++j)if(!p.variables[j].unrestricted && (x[j]<p.variables[j].lowerBound-1e-8 || x[j]>p.variables[j].upperBound+1e-8))valid=false;
        for(const auto& row:p.constraints) {
            const auto lhs=row.coefficients[0]*x[0]+row.coefficients[1]*x[1];
            if((row.relation==Relation::LessEqual && lhs>row.rhs+1e-8) ||
                (row.relation==Relation::GreaterEqual && lhs<row.rhs-1e-8) ||
                (row.relation==Relation::Equal && std::abs(lhs-row.rhs)>1e-8))valid=false;
        }
        if(valid) {const auto z=p.objective[0]*x[0]+p.objective[1]*x[1];if(!best || (p.sense==ObjectiveSense::Maximize?z>*best:z<*best))best=z;}
    }
    return best;
}
void optimum(const PostOptimalResult& r,double value,std::vector<double> values) {
    check(r.after.objective.has_value());near(*r.after.objective,value);check(r.after.values.size()==values.size());
    for(std::size_t j=0;j<values.size();++j)near(r.after.values[j],values[j]);
}
}
int main() {
    const auto p=example();
    auto r=analyzeChanges(p,{2.5,2});optimum(r,9,{2,2});check(r.retained.primalFeasible && r.retained.dualFeasible);
    r=analyzeChanges(p,{1,2});optimum(r,7,{1,3});check(r.retained.primalFeasible && !r.retained.dualFeasible);
    r=analyzeChanges(p,{}, {4.5,2,3});optimum(r,11,{2,2.5});check(r.retained.primalFeasible && r.retained.dualFeasible);
    r=analyzeChanges(p,{}, {6,2,3});optimum(r,12,{2,3});check(!r.retained.primalFeasible && r.retained.dualFeasible);
    r=analyzeChanges(p,{1,2},{6,2,3});optimum(r,8,{2,3});check(!r.retained.primalFeasible && !r.retained.dualFeasible);
    r=analyzeChanges(p,{}, {-1,2,3});check(r.after.status==SimplexStatus::Infeasible && !r.after.objective && r.after.values.empty());
    auto negative=LpProblem::blank(1,1);negative.objective={-1};negative.constraints={{{-1},Relation::LessEqual,-1}};
    r=analyzeChanges(negative,{}, {-2});optimum(r,-2,{2});check(r.retained.primalFeasible && r.retained.dualFeasible);near(r.retained.basisValues[0],2);
    auto minimum=LpProblem::blank(2,1);minimum.sense=ObjectiveSense::Minimize;minimum.objective={1,1};minimum.constraints={{{1,1},Relation::GreaterEqual,2}};
    r=analyzeChanges(minimum,{}, {3});check(r.after.objective.has_value());near(*r.after.objective,3);check(r.retained.primalFeasible && r.retained.dualFeasible);
    auto equality=LpProblem::blank(1,2);equality.objective={1};equality.constraints={{{1},Relation::Equal,1},{{1},Relation::Equal,1}};
    r=analyzeChanges(equality,{}, {1,2});check(!r.retained.primalFeasible && r.after.status==SimplexStatus::Infeasible);
    auto unbounded=LpProblem::blank(2,1);unbounded.objective={-1,-1};unbounded.constraints={{{1,-1},Relation::LessEqual,1}};
    r=analyzeChanges(unbounded,{2,-1});check(r.after.status==SimplexStatus::Unbounded && !r.after.objective);
    auto bounded=p;bounded.variables[0].lowerBound=-1;bounded.variables[0].upperBound=1;
    r=analyzeChanges(bounded,{4,2});optimum(r,10,{1,3});
    auto free=LpProblem::blank(1,2);free.objective={1};free.variables[0].unrestricted=true;
    free.constraints={{{1},Relation::LessEqual,2},{{1},Relation::GreaterEqual,-3}};
    r=analyzeChanges(free,{-1});optimum(r,3,{-3});
    for(int k=0;k<2;++k) {
        try{if(k==0)(void)analyzeChanges(p,{1});else (void)analyzeChanges(p,{}, {1});check(false);}
        catch(const std::invalid_argument&){}
    }
    r=analyzeChanges(p);optimum(r,10,{2,2});check(r.retained.primalFeasible && r.retained.dualFeasible);
    auto shifted=LpProblem::blank(1,1);shifted.objective={1};shifted.variables[0].lowerBound=-2;shifted.variables[0].upperBound=3;
    shifted.constraints={{{1},Relation::LessEqual,10}};r=analyzeChanges(shifted,{-1});optimum(r,2,{-2});
    shifted.variables[0].upperBound=-2;r=analyzeChanges(shifted,{-3});optimum(r,6,{-2});
    for(int k=0;k<5;++k) {
        bool rejected=false;
        try {
            if(k==0)(void)analyzeChanges(p,{std::numeric_limits<double>::quiet_NaN(),2});
            if(k==1)(void)analyzeChanges(p,{}, {std::numeric_limits<double>::infinity(),2,3});
            if(k==2){auto q=p;q.constraints[0].rhs=-1;(void)analyzeChanges(q);}
            if(k==3){auto q=p;q.constraints={{{-1,-1},Relation::LessEqual,0}};(void)analyzeChanges(q);}
            if(k==4){auto q=LpProblem::blank(51,1);for(auto& v:q.variables)v.unrestricted=true;(void)analyzeChanges(q);}
        }catch(const std::invalid_argument&){rejected=true;}
        check(rejected);
    }
    std::mt19937 random(20261015);
    for(int sample=0;sample<100;++sample) {
        auto model=LpProblem::blank(2,3);model.sense=sample%2?ObjectiveSense::Minimize:ObjectiveSense::Maximize;
        std::vector<double> point(2),newCosts(2),newRhs;
        for(std::size_t j=0;j<2;++j) {
            model.variables[j].lowerBound=sample%3==0?-2:0;model.variables[j].upperBound=3+random()%5;
            point[j]=(model.variables[j].lowerBound+model.variables[j].upperBound)/2;
            model.objective[j]=static_cast<int>(random()%9)-4;newCosts[j]=static_cast<int>(random()%11)-5;
        }
        for(auto& row:model.constraints) {
            row.coefficients={double(int(random()%6)-2),double(int(random()%6)-2)};
            row.relation=random()%2?Relation::LessEqual:Relation::GreaterEqual;
            row.rhs=row.coefficients[0]*point[0]+row.coefficients[1]*point[1]+(row.relation==Relation::LessEqual?1:-1)*double(1+random()%3);
            newRhs.push_back(row.rhs+double(int(random()%7)-3));
        }
        const auto result=analyzeChanges(model,newCosts,newRhs);const auto expected=vertices(result.changedProblem);
        check(bool(result.after.objective)==bool(expected));if(expected)near(*result.after.objective,*expected);
        near(*result.before.objective,*vertices(model));
        if(result.retained.primalFeasible && result.retained.dualFeasible){check(expected.has_value());near(result.retained.objective,*expected);}
    }
    check(p.objective==std::vector<double>({3,2}) && p.constraints[0].rhs==4);
}
