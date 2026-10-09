#include <tora/lp/final_solution.hpp>
#include <tora/lp/sensitivity.hpp>
#include <tora/lp/validation.hpp>
#include <algorithm>
#include <cmath>

namespace tora::lp {
namespace {
LpProblem automaticProblem(const LpProblem& original) {
    const auto validation=LpValidator::validate(original);
    if(!validation.valid())throw std::invalid_argument(validation.issues.front().message);
    auto p=original;
    // Direct final captures with both positive and negative costs establish
    // this legacy asymmetry: a negative lower shift affects restored values
    // and objective, but not the automatic path's constraint RHS values.
    for(auto& v:p.variables)if(!v.unrestricted && v.lowerBound<0) {
        v.lowerBound=0;
    }
    return p;
}
}
FinalSolutionSession::FinalSolutionSession(LpProblem problem,std::size_t limit):problem_(std::move(problem)),solver_(automaticProblem(problem_)) {
    solver_.solveAll(limit);
    if(solver_.status()==SimplexStatus::Infeasible || solver_.status()==SimplexStatus::Unbounded)return;
    const auto& table=solver_.history().back().tableau;
    values_.resize(this->problem().variables.size(),0);
    for(std::size_t j=0;j<values_.size();++j) {
        const auto found=std::find(table.basis().begin(),table.basis().end(),j);
        Scalar value=found==table.basis().end()?0:table.rhs(static_cast<std::size_t>(found-table.basis().begin())+1);
        const auto& v=this->problem().variables[j];
        if(v.unrestricted) {if(table.columns()[j].back()=='-')value=-value;}
        else {
            if(table.columns()[j].front()=='U') {
                const auto& automatic=solver_.problem().variables[j];
                value=automatic.upperBound-automatic.lowerBound-value;
            }
            value+=v.lowerBound;
        }
        values_[j]=value;
    }
}
SummaryDisplay FinalSolutionSession::display(core::NumberFormat format,bool includeSensitivity) const {
    const core::NumberFormatter f(format);const auto& p=problem();SummaryDisplay d;
    d.title=p.title;d.iteration=solver_.history().size();
    if(std::any_of(solver_.history().begin(),solver_.history().end(),[](const auto& s){return s.phase==2;}))++d.iteration;
    d.objectiveLabel=p.sense==ObjectiveSense::Maximize?"Objective Value (Max)":"Objective Value (Min)";
    if(solver_.status()==SimplexStatus::Infeasible){d.message="Problem has no feasible solution";return d;}
    if(solver_.status()==SimplexStatus::Unbounded){d.message="Optimum solution is unbounded";return d;}
    if(solver_.status()==SimplexStatus::Alternative)d.message="Alternative solution(s) detected (enter ITERATIONS mode to determine such solutions)";
    Scalar objective=solver_.history().back().tableau.rhs(0);
    for(std::size_t j=0;j<p.variables.size();++j)if(!p.variables[j].unrestricted && p.variables[j].lowerBound<0)objective+=p.objective[j]*p.variables[j].lowerBound;
    d.objectiveValue=f.format(objective);
    SummarySection variables{{"Variable","Value","Obj Coeff","Obj Val Contrib"},{}};
    for(std::size_t j=0;j<values_.size();++j)variables.rows.push_back({p.variableLabel(j)+":",{f.format(values_[j]),f.format(p.objective[j]),f.format(p.objective[j]*values_[j])}});
    d.sections.push_back(std::move(variables));
    SummarySection constraints{{"Constraint","RHS","Slack-/Surplus+"},{}};
    for(std::size_t i=0;i<p.constraints.size();++i) {
        const auto& row=p.constraints[i];Scalar lhs=0,shiftedRhs=row.rhs;
        for(std::size_t j=0;j<values_.size();++j) {
            const auto& v=p.variables[j];
            const auto displayShift=!v.unrestricted && v.lowerBound<0?v.lowerBound:Scalar(0);
            lhs+=row.coefficients[j]*(values_[j]-displayShift);
            if(!v.unrestricted)shiftedRhs-=row.coefficients[j]*solver_.problem().variables[j].lowerBound;
        }
        const Scalar gap=row.relation==Relation::GreaterEqual?lhs-row.rhs:row.rhs-lhs;
        std::string slack=f.format(std::abs(gap));
        if(gap!=0 && row.relation!=Relation::Equal) {
            const bool surplus=(row.relation==Relation::GreaterEqual)!=(shiftedRhs<0);
            slack+=surplus?"+":"-";
        }
        const std::string relation=row.relation==Relation::Equal?"=":row.relation==Relation::LessEqual?"<":">";
        constraints.rows.push_back({std::to_string(i+1)+" ("+relation+")",{f.format(row.rhs),slack}});
    }
    for(std::size_t j=0;j<p.variables.size();++j) {
        const auto& v=p.variables[j];
        if(!v.unrestricted && v.lowerBound>0) {
            const auto slack=values_[j]-v.lowerBound;
            constraints.rows.push_back({"LB-"+p.variableLabel(j),{f.format(v.lowerBound),f.format(slack)+(slack!=0?"+":"")}});
        }
        if(!v.unrestricted && std::isfinite(v.upperBound)) {
            const auto slack=v.upperBound-values_[j];
            constraints.rows.push_back({"UB-"+p.variableLabel(j),{f.format(v.upperBound),f.format(slack)+(slack!=0?"-":"")}});
        }
    }
    d.sections.push_back(std::move(constraints));
    if(includeSensitivity) {
        const auto report=sensitivity(*this);
        const auto range=[&f](Scalar value){return std::isfinite(value)?f.format(value):value<0?std::string("-infinity"):std::string("infinity");};
        SummarySection coefficients{{"Variable","Current Obj Coeff","Min Obj Coeff","Max Obj Coeff","Reduced Cost"},{},"***Sensitivity Analysis***"};
        for(std::size_t j=0;j<report.variables.size();++j) {
            const auto& v=report.variables[j];coefficients.rows.push_back({p.variableLabel(j)+":",{f.format(v.current),range(v.range.minimum),range(v.range.maximum),f.format(v.price)}});
        }
        d.sections.push_back(std::move(coefficients));
        SummarySection rhs{{"Constraint","Current RHS","Min RHS","Max RHS","Dual Price"},{},{}};
        std::vector<std::string> labels;
        for(std::size_t i=0;i<p.constraints.size();++i)labels.push_back(d.sections[1].rows[i].label);
        for(std::size_t j=0;j<p.variables.size();++j) {
            const auto& v=p.variables[j];if(v.unrestricted)continue;
            if(v.lowerBound>0)labels.push_back("LB-"+p.variableLabel(j));
            // The reviewed negative-lower/upper summary omits its sensitivity UB row.
            if(v.lowerBound>=0 && std::isfinite(v.upperBound))labels.push_back("UB-"+p.variableLabel(j));
        }
        for(std::size_t i=0;i<report.constraints.size();++i) {
            const auto& r=report.constraints[i];rhs.rows.push_back({labels.at(i),{f.format(r.current),range(r.range.minimum),range(r.range.maximum),f.format(r.price)}});
        }
        d.sections.push_back(std::move(rhs));
    }
    return d;
}
} // namespace tora::lp
