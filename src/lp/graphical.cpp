#include <tora/lp/graphical.hpp>
#include <tora/lp/bounded.hpp>
#include <tora/lp/validation.hpp>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace tora::lp {
namespace {
Scalar roundedInteger(Scalar value){const auto lower=std::floor(value),fraction=value-lower;return fraction<.5?lower:fraction>.5?lower+1:std::fmod(lower,2)==0?lower:lower+1;}
}
bool GraphicalSession::eligible(const LpProblem& p){return p.variables.size()==2;}
bool GraphicalSession::boundsSupported(const LpProblem& p){return std::all_of(p.variables.begin(),p.variables.end(),[](const Variable& v){return !v.unrestricted && v.lowerBound==0 && std::isinf(v.upperBound) && v.upperBound>0;});}
const char* GraphicalSession::boundsError(){return "LPs with unrestricted vars and/or explicit upper/lower bounds are not supported by graphical method. Click OK then click screen.";}
GraphicalSession::GraphicalSession(LpProblem p,core::NumberFormat format):problem_(std::move(p)),format_(format),visible_(problem_.constraints.size(),false){
    (void)core::NumberFormatter(format_);
    if(!eligible(problem_))throw std::invalid_argument("Graphical solution requires two variables");
    if(!boundsSupported(problem_))throw std::invalid_argument(boundsError());
    const auto validation=LpValidator::validate(problem_);
    if(!validation.valid())throw std::invalid_argument(validation.issues.front().message);
    Scalar largest=0;
    for(const auto& row:problem_.constraints)for(auto a:row.coefficients)if(a!=0){const auto intercept=row.rhs/a;if(std::isfinite(intercept))largest=std::max(largest,intercept);}
    // Observed integral graph coordinates use ties-to-even conversion.
    // A zero converted intercept falls back to the original default range.
    largest=roundedInteger(largest);if(largest==0)largest=5;
    axisUnit_=std::max(Scalar(1),std::round(largest/10));
    initialMaximum_=std::max(largest+1,std::floor(largest*1.1));
    if(initialMaximum_>1e12)throw std::overflow_error("Graphical viewport exceeds the supported finite coordinate range");
    maximum_=initialMaximum_;
    gridStep_=axisUnit_*(largest<20?.15:largest<30?.2:largest<40?.25:largest<60?.3:largest<80?.35:largest<100?.4:.5);

}
Scalar GraphicalSession::tickStep() const {
    const auto digits=std::to_string(static_cast<unsigned long long>(maximum_)).size();
    // An observed label-capacity profile; original internal tick code unknown.
    return axisUnit_*std::max(Scalar(1),std::ceil((maximum_-rangeMin())/axisUnit_*(static_cast<Scalar>(digits)+2)/60));
}
void GraphicalSession::selectConstraint(std::size_t i){if(i>=visible_.size())throw std::out_of_range("Graph constraint index");visible_[i]=true;notice_.clear();}
bool GraphicalSession::drawObjective(){
    if(!std::all_of(visible_.begin(),visible_.end(),[](bool b){return b;})){notice_="Must graph all constraints prior to objective function";return false;}
    solve();objectiveVisible_=true;notice_.clear();return true;
}
void GraphicalSession::drawAll(){std::fill(visible_.begin(),visible_.end(),true);(void)drawObjective();}
void GraphicalSession::resetSignRestrictions(){std::fill(visible_.begin(),visible_.end(),false);objectiveVisible_=false;result_={};notice_.clear();maximum_=initialMaximum_;}
bool GraphicalSession::zoom(int direction){
    if(!std::any_of(visible_.begin(),visible_.end(),[](bool b){return b;}) || direction==0)return false;
    const auto next=roundedInteger(maximum_*(direction>0?.8:1.25));
    if(next<3 || next==maximum_ || next>1e12 || (next-rangeMin())/gridStep_>2048)return false;
    maximum_=next;return true;
}
void GraphicalSession::solve(){
    BoundedSession solver(problem_);solver.solveAll();result_={};
    if(solver.status()==SimplexStatus::Infeasible){result_.message="Problem has no feasible solution";result_.markerLabel=result_.message;return;}
    if(solver.status()==SimplexStatus::Unbounded){result_.message="Optimum solution is unbounded";result_.markerLabel="Optimum solution is UNBOUNDED";return;}
    const auto& t=solver.history().back().tableau;std::array<Scalar,2> values{};
    for(std::size_t j=0;j<2;++j){
        Scalar shifted=0;for(std::size_t i=0;i<t.basis().size();++i)if(t.basis()[i]==j)shifted=t.rhs(i+1);
        const auto& v=problem_.variables[j];const auto& name=t.columns()[j];
        if(v.unrestricted)values[j]=name.back()=='-'?-shifted:shifted;
        else values[j]=name.front()=='U'?v.upperBound-shifted:v.lowerBound+shifted;
    }
    result_.point={values[0],values[1]};result_.objectiveValue=t.rhs(0);
    const core::NumberFormatter f(format_);result_.objective=f.format(result_.objectiveValue);result_.x1=f.format(values[0]);result_.x2=f.format(values[1]);result_.markerLabel="Optimum";
}
bool GraphicalSession::contains(GraphPoint point) const {
    if(point.x<0 || point.y<0)return false;
    for(std::size_t i=0;i<visible_.size();++i)if(visible_[i]){
        const auto& row=problem_.constraints[i];const auto lhs=row.coefficients[0]*point.x+row.coefficients[1]*point.y;
        if(row.relation==Relation::Equal || (row.relation==Relation::LessEqual?lhs>row.rhs:lhs<row.rhs))return false;
    }
    return true;
}
std::optional<GraphSegment> GraphicalSession::segment(Scalar a,Scalar b,Scalar rhs) const {
    std::vector<GraphPoint> points;
    const auto add=[&](GraphPoint p){if(!std::isfinite(p.x)||!std::isfinite(p.y)||p.x<rangeMin()||p.y<rangeMin()||p.x>maximum_||p.y>maximum_)return;for(auto q:points)if(std::abs(p.x-q.x)<1e-12 && std::abs(p.y-q.y)<1e-12)return;points.push_back(p);};
    if(b!=0){add({rangeMin(),(rhs-a*rangeMin())/b});add({maximum_,(rhs-a*maximum_)/b});}
    if(a!=0){add({(rhs-b*rangeMin())/a,rangeMin()});add({(rhs-b*maximum_)/a,maximum_});}
    if(points.size()<2)return {};
    return GraphSegment{points[0],points[1]};
}
std::optional<GraphSegment> GraphicalSession::constraintSegment(std::size_t i) const {const auto& r=problem_.constraints.at(i);return segment(r.coefficients[0],r.coefficients[1],r.rhs);}
std::optional<GraphSegment> GraphicalSession::objectiveSegment() const {if(!objectiveVisible_ || !result_.message.empty())return {};return segment(problem_.objective[0],problem_.objective[1],result_.objectiveValue);}
std::vector<std::string> GraphicalSession::expressionLines() const {
    const core::NumberFormatter f(format_);
    const auto expression=[&](const std::vector<Scalar>& c){return f.format(c[0])+"x1+"+f.format(c[1])+"x2";};
    std::vector<std::string> lines{problem_.title,(problem_.sense==ObjectiveSense::Maximize?"Maximize":"Minimize")+std::string(" z = ")+expression(problem_.objective),"subject to"};
    for(std::size_t i=0;i<problem_.constraints.size();++i){const auto& r=problem_.constraints[i];lines.push_back("( "+std::to_string(i+1)+")  "+expression(r.coefficients)+" "+(r.relation==Relation::LessEqual?"<=":r.relation==Relation::GreaterEqual?">=":"=")+" "+f.format(r.rhs));}
    lines.push_back("all xj >= 0");lines.push_back("Click here to graph LP in one stroke");return lines;
}
} // namespace tora::lp
