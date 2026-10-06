#include <tora/core/number_formatter.hpp>

#include <charconv>
#include <cmath>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace tora::core {
namespace {
struct Decimal {
    std::string digits;
    int exponent;
};

Decimal decimalDigits(double value)
{
    // Oracle neighbours around halfway points establish a 15-significant-
    // digit display intermediate before the requested half-away rounding.
    // This is display-only: numerical storage is never rounded or reparsed.
    char buffer[64];
    const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value, std::chars_format::scientific, 14);
    if (result.ec != std::errc()) throw std::runtime_error("Cannot convert finite numerical value");
    std::string spelling(buffer, result.ptr);
    const auto e = spelling.find('e');
    if (e == std::string::npos) throw std::runtime_error("Scientific conversion lacks an exponent");
    std::string digits;
    for (std::size_t i = 0; i < e; ++i) if (spelling[i] != '.') digits += spelling[i];
    return {std::move(digits), std::stoi(spelling.substr(e + 1))};
}

// Return the integer digits at the requested decimal scale, with half-away
// rounding. The number of retained digits can be negative for very small
// values; only a retained count of zero can round to one unit at this scale.
std::string rounded(const std::string& digits, int retain)
{
    if (retain < 0) return "0";
    std::string result;
    const auto size = static_cast<int>(digits.size());
    if (retain >= size) return digits + std::string(static_cast<std::size_t>(retain - size), '0');
    if (retain > 0) result = digits.substr(0, static_cast<std::size_t>(retain));
    if (digits[static_cast<std::size_t>(retain)] >= '5') {
        std::size_t position = result.size();
        while (position && result[position - 1] == '9') result[--position] = '0';
        if (position) ++result[position - 1]; else result.insert(result.begin(), '1');
    }
    return result.empty() ? "0" : result;
}

std::string fixed(std::string digits, int decimals)
{
    const auto needed = static_cast<std::size_t>(decimals + 1);
    if (digits.size() < needed) digits.insert(0, needed - digits.size(), '0');
    // TORA's D=0 capture has a trailing decimal point: "3.", not "3".
    digits.insert(digits.size() - static_cast<std::size_t>(decimals), 1, '.');
    return digits;
}
}

NumberFormatter::NumberFormatter(NumberFormat settings) : settings_(settings)
{
    // Safe library support envelope, not a claim about TORA's UI extrema.
    // Higher settings need a targeted oracle probe before exposing them.
    if (settings.n < 0 || settings.n > 10 || settings.d < 0 || settings.d > 10
        || (settings.notation == Notation::Decimal && settings.n == 0)
        || (settings.notation != Notation::Decimal && settings.notation != Notation::Scientific))
        throw std::invalid_argument("Number format is outside the supported precision range");
}

std::string NumberFormatter::format(double value) const
{
    if (!std::isfinite(value)) throw std::domain_error("Only finite numerical cells can be formatted");
    const bool negative = value < 0; // An exact IEEE negative zero is not a negative number.
    const auto decimal = decimalDigits(std::abs(value));
    std::string output;
    if (settings_.notation == Notation::Decimal) {
        output = fixed(rounded(decimal.digits, decimal.exponent + 1 + settings_.d), settings_.d);
    } else {
        int exponent = value == 0 ? 0 : decimal.exponent;
        auto digits = rounded(decimal.digits, settings_.n + 1);
        if (digits.size() > static_cast<std::size_t>(settings_.n + 1)) {
            ++exponent;
            digits.pop_back();
        }
        output = fixed(std::move(digits), settings_.n);
        output += exponent < 0 ? "e-" : "e+";
        const auto absoluteExponent = std::to_string(std::abs(exponent));
        if (absoluteExponent.size() < static_cast<std::size_t>(settings_.d))
            output.append(static_cast<std::size_t>(settings_.d) - absoluteExponent.size(), '0');
        output += absoluteExponent;
    }
    if (negative) output.insert(output.begin(), '-');
    return output;
}

std::string NumberFormatter::formatBound(double value) const
{
    if (std::isinf(value)) return value < 0 ? "-infinity" : "infinity";
    return format(value);
}
} // namespace tora::core
