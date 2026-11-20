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
    std::vector<bool> forbidden(rows*columns);
    for(const auto& pair:problem.forbidden) {
        if(pair.first>=rows || pair.second>=columns)throw std::invalid_argument("Forbidden pair is outside the assignment matrix");
        forbidden[pair.first*columns+pair.second]=true;
    }
    const auto allowed=[&](std::size_t i,std::size_t j) {
        return i>rows || j>columns || !forbidden[(i-1)*columns+j-1];
    };
    const core::Scalar sign=problem.sense==Sense::Minimize?1:-1;
    const auto cost=[&](std::size_t i,std::size_t j) {
        return i<=rows && j<=columns?sign*problem.costs(i-1,j-1):core::Scalar(0);
    };
    // Column zero is the augmenting-path sentinel. Processed rows satisfy
    // the dual inequalities and matched edges are tight after augmentation.
    std::vector<core::Scalar> u(n+1),v(n+1);
    std::vector<std::size_t> matched(n+1),previous(n+1);
    Result result;result.transformedCosts=core::Matrix(n,n);
    for(std::size_t i=1;i<=n;++i)for(std::size_t j=1;j<=n;++j)result.transformedCosts(i-1,j-1)=cost(i,j);
    const auto record=[&](const char* event,std::size_t root,std::size_t column,core::Scalar delta,
                          const std::vector<core::Scalar>& distance=std::vector<core::Scalar>{},
                          const std::vector<bool>& used=std::vector<bool>{}) {
        result.history.push_back({event,root,column,delta,u,v,distance,matched,previous,used});
    };
    record("Original costs",0,0,0);
    record("Negate profits if maximizing; pad to square with zero-cost dummy edges",0,0,0);
    record("Initialize zero potentials and empty matching",0,0,0);
    for(std::size_t row=1;row<=n;++row) {
        matched[0]=row;std::size_t column=0;
        std::vector<core::Scalar> distance(n+1,std::numeric_limits<core::Scalar>::infinity());
        std::vector<bool> used(n+1);
        record("Start augmenting-path search",row,column,0,distance,used);
        do {
            used[column]=true;const auto current=matched[column];
            auto delta=std::numeric_limits<core::Scalar>::infinity();std::size_t next=0;
            for(std::size_t j=1;j<=n;++j)if(!used[j]) {
                if(allowed(current,j)) {
                    const auto reduced=finite(finite(cost(current,j)-u[current])-v[j]);
                    if(reduced<distance[j]){distance[j]=reduced;previous[j]=column;}
                }
                if(distance[j]<delta){delta=distance[j];next=j;}
            }
            record("Relax admissible edges; select minimum distance",row,next,delta,distance,used);
            if(!std::isfinite(delta)) {
                result.status=Status::Infeasible;
                record("Infeasible: no admissible augmenting path",row,column,0,distance,used);
                return result;
            }
            finite(delta);
            for(std::size_t j=0;j<=n;++j) {
                if(used[j]){u[matched[j]]=finite(u[matched[j]]+delta);v[j]=finite(v[j]-delta);}
                else if(j && std::isfinite(distance[j]))distance[j]=finite(distance[j]-delta);
            }
            column=next;
            record("Update potentials and remaining distances",row,column,delta,distance,used);
        }while(matched[column]);
        do {const auto before=previous[column];matched[column]=matched[before];column=before;}while(column);
        record("Augment matching along predecessor path",row,column,0,distance,used);
    }
    result.rowToColumn.resize(rows);result.paddedAssignment.resize(n);
    result.rowPotential.assign(u.begin()+1,u.end());result.columnPotential.assign(v.begin()+1,v.end());
    for(std::size_t j=1;j<=n;++j) {
        const auto i=matched[j]-1;result.paddedAssignment[i]=j-1;
        if(i<rows && j<=columns) {
            result.rowToColumn[i]=j-1;
            result.objective=finite(result.objective+problem.costs(i,j-1));
        }
    }
    record("Optimal: complete admissible matching; primal equals dual",0,0,0);
    return result;
}
} // namespace tora::assignment
