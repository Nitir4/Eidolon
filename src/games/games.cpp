#include <tora/games/games.hpp>
#include <tora/core/tableau.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <optional>
#include <stdexcept>

namespace tora::games {
namespace {
void probabilities(std::vector<core::Scalar>& values,const core::Tolerance& tolerance) {
    for(auto& x:values) {
        if(!std::isfinite(x) || tolerance.less(x,0))throw std::runtime_error("Game LP returned an invalid probability weight");
        if(x<0)x=0;
    }
    const auto sum=std::accumulate(values.begin(),values.end(),core::Scalar(0));
    if(!std::isfinite(sum) || sum<=0)throw std::runtime_error("Game LP returned zero probability mass");
    for(auto& x:values)x/=sum;
}
std::optional<std::vector<core::Scalar>> supportSolution(core::Matrix system,
    const std::vector<std::size_t>& support,std::size_t size,const core::Tolerance& tolerance,
    std::vector<Step>& history,const std::string& player) {
    const auto n=support.size();
    std::vector<core::Scalar> indices;for(auto i:support)indices.push_back(static_cast<core::Scalar>(i+1));
    history.push_back({"Support equations: "+player,"Augmented sum=1 and indifference system. Row values identify the one-based strategy indices of its unknowns.",{},{},indices,{},system});
    for(std::size_t j=0;j<n;++j) {
        auto pivot=j;for(std::size_t i=j+1;i<n;++i)if(std::abs(system(i,j))>std::abs(system(pivot,j)))pivot=i;
        if(tolerance.zero(system(pivot,j))){history.push_back({"Singular support system: "+player,"Keep the existing LP probabilities.",{},{},{},{},system});return {};}
        if(pivot!=j)for(std::size_t k=0;k<=n;++k)std::swap(system(j,k),system(pivot,k));
        const auto divisor=system(j,j);for(std::size_t k=j;k<=n;++k)system(j,k)/=divisor;
        for(std::size_t i=0;i<n;++i)if(i!=j) {
            const auto multiplier=system(i,j);
            for(std::size_t k=j;k<=n;++k)system(i,k)-=multiplier*system(j,k);
        }
        history.push_back({"Support Gauss-Jordan update: "+player,"Pivot column "+std::to_string(j+1)+", swap row "+std::to_string(pivot+1)+" with "+std::to_string(j+1)+", divide by "+std::to_string(divisor)+", eliminate that column from all other rows.",{},{},{},{},system});
    }
    std::vector<core::Scalar> result(size);
    for(std::size_t i=0;i<n;++i) {
        const auto value=system(i,n);if(!std::isfinite(value) || tolerance.less(value,0)){history.push_back({"Reject support candidate: "+player,"Nonfinite or negative probability; keep the LP probabilities.",{},{},{},{},system});return {};}
        result[support[i]]=std::max(core::Scalar(0),value);
    }
    return result;
}
void reconstructSupport(const core::Matrix& a,core::Scalar scale,Result& result,const core::Tolerance& tolerance) {
    std::vector<std::size_t> rows,columns;
    for(std::size_t i=0;i<a.rows();++i)if(result.rowStrategy[i]>0)rows.push_back(i);
    for(std::size_t j=0;j<a.columns();++j)if(result.columnStrategy[j]>0)columns.push_back(j);
    if(rows.empty() || rows.size()!=columns.size())return;
    const auto n=rows.size();core::Matrix p(n,n+1),q(n,n+1);
    for(std::size_t k=0;k<n;++k){p(0,k)=1;q(0,k)=1;}p(0,n)=1;q(0,n)=1;
    for(std::size_t i=1;i<n;++i)for(std::size_t k=0;k<n;++k) {
        p(i,k)=a(rows[k],columns[i])/scale-a(rows[k],columns[0])/scale;
        q(i,k)=a(rows[i],columns[k])/scale-a(rows[0],columns[k])/scale;
    }
    // Recover probabilities from indifference and sum=1 on the selected
    // support. This removes shift-induced cancellation in the LP weights.
    if(auto refined=supportSolution(std::move(p),rows,a.rows(),tolerance,result.history,"A"))result.rowStrategy=std::move(*refined);
    if(auto refined=supportSolution(std::move(q),columns,a.columns(),tolerance,result.history,"B"))result.columnStrategy=std::move(*refined);
}
}
Result solve(const core::Matrix& a,const core::Tolerance& tolerance,std::size_t limit) {
    const auto m=a.rows(),n=a.columns();
    if(!m || !n || m>100 || n>100)throw std::invalid_argument("Games require 1..100 strategies per player");
    core::Scalar scale=0;
    for(const auto x:a.data()) {
        if(!std::isfinite(x))throw std::invalid_argument("Game payoffs must be finite");
        scale=std::max(scale,std::abs(x));
    }
    std::vector<core::Scalar> rowMin(m,std::numeric_limits<core::Scalar>::infinity());
    std::vector<core::Scalar> columnMax(n,-std::numeric_limits<core::Scalar>::infinity());
    for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<n;++j){rowMin[i]=std::min(rowMin[i],a(i,j));columnMax[j]=std::max(columnMax[j],a(i,j));}
    const auto maximin=*std::max_element(rowMin.begin(),rowMin.end());
    const auto minimax=*std::min_element(columnMax.begin(),columnMax.end());
    Result result;result.rowStrategy.resize(m);result.columnStrategy.resize(n);
    result.history.push_back({"Saddle-point test","Row minima and column maxima; compare maximin with minimax.",{},{},rowMin,columnMax});
    if(maximin==minimax) {
        for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<n;++j)
            if(rowMin[i]==maximin && columnMax[j]==minimax)result.saddlePoints.emplace_back(i,j);
        const auto first=result.saddlePoints.front();result.rowStrategy[first.first]=1;result.columnStrategy[first.second]=1;
        result.value=maximin;result.lowerBound=maximin;result.upperBound=maximin;
        result.history.push_back({"Pure equilibrium","Maximin equals minimax exactly. Select the first saddle in row-major order; no simplex pivots.",{},{},result.rowStrategy,result.columnStrategy});
        return result;
    }
    // H=A/scale+2 lies in [1,3]. Max sum(y), H*y<=1 has a natural
    // slack basis, avoiding an artificial-variable Phase 1 for this game LP.
    core::Matrix values(m+1,n+m+1);std::vector<std::string> labels;std::vector<std::size_t> basis;
    for(std::size_t j=0;j<n;++j){labels.push_back("B"+std::to_string(j+1));values(0,j)=-1;}
    for(std::size_t i=0;i<m;++i) {
        labels.push_back("s"+std::to_string(i+1));basis.push_back(n+i);
        values(i+1,n+i)=1;values(i+1,n+m)=1;
        for(std::size_t j=0;j<n;++j)values(i+1,j)=a(i,j)/scale+2;
    }
    core::Tableau table(std::move(values),std::move(labels),std::move(basis));
    result.scale=scale;result.shift=2;
    result.history.push_back({"Game-to-LP formulation","H=A/scale+2; maximize sum(y), H*y<=1, y>=0. Dual: minimize sum(x), H^T*x>=1, x>=0. Recover p=x/sum(x), q=y/sum(y), v=scale*(1/sum(y)-2).",{},{},{},{}});
    result.history.push_back({"Initial tableau — primal optimization","Slack basis is feasible because H lies in [1,3] and RHS=1. No artificial-variable phase is used.",table,{},{},{}});
    // This mathematical extension uses incremental pivots. Legacy LP trace
    // reconstruction is deliberately left unchanged in its separate module.
    const core::Tolerance pivotTolerance(tolerance.absolute()*.001,tolerance.relative()*.001);
    std::size_t iterations=0;
    while(true) {
        std::optional<std::size_t> entering,leaving;core::Scalar best=0;
        // Bland's index rule, including slack columns, avoids exact cycling.
        for(std::size_t j=0;j<n+m;++j)
            if(std::find(table.basis().begin(),table.basis().end(),j)==table.basis().end() && pivotTolerance.less(table.values()(0,j),0)){entering=j;break;}
        if(!entering)break;
        if(iterations++>=limit)throw std::runtime_error("Game LP iteration safety limit reached");
        for(std::size_t i=0;i<m;++i) {
            const auto coefficient=table.values()(i+1,*entering);
            if(!pivotTolerance.less(0,coefficient))continue;
            const auto ratio=std::max(core::Scalar(0),table.rhs(i+1))/coefficient;
            if(!leaving || pivotTolerance.less(ratio,best) ||
                (pivotTolerance.equal(ratio,best) && table.basis()[i]<table.basis()[*leaving-1])){leaving=i+1;best=ratio;}
        }
        if(!leaving)throw std::runtime_error("Positive game LP has no finite leaving step");
        const auto pivot=table.pivot(*leaving,*entering,pivotTolerance);
        result.history.push_back({"Simplex pivot","Bland entering index; minimum nonnegative ratio, ties by basic-column index.",table,pivot,{},{}});
    }
    result.history.push_back({"LP optimal","No negative eligible reduced costs remain.",table,{},{},{}});
    for(std::size_t row=0;row<table.basis().size();++row)
        if(table.basis()[row]<n)result.columnStrategy[table.basis()[row]]=table.rhs(row+1);
    for(std::size_t i=0;i<m;++i)result.rowStrategy[i]=table.values()(0,n+i);
    result.history.push_back({"Recover LP weights","Row weights are dual slack prices; column weights are basic y values. Normalize each vector by its sum.",{},{},result.rowStrategy,result.columnStrategy});
    probabilities(result.rowStrategy,tolerance);probabilities(result.columnStrategy,tolerance);
    result.history.push_back({"Normalize probabilities","Divide each nonnegative weight vector by its positive total.",{},{},result.rowStrategy,result.columnStrategy});
    reconstructSupport(a,scale,result,pivotTolerance);
    probabilities(result.rowStrategy,tolerance);probabilities(result.columnStrategy,tolerance);
    result.history.push_back({"Refine selected support","When positive supports have equal sizes, solve sum=1 and indifference equations with partial-pivot Gauss-Jordan elimination. Singular/negative candidates leave the LP probabilities unchanged.",{},{},result.rowStrategy,result.columnStrategy});
    core::Scalar lower=1,upper=-1;
    for(std::size_t j=0;j<n;++j) {
        core::Scalar payoff=0;for(std::size_t i=0;i<m;++i)payoff+=result.rowStrategy[i]*(a(i,j)/scale);
        lower=std::min(lower,payoff);
    }
    for(std::size_t i=0;i<m;++i) {
        core::Scalar payoff=0;for(std::size_t j=0;j<n;++j)payoff+=result.columnStrategy[j]*(a(i,j)/scale);
        upper=std::max(upper,payoff);
    }
    if(!tolerance.equal(lower,upper))throw std::runtime_error("Game strategies fail the saddle-inequality certificate");
    // Each normalized expectation is a convex combination in [-1,1].
    // Clamp only final rounding at those bounds before restoring large units.
    const auto lo=std::clamp(std::min(lower,upper),core::Scalar(-1),core::Scalar(1));
    const auto hi=std::clamp(std::max(lower,upper),core::Scalar(-1),core::Scalar(1));
    result.lowerBound=lo*scale;result.upperBound=hi*scale;result.value=((lo+hi)/2)*scale;
    result.dualityGap=std::max(core::Scalar(0),upper-lower)*scale;
    result.history.push_back({"Certified equilibrium","Evaluate min_j p^T*A[:,j] and max_i A[i,:]*q in original payoff units. Their agreement certifies both strategies and the original game value.",{},{},result.rowStrategy,result.columnStrategy});
    return result;
}
lp::LpProblem rowPlayerLp(const core::Matrix& payoff) {
    const auto m=payoff.rows(),n=payoff.columns();
    if(!m || !n || m>100 || n>100)throw std::invalid_argument("Games require 1..100 strategies per player");
    for(const auto x:payoff.data())if(!std::isfinite(x))throw std::invalid_argument("Game payoffs must be finite");
    lp::LpProblem p;p.title="Player A game formulation";p.variables.resize(m+1);p.objective.resize(m+1);p.objective[0]=1;
    p.variables[0].name="v";p.variables[0].unrestricted=true;p.variables[0].lowerBound=-std::numeric_limits<core::Scalar>::infinity();
    for(std::size_t i=0;i<m;++i)p.variables[i+1].name="x"+std::to_string(i+1);
    for(std::size_t j=0;j<n;++j) {
        lp::Constraint row;row.coefficients.resize(m+1);row.coefficients[0]=1;
        for(std::size_t i=0;i<m;++i)row.coefficients[i+1]=-payoff(i,j);
        p.constraints.push_back(std::move(row));
    }
    lp::Constraint probability;probability.coefficients.assign(m+1,1);probability.coefficients[0]=0;
    probability.relation=lp::Relation::Equal;probability.rhs=1;p.constraints.push_back(std::move(probability));return p;
}
} // namespace tora::games
