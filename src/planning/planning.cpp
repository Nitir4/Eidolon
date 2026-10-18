#include <tora/planning/planning.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <queue>
#include <stdexcept>
namespace tora::planning {
namespace { double finite(long double x){if(!std::isfinite(x)||std::abs(x)>std::numeric_limits<double>::max())throw std::overflow_error("Planning calculation exceeds numeric range.");return static_cast<double>(x);} }
Result solve(const Problem& p){
 if(p.method!=Method::Cpm&&p.method!=Method::Pert)throw std::invalid_argument("Unknown planning method.");
 if(p.activities.empty()||p.activities.size()>10000)throw std::invalid_argument("Enter between 1 and 10000 activities.");
 std::map<int,std::size_t> indices;for(const auto& a:p.activities){if(a.from<=0||a.to<=0||a.from==a.to)throw std::invalid_argument("Activity endpoints must be distinct positive node numbers.");indices.emplace(a.from,0);indices.emplace(a.to,0);}
 if(indices.size()>10000)throw std::invalid_argument("At most 10000 events are supported.");
 Result r;r.activities.resize(p.activities.size());std::size_t n=0;for(auto& pair:indices){pair.second=n++;r.events.push_back({pair.first,0,0});}
 std::vector<std::vector<std::size_t>> out(n);std::vector<std::size_t> degree(n);std::vector<std::size_t> parent(n,p.activities.size());std::vector<long double> early(n),late(n),variance(n);
 for(std::size_t i=0;i<p.activities.size();++i){const auto& a=p.activities[i];double mean=a.duration,var=0;
  if(p.method==Method::Pert){if(!std::isfinite(a.optimistic)||!std::isfinite(a.mostLikely)||!std::isfinite(a.pessimistic)||a.optimistic<0||a.optimistic>a.mostLikely||a.mostLikely>a.pessimistic)throw std::invalid_argument("PERT estimates must satisfy 0 <= a <= m <= b and be finite.");mean=finite((static_cast<long double>(a.optimistic)+4.L*a.mostLikely+a.pessimistic)/6);const auto spread=(static_cast<long double>(a.pessimistic)-a.optimistic)/6;var=finite(spread*spread);}
  if(!std::isfinite(mean)||mean<0)throw std::invalid_argument("Activity durations must be finite and nonnegative.");
  r.activities[i].mean=mean;r.activities[i].variance=var;out[indices.at(a.from)].push_back(i);++degree[indices.at(a.to)];
 }
 std::priority_queue<std::size_t,std::vector<std::size_t>,std::greater<std::size_t>> ready;for(std::size_t i=0;i<n;++i)if(!degree[i])ready.push(i);std::vector<std::size_t> order;
 while(!ready.empty()){auto u=ready.top();ready.pop();order.push_back(u);r.topologicalOrder.push_back(r.events[u].node);for(auto i:out[u]){auto v=indices.at(p.activities[i].to);auto candidate=early[u]+r.activities[i].mean;auto vvar=variance[u]+r.activities[i].variance;const auto previous=early[v];if(candidate>early[v]||(candidate==early[v]&&(parent[v]==p.activities.size()||vvar>variance[v]))){early[v]=candidate;variance[v]=vvar;parent[v]=i;}r.trace.push_back({Pass::Forward,r.events[v].node,i,finite(previous),finite(candidate),finite(early[v])});if(!--degree[v])ready.push(v);}}
 if(order.size()!=n)throw std::invalid_argument("Planning network contains a directed cycle.");
 std::size_t terminal=n;for(std::size_t i=0;i<n;++i)if(out[i].empty()&&(terminal==n||early[i]>early[terminal]||(early[i]==early[terminal]&&variance[i]>variance[terminal])))terminal=i;
 const auto duration=early[terminal];r.duration=finite(duration);r.pathVariance=finite(variance[terminal]);std::fill(late.begin(),late.end(),duration);
 for(auto it=order.rbegin();it!=order.rend();++it)for(auto i:out[*it]){const auto previous=late[*it],candidate=late[indices.at(p.activities[i].to)]-r.activities[i].mean;late[*it]=std::min(previous,candidate);r.trace.push_back({Pass::Backward,r.events[*it].node,i,finite(previous),finite(candidate),finite(late[*it])});}
 const auto tolerance=64.L*std::numeric_limits<double>::epsilon()*std::max(1.L,duration)*static_cast<long double>(n);
 for(std::size_t u=0;u<n;++u){r.events[u].earliest=finite(early[u]);r.events[u].latest=finite(late[u]);}
 for(std::size_t i=0;i<p.activities.size();++i){auto u=indices.at(p.activities[i].from),v=indices.at(p.activities[i].to);auto& a=r.activities[i];a.earliestStart=finite(early[u]);a.earliestFinish=finite(early[u]+a.mean);a.latestFinish=finite(late[v]);a.latestStart=finite(late[v]-a.mean);a.totalFloat=finite(std::max(0.L,late[v]-early[u]-a.mean));a.freeFloat=finite(std::max(0.L,early[v]-early[u]-a.mean));a.critical=std::abs(late[v]-early[u]-a.mean)<=tolerance;}
 while(parent[terminal]!=p.activities.size()){auto i=parent[terminal];r.criticalPath.push_back(i);terminal=indices.at(p.activities[i].from);}std::reverse(r.criticalPath.begin(),r.criticalPath.end());return r;
}
Schedule schedule(const Problem& p,const std::vector<double>& delays){
 const auto solved=solve(p);
 if(delays.size()!=p.activities.size())throw std::invalid_argument("Supply one requested delay per activity.");
 const double tolerance=128*std::numeric_limits<double>::epsilon()*std::max(1.,solved.duration)*solved.events.size();
 std::map<int,std::vector<std::size_t>> outgoing;
 std::map<int,double> available;
 for(std::size_t i=0;i<p.activities.size();++i){
  if(!std::isfinite(delays[i])||delays[i]<0||delays[i]>solved.activities[i].totalFloat+tolerance)throw std::invalid_argument("Activity delay must lie between zero and its total float.");
  outgoing[p.activities[i].from].push_back(i);
 }
 Schedule result;result.starts.resize(delays.size());result.finishes.resize(delays.size());
 for(int node:solved.topologicalOrder)for(auto i:outgoing[node]){
  const auto& a=solved.activities[i];
  const auto start=std::max(available[node],finite(static_cast<long double>(a.earliestStart)+delays[i]));
  const auto finish=finite(static_cast<long double>(start)+a.mean);
  if(start>a.latestStart+tolerance||finish>solved.duration+tolerance)throw std::invalid_argument("Requested schedule exceeds activity total float or project completion time.");
  result.starts[i]=start;result.finishes[i]=finish;
  result.completion=std::max(result.completion,finish);
  available[p.activities[i].to]=std::max(available[p.activities[i].to],finish);
 }
 return result;
}
double approximateDeadlineProbability(const Result& r,double deadline){if(!std::isfinite(deadline)||!std::isfinite(r.duration)||!std::isfinite(r.pathVariance)||r.pathVariance<0)throw std::invalid_argument("Deadline and planning moments must be finite.");if(r.pathVariance==0)return deadline>=r.duration?1:0;return .5*std::erfc(static_cast<double>((static_cast<long double>(r.duration)-deadline)/std::sqrt(2.L*r.pathVariance)));}
}
