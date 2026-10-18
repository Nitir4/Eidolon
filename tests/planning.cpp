#include <tora/planning/planning.hpp>
#include <cmath>
#include <stdexcept>
#include <functional>
#include <random>
#include <algorithm>
#include <limits>
#include <map>
using namespace tora::planning;
void checked(bool ok,int line){if(!ok)throw std::runtime_error("Planning check failed line "+std::to_string(line));}
#define check(value) checked(value,__LINE__)
void near(double a,double b){check(std::abs(a-b)<1e-10*(1+std::abs(a)));}
template<class F>void rejects(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught);}
int main(){Problem p; p.activities={{1,2,"A",3},{1,3,"B",2},{2,4,"C",5},{3,4,"D",1}};auto r=solve(p);near(r.duration,8);check(r.criticalPath==std::vector<std::size_t>({0,2}));near(r.activities[1].totalFloat,5);near(r.activities[1].freeFloat,0);near(r.activities[3].freeFloat,5);near(r.activities[0].latestStart,0);check(r.activities[0].critical&&!r.activities[1].critical);near(approximateDeadlineProbability(r,8),1);near(approximateDeadlineProbability(r,7),0);
 p.method=Method::Pert;p.activities={{1,2,"A",0,1,3,5}};r=solve(p);near(r.duration,3);near(r.pathVariance,4./9);near(approximateDeadlineProbability(r,3),.5);near(approximateDeadlineProbability(r,3+2./3),.8413447460685429);
 Problem disconnected;disconnected.activities={{1,3,"fast",2},{10,20,"slow",5}};r=solve(disconnected);near(r.duration,5);near(r.activities[0].totalFloat,3);check(r.criticalPath==std::vector<std::size_t>({1}));Problem reverseLabels;reverseLabels.activities={{2147483647,1,"A",3},{1,2,"B",2}};r=solve(reverseLabels);near(r.duration,5);check(r.criticalPath==std::vector<std::size_t>({0,1}));
 Problem tie;tie.method=Method::Pert;tie.activities={{1,2,"A",0,2,2,2},{1,3,"B",0,0,2,4},{2,4,"C",0,0,0,0},{3,4,"D",0,0,0,0}};r=solve(tie);check(r.criticalPath==std::vector<std::size_t>({1,3}));near(r.pathVariance,4./9);
 rejects([]{solve({});});rejects([]{Problem q;q.activities={{1,2,"a",1},{2,1,"b",1}};solve(q);});rejects([]{Problem q;q.activities={{1,1,"",1}};solve(q);});rejects([]{Problem q;q.activities={{1,2,"",-1}};solve(q);});rejects([]{Problem q;q.method=Method::Pert;q.activities={{1,2,"",0,3,2,4}};solve(q);});rejects([]{Problem q;q.activities={{1,2,"",std::numeric_limits<double>::max()},{2,3,"",std::numeric_limits<double>::max()}};solve(q);});

 // Observed original CPM schedule experiment, extended to every successor.
 Problem experiment;experiment.activities={{1,2,"A",3},{2,4,"B",5},{1,3,"C",2},{3,4,"D",2}};
 auto scheduled=schedule(experiment,{0,0,3,0});near(scheduled.starts[2],3);near(scheduled.finishes[2],5);near(scheduled.starts[3],5);near(scheduled.completion,8);
 rejects([&]{schedule(experiment,{0,0,5,0});});rejects([&]{schedule(experiment,{1,0,0,0});});rejects([&]{schedule(experiment,{0,0,-1,0});});rejects([&]{schedule(experiment,{0,0,std::numeric_limits<double>::infinity(),0});});rejects([&]{schedule(experiment,{});});
 Problem transitive;transitive.activities={{1,2,"critical",10},{1,3,"C",1},{3,4,"D",1},{4,2,"E",1}};
 scheduled=schedule(transitive,{0,6,0,0});near(scheduled.starts[1],6);near(scheduled.starts[2],7);near(scheduled.starts[3],8);near(scheduled.completion,10);
 scheduled=schedule(transitive,{0,0,0,0});near(scheduled.starts[1],0);near(scheduled.starts[2],1);near(scheduled.starts[3],2);
 std::mt19937 rng(42);for(int trial=0;trial<300;++trial){Problem q;const int n=2+static_cast<int>(rng()%7);for(int i=1;i<n;++i)q.activities.push_back({i,i+1,"",static_cast<double>(rng()%8)});for(int i=1;i<n;++i)for(int j=i+2;j<=n;++j)if(rng()%3==0)q.activities.push_back({i,j,"",static_cast<double>(rng()%12)});auto result=solve(q);check(result.trace.size()==2*q.activities.size());std::map<int,double> early,late;for(const auto& e:result.events){early[e.node]=0;late[e.node]=result.duration;}for(const auto& t:result.trace){const auto& a=q.activities[t.activity];if(t.pass==Pass::Forward){near(t.previous,early[a.to]);near(t.candidate,early[a.from]+a.duration);early[a.to]=std::max(early[a.to],t.candidate);near(t.current,early[a.to]);}else{near(t.previous,late[a.from]);near(t.candidate,late[a.to]-a.duration);late[a.from]=std::min(late[a.from],t.candidate);near(t.current,late[a.from]);}}for(const auto& e:result.events){near(e.earliest,early[e.node]);near(e.latest,late[e.node]);}double longest=0;std::function<void(int,double)> walk=[&](int node,double length){if(node==n){longest=std::max(longest,length);return;}for(const auto& a:q.activities)if(a.from==node)walk(a.to,length+a.duration);};walk(1,0);near(result.duration,longest);double path=0;int cursor=1;for(auto i:result.criticalPath){check(q.activities[i].from==cursor);cursor=q.activities[i].to;check(result.activities[i].critical);path+=q.activities[i].duration;}near(path,longest);for(std::size_t i=0;i<q.activities.size();++i){auto a=result.activities[i];check(a.totalFloat>=0&&a.freeFloat>=0);near(a.latestStart+a.mean,a.latestFinish);near(a.earliestStart+a.mean,a.earliestFinish);near(a.latestStart-a.earliestStart,a.totalFloat);}std::vector<double> delays;for(const auto& a:result.activities)delays.push_back(a.totalFloat*(rng()%101)/100.);auto shifted=schedule(q,delays);check(shifted.completion<=result.duration+1e-9);for(std::size_t i=0;i<q.activities.size();++i){check(shifted.starts[i]>=result.activities[i].earliestStart+delays[i]-1e-9);check(shifted.starts[i]<=result.activities[i].latestStart+1e-9);near(shifted.finishes[i]-shifted.starts[i],q.activities[i].duration);for(std::size_t j=0;j<q.activities.size();++j)if(q.activities[j].to==q.activities[i].from)check(shifted.starts[i]>=shifted.finishes[j]-1e-9);}}
}
