#include <tora/assignment/assignment.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace tora::assignment {
namespace {
core::Scalar finite(core::Scalar value) {
    if(!std::isfinite(value))throw std::overflow_error("Assignment arithmetic exceeds the finite numerical range");
    return value;
}
}
Result solve(const Problem& problem) {
    const auto rows=problem.costs.rows(),columns=problem.costs.columns();
    if(!rows || !columns || rows>100 || columns>100)
        throw std::invalid_argument("Assignment requires 1..100 workers and jobs");
    if(problem.sense!=Sense::Minimize && problem.sense!=Sense::Maximize)
        throw std::invalid_argument("Invalid Assignment objective sense");
    for(const auto value:problem.costs.data())if(!std::isfinite(value))
        throw std::invalid_argument("Assignment costs must be finite");
    const auto n=std::max(rows,columns);
    const core::Scalar sign=problem.sense==Sense::Minimize?1:-1;
    const auto cost=[&](std::size_t i,std::size_t j) {
        return i<=rows && j<=columns?sign*problem.costs(i-1,j-1):core::Scalar(0);
    };
    // Column zero is the augmenting-path sentinel. u/v are feasible dual
    // potentials; matched edges become tight after every augmentation.
    std::vector<core::Scalar> u(n+1),v(n+1);
    std::vector<std::size_t> matched(n+1),previous(n+1);
    for(std::size_t row=1;row<=n;++row) {
        matched[0]=row;std::size_t column=0;
        std::vector<core::Scalar> distance(n+1,std::numeric_limits<core::Scalar>::infinity());
        std::vector<bool> used(n+1);
        do {
            used[column]=true;const auto current=matched[column];
            auto delta=std::numeric_limits<core::Scalar>::infinity();std::size_t next=0;
            for(std::size_t j=1;j<=n;++j)if(!used[j]) {
                const auto reduced=finite(finite(cost(current,j)-u[current])-v[j]);
                if(reduced<distance[j]){distance[j]=reduced;previous[j]=column;}
                if(distance[j]<delta){delta=distance[j];next=j;}
            }
            finite(delta);
            for(std::size_t j=0;j<=n;++j) {
                if(used[j]){u[matched[j]]=finite(u[matched[j]]+delta);v[j]=finite(v[j]-delta);}
                else if(j)distance[j]=finite(distance[j]-delta);
            }
            column=next;
        }while(matched[column]);
        do {const auto before=previous[column];matched[column]=matched[before];column=before;}while(column);
    }
    Result result;result.rowToColumn.resize(rows);result.paddedAssignment.resize(n);
    result.rowPotential.assign(u.begin()+1,u.end());result.columnPotential.assign(v.begin()+1,v.end());
    for(std::size_t j=1;j<=n;++j) {
        const auto i=matched[j]-1;result.paddedAssignment[i]=j-1;
        if(i<rows && j<=columns) {
            result.rowToColumn[i]=j-1;
            result.objective=finite(result.objective+problem.costs(i,j-1));
        }
    }
    return result;
}
} // namespace tora::assignment
