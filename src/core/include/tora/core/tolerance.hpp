#pragma once

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace tora::core {
// Engineering defaults, NOT asserted to be TORA's hidden solver thresholds.
// Each future method must establish and pass its own compatibility policy.
class Tolerance {
public:
    explicit Tolerance(double absolute = 1e-10, double relative = 1e-12)
        : absolute_(absolute), relative_(relative) {
        if (!std::isfinite(absolute) || !std::isfinite(relative) || absolute < 0 || relative < 0)
            throw std::invalid_argument("Tolerances must be finite and nonnegative");
    }
    bool zero(double value) const noexcept { return std::isfinite(value) && std::abs(value) <= absolute_; }
    bool equal(double left, double right) const noexcept {
        if (!std::isfinite(left) || !std::isfinite(right)) return false;
        if (left == right) return true;
        const auto scale = std::max(std::abs(left), std::abs(right));
        // Avoid infinity <= infinity when subtracting extreme finite values.
        const auto difference = std::abs(left - right);
        if (std::isfinite(difference))
            return difference <= absolute_ || (difference - absolute_) / scale <= relative_;
        return std::abs(left / scale - right / scale) <= relative_ + absolute_ / scale;
    }
    // An approximate numerical comparison, not an STL sorting comparator.
    bool less(double left, double right) const noexcept {
        return std::isfinite(left) && std::isfinite(right) && left < right && !equal(left, right);
    }
    double absolute() const noexcept { return absolute_; }
    double relative() const noexcept { return relative_; }
private:
    double absolute_;
    double relative_;
};
} // namespace tora::core
