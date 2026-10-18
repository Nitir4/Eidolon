#include <tora/networks/networks.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <queue>
#include <stdexcept>
namespace tora::networks {
namespace {
constexpr auto missing=std::numeric_limits<std::size_t>::max();
using Wide=long double;
core::Scalar output(Wide value){
 if(!std::isfinite(value)||std::abs(value)>static_cast<Wide>(std::numeric_limits<core::Scalar>::max()))
  throw std::overflow_error("Network result exceeds the finite scalar range.");
 return static_cast<core::Scalar>(value);
}
Wide add(Wide a,Wide b){const auto v=a+b;if(!std::isfinite(v))throw std::overflow_error("Network arithmetic exceeds the working range.");return v;}
void node(const Graph& g,std::size_t n){if(n>=g.nodeCount)throw std::invalid_argument("Network node is outside the graph.");}
class DisjointSets {
 std::vector<std::size_t> parent_;
public:
 explicit DisjointSets(std::size_t n):parent_(n){std::iota(parent_.begin(),parent_.end(),0);}
 std::size_t root(std::size_t i){while(parent_[i]!=i){parent_[i]=parent_[parent_[i]];i=parent_[i];}return i;}
 bool join(std::size_t a,std::size_t b){a=root(a);b=root(b);if(a==b)return false;parent_[std::max(a,b)]=std::min(a,b);return true;}
};
struct Arc {std::size_t from,to,index;Wide value;};
std::vector<Arc> arcs(const Graph& g){std::vector<Arc> out;for(std::size_t i=0;i<g.edges.size();++i){const auto& e=g.edges[i];out.push_back({e.from,e.to,i,e.value});if(!g.directed&&e.from!=e.to)out.push_back({e.to,e.from,i,e.value});}return out;}
bool close(Wide a,Wide b,Wide scale){return std::abs(a-b)<=1e-12L*scale;}
}
void validate(const Graph& g){
 if(!g.nodeCount||g.nodeCount>100||g.edges.size()>10000)throw std::invalid_argument("Networks support 1 to 100 nodes and at most 10000 edges.");
 for(const auto& e:g.edges){node(g,e.from);node(g,e.to);if(!std::isfinite(e.value))throw std::invalid_argument("Network edges must have finite values.");}
}
SpanningForest minimumSpanningForest(const Graph& g){
 validate(g);if(g.directed)throw std::invalid_argument("A spanning forest requires an undirected graph.");
 std::vector<std::size_t> order(g.edges.size());std::iota(order.begin(),order.end(),0);
 std::stable_sort(order.begin(),order.end(),[&](auto a,auto b){return g.edges[a].value<g.edges[b].value;});
 DisjointSets sets(g.nodeCount);SpanningForest result;Wide total=0;
 for(auto i:order){const auto& e=g.edges[i];if(sets.join(e.from,e.to)){result.edgeIndices.push_back(i);total=add(total,e.value);}}
 result.component.resize(g.nodeCount);std::vector<std::size_t> roots;
 for(std::size_t i=0;i<g.nodeCount;++i){const auto root=sets.root(i);auto it=std::find(roots.begin(),roots.end(),root);if(it==roots.end()){roots.push_back(root);it=roots.end()-1;}result.component[i]=static_cast<std::size_t>(it-roots.begin());}
 result.componentCount=roots.size();result.connected=roots.size()==1;result.totalWeight=output(total);
 // Cycle certificate: no input edge improves its unique forest path.
 std::vector<std::vector<std::pair<std::size_t,Wide>>> tree(g.nodeCount);
 for(auto i:result.edgeIndices){const auto& e=g.edges[i];tree[e.from].push_back({e.to,e.value});tree[e.to].push_back({e.from,e.value});}
 for(const auto& e:g.edges){if(e.from==e.to)continue;std::vector<bool> seen(g.nodeCount);std::vector<Wide> maximum(g.nodeCount,-std::numeric_limits<Wide>::infinity());std::queue<std::size_t> q;q.push(e.from);seen[e.from]=true;while(!q.empty()&&!seen[e.to]){auto u=q.front();q.pop();for(auto [v,w]:tree[u])if(!seen[v]){seen[v]=true;maximum[v]=std::max(maximum[u],w);q.push(v);}}if(!seen[e.to]||maximum[e.to]>e.value)throw std::runtime_error("Minimum spanning forest certificate failed.");}
 return result;
}
SpanningForest minimumSpanningForestPrim(const Graph& g,std::size_t start){
 auto result=minimumSpanningForest(g);node(g,start);std::vector<bool> selected(g.nodeCount);result.edgeIndices.clear();selected[start]=true;std::size_t count=1;Wide total=0;
 while(count<g.nodeCount){std::size_t best=missing;for(std::size_t i=0;i<g.edges.size();++i){const auto& e=g.edges[i];if(selected[e.from]==selected[e.to])continue;if(best==missing||e.value<g.edges[best].value)best=i;}if(best==missing){auto it=std::find(selected.begin(),selected.end(),false);selected[static_cast<std::size_t>(it-selected.begin())]=true;++count;continue;}const auto& e=g.edges[best];selected[e.from]=selected[e.to]=true;++count;result.edgeIndices.push_back(best);total=add(total,e.value);}
 if(!close(total,result.totalWeight,std::max(std::abs(total),std::abs(static_cast<Wide>(result.totalWeight)))))throw std::runtime_error("Prim/Kruskal certificate failed.");
 return result;
}

ShortestRoutes shortestRoutes(const Graph& g,std::size_t source){
 validate(g);node(g,source);const auto edges=arcs(g);const auto n=g.nodeCount;
 std::vector<Wide> distance(n,std::numeric_limits<Wide>::infinity());std::vector<std::size_t> parent(n,missing),parentEdge(n,missing);distance[source]=0;
 ShortestRoutes result;result.source=source;result.routes.resize(n);
 for(std::size_t pass=1;pass<n;++pass){bool changed=false;for(const auto& e:edges)if(std::isfinite(distance[e.from])){const auto d=add(distance[e.from],e.value);if(d<distance[e.to]){distance[e.to]=d;parent[e.to]=e.from;parentEdge[e.to]=e.index;if(result.trace.size()<10000&&std::abs(d)<=std::numeric_limits<core::Scalar>::max())result.trace.push_back({pass,e.from,e.to,e.index,output(d)});else result.traceTruncated=true;changed=true;}}if(!changed)break;}
 std::vector<bool> affected(n);std::queue<std::size_t> queue;std::size_t witness=missing;
 for(const auto& e:edges)if(std::isfinite(distance[e.from])){const auto d=add(distance[e.from],e.value);if(d<distance[e.to]){distance[e.to]=d;parent[e.to]=e.from;parentEdge[e.to]=e.index;witness=e.to;if(!affected[e.to]){affected[e.to]=true;queue.push(e.to);}}}
 std::vector<std::vector<std::size_t>> outgoing(n);for(const auto& e:edges)outgoing[e.from].push_back(e.to);
 while(!queue.empty()){auto u=queue.front();queue.pop();for(auto v:outgoing[u])if(!affected[v]){affected[v]=true;queue.push(v);}}
 if(witness!=missing){auto u=witness;for(std::size_t i=0;i<n;++i){if(parent[u]==missing)throw std::runtime_error("Negative cycle predecessor certificate failed.");u=parent[u];}const auto start=u;do{result.negativeCycleNodes.push_back(u);result.negativeCycleEdges.push_back(parentEdge[u]);u=parent[u];if(result.negativeCycleEdges.size()>n)throw std::runtime_error("Negative cycle witness failed.");}while(u!=start);result.negativeCycleNodes.push_back(start);std::reverse(result.negativeCycleNodes.begin(),result.negativeCycleNodes.end());std::reverse(result.negativeCycleEdges.begin(),result.negativeCycleEdges.end());Wide weight=0;for(auto i:result.negativeCycleEdges)weight=add(weight,g.edges[i].value);if(!(weight<0))throw std::runtime_error("Negative cycle weight certificate failed.");}
 for(std::size_t target=0;target<n;++target){auto& route=result.routes[target];if(affected[target]){route.status=RouteStatus::NegativeCycle;continue;}if(!std::isfinite(distance[target]))continue;route.status=RouteStatus::Finite;route.distance=output(distance[target]);auto u=target;route.nodes.push_back(u);while(u!=source){if(parent[u]==missing||route.edgeIndices.size()>=n)throw std::runtime_error("Shortest route predecessor certificate failed.");route.edgeIndices.push_back(parentEdge[u]);u=parent[u];route.nodes.push_back(u);}std::reverse(route.nodes.begin(),route.nodes.end());std::reverse(route.edgeIndices.begin(),route.edgeIndices.end());Wide weight=0;for(auto i:route.edgeIndices)weight=add(weight,g.edges[i].value);if(!close(weight,distance[target],std::max(std::abs(weight),std::abs(distance[target]))))throw std::runtime_error("Shortest route length certificate failed.");}
 // The final distances are feasible potentials on all finite routes.
 for(const auto& e:edges)if(!affected[e.from]&&!affected[e.to]&&std::isfinite(distance[e.from])&&distance[e.to]>add(distance[e.from],e.value))throw std::runtime_error("Shortest route optimality certificate failed.");
 return result;
}

ShortestRoutes dijkstra(const Graph& g,std::size_t source){
 validate(g);node(g,source);for(const auto& e:g.edges)if(e.value<0)throw std::invalid_argument("Dijkstra requires nonnegative edge values. Use Floyd or Bellman-Ford for negative edges.");
 const auto edges=arcs(g);std::vector<Wide> distance(g.nodeCount,std::numeric_limits<Wide>::infinity());distance[source]=0;
 std::vector<bool> permanent(g.nodeCount);std::vector<std::size_t> parent(g.nodeCount,missing),parentEdge(g.nodeCount,missing);std::vector<DijkstraIteration> trace;bool truncated=false;
 for(std::size_t count=0;count<g.nodeCount;++count){std::size_t u=missing;for(std::size_t i=0;i<g.nodeCount;++i)if(!permanent[i]&&std::isfinite(distance[i])&&(u==missing||distance[i]<distance[u]))u=i;if(u==missing)break;permanent[u]=true;
  for(const auto& e:edges)if(e.from==u&&!permanent[e.to]){const auto d=add(distance[u],e.value);if(d<distance[e.to]){distance[e.to]=d;parent[e.to]=u;parentEdge[e.to]=e.index;}}
  DijkstraIteration step;step.permanentNode=u;step.permanent=permanent;step.distance.resize(g.nodeCount);step.predecessor.resize(g.nodeCount);bool representable=true;for(std::size_t i=0;i<g.nodeCount;++i){if(std::isfinite(distance[i])){if(distance[i]>std::numeric_limits<core::Scalar>::max()){representable=false;break;}step.distance[i]=output(distance[i]);}if(parent[i]!=missing)step.predecessor[i]=parent[i];}if(representable)trace.push_back(std::move(step));else truncated=true;
 }
 auto result=shortestRoutes(g,source);result.trace.clear();result.traceTruncated=truncated;result.dijkstraTrace=std::move(trace);
 for(std::size_t i=0;i<g.nodeCount;++i){auto& route=result.routes[i];if(!std::isfinite(distance[i])){if(route.status!=RouteStatus::Unreachable)throw std::runtime_error("Dijkstra reachability certificate failed.");continue;}if(!route.distance||!close(distance[i],*route.distance,std::max(std::abs(distance[i]),std::abs(static_cast<Wide>(*route.distance)))))throw std::runtime_error("Dijkstra/Bellman-Ford distance certificate failed.");route.nodes={i};route.edgeIndices.clear();auto u=i;while(u!=source){if(parent[u]==missing||route.edgeIndices.size()>=g.nodeCount)throw std::runtime_error("Dijkstra predecessor certificate failed.");route.edgeIndices.push_back(parentEdge[u]);u=parent[u];route.nodes.push_back(u);}std::reverse(route.nodes.begin(),route.nodes.end());std::reverse(route.edgeIndices.begin(),route.edgeIndices.end());}
 return result;
}
AllPairsRoutes floydWarshall(const Graph& g,bool recordTrace){
 validate(g);const auto n=g.nodeCount;std::vector<Wide> distance(n*n,std::numeric_limits<Wide>::infinity());std::vector<std::size_t> next(n*n,missing),nextEdge(n*n,missing);for(std::size_t i=0;i<n;++i)distance[i*n+i]=0;
 for(const auto& e:arcs(g))if(e.value<distance[e.from*n+e.to]){distance[e.from*n+e.to]=e.value;next[e.from*n+e.to]=e.to;nextEdge[e.from*n+e.to]=e.index;}
 AllPairsRoutes result;auto snapshot=[&](std::size_t intermediate,bool initial){if(!recordTrace)return;FloydIteration step;step.intermediate=intermediate;step.initial=initial;step.distance.resize(n*n);step.next.resize(n*n);bool representable=true;for(std::size_t i=0;i<n*n;++i){if(next[i]!=missing)step.next[i]=next[i];if(std::isfinite(distance[i])){if(std::abs(distance[i])>std::numeric_limits<core::Scalar>::max()){representable=false;break;}step.distance[i]=output(distance[i]);}}if(representable)result.trace.push_back(std::move(step));else result.traceTruncated=true;};snapshot(0,true);
 for(std::size_t k=0;k<n;++k){for(std::size_t i=0;i<n;++i)for(std::size_t j=0;j<n;++j)if(std::isfinite(distance[i*n+k])&&std::isfinite(distance[k*n+j])){const auto value=add(distance[i*n+k],distance[k*n+j]);if(value<distance[i*n+j]){distance[i*n+j]=value;next[i*n+j]=next[i*n+k];nextEdge[i*n+j]=nextEdge[i*n+k];}}snapshot(k,false);}
 for(std::size_t source=0;source<n;++source){auto routes=shortestRoutes(g,source);routes.trace.clear();routes.traceTruncated=false;for(std::size_t target=0;target<n;++target){bool negative=false;for(std::size_t k=0;k<n;++k)negative|=distance[k*n+k]<0&&std::isfinite(distance[source*n+k])&&std::isfinite(distance[k*n+target]);auto& route=routes.routes[target];if(negative){if(route.status!=RouteStatus::NegativeCycle)throw std::runtime_error("Floyd negative-cycle certificate failed.");continue;}if(!std::isfinite(distance[source*n+target])){if(route.status!=RouteStatus::Unreachable)throw std::runtime_error("Floyd reachability certificate failed.");continue;}if(!route.distance||!close(distance[source*n+target],*route.distance,std::max(std::abs(distance[source*n+target]),std::abs(static_cast<Wide>(*route.distance)))))throw std::runtime_error("Floyd/Bellman-Ford distance certificate failed.");route.distance=output(distance[source*n+target]);route.nodes={source};route.edgeIndices.clear();auto u=source;Wide weight=0;while(u!=target){if(next[u*n+target]==missing||nextEdge[u*n+target]==missing||route.edgeIndices.size()>=n)throw std::runtime_error("Floyd path certificate failed.");const auto edge=nextEdge[u*n+target];route.edgeIndices.push_back(edge);weight=add(weight,g.edges[edge].value);u=next[u*n+target];route.nodes.push_back(u);}if(!close(weight,*route.distance,std::max(std::abs(weight),std::abs(static_cast<Wide>(*route.distance)))))throw std::runtime_error("Floyd path length certificate failed.");}result.sources.push_back(std::move(routes));}
 return result;
}
MaximumFlow maximumFlow(const Graph& g,std::size_t source,std::size_t sink,const FlowOptions& options){
 validate(g);node(g,source);node(g,sink);if(!g.directed||source==sink)throw std::invalid_argument("Maximum flow needs a directed graph and distinct source/sink.");if(!options.iterationLimit)throw std::invalid_argument("Maximum flow iteration limit must be positive.");
 struct Residual {std::size_t to,reverse,index;Wide capacity;bool backward;};
 std::vector<std::vector<Residual>> adjacency(g.nodeCount);std::vector<std::size_t> position(g.edges.size(),missing);
 for(std::size_t i=0;i<g.edges.size();++i){const auto& e=g.edges[i];if(e.value<0)throw std::invalid_argument("Flow capacities must be nonnegative.");if(e.from==e.to)continue;position[i]=adjacency[e.from].size();const auto reverse=adjacency[e.to].size();adjacency[e.from].push_back({e.to,reverse,i,e.value,false});adjacency[e.to].push_back({e.from,position[i],i,0,true});}
 MaximumFlow result;Wide total=0;
 for(;;){std::vector<std::size_t> parent(g.nodeCount,missing),chosen(g.nodeCount,missing);std::queue<std::size_t> q;q.push(source);parent[source]=source;while(!q.empty()&&parent[sink]==missing){auto u=q.front();q.pop();for(std::size_t i=0;i<adjacency[u].size();++i){const auto& e=adjacency[u][i];if(e.capacity>0&&parent[e.to]==missing){parent[e.to]=u;chosen[e.to]=i;q.push(e.to);}}}if(parent[sink]==missing){result.sourceSide.resize(g.nodeCount);for(std::size_t i=0;i<g.nodeCount;++i)result.sourceSide[i]=parent[i]!=missing;break;}
 if(result.augmentationCount>=options.iterationLimit)throw std::runtime_error("Maximum flow iteration limit exhausted.");
 Wide amount=std::numeric_limits<Wide>::infinity();for(auto u=sink;u!=source;u=parent[u])amount=std::min(amount,adjacency[parent[u]][chosen[u]].capacity);
 Augmentation step;if(options.recordTrace&&result.trace.size()<options.maximumTraceSteps){for(auto u=sink;u!=source;u=parent[u]){const auto& e=adjacency[parent[u]][chosen[u]];step.nodes.push_back(u);step.edgeIndices.push_back(e.index);step.backward.push_back(e.backward);}step.nodes.push_back(source);std::reverse(step.nodes.begin(),step.nodes.end());std::reverse(step.edgeIndices.begin(),step.edgeIndices.end());std::reverse(step.backward.begin(),step.backward.end());step.amount=output(amount);result.trace.push_back(std::move(step));}else if(options.recordTrace)result.traceTruncated=true;
 for(auto u=sink;u!=source;u=parent[u]){auto& e=adjacency[parent[u]][chosen[u]];auto& reverse=adjacency[e.to][e.reverse];e.capacity=e.capacity==amount?0:e.capacity-amount;reverse.capacity=add(reverse.capacity,amount);}total=add(total,amount);++result.augmentationCount;
 }
 std::vector<Wide> balance(g.nodeCount),magnitude(g.nodeCount);result.edgeFlow.resize(g.edges.size());Wide cut=0;
 for(std::size_t i=0;i<g.edges.size();++i){const auto& e=g.edges[i];Wide flow=0;if(position[i]!=missing){const auto& forward=adjacency[e.from][position[i]];flow=adjacency[e.to][forward.reverse].capacity;}if(flow<0||flow>e.value)throw std::runtime_error("Flow capacity certificate failed.");result.edgeFlow[i]=output(flow);balance[e.from]=add(balance[e.from],-flow);balance[e.to]=add(balance[e.to],flow);magnitude[e.from]=add(magnitude[e.from],flow);magnitude[e.to]=add(magnitude[e.to],flow);if(result.sourceSide[e.from]&&!result.sourceSide[e.to])cut=add(cut,e.value);}
 for(std::size_t u=0;u<g.nodeCount;++u){const auto expected=u==source?-total:u==sink?total:0;if(!close(balance[u],expected,std::max(magnitude[u],std::abs(expected))))throw std::runtime_error("Flow conservation certificate failed.");}
 if(!close(total,cut,std::max(std::abs(total),std::abs(cut))))throw std::runtime_error("Flow/minimum-cut certificate failed.");
 result.value=output(total);result.cutCapacity=output(cut);return result;
}
}
