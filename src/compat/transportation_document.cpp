#include <tora/compat/transportation_document.hpp>
#include <tora/core/number_formatter.hpp>
#include <charconv>
#include <tora/core/number_parser.hpp>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace tora::compat {
namespace {
constexpr std::size_t MaximumBytes = 4 * 1024 * 1024;
void checkText(std::string_view value) {
    for (const unsigned char c : value) {
        if (c > 127) throw std::invalid_argument("Transportation supports the captured ASCII text profile; legacy code pages remain unverified.");
        if ((c < 32 && c != '\t') || c == 127)
            throw std::invalid_argument("Transportation text cannot contain control characters or line breaks.");
    }
    if (value.size() > MaximumBytes) throw std::invalid_argument("Transportation text exceeds the native 4 MiB limit.");
}
std::size_t size(const std::string& value) {
    std::size_t result = 0;
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() || !result || result > 100)
        throw std::invalid_argument("Invalid Transportation problem size (native limit: 100).");
    return result;
}
int digits(const std::string& value) {
    int result=0;const auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);
    if(parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size())throw std::invalid_argument("Invalid Transportation number format.");
    return result;
}
std::string unquote(const std::string& value) {
    if (value.size() < 2 || value.front() != '"' || value.back() != '"')
        throw std::invalid_argument("Expected a quoted Transportation text record.");
    std::string text;
    for (std::size_t i = 1; i + 1 < value.size(); ++i) {
        if (value[i] == '"') {
            if (i + 2 >= value.size() || value[i + 1] != '"') throw std::invalid_argument("Invalid embedded quote.");
            ++i;
        }
        text += value[i];
    }
    checkText(text); return text;
}
std::string quote(std::string_view value) {
    checkText(value); std::string result = "\"";
    for (char c : value) { result += c; if (c == '"') result += c; }
    return result + "\"";
}
double number(std::string_view value) {
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) value.remove_prefix(1);
    while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) value.remove_suffix(1);
    if (value.empty()) return 0; // Blank new quantities retain their spelling.
    if (value.front() == '+') value.remove_prefix(1);
    double result = 0;
    const auto parsed = core::parseDouble(value.data(), value.data() + value.size(), result);
    if (value.empty() || parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() || !std::isfinite(result))
        throw std::invalid_argument("Expected a finite Transportation numeric cell.");
    return result;
}
std::string lexical(double value) {
    char buffer[128]; const auto result = std::to_chars(buffer, buffer + sizeof buffer, value);
    if (result.ec != std::errc{}) throw std::invalid_argument("Cannot represent Transportation value.");
    return {buffer, result.ptr};
}
}
std::size_t TransportationDocument::index(std::size_t row, std::size_t col) const {
    if (row >= problem_.supply.size() + 2 || col >= problem_.demand.size() + 2)
        throw std::out_of_range("Transportation cell is outside the input grid.");
    return row * (problem_.demand.size() + 2) + col;
}
const std::string& TransportationDocument::cell(std::size_t row, std::size_t col) const {
    return cells_.at(index(row, col));
}
TransportationDocument TransportationDocument::decode(std::string_view bytes) {
    if (bytes.size() > MaximumBytes) throw std::invalid_argument("Transportation file exceeds the native 4 MiB limit.");
    std::vector<std::string> records;
    while (!bytes.empty()) {
        const auto end = bytes.find("\r\n");
        if (end == std::string_view::npos) throw std::invalid_argument("Transportation files require CRLF after each record.");
        const auto line = bytes.substr(0, end); checkText(line);
        records.emplace_back(line); bytes.remove_prefix(end + 2);
    }
    if (records.size() < 7 || records[0] != "4000") throw std::invalid_argument("Select a TORA Transportation file (4000).");
    if(records[3]!="1" && records[3]!="2")throw std::invalid_argument("Invalid Transportation notation (expected decimal or scientific).");
    const core::NumberFormat format{records[3]=="2"?core::Notation::Scientific:core::Notation::Decimal,digits(records[1]),digits(records[2])};
    (void)core::NumberFormatter(format);
    const auto m = size(records[5]), n = size(records[6]);
    if (records.size() != 7 + (m + 2) * (n + 2)) throw std::invalid_argument("Transportation file has an incorrect grid record count.");
    TransportationDocument doc; doc.problem_ = transportation::Problem::blank(m, n);
    doc.problem_.format=format;doc.formatNRecord_=records[1];doc.formatDRecord_=records[2];doc.notationRecord_=records[3];
    doc.sourceCountRecord_ = records[5]; doc.destinationCountRecord_ = records[6];
    doc.problem_.title = unquote(records[4]);
    for (std::size_t k = 7; k < records.size(); ++k) doc.cells_.push_back(unquote(records[k]));
    for (std::size_t j = 0; j < n; ++j) {
        doc.problem_.destinationNames[j] = doc.cell(0, j + 1);
        doc.problem_.demand[j] = number(doc.cell(m + 1, j + 1));
    }
    for (std::size_t i = 0; i < m; ++i) {
        doc.problem_.sourceNames[i] = doc.cell(i + 1, 0);
        doc.problem_.supply[i] = number(doc.cell(i + 1, n + 1));
        for (std::size_t j = 0; j < n; ++j) doc.problem_.costs[i][j] = number(doc.cell(i + 1, j + 1));
    }
    transportation::validate(doc.problem_); return doc;
}
TransportationDocument TransportationDocument::create(transportation::Problem problem) {
    transportation::validate(problem);
    TransportationDocument doc; doc.problem_ = std::move(problem);
    (void)core::NumberFormatter(doc.problem_.format);
    doc.formatNRecord_=std::to_string(doc.problem_.format.n);doc.formatDRecord_=std::to_string(doc.problem_.format.d);
    doc.notationRecord_=doc.problem_.format.notation==core::Notation::Scientific?"2":"1";
    const auto m = doc.problem_.supply.size(), n = doc.problem_.demand.size();
    doc.sourceCountRecord_ = std::to_string(m); doc.destinationCountRecord_ = std::to_string(n);
    doc.cells_.resize((m + 2) * (n + 2)); doc.cells_[0] = "S/D Name";
    for (std::size_t j = 0; j < n; ++j) {
        doc.cells_[doc.index(0, j + 1)] = doc.problem_.destinationNames[j];
        doc.cells_[doc.index(m + 1, j + 1)] = lexical(doc.problem_.demand[j]);
    }
    for (std::size_t i = 0; i < m; ++i) {
        doc.cells_[doc.index(i + 1, 0)] = doc.problem_.sourceNames[i];
        doc.cells_[doc.index(i + 1, n + 1)] = lexical(doc.problem_.supply[i]);
        for (std::size_t j = 0; j < n; ++j) doc.cells_[doc.index(i + 1, j + 1)] = lexical(doc.problem_.costs[i][j]);
    }
    (void)doc.encode(); return doc;
}
TransportationDocument TransportationDocument::blank(std::size_t m, std::size_t n, std::string title, core::NumberFormat format) {
    auto p = transportation::Problem::blank(m, n); p.title = std::move(title);p.format=format;
    auto doc = create(std::move(p));
    for (std::size_t i = 1; i <= m; ++i) {
        for (std::size_t j = 1; j <= n; ++j) doc.cells_[doc.index(i, j)] = core::NumberFormatter(format).format(0);
        doc.cells_[doc.index(i, n + 1)].clear();
    }
    for (std::size_t j = 1; j <= n; ++j) doc.cells_[doc.index(m + 1, j)].clear();
    return doc;
}
void TransportationDocument::setTitle(std::string title) {
    auto next = *this; next.problem_.title = std::move(title);
    (void)next.encode(); *this = std::move(next);
}
void TransportationDocument::setCell(std::size_t row, std::size_t col, std::string value) {
    const auto k = index(row, col), m = problem_.supply.size(), n = problem_.demand.size();
    checkText(value);
    const bool destination = row == 0 && col >= 1 && col <= n;
    const bool source = col == 0 && row >= 1 && row <= m;
    const bool cost = row >= 1 && row <= m && col >= 1 && col <= n;
    const bool supply = row >= 1 && row <= m && col == n + 1;
    const bool demand = row == m + 1 && col >= 1 && col <= n;
    if (!(destination || source || cost || supply || demand)) throw std::invalid_argument("This Transportation header/reserved cell is not editable.");
    // Commit only after all checks pass, preserving the previous model/records.
    auto next = *this;
    if (destination) next.problem_.destinationNames[col - 1] = value;
    if (source) next.problem_.sourceNames[row - 1] = value;
    if (cost || supply || demand) {
        double parsed = number(value);
        if ((supply || demand) && parsed < 0) throw std::invalid_argument("Transportation quantities must be nonnegative.");
        if (cost) value = core::NumberFormatter(problem_.format).format(parsed);
        else if (!value.empty()) value = lexical(std::round(parsed));
        parsed = number(value);
        if (cost) next.problem_.costs[row - 1][col - 1] = parsed;
        if (supply) next.problem_.supply[row - 1] = parsed;
        if (demand) next.problem_.demand[col - 1] = parsed;
    }
    next.cells_[k] = std::move(value); (void)next.encode(); *this = std::move(next);
}
std::vector<std::string> TransportationDocument::line(Axis axis, std::size_t position) const {
    const auto m = problem_.supply.size(), n = problem_.demand.size();
    if (position >= (axis == Axis::Source ? m : n)) throw std::out_of_range("Select a Transportation source or destination.");
    std::vector<std::string> result;
    if (axis == Axis::Source) {
        for (std::size_t col = 0; col < n + 2; ++col) result.push_back(cell(position + 1, col));
    } else {
        for (std::size_t row = 0; row < m + 2; ++row) result.push_back(cell(row, position + 1));
    }
    return result;
}
void TransportationDocument::replaceGrid(std::size_t m, std::size_t n, std::vector<std::string> cells) {
    // Validate the complete candidate before replacing any state.
    const auto sources = m == problem_.supply.size() ? sourceCountRecord_ : std::to_string(m);
    const auto destinations = n == problem_.demand.size() ? destinationCountRecord_ : std::to_string(n);
    std::string bytes = prefix() + sources + "\r\n" + destinations + "\r\n";
    for (const auto& value : cells) {
        bytes += quote(value) + "\r\n";
        if (bytes.size() > MaximumBytes) throw std::invalid_argument("Transportation file exceeds the native 4 MiB limit.");
    }
    auto next = decode(bytes); *this = std::move(next);
}
void TransportationDocument::insert(Axis axis, std::size_t position) {
    const auto m = problem_.supply.size(), n = problem_.demand.size();
    const bool source = axis == Axis::Source;
    const auto count = source ? m : n;
    if (position > count) throw std::out_of_range("Transportation insertion is outside the grid.");
    if (count >= 100) throw std::invalid_argument("Transportation native dimension limit is 100.");
    const auto mm = m + (source ? 1 : 0), nn = n + (source ? 0 : 1), at = position + 1;
    std::vector<std::string> cells((mm + 2) * (nn + 2));
    for (std::size_t row = 0; row < mm + 2; ++row) for (std::size_t col = 0; col < nn + 2; ++col) {
        auto& value = cells[row * (nn + 2) + col];
        if ((source && row == at) || (!source && col == at)) {
            if (row >= 1 && row <= mm && col >= 1 && col <= nn) value = core::NumberFormatter(problem_.format).format(0);
        } else value = cell(row - (source && row > at ? 1 : 0), col - (!source && col > at ? 1 : 0));
    }
    replaceGrid(mm, nn, std::move(cells));
}
void TransportationDocument::erase(Axis axis, std::size_t position) {
    const auto m = problem_.supply.size(), n = problem_.demand.size();
    const bool source = axis == Axis::Source;
    const auto count = source ? m : n;
    if (position >= count) throw std::out_of_range("Transportation deletion is outside the grid.");
    if (count <= 1) throw std::invalid_argument("Keep at least one Transportation source and destination.");
    const auto mm = m - (source ? 1 : 0), nn = n - (source ? 0 : 1), at = position + 1;
    std::vector<std::string> cells; cells.reserve((mm + 2) * (nn + 2));
    for (std::size_t row = 0; row < mm + 2; ++row) for (std::size_t col = 0; col < nn + 2; ++col)
        cells.push_back(cell(row + (source && row >= at ? 1 : 0), col + (!source && col >= at ? 1 : 0)));
    replaceGrid(mm, nn, std::move(cells));
}
void TransportationDocument::paste(Axis axis, std::size_t position, const std::vector<std::string>& values) {
    if (values.size() != line(axis, position).size()) throw std::invalid_argument("Copied Transportation line does not fit this grid.");
    auto cells = cells_;
    for (std::size_t i = 0; i < values.size(); ++i)
        cells[index(axis == Axis::Source ? position + 1 : i, axis == Axis::Source ? i : position + 1)] = values[i];
    replaceGrid(problem_.supply.size(), problem_.demand.size(), std::move(cells));
}
std::string TransportationDocument::prefix() const {
    return "4000\r\n" + formatNRecord_ + "\r\n" + formatDRecord_ + "\r\n" + notationRecord_ + "\r\n" + quote(problem_.title) + "\r\n";
}
std::string TransportationDocument::encode() const {
    std::string bytes = prefix()
        + sourceCountRecord_ + "\r\n" + destinationCountRecord_ + "\r\n";
    for (const auto& value : cells_) {
        bytes += quote(value) + "\r\n";
        if (bytes.size() > MaximumBytes) throw std::invalid_argument("Transportation file exceeds the native 4 MiB limit.");
    }
    return bytes;
}
}
