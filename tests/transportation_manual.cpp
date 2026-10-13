#include <tora/transportation/north_west.hpp>
#include <iostream>
#include <stdexcept>
using namespace tora::transportation;
static void check(bool condition,const char* message) {if(!condition)throw std::runtime_error(message);}
int main() {
    try {
        auto p=Problem::blank(2,3);p.costs={{2,3,1},{5,4,8}};p.supply={20,30};p.demand={10,25,15};
        const auto trace=solveNorthWest(p);const auto& first=trace[0];
        // These roles were independently read from original clicked-cell
        // screenshots, not generated from the clone's loop ordering.
        check(loopRole(first,{0,2})==LoopRole::Entering,"Pilot entering feedback mismatch");
        check(loopRole(first,{0,1})==LoopRole::Leaving,"Pilot leaving feedback mismatch");
        check(loopRole(first,{1,1})==LoopRole::Other && loopRole(first,{1,2})==LoopRole::Other,"Pilot other loop feedback mismatch");
        check(loopRole(first,{1,0})==LoopRole::None,"Wrong shipment received feedback");
        check(loopRole(first,{0,2})==LoopRole::Entering,"Repeat classification changed feedback");
        check(loopRole(first,{100,100})==LoopRole::None,"Out-of-grid click received feedback");
        for(std::size_t r=0;r<2;++r)for(std::size_t c=0;c<3;++c)
            check(loopRole(trace.back(),{r,c})==LoopRole::None,"Terminal tableau received feedback");
        auto tied=Problem::blank(3,3);tied.costs={{6,6,2},{1,5,5},{7,1,3}};tied.supply={2,9,3};tied.demand={1,8,5};
        const auto enteringTie=solveNorthWest(tied)[0];
        check(loopRole(enteringTie,{1,0})==LoopRole::None,"Alternate tied entering was accepted");
        check(loopRole(enteringTie,{0,2})==LoopRole::Entering && loopRole(enteringTie,{0,1})==LoopRole::Leaving,"Computed tied loop feedback mismatch");
        tied.costs={{7,0,8},{9,5,3},{9,0,5}};tied.supply={2,3,8};tied.demand={6,4,3};
        const auto leavingTie=solveNorthWest(tied)[0];
        check(loopRole(leavingTie,{1,0})==LoopRole::Leaving && loopRole(leavingTie,{2,2})==LoopRole::Other,"Leaving tie feedback mismatch");
        check(loopRole(leavingTie,{1,2})==LoopRole::Entering,"Leaving tie entering feedback mismatch");
        p.supply={0,50};const auto zero=solveNorthWest(p)[0];
        check(zero.shipments[0][0] && *zero.shipments[0][0]==0 && loopRole(zero,{0,0})==LoopRole::Leaving,"Zero basic leaving feedback missing");
        check(loopRole(zero,{0,2})==LoopRole::Entering,"Zero-source entering feedback mismatch");
        p.costs={{2,2,5},{2,4,1}};p.supply={17,23};p.demand={11,13,16};
        const auto least=solveLeastCost(p)[0];
        check(loopRole(least,{1,0})==LoopRole::Entering && loopRole(least,{1,1})==LoopRole::Leaving,"Observed Least-cost feedback mismatch");
        const auto vogel=solveVogel(tied)[0];
        check(loopRole(vogel,{0,0})==LoopRole::Entering && loopRole(vogel,{0,1})==LoopRole::Leaving,"Observed Vogel feedback mismatch");
        check(vogel.shipments[2][2] && *vogel.shipments[2][2]==0 && loopRole(vogel,{2,2})==LoopRole::None,"Nonloop zero received Vogel feedback");
        check(first.objective==230 && first.shipments[0][0]==10 && trace[1].objective==170,"Feedback changed numerical trace");
        std::cout<<"Observed manual loop, tie, zero and terminal feedback roles passed\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
