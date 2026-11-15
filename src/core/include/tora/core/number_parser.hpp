#pragma once
#include <charconv>
#if defined(__APPLE__) || defined(TORA_USE_C_LOCALE_NUMBER_PARSER)
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <locale.h>
#include <string>
#endif

namespace tora::core {
// Callers require finite decimal cells. Apple libc++ does not provide the
// floating from_chars overload; use a per-call C locale without global changes.
inline std::from_chars_result parseDouble(const char* first, const char* last, double& value) {
#if defined(__APPLE__) || defined(TORA_USE_C_LOCALE_NUMBER_PARSER)
    const auto digit = [](char c) { return c >= '0' && c <= '9'; };
    const char* end = first;
    if (end != last && *end == '-') ++end;
    bool digits = false;
    while (end != last && digit(*end)) { digits = true; ++end; }
    if (end != last && *end == '.') {
        ++end;
        while (end != last && digit(*end)) { digits = true; ++end; }
    }
    if (!digits) return {first, std::errc::invalid_argument};
    if (end != last && (*end == 'e' || *end == 'E')) {
        const char* exponent = end + 1;
        if (exponent != last && (*exponent == '+' || *exponent == '-')) ++exponent;
        const char* start = exponent;
        while (exponent != last && digit(*exponent)) ++exponent;
        if (exponent != start) end = exponent;
    }
    struct ClassicLocale {
        locale_t handle = newlocale(LC_NUMERIC_MASK, "C", nullptr);
        ~ClassicLocale() { if (handle) freelocale(handle); }
    };
    static const ClassicLocale classic;
    if (!classic.handle) return {first, std::errc::invalid_argument};
    const std::string token(first, end);
    const int previousError = errno;
    errno = 0;
    const double parsed = strtod_l(token.c_str(), nullptr, classic.handle);
    const int error = errno;
    errno = previousError;
    // Representable subnormals are valid even when strtod reports ERANGE.
    if (!std::isfinite(parsed) || (error == ERANGE && parsed == 0))
        return {end, std::errc::result_out_of_range};
    value = parsed;
    return {end, {}};
#else
    return std::from_chars(first, last, value, std::chars_format::general);
#endif
}
}
