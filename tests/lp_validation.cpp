#include <tora/lp/validation.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
using namespace tora::lp;
void require(bool condition,const std::string& message){if(!condition)throw std::runtime_error(message);}
const ValidationIssue& issue(const ValidationReport& report, IssueCode code) {
    for(const auto& item:report.issues)if(item.code==code)return item;
    throw std::runtime_error("Required diagnostic missing");
}
void error(const LpProblem& model,IssueCode code){const auto report=LpValidator::validate(model);require(!report.valid(),"Invalid model reported valid");require(issue(report,code).severity==Severity::Error,"Expected blocking error");}
}
int main() {
    try {
        const auto zero=LpValidator::dimensions(0,1);
        require(!zero.valid() && issue(zero,IssueCode::NoVariables).message=="Invalid problem size" && issue(zero,IssueCode::NoVariables).title=="Problem Size" && issue(zero,IssueCode::NoVariables).origin==DiagnosticOrigin::ObservedTora,"Observed zero-variable message");
        const auto zeroConstraints=LpValidator::dimensions(2,0);
        require(!zeroConstraints.valid() && issue(zeroConstraints,IssueCode::NoConstraints).message=="Invalid problem size" && issue(zeroConstraints,IssueCode::NoConstraints).origin==DiagnosticOrigin::ObservedTora,"Observed zero-constraint diagnostic");
        for(const auto counts:{std::pair<std::size_t,std::size_t>{0,1},{2,0},{0,0}}){
            bool rejected=false;try{(void)LpProblem::blank(counts.first,counts.second);}catch(const std::invalid_argument&){rejected=true;}
            require(rejected,"Factory rejects empty dimensions");
        }
        require(LpValidator::dimensions(1,1).valid() && LpValidator::dimensions(100,100).valid(),"Observed dimension bounds");
        for(const auto count:{std::size_t(101),std::numeric_limits<std::size_t>::max()}) {
            const auto a=LpValidator::dimensions(count,1);const auto b=LpValidator::dimensions(1,count);
            require(!a.valid() && !b.valid(),"Student size limits on each axis");
            require(issue(a,IssueCode::StudentSizeLimit).message=="Student version limits maximum size to 100" && issue(a,IssueCode::StudentSizeLimit).origin==DiagnosticOrigin::ObservedTora,"Exact size-limit message");
            bool rejected=false;try{(void)LpProblem::blank(count,1);}catch(const std::invalid_argument&){rejected=true;}
            require(rejected,"Factory must check dimensions before allocating");
        }
        auto model=LpProblem::blank(2,3);model.sense=static_cast<ObjectiveSense>(99);error(model,IssueCode::InvalidSense);
        model=LpProblem::blank(2,3);model.objective.pop_back();error(model,IssueCode::ObjectiveSize);
        model=LpProblem::blank(2,3);model.constraints[1].coefficients.push_back(3);error(model,IssueCode::ConstraintSize);
        model=LpProblem::blank(2,3);model.constraints[2].relation=static_cast<Relation>(99);error(model,IssueCode::InvalidRelation);
        const auto inf=std::numeric_limits<Scalar>::infinity();const auto nan=std::numeric_limits<Scalar>::quiet_NaN();
        for(const auto value:{inf,-inf,nan}) {
            model=LpProblem::blank(2,3);model.objective[1]=value;error(model,IssueCode::NonfiniteObjective);
            auto report=LpValidator::validate(model);require(issue(report,IssueCode::NonfiniteObjective).variable==1,"Objective error location");
            model=LpProblem::blank(2,3);model.constraints[1].coefficients[0]=value;error(model,IssueCode::NonfiniteCoefficient);
            report=LpValidator::validate(model);require(issue(report,IssueCode::NonfiniteCoefficient).constraint==1 && issue(report,IssueCode::NonfiniteCoefficient).variable==0,"Matrix error location");
            model=LpProblem::blank(2,3);model.constraints[0].rhs=value;error(model,IssueCode::NonfiniteRhs);
            model=LpProblem::blank(2,3);model.variables[0].lowerBound=value;error(model,IssueCode::InvalidLowerBound);
        }
        for(const auto value:{-inf,nan}){model=LpProblem::blank(2,3);model.variables[1].upperBound=value;error(model,IssueCode::InvalidUpperBound);}
        model=LpProblem::blank(2,3);model.variables[0].lowerBound=1;model.variables[0].upperBound=1;
        require(LpValidator::validate(model).valid(),"Equal bounds admitted in TORA");
        model.variables[0].lowerBound=3;
        auto report=LpValidator::validate(model);
        require(!report.valid(),"Reversed bounds blocked");
        const auto& bounds=issue(report,IssueCode::InconsistentBounds);
        require(bounds.variable==0 && bounds.title=="Input Data Error" && bounds.message=="Lower bound exceeds upper bound for variable 1.\nReenter correct bounds" && bounds.origin==DiagnosticOrigin::ObservedTora,"Exact reversed-bound diagnostic and variable index");
        model=LpProblem::blank(2,3);model.variables[1].lowerBound=std::nextafter(1.0,2.0);model.variables[1].upperBound=1;
        report=LpValidator::validate(model);
        require(!report.valid() && issue(report,IssueCode::InconsistentBounds).variable==1 && issue(report,IssueCode::InconsistentBounds).message=="Lower bound exceeds upper bound for variable 2.\nReenter correct bounds","Strict consistency guard, no tolerance, correct one-based message");
        model=LpProblem::blank(2,3);
        model.variables[0].lowerBound=-3;model.variables[0].upperBound=-1;
        require(LpValidator::validate(model).valid(),"Signed finite bounds retained");
        model=LpProblem::blank(2,3);model.variables[0].unrestricted=true;
        require(LpValidator::validate(model).valid() && model.variables.size()==2,"Unrestricted flag stored without expansion");
        model.variables[0].lowerBound=-inf;
        require(LpValidator::validate(model).valid(),"Captured y entry sets negative infinite lower bound");
        model.variables[0].lowerBound=3;model.variables[0].upperBound=1;
        report=LpValidator::validate(model);
        require(!report.valid() && issue(report,IssueCode::InconsistentBounds).origin==DiagnosticOrigin::ObservedTora,"TORA checks reversed bounds even with unrestricted y");
        model=LpProblem::blank(2,3);
        model.objective[0]=-0.0;model.constraints[0].rhs=-0.0;
        require(LpValidator::validate(model).valid() && std::signbit(model.objective[0]) && std::signbit(model.constraints[0].rhs),"Validation does not snap signed zero");
        model.objective[1]=1.0049999999999997;model.constraints[0].coefficients[1]=1e-12;
        require(LpValidator::validate(model).valid() && model.objective[1]==1.0049999999999997 && model.constraints[0].coefficients[1]==1e-12,"Validation must not round or apply solver tolerance");
        model.objective[0]=nan;model.constraints[0].rhs=inf;
        report=LpValidator::validate(model);require(report.issues.size()==2 && report.issues[0].code==IssueCode::NonfiniteObjective && report.issues[1].code==IssueCode::NonfiniteRhs,"Stable complete diagnostic ordering");
        require(report.issues[0].origin==DiagnosticOrigin::Engineering,"Do not invent TORA's malformed-number wording");
        std::cout<<"Dimension/bound oracle messages, structural/numeric safety and non-mutating validation passed\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
