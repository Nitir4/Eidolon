#pragma once
#include <tora/core/number_formatter.hpp>
#include <tora/lp/problem.hpp>
#include <tora/lp/validation.hpp>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace tora::compat {
// User-approved recovery of doubled quotes written by TORA itself, even when
// its own reader corrupts the model. Strict rejection remains opt-in.
enum class QuotePolicy { Reject, Recover };
inline constexpr QuotePolicy DefaultQuotePolicy = QuotePolicy::Recover;
enum class FileErrorCode {
    Io, ReadOnly, MissingFilename, TooLarge, Framing, Marker, Header,
    UnsupportedFormat, RecordCount, QuotedField, Number, Relation, Flag, Model
};
class FileError : public std::runtime_error {
public:
    FileError(FileErrorCode code, std::string message,
              std::optional<std::size_t> record = {},
              lp::DiagnosticOrigin origin = lp::DiagnosticOrigin::Engineering,
              std::string title = "LP File Error");
    FileErrorCode code() const noexcept { return code_; }
    std::optional<std::size_t> record() const noexcept { return record_; }
    lp::DiagnosticOrigin origin() const noexcept { return origin_; }
    const std::string& title() const noexcept { return title_; }
private:
    FileErrorCode code_;
    std::optional<std::size_t> record_;
    lp::DiagnosticOrigin origin_;
    std::string title_;
};

class LpDocument {
public:
    lp::LpProblem problem;
    core::NumberFormat inputFormat;
    const std::filesystem::path& sourcePath() const noexcept { return path_; }
    bool readOnly() const noexcept { return readOnly_; }
    bool dirty() const;
    QuotePolicy quotePolicy() const noexcept { return quotePolicy_; }
private:
    friend class ToraLpCodec;
    friend class LpFileStore;
    std::filesystem::path path_;
    bool readOnly_ = false;
    QuotePolicy quotePolicy_ = DefaultQuotePolicy;
    std::vector<std::string> originalRecords_;
    std::optional<lp::LpProblem> originalProblem_;
    core::NumberFormat originalFormat_;
    std::optional<lp::LpProblem> savedProblem_;
    core::NumberFormat savedFormat_;
};

class ToraLpCodec {
public:
    // Title/alias strings at this low-level legacy boundary contain original
    // file text bytes, not UTF-8 normalization. A native presenter must decode
    // them with the selected legacy code page (observed reference: Windows-1252).
    static constexpr std::size_t MaxFileBytes = 4 * 1024 * 1024;
    static LpDocument create(lp::LpProblem problem, core::NumberFormat format = {}, QuotePolicy quotes = DefaultQuotePolicy);
    static LpDocument decode(std::string_view bytes, QuotePolicy quotes = DefaultQuotePolicy);
    static std::string encode(const LpDocument& document);
    // Ordered legacy cell spellings, excluding the eight header records.
    // Applying a complete grid is transactional and retains the document's
    // source/read-only/saved semantic checkpoint. Values are parsed by the
    // same codec; no presenter-specific formatting occurs here.
    static std::vector<std::string> gridText(const LpDocument& document);
    static void setGridText(LpDocument& document, const std::vector<std::string>& cells);
};

class LpFileStore {
public:
    static LpDocument open(const std::filesystem::path& path, bool readOnly = false, QuotePolicy quotes = DefaultQuotePolicy);
    static void save(LpDocument& document);
    static void saveAs(LpDocument& document, const std::filesystem::path& path);
};
} // namespace tora::compat
