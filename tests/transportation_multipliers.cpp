#include <tora/transportation/north_west.hpp>
#include <functional>
#include <iostream>
#include <stdexcept>
using namespace tora::transportation;
static void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
static void rejects(const std::function<void()>& action) {
    try { action(); } catch (const std::exception&) { return; }
    throw std::runtime_error("Invalid multiplier anchor/basis accepted");
}
int main() {
    try {
        auto p=Problem::blank(2,3);p.costs={{2,3,1},{5,4,8}};p.supply={20,30};p.demand={10,25,15};
        const auto trace=solveNorthWest(p);
        const auto anchored=initializeMultipliers(p,trace[0],{MultiplierAxis::Destination,2});
        check(anchored.u==std::vector<double>({7,8}) && anchored.v==std::vector<double>({-5,-4,0}),"Observed v3 zero anchor mismatch");
        check(anchored.shipments==trace[0].shipments && anchored.reducedCosts==trace[0].reducedCosts && anchored.objective==230,"Anchor changed solution data");
        check(anchored.nextPivot && anchored.nextPivot->entering.source==trace[0].nextPivot->entering.source && anchored.nextPivot->entering.destination==trace[0].nextPivot->entering.destination,"Anchor changed entering pivot");
        check(trace[0].u[0]==0,"Anchor mutated input tableau");
        const auto shortDisplay=display(p,anchored,1,true);
        check(shortDisplay.u==std::vector<std::string>({"u1=7","u2=8"}) && shortDisplay.v==std::vector<std::string>({"v1=-5","v2=-4","v3=0"}),"Observed short refresh formatting mismatch");
        const auto second=initializeMultipliers(p,trace[1],{MultiplierAxis::Destination,2});
        check(display(p,second,2).u==std::vector<std::string>({"u1=1.00","u2=8.00"}),"Observed later-tableau anchor formatting mismatch");
        for (const auto axis : {MultiplierAxis::Source,MultiplierAxis::Destination}) {
            const auto count=axis==MultiplierAxis::Source?2u:3u;
            for(std::size_t index=0;index<count;++index) {
                const auto candidate=initializeMultipliers(p,trace[0],{axis,index});
                check((axis==MultiplierAxis::Source?candidate.u[index]:candidate.v[index])==0,"Selected anchor is not zero");
                for(std::size_t r=0;r<2;++r)for(std::size_t c=0;c<3;++c)
                    if(candidate.shipments[r][c])check(candidate.u[r]+candidate.v[c]==p.costs[r][c],"Basic equation violated");
            }
        }
        rejects([&]{initializeMultipliers(p,trace[0],{MultiplierAxis::Source,2});});
        rejects([&]{initializeMultipliers(p,trace[0],{MultiplierAxis::Destination,3});});
        rejects([&]{initializeMultipliers(p,trace[0],{static_cast<MultiplierAxis>(99),0});});
        auto broken=trace[0];broken.shipments[0][0].reset();
        rejects([&]{initializeMultipliers(p,broken,{MultiplierAxis::Source,0});});
        broken=trace[0];broken.shipments[1][0]=1;
        rejects([&]{initializeMultipliers(p,broken,{MultiplierAxis::Source,0});});
        broken=trace[0];broken.shipments.pop_back();
        rejects([&]{initializeMultipliers(p,broken,{MultiplierAxis::Source,0});});
        auto degenerate=Problem::blank(3,3);degenerate.costs={{7,0,8},{9,5,3},{9,0,5}};degenerate.supply={2,3,8};degenerate.demand={6,4,3};
        const auto zeroBasis=initializeMultipliers(degenerate,solveNorthWest(degenerate)[0],{MultiplierAxis::Source,2});
        check(zeroBasis.u==std::vector<double>({-2,0,0}) && zeroBasis.v==std::vector<double>({9,0,5}),"Observed zero-basic-edge anchor mismatch");
        auto emptySource=p;emptySource.supply={0,50};
        const auto originalZero=solveNorthWest(emptySource)[0];
        check(originalZero.shipments[0][0] && *originalZero.shipments[0][0]==0,"Missing zero basic test edge");
        const auto selectedZero=initializeMultipliers(emptySource,originalZero,{MultiplierAxis::Source,1});
        check(selectedZero.u[0]==-3 && selectedZero.u[1]==0 && selectedZero.v[0]==5,"Zero shipment edge was ignored");
        std::cout<<"Multiplier anchor, basis, formatting and preservation checks passed\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
