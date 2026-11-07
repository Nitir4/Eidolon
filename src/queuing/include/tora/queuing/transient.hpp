#pragma once
#include <tora/queuing/queuing.hpp>
namespace tora::queuing {
enum class TransientKind { PureBirth, PureDeath };
struct TransientResult {
    TransientKind kind=TransientKind::PureBirth;
    double time=1, mean=0;
    std::size_t initialPopulation=0;
    std::vector<double> probabilities,cumulative;
    // Explicit numerical summation error bound for the omitted Poisson tail.
    double omittedMass=0;
};
// Original boundary convention: unit time, constant rate, no dependence on c.
// Pure death starts with the explicitly supplied N customers. The native UI
// keeps its requested mathematical profile separate from the observed original
// 500-population cap; the original 9999 label is not its effective population.
// Explicit time and N are mathematical extensions. rate*time<=100000, N<=100000.
TransientResult pureBirth(double rate,double time=1);
TransientResult pureDeath(double rate,std::size_t initialPopulation,double time=1);
double probability(const TransientResult&,std::size_t state);
double cumulativeProbability(const TransientResult&,std::size_t state);
}
