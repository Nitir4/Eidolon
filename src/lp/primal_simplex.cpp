#include <tora/lp/primal_simplex.hpp>
#include <tora/lp/validation.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace tora::lp {
namespace {
LpProblem normalized(LpProblem p) {
    const auto validation=LpValidator::validate(p);
    if(!validation.valid())throw std::invalid_argument(validation.issues.front().message);
    for(auto& row:p.constraints) {
        for(std::size_t j=0;j<p.variables.size();++j)
            if(!p.variables[j].unrestricted && std::isfinite(p.variables[j].lowerBound))row.rhs-=row.coefficients[j]*p.variables[j].lowerBound;
        if(!std::isfinite(row.rhs))throw std::overflow_error("LP lower-bound shift overflowed");
        if(row.rhs<0) {
            row.rhs=-row.rhs;for(auto& a:row.coefficients)a=-a;
            if(row.relation!=Relation::Equal)row.relation=row.relation==Relation::LessEqual?Relation::GreaterEqual:Relation::LessEqual;
        }
    }
    return p;
}
std::string bound(Scalar x,const core::NumberFormatter& formatter) {
    return std::isfinite(x)?formatter.format(x):(x<0?"-infinity":"infinity");
}
core::Tableau retainingBlocks(core::Tableau next,const core::Tableau& previous) {
    for(std::size_t j=0;j<previous.columns().size();++j)next.setObjectiveBlocked(j,previous.objectiveBlocked(j));
    return next;
}
}
bool PrimalSimplexSession::requiresArtificial(const LpProblem& problem) {
    const auto p=normalized(problem);
    return std::any_of(p.constraints.begin(),p.constraints.end(),[](const auto& r){return r.relation!=Relation::LessEqual;});
}
PrimalSimplexSession::PrimalSimplexSession(LpProblem problem,PrimalMethod method,Scalar penalty):method_(method),problem_(std::move(problem)) {
    const auto p=normalized(problem_);
    if(!std::isfinite(penalty))throw std::invalid_argument("M must be a finite number");
    const bool artificial=requiresArtificial(problem_);
    if(artificial && method==PrimalMethod::AllSlack)throw std::invalid_argument(redirectWarning());
    if(!artificial)method_=PrimalMethod::AllSlack;
    const auto n=p.variables.size(),m=p.constraints.size();
    const auto surplus=static_cast<std::size_t>(std::count_if(p.constraints.begin(),p.constraints.end(),[](const auto& r){return r.relation==Relation::GreaterEqual;}));
    const auto cols=n+surplus+m;
    const bool twoPhase=method_==PrimalMethod::TwoPhase;
    originalObjective_.resize(cols+1,0);
    core::Matrix values(m+1,cols+1);std::vector<std::string> columns;std::vector<std::size_t> basis;
    for(std::size_t j=0;j<n;++j) {
        const auto& v=p.variables[j];
        ranges_.push_back(v.unrestricted?std::numeric_limits<Scalar>::infinity():v.upperBound-v.lowerBound);
        Scalar feasibilityCost=0;
        if(twoPhase)for(const auto& r:p.constraints)if(r.relation!=Relation::LessEqual)feasibilityCost+=r.coefficients[j];
        const bool negative=v.unrestricted && (twoPhase?feasibilityCost<0:(p.sense==ObjectiveSense::Maximize?p.objective[j]<0:p.objective[j]>0));
        columns.push_back(v.unrestricted?p.variableLabel(j)+(negative?"-":"+"):(v.lowerBound!=0?"L":"")+p.variableLabel(j));
        originalObjective_[j]=negative?p.objective[j]:-p.objective[j];
        if(!v.unrestricted)originalObjective_[cols]+=p.objective[j]*v.lowerBound;
        if(!twoPhase){values(0,j)=originalObjective_[j];if(!v.unrestricted)values(0,cols)+=p.objective[j]*v.lowerBound;}
    }
    ranges_.resize(cols,std::numeric_limits<Scalar>::infinity());
    for(std::size_t k=0;k<surplus;++k)columns.push_back("Sx"+std::to_string(n+k+1));
    std::size_t k=0;
    const auto signedPenalty=twoPhase?Scalar(1):(p.sense==ObjectiveSense::Maximize?-penalty:penalty);
    for(std::size_t i=0;i<m;++i) {
        const bool slack=p.constraints[i].relation==Relation::LessEqual;
        const auto basicColumn=n+surplus+i;
        columns.push_back(std::string(slack?"sx":"Rx")+std::to_string(basicColumn+1));basis.push_back(basicColumn);
        for(std::size_t j=0;j<n;++j)values(i+1,j)=columns[j].back()=='-'?-p.constraints[i].coefficients[j]:p.constraints[i].coefficients[j];
        if(p.constraints[i].relation==Relation::GreaterEqual)values(i+1,n+k++)=-1;
        values(i+1,basicColumn)=1;values(i+1,cols)=p.constraints[i].rhs;
        if(!slack)for(std::size_t j=0;j<=cols;++j) {
            if(j!=basicColumn)values(0,j)+=signedPenalty*values(i+1,j);
        }
    }
    history_.push_back({core::Tableau(std::move(values),std::move(columns),std::move(basis)),{}, {},twoPhase?1:0});
    assess();
}
ObjectiveSense PrimalSimplexSession::objectiveSense() const {
    return history_.back().phase==1?ObjectiveSense::Minimize:problem_.sense;
}
bool PrimalSimplexSession::basic(std::size_t column) const {
    const auto& basis=history_.back().tableau.basis();return std::find(basis.begin(),basis.end(),column)!=basis.end();
}
std::optional<SimplexChoice> PrimalSimplexSession::choice(std::size_t column) const {
    const auto& t=history_.back().tableau;
    if(column>=t.columns().size() || basic(column))return {};
    SimplexChoice result{column,{},false,false,std::numeric_limits<Scalar>::infinity()};
    for(std::size_t row=1;row<t.values().rows();++row) {
        const Scalar a=t.values()(row,column);
        Scalar step=std::numeric_limits<Scalar>::infinity();bool upper=false;
        if(a>0)step=t.rhs(row)/a;
        else if(a<0 && std::isfinite(ranges_[t.basis()[row-1]])) {
            step=(ranges_[t.basis()[row-1]]-t.rhs(row))/(-a);upper=true;
        }
        if(step>=0 && step<result.step) {result.step=step;result.row=row;result.leavingUpperBound=upper;}
    }
    if(ranges_[column]<result.step) {result.row.reset();result.step=ranges_[column];result.upperBound=true;}
    return result;
}
std::optional<SimplexChoice> PrimalSimplexSession::recommendation() const {
    const auto& t=history_.back().tableau;std::optional<std::size_t> col;Scalar best=0;
    for(std::size_t j=0;j<t.columns().size();++j) {
        if(basic(j) || t.objectiveBlocked(j))continue;
        const Scalar a=t.values()(0,j);
        if((objectiveSense()==ObjectiveSense::Maximize && a<best) || (objectiveSense()==ObjectiveSense::Minimize && a>best)){best=a;col=j;}
    }
    return col?choice(*col):std::optional<SimplexChoice>{};
}
void PrimalSimplexSession::assess() {
    // A free variable has one oriented column in TORA, not two split columns.
    // Reorient a nonbasic free column when its opposite direction improves z.
    auto& table=history_.back().tableau;
    auto values=table.values();auto columns=table.columns();bool changed=false;
    for(std::size_t j=0;j<problem_.variables.size();++j) {
        if(!problem_.variables[j].unrestricted || basic(j))continue;
        const auto cost=values(0,j);
        if(objectiveSense()==ObjectiveSense::Maximize?cost>0:cost<0) {
            for(std::size_t i=0;i<values.rows();++i)values(i,j)=-values(i,j);
            columns[j].back()=columns[j].back()=='+'?'-':'+';changed=true;
        }
    }
    if(changed)table=retainingBlocks(core::Tableau(std::move(values),std::move(columns),table.basis()),table);
    if(recommendation()){status_=SimplexStatus::Running;return;}
    if(history_.back().phase==1) {
        phaseReady_=table.rhs(0)<=0;
        status_=phaseReady_?SimplexStatus::Running:SimplexStatus::Infeasible;
        return;
    }
    status_=SimplexStatus::Optimal;
    const auto& t=history_.back().tableau;
    for(std::size_t j=0;j<t.columns().size();++j)
        if(!basic(j) && !t.objectiveBlocked(j) && t.values()(0,j)==0){status_=SimplexStatus::Alternative;break;}
}
void PrimalSimplexSession::selectEntering(std::size_t col) {
    const auto recommended=recommendation();const auto& t=history_.back().tableau;
    if(col>=t.columns().size() || basic(col) || t.objectiveBlocked(col) || status_==SimplexStatus::Unbounded || status_==SimplexStatus::Infeasible ||
        (status_==SimplexStatus::Running?(!recommended || recommended->column!=col):t.values()(0,col)!=0))
        throw std::invalid_argument(enteringError());
    history_.back().entering=col;history_.back().leaving.reset();
}
void PrimalSimplexSession::selectLeaving(std::size_t row) {
    const auto col=history_.back().entering;
    auto picked=col?choice(*col):recommendation();
    if(!picked || !picked->row || *picked->row!=row)throw std::invalid_argument(leavingError());
    history_.back().entering=picked->column;history_.back().leaving=row;
}
core::Tableau PrimalSimplexSession::complemented(core::Tableau t,std::size_t column) const {
    auto values=t.values();auto columns=t.columns();const auto rhs=columns.size();
    for(std::size_t row=0;row<values.rows();++row) {
        values(row,rhs)-=values(row,column)*ranges_[column];values(row,column)=-values(row,column);
    }
    const auto found=std::find(t.basis().begin(),t.basis().end(),column);
    if(found!=t.basis().end()) {
        const auto row=static_cast<std::size_t>(found-t.basis().begin())+1;
        for(std::size_t j=0;j<values.columns();++j)values(row,j)=-values(row,j);
    }
    if(columns[column].front()=='U')columns[column].erase(0,1);else columns[column].insert(0,"U");
    return retainingBlocks(core::Tableau(std::move(values),std::move(columns),t.basis()),t);
}
core::Tableau PrimalSimplexSession::reconstructed(const core::Tableau& table,bool restoreObjective) const {
    // Reconstruct from the original equations and the ordered basis instead
    // of repeatedly rounding previously eliminated coefficients. This is a
    // numerical implementation choice, not a claim about TORA's internals.
    // The scientific free-direction oracle exposes a -5.55112e-17 residual
    // that is lost by forcing every basic-column off-diagonal to exact zero.
    const auto& initial=history_.front().tableau;
    auto original=initial.values();const auto m=table.basis().size(),cols=table.columns().size();
    if(restoreObjective || history_.back().phase==2)for(std::size_t j=0;j<=cols;++j)original(0,j)=originalObjective_[j];
    else if(method_==PrimalMethod::TwoPhase) {
        // Rebuild the feasibility dual from the original -sum(Rx) costs.
        // Reusing its already canonical initial row introduces residuals in
        // basic decision costs that the full scientific oracle excludes.
        for(std::size_t j=0;j<=cols;++j)original(0,j)=j<cols && initial.columns()[j].rfind("Rx",0)==0?Scalar(-1):Scalar(0);
    }
    for(std::size_t j=0;j<problem_.variables.size();++j) {
        if(table.columns()[j]==initial.columns()[j])continue;
        for(std::size_t i=0;i<original.rows();++i) {
            if(!problem_.variables[j].unrestricted)original(i,cols)-=original(i,j)*ranges_[j];
            original(i,j)=-original(i,j);
        }
    }
    auto ordered=table.basis();std::sort(ordered.begin(),ordered.end());
    core::Matrix inverse(m,2*m);
    for(std::size_t i=0;i<m;++i){for(std::size_t j=0;j<m;++j)inverse(i,j)=original(i+1,ordered[j]);inverse(i,m+i)=1;}
    for(std::size_t j=0;j<m;++j) {
        auto row=j;while(row<m && inverse(row,j)==0)++row;
        if(row==m)throw std::domain_error("Selected simplex basis is singular");
        if(row!=j)for(std::size_t k=0;k<2*m;++k)std::swap(inverse(row,k),inverse(j,k));
        const auto pivot=inverse(j,j);
        if(method_!=PrimalMethod::AllSlack) {
            // Cross-product elimination avoids introducing fractions until
            // after each row update; the M scientific oracle distinguishes
            // this order from dividing the pivot row first.
            for(std::size_t i=0;i<m;++i){if(i==j)continue;const auto a=inverse(i,j);for(std::size_t k=0;k<2*m;++k)inverse(i,k)=(inverse(i,k)*pivot-a*inverse(j,k))/pivot;}
            for(std::size_t k=0;k<2*m;++k)inverse(j,k)/=pivot;
        }else {
            for(std::size_t k=0;k<2*m;++k)inverse(j,k)/=pivot;
            for(std::size_t i=0;i<m;++i){if(i==j)continue;const auto a=inverse(i,j);for(std::size_t k=0;k<2*m;++k)inverse(i,k)-=a*inverse(j,k);}
        }
    }
    core::Matrix result(m+1,cols+1);
    for(std::size_t i=0;i<m;++i) {
        const auto pos=static_cast<std::size_t>(std::find(ordered.begin(),ordered.end(),table.basis()[i])-ordered.begin());
        for(std::size_t j=0;j<=cols;++j)for(std::size_t k=0;k<m;++k)result(i+1,j)+=inverse(pos,m+k)*original(k+1,j);
    }
    if(method_!=PrimalMethod::AllSlack) {
        std::vector<Scalar> dual(m,0);
        for(std::size_t i=0;i<m;++i){const auto pos=static_cast<std::size_t>(std::find(ordered.begin(),ordered.end(),table.basis()[i])-ordered.begin());for(std::size_t k=0;k<m;++k)dual[k]+=original(0,table.basis()[i])*inverse(pos,m+k);}
        for(std::size_t j=0;j<=cols;++j){result(0,j)=original(0,j);for(std::size_t k=0;k<m;++k)result(0,j)-=dual[k]*original(k+1,j);}
    }else for(std::size_t j=0;j<=cols;++j) {
        result(0,j)=original(0,j);
        for(std::size_t i=0;i<m;++i)result(0,j)-=original(0,table.basis()[i])*result(i+1,j);
    }
    // Two-phase feasibility/restored objectives are canonical on the basis.
    // Preserve the separately observed constraint residuals; only these
    // mathematically zero basic reduced costs are written as exact zero.
    if(method_==PrimalMethod::TwoPhase)for(const auto column:table.basis())result(0,column)=0;
    return retainingBlocks(core::Tableau(std::move(result),table.columns(),table.basis()),table);
}
bool PrimalSimplexSession::next() {
    if(status_==SimplexStatus::Unbounded || status_==SimplexStatus::Infeasible)return false;
    if(phaseReady_) {
        auto table=reconstructed(history_.back().tableau,true);
        for(std::size_t j=0;j<table.columns().size();++j)
            if(history_.front().tableau.columns()[j].rfind("Rx",0)==0)table.setObjectiveBlocked(j,true);
        history_.push_back({std::move(table),{}, {},2});phaseReady_=false;assess();return true;
    }
    auto picked=history_.back().entering?choice(*history_.back().entering):recommendation();
    if(!picked){assess();return false;}
    if(!std::isfinite(picked->step)){
        history_.back().entering=picked->column;history_.back().leaving.reset();
        status_=SimplexStatus::Unbounded;return false;
    }
    auto next=history_.back().tableau;
    if(picked->upperBound)next=complemented(std::move(next),picked->column);
    else {
        if(picked->leavingUpperBound) {
            const auto leaving=next.basis()[*picked->row-1];next=complemented(std::move(next),leaving);
        }
        next.pivot(*picked->row,picked->column,core::Tolerance(0,0));
    }
    next=reconstructed(next);
    history_.back().entering=picked->column;history_.back().leaving=picked->row;
    const int phase=history_.back().phase;
    history_.push_back({std::move(next),{}, {},phase});assess();return true;
}
void PrimalSimplexSession::solveAll(std::size_t limit) {
    std::size_t count=0;
    while(status_==SimplexStatus::Running) {
        if(count++>=limit)throw std::runtime_error("Native simplex iteration safety limit reached; the partial trace is retained");
        next();
    }
}
SimplexDisplay PrimalSimplexSession::display(std::size_t index,core::NumberFormat format) const {
    const auto& step=history_.at(index);const auto& t=step.tableau;const core::NumberFormatter f(format);
    SimplexDisplay result;result.heading="Iteration "+std::to_string(index+1);result.columns=t.columns();result.columns.push_back("Solution");
    if(step.phase)result.heading="Phase "+std::to_string(step.phase)+" (Iter "+std::to_string(index+1);
    for(std::size_t i=0;i<t.values().rows();++i) {
        SimplexDisplayRow row{i==0?(step.phase!=1 && problem_.sense==ObjectiveSense::Maximize?"z (max)":"z (min)"):t.basicLabel(i),{}};
        for(std::size_t j=0;j<t.values().columns();++j)row.values.push_back(i==0 && j<t.columns().size() && t.objectiveBlocked(j)?"blocked":f.format(t.values()(i,j)));
        result.rows.push_back(std::move(row));
    }
    for(int i=0;i<3;++i) {
        SimplexDisplayRow row{i==0?"Lower Bound":i==1?"Upper Bound":"Unrestr'd (y/n)?",std::vector<std::string>(t.values().columns())};
        for(std::size_t j=0;j<problem_.variables.size();++j) {
            const auto& v=problem_.variables[j];row.values[j]=i==0?bound(v.lowerBound,f):i==1?bound(v.upperBound,f):(v.unrestricted?"y":"n");
        }
        result.rows.push_back(std::move(row));
    }
    if(step.entering)result.entering=t.columns()[*step.entering];
    if(step.leaving)result.leaving=t.basicLabel(*step.leaving);
    return result;
}
std::string PrimalSimplexSession::terminalMessage() const {
    if(status_==SimplexStatus::Infeasible)return "Phase 2 nixed. Phase1 shows the problem has no feasible solution";
    if(status_==SimplexStatus::Unbounded)return "Last tableau shows solution is unbounded";
    if(status_==SimplexStatus::Running)return {};
    auto message="Iteration "+std::to_string(history_.size())+" is optimal";
    if(status_==SimplexStatus::Alternative)message+=". Alternative soltuion(s) detected: Click a nonbasic var with zj-cj=0 in last tableau, then click NEXT ITERATION button.\nOutput Summary cannot be used to view alternative optima solution";
    return message;
}
const char* PrimalSimplexSession::enteringError(){return "If there is a tie, TORA always selects left-most TIED col only. Else, your selection is not consistent with Optimality Condition. Re-select or click NEXT/ALL ITERATIONS button.";}
const char* PrimalSimplexSession::leavingError(){return "If there is a tie, TORA always selects top-most TIED row only. Else, your selection is not consistent with Feasibility Condition. Reselect or click NEXT/ALL ITERATIONS button.";}
const char* PrimalSimplexSession::redirectWarning(){return "Problem has >= and/or = constraints, thus requiring M or Two-phase method. Two-phase method will be used. Else click CANCEL to use M-method";}
} // namespace tora::lp
