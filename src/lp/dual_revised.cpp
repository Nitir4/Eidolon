#include <tora/lp/dual_revised.hpp>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace tora::lp {
namespace {
void finite(Scalar x) {
    if(!std::isfinite(x))throw std::runtime_error("Revised dual nonfinite arithmetic/input");
}
std::vector<Scalar> solve(core::Matrix a,std::vector<Scalar> rhs,Scalar tol) {
    const auto m=rhs.size();
    const auto original=a;const auto originalRhs=rhs;
    for(std::size_t k=0;k<m;++k) {
        auto p=k;
        for(std::size_t i=k+1;i<m;++i)if(std::abs(a(i,k))>std::abs(a(p,k)))p=i;
        if(std::abs(a(p,k))<=tol)throw std::runtime_error("Revised dual singular or ill-scaled basis");
        for(std::size_t j=k;j<m;++j)std::swap(a(k,j),a(p,j));
        std::swap(rhs[k],rhs[p]);
        for(std::size_t i=k+1;i<m;++i) {
            const auto f=a(i,k)/a(k,k);
            for(std::size_t j=k+1;j<m;++j){a(i,j)-=f*a(k,j);finite(a(i,j));}
            rhs[i]-=f*rhs[k];finite(rhs[i]);
        }
    }
    std::vector<Scalar> x(m);
    for(std::size_t i=m;i-->0;) {
        auto v=rhs[i];
        for(std::size_t j=i+1;j<m;++j)v-=a(i,j)*x[j];
        x[i]=v/a(i,i);finite(x[i]);
    }
    for(std::size_t i=0;i<m;++i) {
        Scalar residual=-originalRhs[i],scale=1+std::abs(originalRhs[i]);
        for(std::size_t j=0;j<m;++j){residual+=original(i,j)*x[j];scale+=std::abs(original(i,j)*x[j]);}
        finite(residual);finite(scale);
        if(std::abs(residual)>tol*scale)throw std::runtime_error("Revised dual basis solve residual too large");
    }
    return x;
}
}
RevisedDualResult solveDualRevised(const core::Matrix& a,
    const std::vector<Scalar>& b,const std::vector<Scalar>& c,
    std::vector<std::size_t> basis,std::size_t limit,Scalar tol) {
    const auto m=a.rows(), n=a.columns();
    if(m==0 || m>100 || n<m || n>200 || b.size()!=m || c.size()!=n || basis.size()!=m
       || !std::isfinite(tol) || tol<=0)throw std::invalid_argument("Revised dual dimensions/tolerance invalid");
    for(auto v:a.data())finite(v);
    for(auto v:b)finite(v);
    for(auto v:c)finite(v);
    std::vector<bool> used(n,false);
    for(auto j:basis){if(j>=n || used[j])throw std::invalid_argument("Revised dual invalid basis indices");used[j]=true;}
    RevisedDualResult result;
    for(std::size_t iteration=0;;++iteration) {
        core::Matrix matrix(m,m), transpose(m,m);
        std::vector<Scalar> cb(m);
        std::fill(used.begin(),used.end(),false);
        for(std::size_t j=0;j<m;++j) {
            used[basis[j]]=true;cb[j]=c[basis[j]];
            for(std::size_t i=0;i<m;++i){matrix(i,j)=a(i,basis[j]);transpose(j,i)=matrix(i,j);}
        }
        RevisedDualStep step;
        step.basis=basis;step.basicValues=solve(matrix,b,tol);step.dual=solve(transpose,cb,tol);
        step.reducedCosts=c;
        for(std::size_t j=0;j<n;++j) {
            for(std::size_t i=0;i<m;++i)step.reducedCosts[j]-=a(i,j)*step.dual[i];
            finite(step.reducedCosts[j]);
            if(step.reducedCosts[j]<-tol)throw std::runtime_error("Revised dual basis is not dual feasible");
        }
        for(std::size_t i=0;i<m;++i)
            if(step.basicValues[i]<-tol && (!step.leavingRow || basis[i]<basis[*step.leavingRow]))step.leavingRow=i;
        result.values.assign(n,0);result.objective=0;
        for(std::size_t i=0;i<m;++i)result.values[basis[i]]=step.basicValues[i];
        for(std::size_t j=0;j<n;++j)result.objective+=c[j]*result.values[j];
        finite(result.objective);
        if(!step.leavingRow){result.status=RevisedDualStatus::Optimal;result.trace.push_back(std::move(step));return result;}
        std::vector<Scalar> unit(m,0);unit[*step.leavingRow]=1;
        auto w=solve(transpose,unit,tol);
        Scalar best=0;
        for(std::size_t j=0;j<n;++j)if(!used[j]) {
            Scalar coefficient=0;
            for(std::size_t i=0;i<m;++i)coefficient+=w[i]*a(i,j);
            finite(coefficient);
            if(coefficient>=-tol)continue;
            const auto ratio=std::max(Scalar(0),step.reducedCosts[j])/(-coefficient);finite(ratio);
            if(!step.enteringColumn || ratio<best){best=ratio;step.enteringColumn=j;}
        }
        if(!step.enteringColumn){result.status=RevisedDualStatus::Infeasible;result.infeasibilityWitness=std::move(w);result.trace.push_back(std::move(step));return result;}
        result.trace.push_back(step);
        if(iteration>=limit)return result;
        basis[*step.leavingRow]=*step.enteringColumn;
    }
}
}
