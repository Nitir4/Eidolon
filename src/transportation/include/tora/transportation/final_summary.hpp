#pragma once
#include <tora/transportation/north_west.hpp>

namespace tora::transportation {
struct FinalSummary {
    std::string title, iteration, objective;
    std::vector<std::vector<std::string>> rows;
};
// The captured default final route uses the Least-cost terminal basis.
// Basic zero rows are retained; displayed amounts do not determine contributions.
FinalSummary finalSummary(const Problem& problem);
}
