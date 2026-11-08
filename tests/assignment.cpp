#include <tora/assignment/assignment.hpp>
#include <algorithm>
#include <string>
#include <cmath>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
using namespace tora::assignment;
namespace {
void require(bool condition,const char* expression,int line) {
    if(!condition)throw std::runtime_error(std::string(expression)+" at line "+std::to_string(line));
}
#define CHECK(value) require(static_cast<bool>(value),#value,__LINE__)
double cost(const Problem& p,std::size_t i,std::size_t j) {
    return i<p.costs.rows() && j<p.costs.columns()?p.costs(i,j):0;
}
double exhaustive(const Problem& p) {
    const auto n=std::max(p.costs.rows(),p.costs.columns());
    std::vector<std::size_t> perm(n);std::iota(perm.begin(),perm.end(),0);
    double best=p.sense==Sense::Minimize?std::numeric_limits<double>::infinity():-std::numeric_limits<double>::infinity();
    do {
        double total=0;for(std::size_t i=0;i<n;++i)total+=cost(p,i,perm[i]);
        best=p.sense==Sense::Minimize?std::min(best,total):std::max(best,total);
    }while(std::next_permutation(perm.begin(),perm.end()));
    return best;
}
void certificate(const Problem& p,const Result& r) {
    const auto n=std::max(p.costs.rows(),p.costs.columns());const double sign=p.sense==Sense::Minimize?1:-1;
    CHECK(r.rowToColumn.size()==p.costs.rows() && r.paddedAssignment.size()==n);
    CHECK(r.rowPotential.size()==n && r.columnPotential.size()==n);
    std::vector<bool> used(n);std::size_t real=0;double primal=0,dual=0;
    for(std::size_t i=0;i<n;++i) {
        const auto j=r.paddedAssignment[i];CHECK(j<n && !used[j]);used[j]=true;
        const double scale=1+std::abs(cost(p,i,j));
        CHECK(std::abs(sign*cost(p,i,j)-r.rowPotential[i]-r.columnPotential[j])<=1e-10*scale);
        dual+=r.rowPotential[i]+r.columnPotential[i];
        for(std::size_t k=0;k<n;++k)CHECK(r.rowPotential[i]+r.columnPotential[k]<=sign*cost(p,i,k)+1e-10*(1+std::abs(cost(p,i,k))));
        if(i<p.costs.rows()) {
            if(j<p.costs.columns()){CHECK(r.rowToColumn[i]==j);++real;primal+=p.costs(i,j);}
            else CHECK(!r.rowToColumn[i]);
        }
    }
    CHECK(real==std::min(p.costs.rows(),p.costs.columns()));
    CHECK(std::abs(primal-r.objective)<=1e-10*(1+std::abs(primal)));
    CHECK(std::abs(sign*primal-dual)<=1e-10*(1+std::abs(primal)));
}
void check(Problem p) {const auto saved=p.costs.data();const auto r=solve(p);certificate(p,r);CHECK(r.objective==exhaustive(p));CHECK(p.costs.data()==saved);CHECK(solve(p).paddedAssignment==r.paddedAssignment);}
template<class F>void rejects(F f){try{f();CHECK(false);}catch(const std::invalid_argument&){}catch(const std::overflow_error&){}}
}
int main() {
    Problem example{tora::core::Matrix(3,3,{9,2,7,6,4,3,5,8,1})};
    const auto first=solve(example);CHECK(first.objective==9);CHECK((first.paddedAssignment==std::vector<std::size_t>{1,0,2}));check(example);
    Problem profit{tora::core::Matrix(3,3,{10,5,13,3,9,18,10,7,2}),Sense::Maximize};CHECK(solve(profit).objective==35);check(profit);
    std::mt19937 random(20261014);
    for(std::size_t m=1;m<=5;++m)for(std::size_t n=1;n<=5;++n)for(int repeat=0;repeat<12;++repeat) {
        Problem p{tora::core::Matrix(m,n)};for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<n;++j)p.costs(i,j)=static_cast<int>(random()%36)-15;
        check(p);p.sense=Sense::Maximize;check(p);
    }
    check({tora::core::Matrix(2,3,{.125,.25,.5,.75,.375,.625})});
    check({tora::core::Matrix(3,2,{-3,-4,-1,-2,-5,-6}),Sense::Maximize});
    Problem large{tora::core::Matrix(100,100)};for(std::size_t i=0;i<100;++i)for(std::size_t j=0;j<100;++j)large.costs(i,j)=i==j?0:100;
    const auto result=solve(large);certificate(large,result);CHECK(result.objective==0);
    rejects([]{solve({});});rejects([]{solve({tora::core::Matrix(101,1)});});
    rejects([]{solve({tora::core::Matrix(1,1,{std::numeric_limits<double>::infinity()})});});
    rejects([]{solve({tora::core::Matrix(1,1,{std::numeric_limits<double>::quiet_NaN()})});});
    rejects([]{solve({tora::core::Matrix(1,1,{0}),static_cast<Sense>(99)});});
    rejects([]{solve({tora::core::Matrix(2,2,{1e308,1e308,1e308,1e308})});});
}
