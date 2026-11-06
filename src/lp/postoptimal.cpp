#include <tora/lp/postoptimal.hpp>
#include <tora/lp/validation.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace tora::lp {
namespace {
Scalar finite(Scalar value) {
    if(!std::isfinite(value))throw std::overflow_error("Post-optimal arithmetic exceeds the finite numerical range");
    return value;
}
void validate(const LpProblem& p) {
    const auto report=LpValidator::validate(p);
    if(!report.valid())throw std::invalid_argument(report.issues.front().message);
}
struct Canonical {
    LpProblem problem;
    std::vector<VariableMapping> mappings;
    std::vector<std::string> labels;
};
Canonical canonicalize(const LpProblem& original) {
    Canonical out;out.problem.title=original.title;out.problem.sense=original.sense;
    for(std::size_t j=0;j<original.variables.size();++j) {
        const auto& variable=original.variables[j];VariableMapping mapping;
        const auto column=out.problem.variables.size();
        out.problem.variables.push_back({});out.problem.objective.push_back(original.objective[j]);
        mapping.terms.push_back({column,1});
        if(variable.unrestricted) {
            out.labels.push_back(original.variableLabel(j)+"+");
            mapping.terms.push_back({column+1,-1});out.problem.variables.push_back({});
            out.problem.objective.push_back(-original.objective[j]);out.labels.push_back(original.variableLabel(j)+"-");
        }else {
            mapping.shift=variable.lowerBound;
            out.labels.push_back(variable.lowerBound==0?original.variableLabel(j):original.variableLabel(j)+" - lower bound");
        }
        out.mappings.push_back(std::move(mapping));
    }
    for(const auto& row:original.constraints) {
        Constraint transformed;transformed.relation=row.relation;transformed.rhs=row.rhs;
        transformed.coefficients.resize(out.problem.variables.size());
        for(std::size_t j=0;j<original.variables.size();++j) {
            transformed.rhs=finite(transformed.rhs-finite(row.coefficients[j]*out.mappings[j].shift));
            for(const auto& term:out.mappings[j].terms)transformed.coefficients[term.column]=finite(row.coefficients[j]*term.coefficient);
        }
        out.problem.constraints.push_back(std::move(transformed));
    }
    for(std::size_t j=0;j<original.variables.size();++j) {
        const auto& variable=original.variables[j];
        if(!variable.unrestricted && std::isfinite(variable.upperBound)) {
            Constraint upper;upper.coefficients.resize(out.problem.variables.size());
            upper.coefficients[out.mappings[j].terms.front().column]=1;
            upper.rhs=finite(variable.upperBound-variable.lowerBound);out.problem.constraints.push_back(std::move(upper));
        }
    }
    validate(out.problem);return out;
}
std::vector<Scalar> restore(const std::vector<Scalar>& values,const std::vector<VariableMapping>& mappings) {
    std::vector<Scalar> restored;
    for(const auto& mapping:mappings) {
        auto value=mapping.shift;
        for(const auto& term:mapping.terms)value=finite(value+finite(term.coefficient*values.at(term.column)));
        restored.push_back(value);
    }
    return restored;
}
Scalar objectiveValue(const LpProblem& p,const std::vector<Scalar>& values) {
    Scalar total=0;for(std::size_t j=0;j<values.size();++j)total=finite(total+finite(p.objective[j]*values[j]));return total;
}
bool feasible(const LpProblem& p,const std::vector<Scalar>& values,const core::Tolerance& tolerance) {
    for(std::size_t j=0;j<values.size();++j) {
        const auto& v=p.variables[j];
        if(!v.unrestricted && (tolerance.less(values[j],v.lowerBound) ||
            (std::isfinite(v.upperBound) && tolerance.less(v.upperBound,values[j]))))return false;
    }
    for(const auto& row:p.constraints) {
        Scalar lhs=0;for(std::size_t j=0;j<values.size();++j)lhs=finite(lhs+finite(row.coefficients[j]*values[j]));
        if((row.relation==Relation::LessEqual && tolerance.less(row.rhs,lhs)) ||
            (row.relation==Relation::GreaterEqual && tolerance.less(lhs,row.rhs)) ||
            (row.relation==Relation::Equal && !tolerance.equal(lhs,row.rhs)))return false;
    }
    return true;
}
PostOptimalOutcome outcome(const LpProblem& original,const Canonical& canonical,
    const core::Tolerance& tolerance,std::size_t limit) {
    const FinalSolutionSession session(canonical.problem,limit);PostOptimalOutcome out;
    out.status=session.solver().status();out.trace=session.solver().history();
    if(out.status==SimplexStatus::Infeasible || out.status==SimplexStatus::Unbounded)return out;
    out.values=restore(session.values(),canonical.mappings);out.objective=objectiveValue(original,out.values);
    if(!feasible(original,out.values,tolerance))throw std::runtime_error("Changed LP solver returned a numerically infeasible result");
    const auto& table=out.trace.back().tableau;
    for(std::size_t row=0;row<table.basis().size();++row)
        if(table.objectiveBlocked(table.basis()[row]) && !tolerance.zero(table.rhs(row+1)))
            throw std::runtime_error("Changed LP has a nonzero artificial basic variable");
    for(std::size_t column=0;column<table.columns().size();++column) {
        if(table.objectiveBlocked(column) || std::find(table.basis().begin(),table.basis().end(),column)!=table.basis().end())continue;
        Scalar reduced=column<canonical.problem.objective.size()?-canonical.problem.objective[column]:0;
        for(std::size_t row=0;row<table.basis().size();++row) {
            const auto basic=table.basis()[row];const Scalar cost=basic<canonical.problem.objective.size()?canonical.problem.objective[basic]:0;
            reduced=finite(reduced+finite(cost*table.values()(row+1,column)));
        }
        if(original.sense==ObjectiveSense::Maximize?tolerance.less(reduced,0):tolerance.less(0,reduced))
            throw std::runtime_error("Changed LP solver returned a numerically nonoptimal basis");
    }
    return out;
}
}
PostOptimalResult analyzeChanges(const LpProblem& problem,const std::vector<Scalar>& objective,
    const std::vector<Scalar>& rhs,const core::Tolerance& tolerance,std::size_t limit) {
    validate(problem);
    if((!objective.empty() && objective.size()!=problem.variables.size()) ||
        (!rhs.empty() && rhs.size()!=problem.constraints.size()))throw std::invalid_argument("Changed Cj/RHS vector has an incorrect size");
    auto changed=problem;if(!objective.empty())changed.objective=objective;
    if(!rhs.empty())for(std::size_t i=0;i<rhs.size();++i)changed.constraints[i].rhs=rhs[i];
    validate(changed);const auto base=canonicalize(problem),next=canonicalize(changed);
    PostOptimalResult result;result.changedProblem=changed;result.canonicalProblem=next.problem;
    result.mappings=base.mappings;result.canonicalVariables=base.labels;
    result.before=outcome(problem,base,tolerance,limit);
    if(!result.before.objective)throw std::invalid_argument("Post-optimal analysis requires an optimal baseline LP");
    const auto& initial=result.before.trace.front().tableau;
    const auto& table=result.before.trace.back().tableau;
    auto& retained=result.retained;retained.columns=table.basis();retained.primalFeasible=true;retained.dualFeasible=true;
    std::vector<Scalar> canonicalValues(base.problem.variables.size());
    for(std::size_t row=0;row<table.basis().size();++row) {
        Scalar value=table.rhs(row+1);
        for(std::size_t i=0;i<base.problem.constraints.size();++i) {
            // The initial basis columns form identity AFTER RHS normalization.
            const Scalar sign=base.problem.constraints[i].rhs<0?-1:1;
            const auto delta=finite(next.problem.constraints[i].rhs-base.problem.constraints[i].rhs);
            value=finite(value+finite(table.values()(row+1,initial.basis()[i])*finite(sign*delta)));
        }
        retained.basisValues.push_back(value);
        const auto column=table.basis()[row];
        if(tolerance.less(value,0) || (table.objectiveBlocked(column) && !tolerance.zero(value)))retained.primalFeasible=false;
        if(column<canonicalValues.size())canonicalValues[column]=value;
    }
    retained.values=restore(canonicalValues,base.mappings);retained.objective=objectiveValue(changed,retained.values);
    retained.primalFeasible=retained.primalFeasible && feasible(changed,retained.values,tolerance);
    for(std::size_t column=0;column<table.columns().size();++column) {
        Scalar reduced=column<next.problem.objective.size()?-next.problem.objective[column]:0;
        for(std::size_t row=0;row<table.basis().size();++row) {
            const auto basic=table.basis()[row];const Scalar cost=basic<next.problem.objective.size()?next.problem.objective[basic]:0;
            reduced=finite(reduced+finite(cost*table.values()(row+1,column)));
        }
        retained.reducedCosts.push_back(reduced);
        const bool basic=std::find(table.basis().begin(),table.basis().end(),column)!=table.basis().end();
        if(!basic && !table.objectiveBlocked(column) &&
            (changed.sense==ObjectiveSense::Maximize?tolerance.less(reduced,0):tolerance.less(0,reduced)))retained.dualFeasible=false;
    }
    result.after=outcome(changed,next,tolerance,limit);
    if(retained.primalFeasible && retained.dualFeasible &&
        (!result.after.objective || !tolerance.equal(retained.objective,*result.after.objective)))
        throw std::runtime_error("Retained-basis certificate disagrees with the changed LP solution");
    return result;
}
} // namespace tora::lp
