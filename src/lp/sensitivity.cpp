#include <tora/lp/sensitivity.hpp>
#include <algorithm>
#include <cmath>

namespace tora::lp {
namespace {
// Intersect base+slope*delta >= 0. Exact-zero handling matches the verified
// simplex profile; no unobserved tolerance is inserted into the display rules.
void nonnegative(SensitivityRange& range,Scalar base,Scalar slope) {
    if(slope>0)range.minimum=std::max(range.minimum,-base/slope);
    else if(slope<0)range.maximum=std::min(range.maximum,-base/slope);
}
SensitivityRange shifted(SensitivityRange range,Scalar current) {
    range.minimum+=current;range.maximum+=current;return range;
}
}
SensitivityReport sensitivity(const FinalSolutionSession& session) {
    SensitivityReport result;
    const auto& solver=session.solver();
    if(solver.status()==SimplexStatus::Infeasible || solver.status()==SimplexStatus::Unbounded)return result;
    const auto& p=session.problem();
    const auto& table=solver.history().back().tableau;
    const auto& initial=solver.history().front().tableau;
    const auto n=p.variables.size();
    const Scalar optimalitySign=p.sense==ObjectiveSense::Maximize?1:-1;
    const auto basic=[&](std::size_t j){return std::find(table.basis().begin(),table.basis().end(),j);};
    for(std::size_t j=0;j<n;++j) {
        SensitivityRange change;const auto found=basic(j);
        const auto row=found==table.basis().end()?std::size_t(0):static_cast<std::size_t>(found-table.basis().begin())+1;
        const Scalar direction=table.columns()[j].front()=='U'?-1:1;
        for(std::size_t k=0;k<table.columns().size();++k) {
            if(table.objectiveBlocked(k) || basic(k)!=table.basis().end())continue;
            const Scalar slope=row?direction*table.values()(row,k):(j==k?-direction:0);
            nonnegative(change,optimalitySign*table.values()(0,k),optimalitySign*slope);
        }
        result.variables.push_back({p.objective[j],shifted(change,p.objective[j]),table.values()(0,j)});
    }
    const auto feasible=[&](const std::vector<Scalar>& slopes,std::optional<std::size_t> boundVariable={},Scalar rangeSlope=0) {
        SensitivityRange change;
        for(std::size_t i=0;i<table.basis().size();++i) {
            const auto col=table.basis()[i];
            nonnegative(change,table.rhs(i+1),slopes[i]);
            if(col<n && !p.variables[col].unrestricted && std::isfinite(p.variables[col].upperBound)) {
                const auto& automatic=solver.problem().variables[col];
                const Scalar width=automatic.upperBound-automatic.lowerBound;
                nonnegative(change,width-table.rhs(i+1),(boundVariable && *boundVariable==col?rangeSlope:0)-slopes[i]);
            }
        }
        return change;
    };
    for(std::size_t k=0;k<p.constraints.size();++k) {
        const auto identityColumn=initial.basis()[k];std::vector<Scalar> slopes;
        for(std::size_t i=1;i<table.values().rows();++i)slopes.push_back(table.values()(i,identityColumn));
        // TORA anchors the normalized RHS delta at the entered RHS, even
        // when normalization negated it. Original negative-RHS captures
        // establish this legacy summary convention (e.g. -8..infinity).
        result.constraints.push_back({p.constraints[k].rhs,shifted(feasible(slopes),p.constraints[k].rhs),table.values()(0,identityColumn)});
    }
    for(std::size_t j=0;j<n;++j) {
        const auto& v=p.variables[j];if(v.unrestricted)continue;
        if(v.lowerBound>0) {
            std::vector<Scalar> slopes(table.basis().size(),0);
            if(table.columns()[j].front()!='U')for(std::size_t i=0;i<slopes.size();++i)slopes[i]=-table.values()(i+1,j);
            auto change=feasible(slopes,j,-1);
            nonnegative(change,v.lowerBound,1);
            if(std::isfinite(v.upperBound))nonnegative(change,v.upperBound-v.lowerBound,-1);
            result.constraints.push_back({v.lowerBound,shifted(change,v.lowerBound),-table.values()(0,j)});
        }
        if(v.lowerBound>=0 && std::isfinite(v.upperBound)) {
            const bool upper=table.columns()[j].front()=='U';std::vector<Scalar> slopes(table.basis().size(),0);
            if(upper)for(std::size_t i=0;i<slopes.size();++i)slopes[i]=table.values()(i+1,j);
            // Nonbasic U variables do not impose a separate zero-width floor
            // in TORA's displayed interval (the min-upper oracle is -infinity).
            auto change=feasible(slopes,j,1);
            result.constraints.push_back({v.upperBound,shifted(change,v.upperBound),upper?table.values()(0,j):0});
        }
    }
    return result;
}
} // namespace tora::lp
