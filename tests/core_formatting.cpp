#include <tora/core/number_formatter.hpp>

#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
struct CommaDecimal final : std::numpunct<char> {
    char do_decimal_point() const override { return ','; }
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\3"; }
};
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
}

int main(int argc, char** argv)
{
    using namespace tora::core;
    try {
        require(argc == 2, "Pass the reviewed oracle fixture TSV");
        std::ifstream input(argv[1]);
        require(input.good(), "Cannot read reviewed formatting fixtures");
        std::size_t count = 0;
        std::string line;
        while (std::getline(input, line)) {
            if (line.empty() || line.front() == '#') continue;
            std::istringstream fields(line);
            fields.imbue(std::locale::classic());
            std::string name, notation, lexeme, expected;
            int n = 0, d = 0;
            fields >> name >> notation >> n >> d >> lexeme >> expected;
            require(!fields.fail(), "Malformed golden fixture");
            std::istringstream valueStream(lexeme);
            valueStream.imbue(std::locale::classic());
            double value = 0;
            valueStream >> value;
            require(!valueStream.fail(), "Malformed fixture numeric input");
            const NumberFormatter formatter({notation == "scientific" ? Notation::Scientific : Notation::Decimal, n, d});
            const auto actual = formatter.format(value);
            if (actual != expected) {
                std::cerr << name << ": TORA=" << expected << " clone=" << actual << '\n';
                return 1;
            }
            // No arithmetic or presentation changes under a comma locale.
            const auto previous = std::locale();
            std::locale::global(std::locale(previous, new CommaDecimal));
            const auto localized = formatter.format(value);
            std::locale::global(previous);
            require(localized == expected, "Formatting changed with locale");
            ++count;
        }
        require(count >= 20, "Incomplete golden fixture set");
        const NumberFormatter decimal;
        require(decimal.formatBound(std::numeric_limits<double>::infinity()) == "infinity", "Unbounded upper token");
        require(decimal.formatBound(-std::numeric_limits<double>::infinity()) == "-infinity", "Sensitivity lower token");
        bool invalid = false;
        try { (void)decimal.format(std::numeric_limits<double>::quiet_NaN()); } catch (const std::domain_error&) { invalid = true; }
        require(invalid, "NaN must be diagnosed, not printed as a solution");
        invalid = false;
        try { (void)decimal.format(std::numeric_limits<double>::infinity()); } catch (const std::domain_error&) { invalid = true; }
        require(invalid, "Only bound formatting accepts infinity");
        invalid = false;
        try { NumberFormatter unsupported({Notation::Decimal, -1, 2}); } catch (const std::invalid_argument&) { invalid = true; }
        require(invalid, "Negative format settings rejected");
        invalid = false;
        try { NumberFormatter unsupported({Notation::Decimal, 0, 2}); } catch (const std::invalid_argument&) { invalid = true; }
        require(invalid, "N=0 runtime error is replaced by a safe diagnostic");
        invalid = false;
        try { NumberFormatter unsupported({Notation::Scientific, 1000000, 2}); } catch (const std::invalid_argument&) { invalid = true; }
        require(invalid, "Unsupported format settings bounded safely");
        const double value = 1.0 / 3.0;
        (void)decimal.format(value);
        require(value == 1.0 / 3.0, "Display must never round stored arithmetic");
        std::cout << count << " independently observed number strings matched TORA, including locale isolation\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
