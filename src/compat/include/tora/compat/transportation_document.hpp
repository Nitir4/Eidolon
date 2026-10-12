#pragma once
#include <tora/transportation/north_west.hpp>
#include <string_view>

namespace tora::compat {
// Default 4000 profile. Retain untouched lexemes; UI costs use D=2 and
// quantities commit to whole numbers with observed half-away rounding.
// An incomplete/unbalanced draft is valid input; solving has stricter checks.
class TransportationDocument final {
public:
    static TransportationDocument decode(std::string_view bytes);
    static TransportationDocument create(transportation::Problem problem);
    static TransportationDocument blank(std::size_t sources, std::size_t destinations, std::string title = {});
    const transportation::Problem& problem() const { return problem_; }
    const std::string& cell(std::size_t row, std::size_t column) const;
    void setCell(std::size_t row, std::size_t column, std::string value);
    void setTitle(std::string title);
    std::string encode() const;
private:
    TransportationDocument() = default;
    std::size_t index(std::size_t row, std::size_t column) const;
    transportation::Problem problem_;
    std::string sourceCountRecord_, destinationCountRecord_;
    std::vector<std::string> cells_;
};
}
