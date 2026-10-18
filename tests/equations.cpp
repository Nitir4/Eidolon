#include <tora/equations/equations.hpp>
#include <cmath>
#include <random>
#include <stdexcept>
#include <limits>
using tora::core::Matrix;
using namespace tora::equations;
namespace {
void check(bool value){if(!value)throw std::runtime_error("Equations regression failed");}
void near(double a,double b){check(std::isfinite(a) && std::abs(a-b)<=1e-8*(1+std::abs(b)));}
void certificate(const Matrix& a,const std::vector<double>& b,const Result& r){
    if(r.status==Status::Inconsistent){check(r.augmentedRank==r.rank+1);double value=0;for(std::size_t i=0;i<a.rows();++i)value+=r.inconsistencyWitness[i]*b[i];check(std::abs(value)>1e-8);for(std::size_t j=0;j<a.columns();++j){double v=0;for(std::size_t i=0;i<a.rows();++i)v+=r.inconsistencyWitness[i]*a(i,j);near(v,0);}return;}
    check(r.solution.size()==a.columns());for(std::size_t i=0;i<a.rows();++i){double v=0;for(std::size_t j=0;j<a.columns();++j)v+=a(i,j)*r.solution[j];near(v,b[i]);}
    check(r.nullspace.size()==a.columns()-r.rank);for(const auto& z:r.nullspace)for(std::size_t i=0;i<a.rows();++i){double v=0;for(std::size_t j=0;j<a.columns();++j)v+=a(i,j)*z[j];near(v,0);}
}
}
int main(){
    Matrix a(2,2,{2,1,1,-1});std::vector<double>b{5,1};auto r=solve(a,b);check(r.status==Status::Unique);near(r.solution[0],2);near(r.solution[1],1);certificate(a,b,r);
    auto inv=invert(a);check(inv.invertible);near(inv.inverse(0,0),1.0/3);near(inv.inverse(0,1),1.0/3);near(inv.inverse(1,0),1.0/3);near(inv.inverse(1,1),-2.0/3);near(inv.maximumIdentityResidual,0);check(inv.determinant.has_value());near(*inv.determinant,-3);
    a=Matrix(2,3,{1,2,3,2,4,6});b={4,8};r=solve(a,b);check(r.status==Status::Infinite && r.rank==1);certificate(a,b,r);
    b[1]=9;r=solve(a,b);check(r.status==Status::Inconsistent);certificate(a,b,r);
    a=Matrix(3,2,{1,0,0,1,1,1});b={2,3,5};r=solve(a,b);check(r.status==Status::Unique);certificate(a,b,r);
    a=Matrix(2,2);b={0,0};r=solve(a,b);check(r.status==Status::Infinite && r.rank==0);certificate(a,b,r);check(!invert(a).invertible);
    b={0,1};r=solve(a,b);certificate(a,b,r);
    a=Matrix(2,2,{0,2,3,4});b={2,7};Options options;options.recordTrace=true;r=solve(a,b,options);certificate(a,b,r);check(!r.trace.empty() && !r.traceTruncated);bool swap=false;Matrix replay(2,3,{0,2,2,3,4,7});
    for(const auto& step:r.trace){
        swap|=step.operation==Operation::Swap;
        for(std::size_t j=0;j<3;++j){
            if(step.operation==Operation::Swap)std::swap(replay(step.row,j),replay(step.otherRow,j));
            if(step.operation==Operation::Scale)replay(step.row,j)*=step.factor;
            if(step.operation==Operation::Eliminate)replay(step.row,j)+=step.factor*replay(step.otherRow,j);
        }
        for(std::size_t i=0;i<2;++i)for(std::size_t j=0;j<3;++j)near(replay(i,j),step.augmented(i,j));
    }
    check(swap);
    inv=invert(a,options);check(inv.invertible&&!inv.trace.empty());near(*inv.determinant,-6);
    Matrix replayInverse(2,4,{0,2,1,0,3,4,0,1});
    for(const auto& step:inv.trace){for(std::size_t j=0;j<4;++j){if(step.operation==Operation::Swap)std::swap(replayInverse(step.row,j),replayInverse(step.otherRow,j));if(step.operation==Operation::Scale)replayInverse(step.row,j)*=step.factor;if(step.operation==Operation::Eliminate)replayInverse(step.row,j)+=step.factor*replayInverse(step.otherRow,j);}for(std::size_t i=0;i<2;++i)for(std::size_t j=0;j<4;++j)near(replayInverse(i,j),step.augmented(i,j));}
    inv=invert(Matrix(2,2,{1e200,0,0,1e200}));check(inv.invertible&&!inv.determinant&&inv.determinantSign==1);near(static_cast<double>(inv.logAbsDeterminant),400*std::log(10.));
    inv=invert(Matrix(2,2,{1e-200,0,0,1e-200}));check(inv.invertible&&!inv.determinant&&inv.determinantSign==1);
    options.maximumTraceSteps=1;r=solve(a,b,options);check(r.trace.size()==1 && r.traceTruncated);
    a=Matrix(2,2,{2e-200,1e-200,1e200,-1e200});b={5e-200,1e200};r=solve(a,b);near(r.solution[0],2);near(r.solution[1],1);check(r.relativeResidual<1e-14);
    std::mt19937 rng(20261016);
    for(int sample=0;sample<150;++sample){const std::size_t n=1+rng()%12;Matrix d(n,n);std::vector<double>x(n),rhs(n);for(auto& v:x)v=int(rng()%21)-10;for(std::size_t i=0;i<n;++i){double sum=0;for(std::size_t j=0;j<n;++j)if(i!=j){d(i,j)=int(rng()%11)-5;sum+=std::abs(d(i,j));}d(i,i)=sum+1+rng()%5;for(std::size_t j=0;j<n;++j)rhs[i]+=d(i,j)*x[j];}r=solve(d,rhs);check(r.status==Status::Unique);certificate(d,rhs,r);for(std::size_t i=0;i<n;++i)near(r.solution[i],x[i]);inv=invert(d);check(inv.invertible && inv.maximumIdentityResidual<1e-10);}
    for(int sample=0;sample<100;++sample){const std::size_t n=2+rng()%10;Matrix d(n,n+1);std::vector<double>rhs(n);for(std::size_t i=0;i<n-1;++i){d(i,i)=1;d(i,n-1)=int(rng()%11)-5;d(i,n)=int(rng()%11)-5;rhs[i]=int(rng()%11)-5;}for(std::size_t j=0;j<n+1;++j)d(n-1,j)=2*d(0,j);rhs[n-1]=2*rhs[0];r=solve(d,rhs);check(r.status==Status::Infinite && r.rank==n-1);certificate(d,rhs,r);rhs[n-1]+=1;r=solve(d,rhs);check(r.status==Status::Inconsistent);certificate(d,rhs,r);}
    for(int k=0;k<7;++k){bool rejected=false;try{if(k==0)(void)solve({},{});if(k==1)(void)solve(Matrix(101,1),std::vector<double>(101));if(k==2)(void)solve(Matrix(1,1),{});if(k==3)(void)solve(Matrix(1,1,{INFINITY}),{0});if(k==4)(void)solve(Matrix(1,1),{NAN});if(k==5){Options bad;bad.relativeTolerance=0;(void)solve(Matrix(1,1),{0},bad);}if(k==6)(void)invert(Matrix(1,2));}catch(const std::invalid_argument&){rejected=true;}check(rejected);}
    a=Matrix(100,100);b.assign(100,1);for(std::size_t i=0;i<100;++i)a(i,i)=1;r=solve(a,b);check(r.rank==100);certificate(a,b,r);
}
