#include <tora/core/number_parser.hpp>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>

int main() {
    const std::array<const char*, 27> cells{{"0", "-0", "1", "1.5", ".25", "1.", "-2e-3", "1e+2",
        "1e", "1e+", "12tail", "", "-", ".", "+1", " 1", "0x12", "1,2", "1e9999", "1e-9999",
        "4.9406564584124654e-324", "2.2250738585072014e-308", "1.7976931348623157e308",
        "-1.7976931348623157e308", "-4.9406564584124654e-324", "0e-9999", "-0e9999"}};
    auto compare = [](const std::string& text) {
        double expected = 42, actual = 42;
        const auto native = std::from_chars(text.data(), text.data()+text.size(), expected);
        const auto fallback = tora::core::parseDouble(text.data(), text.data()+text.size(), actual);
        if (native.ec != fallback.ec || native.ptr-text.data() != fallback.ptr-text.data() ||
            std::memcmp(&expected, &actual, sizeof(double))) {
            std::cerr << "Decimal parser mismatch: " << text << '\n';
            return false;
        }
        return true;
    };
    for (const auto* cell : cells) if (!compare(cell)) return 1;
    std::uint64_t state = 0x128734678ab19ULL;
    for (int i = 0; i < 10000; ++i) {
        state ^= state << 13; state ^= state >> 7; state ^= state << 17;
        double value;
        std::memcpy(&value, &state, sizeof value);
        if (!std::isfinite(value)) continue;
        char buffer[128];
        const auto result = std::to_chars(buffer, buffer+sizeof buffer, value);
        if (result.ec != std::errc{} || !compare(std::string(buffer, result.ptr))) return 1;
    }
    std::cout << "Finite decimal fallback matches native parser, including subnormals/range/prefix cases.\n";
}
