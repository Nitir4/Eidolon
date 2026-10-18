#include <tora/lp/dual_revised_model.hpp>
#include <tora/lp/final_solution.hpp>
#include <tora/lp/validation.hpp>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace tora::lp {
namespace {
Scalar finite(Scalar value) {
    if(!std::isfinite(value))throw std::overflow_error("Revised dual model arithmetic overflow");
    return value;
}
void validate(const LpProblem& problem) {
    const auto report=LpValidator::validate(problem);
    if(!report.valid())throw std::invalid_argument(report.issues.front().message);
}
void checkTolerance(Scalar tolerance) {
    if(!std::isfinite(tolerance) || tolerance<=0)throw std::invalid_argument("Revised dual invalid tolerance");
}
}
RevisedDualModel makeDualRevisedModel(const LpProblem& problem) {
    validate(problem);RevisedDualModel model;model.original=problem;
    auto& p=model.nonnegative;p.title=problem.title;p.sense=problem.sense;
    model.objectiveSign=problem.sense==ObjectiveSense::Minimize?1:-1;
    for(std::size_t j=0;j<problem.variables.size();++j) {
        const auto& v=problem.variables[j];RevisedDualVariable map;
        map.positiveColumn=p.variables.size();p.variables.push_back({});p.objective.push_back(problem.objective[j]);
        if(v.unrestricted) {
            map.negativeColumn=p.variables.size();p.variables.push_back({});p.objective.push_back(-problem.objective[j]);
        }else map.shift=v.lowerBound;
        model.objectiveOffset=finite(model.objectiveOffset+finite(map.shift*problem.objective[j]));
        model.variables.push_back(map);
    }
    for(const auto& row:problem.constraints) {
        Constraint transformed;transformed.relation=row.relation;transformed.rhs=row.rhs;
        transformed.coefficients.resize(p.variables.size());
        for(std::size_t j=0;j<model.variables.size();++j) {
            const auto& map=model.variables[j];
            transformed.rhs=finite(transformed.rhs-finite(row.coefficients[j]*map.shift));
            transformed.coefficients[map.positiveColumn]=row.coefficients[j];
            if(map.negativeColumn)transformed.coefficients[*map.negativeColumn]=-row.coefficients[j];
        }
        p.constraints.push_back(std::move(transformed));
    }
    for(std::size_t j=0;j<problem.variables.size();++j) {
        const auto& v=problem.variables[j];
        if(!v.unrestricted && std::isfinite(v.upperBound)) {
            Constraint row;row.coefficients.resize(p.variables.size());row.coefficients[model.variables[j].positiveColumn]=1;
            row.rhs=finite(v.upperBound-v.lowerBound);p.constraints.push_back(std::move(row));
        }
    }
    validate(p);const auto n=p.variables.size(),m=p.constraints.size();
    std::size_t columns=n;
    for(const auto& row:p.constraints)model.rowSlacks.push_back(row.relation==Relation::Equal?std::optional<std::size_t>{}:columns++);
    model.matrix=core::Matrix(m,columns);model.rhs.resize(m);model.costs.resize(columns,0);
    for(std::size_t j=0;j<n;++j)model.costs[j]=model.objectiveSign*p.objective[j];
    for(std::size_t i=0;i<m;++i) {
        const auto& row=p.constraints[i];const Scalar sign=row.relation==Relation::GreaterEqual?-1:1;
        model.rhs[i]=sign*row.rhs;
        for(std::size_t j=0;j<n;++j)model.matrix(i,j)=sign*row.coefficients[j];
        if(model.rowSlacks[i])model.matrix(i,*model.rowSlacks[i])=1;
    }
    return model;
}
RevisedDualLpResult solveDualRevisedModel(const RevisedDualModel& model,
    std::vector<std::size_t> basis,std::size_t limit,Scalar tolerance) {
    if(basis.empty())for(const auto& slack:model.rowSlacks) {
        if(!slack)throw std::invalid_argument("Revised dual equality requires an explicit basis");
        basis.push_back(*slack);
    }
    RevisedDualLpResult result;
    result.canonical=solveDualRevised(model.matrix,model.rhs,model.costs,std::move(basis),limit,tolerance);
    if(result.canonical.status!=RevisedDualStatus::Optimal)return result;
    for(const auto& map:model.variables) {
        auto value=finite(map.shift+result.canonical.values.at(map.positiveColumn));
        if(map.negativeColumn)value=finite(value-result.canonical.values.at(*map.negativeColumn));
        result.values.push_back(value);
    }
    result.objective=finite(model.objectiveOffset+model.objectiveSign*result.canonical.objective);
    return result;
}
std::vector<std::size_t> prepareDualRevisedBasis(const RevisedDualModel& model,
    std::size_t limit,Scalar tolerance) {
    checkTolerance(tolerance);
    const FinalSolutionSession baseline(model.nonnegative,limit);
    if(baseline.solver().status()!=SimplexStatus::Optimal && baseline.solver().status()!=SimplexStatus::Alternative)
        throw std::invalid_argument("Revised dual bootstrap requires a finite optimal baseline");
    auto table=baseline.solver().history().back().tableau;
    const auto n=model.nonnegative.variables.size(),m=model.nonnegative.constraints.size();
    std::vector<std::optional<std::size_t>> columns(table.columns().size());
    for(std::size_t j=0;j<n;++j)columns[j]=j;
    std::size_t surplusCount=0;
    for(std::size_t i=0;i<m;++i) {
        const auto& row=model.nonnegative.constraints[i];auto relation=row.relation;
        if(row.rhs<0 && relation!=Relation::Equal)relation=relation==Relation::LessEqual?Relation::GreaterEqual:Relation::LessEqual;
        if(relation==Relation::GreaterEqual)columns[n+surplusCount++]=model.rowSlacks[i];
    }
    for(std::size_t i=0;i<m;++i) {
        const auto& row=model.nonnegative.constraints[i];
        const bool slack=(row.relation==Relation::LessEqual && row.rhs>=0) || (row.relation==Relation::GreaterEqual && row.rhs<0);
        if(slack)columns[n+surplusCount+i]=model.rowSlacks[i];
    }
    // Evict zero artificial variables using real columns; dependent equalities
    // have no candidate and require a separate rank-reduction policy.
    for(std::size_t i=0;i<m;++i)if(!columns.at(table.basis()[i])) {
        if(std::abs(table.rhs(i+1))>tolerance)throw std::runtime_error("Revised dual baseline has a nonzero artificial basic variable");
        std::optional<std::size_t> candidate;
        for(std::size_t j=0;j<columns.size();++j)
            if(columns[j] && std::find(table.basis().begin(),table.basis().end(),j)==table.basis().end() && std::abs(table.values()(i+1,j))>tolerance){candidate=j;break;}
        if(!candidate)throw std::invalid_argument("Revised dual bootstrap cannot retain rank-dependent rows; supply a reduced model");
        table.pivot(i+1,*candidate,core::Tolerance{tolerance,0});
    }
    std::vector<std::size_t> basis;
    for(auto column:table.basis())basis.push_back(*columns.at(column));
    // Independently certify the mapped basis using revised systems, with no pivots.
    if(solveDualRevisedModel(model,basis,0,tolerance).canonical.status!=RevisedDualStatus::Optimal)
        throw std::runtime_error("Revised dual bootstrap basis certificate failed");
    return basis;
}
RevisedDualLpResult reoptimizeDualRevisedRhs(const LpProblem& baseline,
    const std::vector<Scalar>& rhs,std::size_t limit,Scalar tolerance) {
    if(!rhs.empty() && rhs.size()!=baseline.constraints.size())throw std::invalid_argument("Revised dual replacement RHS size invalid");
    auto changed=baseline;
    if(!rhs.empty())for(std::size_t i=0;i<rhs.size();++i)changed.constraints[i].rhs=rhs[i];
    // Validate changes before spending work on the baseline solve.
    const auto next=makeDualRevisedModel(changed),base=makeDualRevisedModel(baseline);
    const auto basis=prepareDualRevisedBasis(base,limit,tolerance);
    return solveDualRevisedModel(next,basis,limit,tolerance);
}
}
