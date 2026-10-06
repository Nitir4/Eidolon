#include <tora/core/matrix.hpp>
#include <tora/core/number_formatter.hpp>
#include <tora/core/tableau.hpp>
#include <tora/core/tolerance.hpp>

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
template<class Exception, class Operation> void rejects(Operation operation) {
    bool rejected = false;
    try { operation(); } catch (const Exception&) { rejected = true; }
    require(rejected, "Expected exception was not raised");
}
}

int main()
{
    using namespace tora::core;
    try {
        Matrix matrix(2, 3, {1, 2, 3, 4, 5, 6});
        require(matrix(1, 2) == 6 && matrix.rows() == 2 && matrix.columns() == 3, "Row-major storage");
        rejects<std::out_of_range>([&] { (void)matrix(2, 0); });
        rejects<std::out_of_range>([&] { (void)matrix(0, 3); });
        rejects<std::invalid_argument>([] { Matrix invalid(2, 3, {1, 2}); });
        rejects<std::length_error>([] { Matrix invalid(std::numeric_limits<std::size_t>::max(), 2); });
        Matrix empty;
        require(empty.rows() == 0 && empty.columns() == 0, "Empty matrix");
        auto copy = matrix;
        copy(0, 0) = 12;
        require(matrix(0, 0) == 1, "Independent snapshot storage");
        DenseMatrix<long double> wider(1, 2, {1.0L, 3.0L});
        require(wider(0, 1) == 3.0L, "Scalar storage is replaceable");

        Tolerance strict(0, 0);
        require(strict.equal(1, 1) && !strict.equal(1, std::nextafter(1.0, 2.0)), "Exact comparison option");
        Tolerance tolerance(1e-10, 1e-12);
        require(tolerance.zero(1e-11) && !tolerance.zero(1e-8), "Absolute threshold");
        require(tolerance.equal(1e8, 1e8 + 1e-5), "Relative scale");
        require(Tolerance(0, 1e-6).equal(0.1, 0.10000001), "Relative comparison below unit scale");
        require(!Tolerance(0, 0).equal(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()), "Overflow cannot compare equal");
        require(tolerance.less(0, 1) && !tolerance.less(1, 1 + 1e-11), "Tolerance ordering");
        require(!tolerance.zero(std::numeric_limits<double>::quiet_NaN()), "NaN cannot be zero");
        require(!tolerance.equal(std::numeric_limits<double>::infinity(), 1), "Infinity not finite");
        require(!tolerance.equal(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()), "Nonfinite values rejected even when identical");
        rejects<std::invalid_argument>([] { Tolerance invalid(-1, 0); });
        rejects<std::invalid_argument>([] { Tolerance invalid(0, std::numeric_limits<double>::infinity()); });

        // Original input and all values/labels below are independently reviewed
        // TORA normal-case initial/Iterations 2/3 captures. This tests pivot
        // arithmetic only: a later solver module must choose its own pivots.
        Tableau tableau(Matrix(4, 6, {-3,-2,0,0,0,0, 1,1,1,0,0,4, 1,0,0,1,0,2, 0,1,0,0,1,3}),
                        {"x1","x2","sx3","sx4","sx5"}, {2,3,4});
        auto original = tableau;
        require(tableau.columns() == std::vector<std::string>({"x1","x2","sx3","sx4","sx5"}), "TORA column order");
        require(tableau.basis() == std::vector<std::size_t>({2,3,4}), "TORA initial basis order");
        const auto first = tableau.pivot(2, 0, strict);
        require(first.previousBasicColumn == 3 && first.value == 1, "First pivot metadata");
        require(tableau.values().data() == std::vector<double>({0,-2,0,3,0,6, 0,1,1,-1,0,2, 1,0,0,1,0,2, 0,1,0,0,1,3}), "TORA Iteration 2 cells");
        require(tableau.basicLabel(2) == "x1", "TORA Iteration 2 basis");
        tableau.pivot(1, 1, strict);
        require(tableau.values().data() == std::vector<double>({0,0,2,1,0,10, 0,1,1,-1,0,2, 1,0,0,1,0,2, 0,0,-1,1,1,1}), "TORA Iteration 3 cells");
        require(tableau.basicLabel(1) == "x2" && tableau.basicLabel(3) == "sx5", "TORA final basis order");
        require(original.values()(0, 0) == -3, "Previous iteration remains immutable");
        require(tableau.rhs(0) == 10, "Objective RHS access");
        tableau.setObjectiveBlocked(2, true);
        require(tableau.objectiveBlocked(2), "Artificial objective display metadata");
        auto snapshot = tableau;
        rejects<std::invalid_argument>([&] { tableau.pivot(0, 0, strict); });
        rejects<std::out_of_range>([&] { tableau.pivot(1, 5, strict); });
        rejects<std::domain_error>([&] { tableau.pivot(2, 2, strict); });
        require(tableau.values().data() == snapshot.values().data(), "Rejected pivot leaves snapshot intact");
        rejects<std::invalid_argument>([] { Tableau invalid(Matrix(2, 2), {"x"}, {3}); });
        rejects<std::invalid_argument>([] { Tableau invalid(Matrix(3, 3), {"x","x"}, {0,1}); });
        rejects<std::invalid_argument>([] { Tableau invalid(Matrix(3, 3), {"x","y"}, {0,0}); });
        rejects<std::domain_error>([] { Tableau invalid(Matrix(2, 2, {0,0,1,std::numeric_limits<double>::infinity()}), {"x"}, {0}); });
        Tableau fractional(Matrix(2, 3, {-1,0,0, 3,1,1}), {"x","s"}, {1});
        fractional.pivot(1, 0, strict);
        require(std::abs(fractional.values()(1, 1) - 1.0 / 3.0) < 1e-15, "Unrounded fractions");
        // TORA computed-halfway summary: max x1 with .017*x1 <= .036125.
        // Binary64 division lies below 2.125; display must still match 2.13.
        Tableau computedHalf(Matrix(2, 3, {-1,0,0, .017,1,.036125}), {"x1","s"}, {1});
        computedHalf.pivot(1, 0, strict);
        require(computedHalf.rhs(0) == .036125 / .017, "Computed value retained at arithmetic precision");
        require(NumberFormatter().format(computedHalf.rhs(0)) == "2.13", "TORA computed halfway display");
        Tableau tiny(Matrix(2, 3, {-1,0,0, 1e-12,1,1}), {"x","s"}, {1});
        rejects<std::domain_error>([&] { tiny.pivot(1, 0, tolerance); });
        tiny.pivot(1, 0, strict);
        require(tiny.rhs(1) == 1e12, "Caller controls pivot tolerance");
        Tableau signedZero(Matrix(2, 3, {0,0,0, -1,1,0}), {"x","s"}, {1});
        signedZero.pivot(1, 0, strict);
        require(std::signbit(signedZero.rhs(1)), "Pivot does not silently erase signed zero");
        const double huge = std::numeric_limits<double>::max();
        Tableau overflow(Matrix(2, 3, {huge,0,huge, 0.5,1,huge}), {"x","s"}, {1});
        const auto before = overflow.values().data();
        rejects<std::overflow_error>([&] { overflow.pivot(1, 0, strict); });
        require(overflow.values().data() == before && overflow.basicLabel(1) == "s", "Overflow is transactional");
        std::cout << "Core storage, tolerance, observed normal pivots and failure handling passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
