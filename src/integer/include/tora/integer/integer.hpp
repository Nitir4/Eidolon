#pragma once
#include <tora/lp/problem.hpp>
#include <optional>
namespace tora::integer {
enum class Status {Optimal,Infeasible,NodeLimit,ModelLimit,UnboundedRelaxation};
enum class NodeState {AwaitingBranch,Infeasible,BoundPruned,IntegerSolution,Branched,Unbounded};
struct Problem {lp::LpProblem lp;std::vector<bool> integral;};
struct BranchConstraint {std::size_t variable;lp::Relation relation;double bound;};
struct Node {std::optional<BranchConstraint> incoming;std::optional<std::size_t> parent;std::size_t depth=0;NodeState state=NodeState::Infeasible;std::optional<double> bound;std::optional<std::size_t> branchVariable;double branchValue=0;std::vector<double> values,lower,upper;};
struct Result {Status status=Status::NodeLimit;std::optional<double> objective;std::vector<double> values;std::vector<Node> nodes;};
// Incremental user-guided search. A processed fractional relaxation waits for
// branch(variable); next() chooses a pending subproblem (default: depth first).
// Every branch must select a fractional variable marked integer in the model.
struct PendingNode {std::optional<BranchConstraint> incoming;std::optional<std::size_t> parent;std::size_t depth=0;std::vector<double> lower,upper;};
class Session {
public:
 explicit Session(Problem,std::size_t nodeLimit=4096,double tolerance=1e-8);
 const Result& result()const{return result_;}
 bool finished()const{return finished_;}
 std::optional<std::size_t> awaitingBranch()const{return awaiting_;}
 std::vector<PendingNode> pending()const;
 void next(std::optional<std::size_t> pendingIndex={});
 void branch(std::size_t variable);
 void setBoundPruning(bool enabled){pruning_=enabled;}
 bool boundPruning()const{return pruning_;}
private:
 struct Pending{lp::LpProblem p;std::optional<std::size_t> parent;std::size_t depth;std::optional<BranchConstraint> incoming={};};
 void finishIfExhausted();
 Problem problem_;std::size_t limit_;double tolerance_;bool finished_=false,pruning_=true;
 std::vector<Pending> pending_;std::optional<Pending> current_;
 std::optional<std::size_t> awaiting_;Result result_;std::optional<double> incumbent_;
};
// Mixed integer branch and bound; bounded finite LP relaxations, explicit limits.
// Independent two-phase LP relaxations transform negative lower shifts and
// split free variables. Original input limit is100; internal branch row cap500.
Result solve(const Problem&,std::size_t nodeLimit=4096,double tolerance=1e-8);
}
