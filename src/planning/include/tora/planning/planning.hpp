#pragma once
#include <cstddef>
#include <string>
#include <vector>
namespace tora::planning {
enum class Method { Cpm, Pert };
struct Activity { int from=0,to=0; std::string symbol; double duration=0,optimistic=0,mostLikely=0,pessimistic=0; };
struct Problem { Method method=Method::Cpm; std::vector<Activity> activities; };
struct ActivityResult { double mean=0,variance=0,earliestStart=0,earliestFinish=0,latestStart=0,latestFinish=0,totalFloat=0,freeFloat=0; bool critical=false; };
struct EventResult { int node=0; double earliest=0,latest=0; };
enum class Pass { Forward, Backward };
struct TraceStep { Pass pass=Pass::Forward; int event=0; std::size_t activity=0; double previous=0,candidate=0,current=0; };
struct Result { double duration=0,pathVariance=0; std::vector<ActivityResult> activities; std::vector<EventResult> events; std::vector<std::size_t> criticalPath; std::vector<int> topologicalOrder; std::vector<TraceStep> trace; };
// AOA acyclic networks; disconnected components share project start/end.
Result solve(const Problem& problem);
struct Schedule { std::vector<double> starts,finishes; double completion=0; };
// Requested delays are measured from each activity's original earliest start.
// All successor activities are delayed transitively to preserve AOA precedence.
// Rejects requests outside total float or schedules beyond the original finish.
Schedule schedule(const Problem& problem,const std::vector<double>& requestedDelays);
// Normal approximation for the selected longest-mean path, assuming independent
// activity durations. Not an exact project completion probability.
double approximateDeadlineProbability(const Result& result,double deadline);
}
