#pragma once

#include <string>

namespace tora::core {
enum class Notation { Decimal, Scientific };
struct NumberFormat {
    Notation notation = Notation::Decimal;
    int n = 5;
    int d = 2;
};

class NumberFormatter {
public:
    explicit NumberFormatter(NumberFormat settings = {});
    const NumberFormat& settings() const noexcept { return settings_; }
    std::string format(double value) const;
    std::string formatBound(double value) const;
private:
    NumberFormat settings_;
};
} // namespace tora::core
