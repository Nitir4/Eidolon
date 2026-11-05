#pragma once
#include <tora/transportation/north_west.hpp>
#include <string_view>

namespace tora::compat {
// 4000 decimal/scientific profile. Retain untouched lexemes; UI costs use the selected format and
// quantities commit to whole numbers with observed half-away rounding.
// An incomplete/unbalanced draft is valid input; solving has stricter checks.
class TransportationDocument final {
public:
    enum class Axis { Source, Destination };
    static TransportationDocument decode(std::string_view bytes);
    static TransportationDocument create(transportation::Problem problem);
    static TransportationDocument blank(std::size_t sources, std::size_t destinations, std::string title = {}, core::NumberFormat format = {});
    const transportation::Problem& problem() const { return problem_; }
    const std::string& cell(std::size_t row, std::size_t column) const;
    void setCell(std::size_t row, std::size_t column, std::string value);
    void setTitle(std::string title);
    // Line positions are zero-based; insertion accepts count as append.
    std::vector<std::string> line(Axis axis, std::size_t position) const;
    void insert(Axis axis, std::size_t position);
    void erase(Axis axis, std::size_t position);
    void paste(Axis axis, std::size_t position, const std::vector<std::string>& values);
    std::string encode() const;
private:
    TransportationDocument() = default;
    std::size_t index(std::size_t row, std::size_t column) const;
    void replaceGrid(std::size_t sources, std::size_t destinations, std::vector<std::string> cells);
    std::string prefix() const;
    transportation::Problem problem_;
    std::string sourceCountRecord_, destinationCountRecord_;
    std::string formatNRecord_="5", formatDRecord_="2", notationRecord_="1";
    std::vector<std::string> cells_;
};
}
