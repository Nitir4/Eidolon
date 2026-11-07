#pragma once
#include <cstddef>
#include <optional>
#include <vector>
namespace tora::queuing {
struct Model {
    double arrivalRate = 0; // Per idle source when sourceLimit is finite.
    double serviceRate = 1; // Per busy server.
    std::size_t servers = 1;
    std::optional<std::size_t> systemLimit;
    std::optional<std::size_t> sourceLimit;
};
enum class Status { Stationary, Unstable };
struct Metrics {
    double meanSystem = 0, meanQueue = 0, meanBusyServers = 0, utilization = 0;
    double throughput = 0, attemptedArrivalRate = 0, rejectedArrivalRate = 0;
    // Original TORA rho/c display uses raw lambda/(c*mu), distinct from utilization.
    std::optional<double> offeredLoadPerServer;
    double meanSystemTime = 0, meanQueueTime = 0;
    double probabilityEmpty = 1, probabilityFull = 0;
    double probabilityWait = 0, blockingProbability = 0;
    double maximumRelativeBalanceResidual = 0;
    bool zeroArrivalLimit = false;
    bool absorbingNoService = false;
    bool arrivalBlockingUndefined = false;
};
struct Result {
    Status status = Status::Unstable;
    std::optional<Metrics> metrics;
    // Finite models contain all states 0..capacity. Infinite models contain
    // 0..servers; states after servers have an exact geometric tail.
    std::vector<double> probabilities;
    std::optional<std::size_t> capacity;
    double tailRatio = 0, tailMass = 0;
    double logTailRatio = 0;
};
// c<=10000; finite capacity/source <=100000. Nonnegative finite lambda,
// nonnegative finite mu (not both rates zero); finite mu=0 is absorbing.
// All specified population/capacity limits positive.
Result solve(const Model& model);
double probability(const Result& result,std::size_t state);
} // namespace tora::queuing
