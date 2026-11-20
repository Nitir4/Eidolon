#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <tora/core/number_formatter.hpp>

namespace tora::transportation {
struct Problem {
    std::string title;
    std::vector<std::vector<double>> costs;
    std::vector<double> supply, demand;
    std::vector<std::string> sourceNames, destinationNames;
    core::NumberFormat format;
    bool maximize=false;
    std::vector<std::size_t> dummySources,dummyDestinations;
    static Problem blank(std::size_t sources, std::size_t destinations);
};
// General model validation is distinct from the captured numerical profile.
// The size bound is a native allocation safeguard, not a claimed TORA limit.
void validate(const Problem& problem);
enum class StartMethod { NorthWest, LeastCost, Vogel };
struct Cell {
    std::size_t source, destination;
};
struct Pivot {
    Cell entering, leaving;
    std::vector<Cell> loop;
};
struct Tableau {
    std::vector<std::vector<std::optional<double>>> shipments;
    std::vector<double> u,v;
    std::vector<std::vector<double>> reducedCosts;
    double objective=0;
    std::optional<Pivot> nextPivot;
};
enum class LoopRole { None, Entering, Leaving, Other };
// Original manual clicks provide feedback for the computed improvement loop.
// They do not select a different pivot or change allocation state.
LoopRole loopRole(const Tableau& tableau, Cell cell);
struct Display {
    std::string label, objective;
    std::vector<std::vector<std::string>> costs, shipments, reducedCosts;
    std::vector<std::string> u,v,supply,demand;
};
enum class MultiplierAxis { Source, Destination };
struct MultiplierAnchor {
    MultiplierAxis axis=MultiplierAxis::Source;
    std::size_t index=0;
};
// Reinitialize a connected basic tree without altering allocations/pivots.
// Returns a candidate; invalid anchors/bases leave the supplied tableau intact.
Tableau initializeMultipliers(const Problem& problem, const Tableau& tableau, MultiplierAnchor anchor);
// Automatic minimum-cost North-west start and MODI improvements, u1=0.
// Nonbinary single-precision quantities are deliberately rejected until their
// observed long/clipped TORA display has a complete compatibility contract.
std::vector<Tableau> solve(const Problem& problem, StartMethod method);
std::vector<Tableau> solveNorthWest(const Problem& problem);
std::vector<Tableau> solveLeastCost(const Problem& problem);
std::vector<Tableau> solveVogel(const Problem& problem);
Display display(const Problem& problem, const Tableau& tableau, std::size_t iteration, bool shortMultipliers=false);
std::string quantity(double value);
} // namespace tora::transportation
