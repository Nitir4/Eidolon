#pragma once
#include <tora/core/matrix.hpp>
#include <optional>
#include <vector>
namespace tora::networks {
struct Edge {std::size_t from=0,to=0;core::Scalar value=0;};
struct Graph {
 std::size_t nodeCount=0;
 bool directed=true;
 std::vector<Edge> edges;
};
// Native bounded profile: 1..100 nodes, <=10000 finite-valued edges.
// Parallel edges and self-loops are explicit; indices retain input order.
void validate(const Graph& graph);
struct SpanningForest {
 bool connected=false;
 core::Scalar totalWeight=0;
 std::size_t componentCount=0;
 std::vector<std::size_t> component,edgeIndices;
};
// Undirected graphs only; a disconnected graph returns its minimum forest.
SpanningForest minimumSpanningForest(const Graph& graph);
// Prim edge order from a selected start, independently checked against Kruskal.
SpanningForest minimumSpanningForestPrim(const Graph& graph,std::size_t start);
enum class RouteStatus {Finite,Unreachable,NegativeCycle};
struct Route {
 RouteStatus status=RouteStatus::Unreachable;
 std::optional<core::Scalar> distance;
 std::vector<std::size_t> nodes,edgeIndices;
};
struct RouteRelaxation {
 std::size_t pass=0,from=0,to=0,edgeIndex=0;
 core::Scalar distance=0;
};
struct DijkstraIteration {
 std::size_t permanentNode=0;
 std::vector<std::optional<core::Scalar>> distance;
 std::vector<std::optional<std::size_t>> predecessor;
 std::vector<bool> permanent;
};
struct ShortestRoutes {
 std::size_t source=0;
 std::vector<Route> routes;
 // Actual Bellman-Ford relaxations; bounded storage does not stop solving.
 std::vector<RouteRelaxation> trace;
 std::vector<DijkstraIteration> dijkstraTrace;
 bool traceTruncated=false;
 // One reachable negative cycle, nodes.front()==nodes.back().
 std::vector<std::size_t> negativeCycleNodes,negativeCycleEdges;
};
// Bellman-Ford permits negative edges; only nodes reachable from a reachable
// negative cycle receive NegativeCycle status. Unreachable cycles do not matter.
ShortestRoutes shortestRoutes(const Graph& graph,std::size_t source);
// Nonnegative edges only; settles one reachable node per recorded iteration.
ShortestRoutes dijkstra(const Graph& graph,std::size_t source);
struct FloydIteration {
 std::size_t intermediate=0;
 bool initial=false;
 std::vector<std::optional<std::size_t>> next;
 // Row-major distances. nullopt denotes an unreachable pair.
 std::vector<std::optional<core::Scalar>> distance;
};
struct AllPairsRoutes {
 std::vector<ShortestRoutes> sources;
 std::vector<FloydIteration> trace;
 bool traceTruncated=false;
};
// Floyd-Warshall, including reachable negative-cycle classification.
AllPairsRoutes floydWarshall(const Graph& graph,bool recordTrace=true);

struct Augmentation {
 std::vector<std::size_t> nodes,edgeIndices;
 std::vector<bool> backward;
 core::Scalar amount=0;
};
struct FlowOptions {
 std::size_t iterationLimit=100000,maximumTraceSteps=10000;
 bool recordTrace=false;
};
struct MaximumFlow {
 core::Scalar value=0,cutCapacity=0;
 std::vector<core::Scalar> edgeFlow;
 std::vector<bool> sourceSide;
 std::vector<Augmentation> trace;
 std::size_t augmentationCount=0;
 bool traceTruncated=false;
};
// Directed nonnegative capacities; source != sink. Edmonds-Karp with distinct
// reverse arcs supports parallel and antiparallel input arcs. Self-loop flow=0.
// Final capacity/conservation/min-cut certificates are checked before returning.
// Nonrepresentable finite output throws overflow_error, invalid input throws
// invalid_argument, exhausted iteration limits throw runtime_error.
MaximumFlow maximumFlow(const Graph& graph,std::size_t source,std::size_t sink,const FlowOptions& options={});
}
