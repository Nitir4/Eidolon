#include <tora/transportation/final_summary.hpp>
#include <tora/core/number_formatter.hpp>
#include <cmath>

namespace tora::transportation {
namespace {
std::string identity(const char* prefix,std::size_t index,const std::string& alias) {
    return std::string(prefix)+std::to_string(index+1)+":"+(alias.empty()?"":" "+alias);
}
std::string amount(double value) {
    // Independently observed summary profile: subunit quantities display zero;
    // quantities >=1 round to whole numbers with positive half ties upward.
    return value<1?"0":quantity(std::round(value));
}
}
FinalSummary finalSummary(const Problem& problem) {
    const auto trace=solveLeastCost(problem);const auto& terminal=trace.back();
    const core::NumberFormatter formatter(problem.format);
    FinalSummary result{problem.title,std::to_string(trace.size()),formatter.format(terminal.objective),{},problem.maximize};
    for(std::size_t i=0;i<problem.supply.size();++i)for(std::size_t j=0;j<problem.demand.size();++j)
        if(const auto shipment=terminal.shipments[i][j])
            result.rows.push_back({identity("S",i,problem.sourceNames[i]),identity("D",j,problem.destinationNames[j]),
                amount(*shipment),formatter.format(problem.costs[i][j]),formatter.format(*shipment*problem.costs[i][j])});
    return result;
}
}
