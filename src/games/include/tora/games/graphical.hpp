#pragma once
#include <tora/games/games.hpp>
#include <array>

namespace tora::games {
enum class GraphicalPlayer { RowA, ColumnB };
struct PayoffLine {
    std::size_t strategy=0;
    core::Scalar atZero=0,atOne=0;
};
struct EnvelopeSegment {
    core::Scalar from=0,to=1;
    std::size_t strategy=0;
};
struct GraphicalResult {
    GraphicalPlayer player=GraphicalPlayer::RowA;
    std::vector<PayoffLine> lines;
    std::vector<EnvelopeSegment> envelope;
    // Probability of the selected player's first strategy; choose the
    // smallest maximizing/minimizing breakpoint when optima tie exactly.
    core::Scalar probability=0;
    std::array<core::Scalar,2> optimalInterval{0,0};
    Result equilibrium;
    std::vector<core::Scalar> expectedPayoffs(core::Scalar probability) const;
};
// RowA: 2xn, maximize lower envelope. ColumnB: mx2, minimize upper
// envelope. ColumnB is a mathematical extension unless observed in TORA.
GraphicalResult solveGraphical(const core::Matrix& payoff,
    GraphicalPlayer player=GraphicalPlayer::RowA,
    const core::Tolerance& tolerance=core::Tolerance(1e-9,1e-10));
} // namespace tora::games
