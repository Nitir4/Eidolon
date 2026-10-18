#include <tora/equations/equations.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace tora::equations {
namespace {
double narrow(long double x) {
    if (!std::isfinite(x) || std::abs(x)>std::numeric_limits<double>::max())
        throw std::runtime_error("Linear equations arithmetic exceeds binary64 output range");
    return static_cast<double>(x);
}
}
Result solve(const core::Matrix& a,const std::vector<double>& b,const Options& options) {
    const auto m=a.rows(), n=a.columns();
    if (!m || !n || m>100 || n>100 || b.size()!=m)
        throw std::invalid_argument("Linear equations require 1..100 rows/columns and matching RHS");
    if (!std::isfinite(options.relativeTolerance) || options.relativeTolerance<=0 || options.relativeTolerance>=1 || options.maximumTraceSteps>10000)
        throw std::invalid_argument("Invalid linear equations tolerance or trace limit");
    for(auto x:a.data())if(!std::isfinite(x))throw std::invalid_argument("Nonfinite coefficient");
    for(auto x:b)if(!std::isfinite(x))throw std::invalid_argument("Nonfinite RHS");
    core::DenseMatrix<long double> r(m,n+1), transform(m,m);
    Result out;
    auto snapshot=[&](){core::Matrix s(m,n+1);for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<=n;++j)s(i,j)=narrow(r(i,j));return s;};
    auto trace=[&](Operation op,std::size_t row,std::size_t other,std::size_t col,long double factor){
        if(!options.recordTrace)return;
        if(out.trace.size()>=options.maximumTraceSteps){out.traceTruncated=true;return;}
        out.trace.push_back({op,row,other,col,narrow(factor),snapshot()});
    };
    for(std::size_t i=0;i<m;++i){for(std::size_t j=0;j<n;++j)r(i,j)=a(i,j);r(i,n)=b[i];transform(i,i)=1;}
    // Row equilibration prevents absolute magnitude from determining numerical rank.
    for(std::size_t i=0;i<m;++i){
        long double scale=0;for(std::size_t j=0;j<n;++j)scale=std::max(scale,std::abs(static_cast<long double>(a(i,j))));
        if(scale==0)scale=std::abs(static_cast<long double>(b[i]));
        if(scale==0)scale=1;
        for(std::size_t j=0;j<n;++j)r(i,j)=static_cast<long double>(a(i,j))/scale;
        r(i,n)=static_cast<long double>(b[i])/scale;transform(i,i)=1/scale;
        trace(Operation::Scale,i,i,0,1/scale);
    }
    const long double eps=options.relativeTolerance;
    std::size_t rank=0;
    for(std::size_t col=0;col<n && rank<m;++col){
        std::size_t pivot=rank;for(std::size_t i=rank+1;i<m;++i)if(std::abs(r(i,col))>std::abs(r(pivot,col)))pivot=i;
        if(std::abs(r(pivot,col))<=eps)continue;
        if(pivot!=rank){for(std::size_t j=0;j<=n;++j)std::swap(r(rank,j),r(pivot,j));for(std::size_t j=0;j<m;++j)std::swap(transform(rank,j),transform(pivot,j));trace(Operation::Swap,rank,pivot,col,1);}
        const auto divisor=r(rank,col);
        for(std::size_t j=0;j<=n;++j)r(rank,j)/=divisor;
        for(std::size_t j=0;j<m;++j)transform(rank,j)/=divisor;
        trace(Operation::Scale,rank,rank,col,1/divisor);
        for(std::size_t i=0;i<m;++i)if(i!=rank && r(i,col)!=0){const auto factor=r(i,col);for(std::size_t j=0;j<=n;++j)r(i,j)-=factor*r(rank,j);r(i,col)=0;for(std::size_t j=0;j<m;++j)transform(i,j)-=factor*transform(rank,j);trace(Operation::Eliminate,i,rank,col,-factor);}
        out.pivotColumns.push_back(col);++rank;
    }
    out.rank=rank;out.augmentedRank=rank;
    for(std::size_t i=rank;i<m;++i){
        // Compare RHS cancellation against the original RHS under the same row transform.
        long double rhsScale=0;for(std::size_t k=0;k<m;++k)rhsScale+=std::abs(transform(i,k)*static_cast<long double>(b[k]));
        if(std::abs(r(i,n))>eps*std::max(1.0L,rhsScale)){
            out.status=Status::Inconsistent;out.augmentedRank=rank+1;
            long double norm=0;for(std::size_t k=0;k<m;++k)norm=std::max(norm,std::abs(transform(i,k)));
            out.inconsistencyWitness.resize(m);for(std::size_t k=0;k<m;++k)out.inconsistencyWitness[k]=narrow(transform(i,k)/norm);
            long double wr=0;for(std::size_t k=0;k<m;++k)wr+=static_cast<long double>(out.inconsistencyWitness[k])*b[k];out.witnessRhs=narrow(wr);
            for(std::size_t j=0;j<n;++j){long double v=0;for(std::size_t k=0;k<m;++k)v+=static_cast<long double>(out.inconsistencyWitness[k])*a(k,j);out.witnessCoefficientResidual=std::max(out.witnessCoefficientResidual,narrow(std::abs(v)));}
            out.reducedAugmented=snapshot();return out;
        }
    }
    out.status=rank==n?Status::Unique:Status::Infinite;out.solution.assign(n,0);
    for(std::size_t i=0;i<rank;++i)out.solution[out.pivotColumns[i]]=narrow(r(i,n));
    for(std::size_t j=0;j<n;++j)if(std::find(out.pivotColumns.begin(),out.pivotColumns.end(),j)==out.pivotColumns.end()){
        out.freeColumns.push_back(j);std::vector<double> z(n);z[j]=1;for(std::size_t i=0;i<rank;++i)z[out.pivotColumns[i]]=narrow(-r(i,j));out.nullspace.push_back(std::move(z));
    }
    long double worst=0,relative=0;
    for(std::size_t i=0;i<m;++i){long double value=0,scale=std::abs(static_cast<long double>(b[i]));for(std::size_t j=0;j<n;++j){const auto term=static_cast<long double>(a(i,j))*out.solution[j];value+=term;scale+=std::abs(term);}const auto error=std::abs(value-b[i]);worst=std::max(worst,error);relative=std::max(relative,scale?error/scale:error);}
    out.maximumResidual=narrow(worst);out.relativeResidual=narrow(relative);out.reducedAugmented=snapshot();return out;
}
InverseResult invert(const core::Matrix& a,const Options& options) {
    if(a.rows()!=a.columns())throw std::invalid_argument("Matrix inversion requires a square matrix");
    // Reuse solve's input validation and numerical-rank convention.
    const auto n=a.rows();
    const auto rankResult=solve(a,std::vector<double>(n),Options{options.relativeTolerance,false,options.maximumTraceSteps});
    InverseResult out;out.rank=rankResult.rank;
    if(out.rank!=n){out.determinant=0;return out;}
    core::DenseMatrix<long double> r(n,2*n);
    for(std::size_t i=0;i<n;++i){for(std::size_t j=0;j<n;++j)r(i,j)=a(i,j);r(i,n+i)=1;}
    auto trace=[&](Operation op,std::size_t row,std::size_t other,std::size_t col,long double factor){
        if(!options.recordTrace)return;
        if(out.trace.size()>=options.maximumTraceSteps){out.traceTruncated=true;return;}
        core::Matrix snapshot(n,2*n);for(std::size_t i=0;i<n;++i)for(std::size_t j=0;j<2*n;++j)snapshot(i,j)=narrow(r(i,j));
        out.trace.push_back({op,row,other,col,narrow(factor),std::move(snapshot)});
    };
    out.determinantSign=1;
    for(std::size_t i=0;i<n;++i){long double scale=0;for(std::size_t j=0;j<n;++j)scale=std::max(scale,std::abs(r(i,j)));out.logAbsDeterminant+=std::log(scale);for(std::size_t j=0;j<2*n;++j)r(i,j)/=scale;trace(Operation::Scale,i,i,0,1/scale);}
    for(std::size_t col=0;col<n;++col){
        std::size_t pivot=col;for(std::size_t i=col+1;i<n;++i)if(std::abs(r(i,col))>std::abs(r(pivot,col)))pivot=i;
        if(std::abs(r(pivot,col))<=options.relativeTolerance)throw std::runtime_error("Inconsistent numerical rank in inverse computation");
        if(pivot!=col){for(std::size_t j=0;j<2*n;++j)std::swap(r(col,j),r(pivot,j));out.determinantSign=-out.determinantSign;trace(Operation::Swap,col,pivot,col,1);}
        const auto divisor=r(col,col);if(divisor<0)out.determinantSign=-out.determinantSign;out.logAbsDeterminant+=std::log(std::abs(divisor));
        for(std::size_t j=0;j<2*n;++j)r(col,j)/=divisor;
        trace(Operation::Scale,col,col,col,1/divisor);
        for(std::size_t i=0;i<n;++i)if(i!=col&&r(i,col)!=0){const auto factor=r(i,col);for(std::size_t j=0;j<2*n;++j)r(i,j)-=factor*r(col,j);r(i,col)=0;trace(Operation::Eliminate,i,col,col,-factor);}
    }
    const auto magnitude=std::exp(out.logAbsDeterminant);
    if(std::isfinite(magnitude)&&magnitude<=std::numeric_limits<double>::max()&&magnitude>=std::numeric_limits<double>::denorm_min())out.determinant=out.determinantSign*static_cast<double>(magnitude);
    out.invertible=true;out.inverse=core::Matrix(n,n);
    for(std::size_t i=0;i<n;++i)for(std::size_t j=0;j<n;++j)out.inverse(i,j)=narrow(r(i,n+j));
    for(std::size_t i=0;i<n;++i)for(std::size_t j=0;j<n;++j){long double left=0,right=0;for(std::size_t k=0;k<n;++k){left+=static_cast<long double>(a(i,k))*out.inverse(k,j);right+=static_cast<long double>(out.inverse(i,k))*a(k,j);}const long double expected=i==j?1:0;out.maximumIdentityResidual=std::max(out.maximumIdentityResidual,narrow(std::max(std::abs(left-expected),std::abs(right-expected))));}
    return out;
}
}
