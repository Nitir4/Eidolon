#include <tora/integer/integer.hpp>
#include <tora/lp/primal_simplex.hpp>
#include <tora/lp/validation.hpp>
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace tora::integer {
namespace {
double finite(double x){if(!std::isfinite(x))throw std::overflow_error("Integer arithmetic exceeds finite range");return x;}
struct Relaxation {lp::SimplexStatus status;std::vector<double> values;double objective=0;};
// Independent two-phase tableau for node relaxations. The interactive LP
// engine deliberately preserves legacy display arithmetic; node bounds need
// a mathematical feasibility certificate instead of those compatibility quirks.
struct NodeSimplex {
    std::size_t m,n,pivots=0;std::vector<int> basic,nonbasic;std::vector<std::vector<double>> d;
    NodeSimplex(const std::vector<std::vector<double>>& a,const std::vector<double>& b,const std::vector<double>& c):m(b.size()),n(c.size()),basic(m),nonbasic(n+1),d(m+2,std::vector<double>(n+2)) {
        for(std::size_t i=0;i<m;++i){for(std::size_t j=0;j<n;++j)d[i][j]=a[i][j];basic[i]=static_cast<int>(n+i);d[i][n]=-1;d[i][n+1]=b[i];}
        for(std::size_t j=0;j<n;++j){nonbasic[j]=static_cast<int>(j);d[m][j]=-c[j];}nonbasic[n]=-1;d[m+1][n]=1;
    }
    void pivot(std::size_t r,std::size_t s){if(++pivots>50000)throw std::runtime_error("Integer LP relaxation iteration limit reached");const double inv=1/d[r][s];for(std::size_t i=0;i<m+2;++i)if(i!=r)for(std::size_t j=0;j<n+2;++j)if(j!=s)d[i][j]=finite(d[i][j]-d[r][j]*d[i][s]*inv);for(std::size_t j=0;j<n+2;++j)if(j!=s)d[r][j]=finite(d[r][j]*inv);for(std::size_t i=0;i<m+2;++i)if(i!=r)d[i][s]=finite(-d[i][s]*inv);d[r][s]=inv;std::swap(basic[r],nonbasic[s]);}
    bool simplex(bool phaseOne){const auto objective=phaseOne?m+1:m;for(;;){std::optional<std::size_t> s;for(std::size_t j=0;j<=n;++j)if((phaseOne || nonbasic[j]!=-1) && d[objective][j]<-1e-10 && (!s || nonbasic[j]<nonbasic[*s]))s=j;if(!s)return true;std::optional<std::size_t> r;for(std::size_t i=0;i<m;++i)if(d[i][*s]>1e-10){if(!r || d[i][n+1]/d[i][*s]<d[*r][n+1]/d[*r][*s]-1e-10 || (std::abs(d[i][n+1]/d[i][*s]-d[*r][n+1]/d[*r][*s])<=1e-10 && basic[i]<basic[*r]))r=i;}if(!r)return false;pivot(*r,*s);}}
    lp::SimplexStatus solve(std::vector<double>& values){std::size_t r=0;for(std::size_t i=1;i<m;++i)if(d[i][n+1]<d[r][n+1])r=i;if(m && d[r][n+1]<-1e-10){pivot(r,n);if(!simplex(true) || d[m+1][n+1]<-1e-9 || std::abs(d[m+1][n+1])>1e-9)return lp::SimplexStatus::Infeasible;for(std::size_t i=0;i<m;++i)if(basic[i]==-1){std::optional<std::size_t> s;for(std::size_t j=0;j<=n;++j)if(std::abs(d[i][j])>1e-10 && (!s || nonbasic[j]<nonbasic[*s]))s=j;if(s)pivot(i,*s);}}
        if(!simplex(false))return lp::SimplexStatus::Unbounded;
        values.assign(n,0);for(std::size_t i=0;i<m;++i)if(basic[i]>=0 && static_cast<std::size_t>(basic[i])<n)values[static_cast<std::size_t>(basic[i])]=d[i][n+1];return lp::SimplexStatus::Optimal;}
};
Relaxation relax(const lp::LpProblem& p){
    struct Map{double shift;std::size_t positive;std::optional<std::size_t> negative;bool fixed=false;};std::vector<Map> maps;lp::LpProblem q;q.sense=p.sense;q.title=p.title;
    for(std::size_t j=0;j<p.variables.size();++j){const auto& v=p.variables[j];Map map{v.unrestricted?0:v.lowerBound,q.variables.size(),{},!v.unrestricted && v.lowerBound==v.upperBound};if(map.fixed){maps.push_back(map);continue;}q.variables.push_back({});q.objective.push_back(p.objective[j]);if(v.unrestricted){map.negative=q.variables.size();q.variables.push_back({});q.objective.push_back(-p.objective[j]);}maps.push_back(map);}
    for(const auto& row:p.constraints){lp::Constraint r;r.rhs=row.rhs;r.relation=row.relation;r.coefficients.resize(q.variables.size());for(std::size_t j=0;j<maps.size();++j){r.rhs=finite(r.rhs-finite(row.coefficients[j]*maps[j].shift));if(!maps[j].fixed)r.coefficients[maps[j].positive]=row.coefficients[j];if(maps[j].negative)r.coefficients[*maps[j].negative]=-row.coefficients[j];}q.constraints.push_back(std::move(r));}
    for(std::size_t j=0;j<maps.size();++j)if(!maps[j].fixed && !p.variables[j].unrestricted && std::isfinite(p.variables[j].upperBound)){lp::Constraint row;row.coefficients.assign(q.variables.size(),0);row.coefficients[maps[j].positive]=1;row.rhs=finite(p.variables[j].upperBound-p.variables[j].lowerBound);q.constraints.push_back(std::move(row));}
    if(q.variables.empty()){Relaxation r;r.status=lp::SimplexStatus::Optimal;for(const auto& row:q.constraints)if(row.relation==lp::Relation::LessEqual?row.rhs<-1e-10:row.relation==lp::Relation::GreaterEqual?row.rhs>1e-10:std::abs(row.rhs)>1e-10){r.status=lp::SimplexStatus::Infeasible;return r;}for(std::size_t j=0;j<maps.size();++j){r.values.push_back(maps[j].shift);r.objective=finite(r.objective+finite(p.objective[j]*maps[j].shift));}return r;}
    std::vector<std::vector<double>> a;std::vector<double> b,c=q.objective;for(auto& cost:c)if(q.sense==lp::ObjectiveSense::Minimize)cost=-cost;
    for(const auto& row:q.constraints){if(row.relation!=lp::Relation::GreaterEqual){a.push_back(row.coefficients);b.push_back(row.rhs);}if(row.relation!=lp::Relation::LessEqual){auto coeff=row.coefficients;for(auto& x:coeff)x=-x;a.push_back(std::move(coeff));b.push_back(-row.rhs);}}
    NodeSimplex session(a,b,c);std::vector<double> values;Relaxation r;r.status=session.solve(values);if(r.status!=lp::SimplexStatus::Optimal)return r;
    for(std::size_t j=0;j<maps.size();++j){double x=maps[j].fixed?maps[j].shift:finite(maps[j].shift+values[maps[j].positive]);if(maps[j].negative)x=finite(x-values[*maps[j].negative]);r.values.push_back(x);r.objective=finite(r.objective+finite(p.objective[j]*x));}return r;
}
bool feasible(const lp::LpProblem& p,const std::vector<double>& x,double tol){for(std::size_t j=0;j<x.size();++j){const auto& v=p.variables[j];if(!v.unrestricted && (x[j]<v.lowerBound-tol || x[j]>v.upperBound+tol))return false;}for(const auto& r:p.constraints){double lhs=0;for(std::size_t j=0;j<x.size();++j)lhs=finite(lhs+finite(r.coefficients[j]*x[j]));const double margin=tol*(1+std::max(std::abs(lhs),std::abs(r.rhs)));if(r.relation==lp::Relation::LessEqual?lhs>r.rhs+margin:r.relation==lp::Relation::GreaterEqual?lhs<r.rhs-margin:std::abs(lhs-r.rhs)>margin)return false;}return true;}
}
Session::Session(Problem problem,std::size_t limit,double tol):problem_(std::move(problem)),limit_(limit),tolerance_(tol){
 const auto validation=lp::LpValidator::validate(problem_.lp);
 if(!validation.valid())throw std::invalid_argument(validation.issues.front().message);
 if(problem_.integral.size()!=problem_.lp.variables.size() || !limit || limit>100000 || !std::isfinite(tol) || tol<=0 || tol>=.1)throw std::invalid_argument("Invalid Integer flags/node limit/tolerance");
 pending_.push_back({problem_.lp,{},0});
}
std::vector<PendingNode> Session::pending()const{
 std::vector<PendingNode> out;for(const auto& p:pending_){PendingNode n;n.incoming=p.incoming;n.parent=p.parent;n.depth=p.depth;for(const auto& v:p.p.variables){n.lower.push_back(v.lowerBound);n.upper.push_back(v.upperBound);}out.push_back(std::move(n));}return out;
}
void Session::finishIfExhausted(){if(!awaiting_ && pending_.empty()){finished_=true;result_.status=result_.objective?Status::Optimal:Status::Infeasible;}}
void Session::next(std::optional<std::size_t> selected){
 if(finished_)throw std::logic_error("Integer search is already complete");
 if(awaiting_)throw std::logic_error("Choose a branching variable before solving another subproblem");
 if(result_.nodes.size()>=limit_){finished_=true;result_.status=Status::NodeLimit;return;}
 if(pending_.empty()){finishIfExhausted();return;}
 const auto index=selected.value_or(pending_.size()-1);if(index>=pending_.size())throw std::out_of_range("Invalid pending subproblem");
 Pending current=pending_[index];
 const auto commit=[&]{current_=std::move(current);pending_.erase(pending_.begin()+static_cast<std::ptrdiff_t>(index));};
 const auto& p=current.p;const auto solved=relax(p);Node node;node.incoming=current.incoming;node.parent=current.parent;node.depth=current.depth;
 for(const auto& v:p.variables){node.lower.push_back(v.lowerBound);node.upper.push_back(v.upperBound);}
 if(solved.status==lp::SimplexStatus::Infeasible){commit();result_.nodes.push_back(std::move(node));current_.reset();finishIfExhausted();return;}
 if(solved.status==lp::SimplexStatus::Unbounded){commit();node.state=NodeState::Unbounded;result_.nodes.push_back(std::move(node));finished_=true;result_.status=Status::UnboundedRelaxation;current_.reset();return;}
 if(!feasible(p,solved.values,tolerance_))throw std::runtime_error("Integer LP relaxation returned infeasible values");
 node.bound=solved.objective;node.values=solved.values;
 const double sign=problem_.lp.sense==lp::ObjectiveSense::Minimize?1:-1,normalized=sign*solved.objective;
 if(pruning_ && incumbent_ && normalized>=*incumbent_-tolerance_*(1+std::abs(*incumbent_))){commit();node.state=NodeState::BoundPruned;result_.nodes.push_back(std::move(node));current_.reset();finishIfExhausted();return;}
 bool fractional=false;auto rounded=solved.values;
 for(std::size_t j=0;j<problem_.integral.size();++j)if(problem_.integral[j]){const double nearest=std::round(rounded[j]);if(std::abs(rounded[j]-nearest)>tolerance_)fractional=true;else rounded[j]=nearest;}
 if(!fractional){
  if(!feasible(p,rounded,tolerance_))throw std::runtime_error("Rounded integer candidate violates constraints");
  double value=0;for(std::size_t j=0;j<rounded.size();++j)value=finite(value+finite(problem_.lp.objective[j]*rounded[j]));
  commit();
  if(!incumbent_ || sign*value<*incumbent_-tolerance_*(1+std::abs(*incumbent_))){incumbent_=sign*value;result_.objective=value;result_.values=rounded;}
  node.state=NodeState::IntegerSolution;result_.nodes.push_back(std::move(node));current_.reset();finishIfExhausted();return;
 }
 commit();node.state=NodeState::AwaitingBranch;awaiting_=result_.nodes.size();result_.nodes.push_back(std::move(node));
}
void Session::branch(std::size_t j){
 if(finished_ || !awaiting_ || !current_)throw std::logic_error("No fractional subproblem is waiting for a branch");
 auto& node=result_.nodes[*awaiting_];
 if(j>=problem_.integral.size() || !problem_.integral[j])throw std::invalid_argument("Choose a variable marked integer");
 const double x=node.values[j],floor=std::floor(x),ceil=std::ceil(x);
 if(std::abs(x-std::round(x))<=tolerance_)throw std::invalid_argument("Chosen variable is already integer");
 auto high=current_->p,low=current_->p;const auto parent=*awaiting_,depth=current_->depth+1;
 if(current_->p.variables[j].unrestricted){
  if(current_->p.constraints.size()>=500){finished_=true;result_.status=Status::ModelLimit;return;}
  lp::Constraint row;row.coefficients.assign(problem_.lp.variables.size(),0);row.coefficients[j]=1;
  row.relation=lp::Relation::GreaterEqual;row.rhs=ceil;high.constraints.push_back(row);
  row.relation=lp::Relation::LessEqual;row.rhs=floor;low.constraints.push_back(row);
  pending_.push_back({std::move(high),parent,depth,BranchConstraint{j,lp::Relation::GreaterEqual,ceil}});pending_.push_back({std::move(low),parent,depth,BranchConstraint{j,lp::Relation::LessEqual,floor}});
 }else{
  high.variables[j].lowerBound=std::max(high.variables[j].lowerBound,ceil);
  low.variables[j].upperBound=std::min(low.variables[j].upperBound,floor);
  if(high.variables[j].lowerBound<=high.variables[j].upperBound)pending_.push_back({std::move(high),parent,depth,BranchConstraint{j,lp::Relation::GreaterEqual,ceil}});
  if(low.variables[j].lowerBound<=low.variables[j].upperBound)pending_.push_back({std::move(low),parent,depth,BranchConstraint{j,lp::Relation::LessEqual,floor}});
 }
 node.state=NodeState::Branched;node.branchVariable=j;node.branchValue=x;awaiting_.reset();current_.reset();finishIfExhausted();
}
Result solve(const Problem& problem,std::size_t limit,double tol){
 Session session(problem,limit,tol);
 while(!session.finished()){
  if(const auto index=session.awaitingBranch()){
   const auto& point=session.result().nodes[*index].values;std::size_t j=0;
   while(j<point.size() && (!problem.integral[j] || std::abs(point[j]-std::round(point[j]))<=tol))++j;
   session.branch(j);
  }else session.next();
 }
 return session.result();
}
}
