#include <tora/transportation/north_west.hpp>
#include <tora/compat/transportation_reader.hpp>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace tora::transportation;
static void rejects(const std::function<void()>& action) {
    try {action();}catch(const std::exception&){return;}
    throw std::runtime_error("Unsafe or unsupported input was accepted");
}
int main() {
    try {
        rejects([]{Problem::blank(0,3);});rejects([]{Problem::blank(101,3);});
        auto p=Problem::blank(2,3);p.supply={20,30};p.demand={10,25,15};
        p.costs={{2,3,1},{5,4,8}};
        auto changed=p;changed.costs[0].pop_back();rejects([&]{validate(changed);});
        changed=p;changed.supply[0]=-1;rejects([&]{validate(changed);});
        changed=p;changed.costs[0][0]=std::numeric_limits<double>::infinity();rejects([&]{validate(changed);});
        changed=p;changed.demand[0]=11;rejects([&]{solveNorthWest(changed);});
        changed=p;changed.supply[0]=20.1;rejects([&]{solveNorthWest(changed);});
        rejects([&]{solve(p,static_cast<StartMethod>(999));});
        changed=p;changed.demand[0]=11;rejects([&]{solveLeastCost(changed);});
        changed=p;changed.supply[0]=20.1;rejects([&]{solveLeastCost(changed);});
        const auto least=solveLeastCost(p),leastAgain=solveLeastCost(p);
        if(least.size()!=1 || leastAgain.size()!=1 || least.front().objective!=150 || p.supply!=std::vector<double>{20,30})
            throw std::runtime_error("Least-cost solver mutated input or is nondeterministic");
        auto zero=Problem::blank(3,3);const auto zeros=solveLeastCost(zero);
        if(zeros.size()!=1 || zeros[0].objective!=0)throw std::runtime_error("Zero-quantity Least-cost model failed");
        auto single=Problem::blank(1,1);single.costs={{-2}};single.supply={4};single.demand={4};
        if(solveLeastCost(single).back().objective!=-8)throw std::runtime_error("Single-cell Least-cost model failed");
        const auto first=solveNorthWest(p),second=solveNorthWest(p);
        if(first.size()!=3 || second.size()!=3 || first.back().objective!=150 || p.supply!=std::vector<double>{20,30})
            throw std::runtime_error("Solver mutated the input or is nondeterministic");
        if(!first[0].nextPivot || first.back().nextPivot)throw std::runtime_error("Pivot/termination state incorrect");
        // Independently authored 1x1 input exercises text framing and aliases.
        const std::string file="4000\r\n5\r\n2\r\n1\r\n\"A \"\"quoted\"\" title\"\r\n1\r\n1\r\n"
            "\"S/D Name\"\r\n\"Depot\"\r\n\"\"\r\n\"Factory\"\r\n\" +2.5 \"\r\n\"4\"\r\n\"\"\r\n\"4\"\r\n\"\"\r\n";
        const auto parsed=tora::compat::readTransportation(file);
        if(parsed.title!="A \"quoted\" title" || parsed.sourceNames[0]!="Factory" || parsed.destinationNames[0]!="Depot" || solveNorthWest(parsed).back().objective!=10)
            throw std::runtime_error("Transportation text/alias parsing failed");
        rejects([&]{tora::compat::readTransportation(file.substr(0,file.size()-1));});
        rejects([&]{tora::compat::readTransportation(file+"\"extra\"\r\n");});
        auto corrupt=file;corrupt.replace(0,4,"2000");rejects([&]{tora::compat::readTransportation(corrupt);});
        corrupt=file;corrupt[6]='6';rejects([&]{tora::compat::readTransportation(corrupt);});
        corrupt=file;corrupt.replace(corrupt.find(" +2.5 "),6,"nan");rejects([&]{tora::compat::readTransportation(corrupt);});
        corrupt=file;corrupt[corrupt.find("Depot")]='\0';rejects([&]{tora::compat::readTransportation(corrupt);});
        corrupt=file;corrupt[corrupt.find("Depot")]=static_cast<char>(0xe9);rejects([&]{tora::compat::readTransportation(corrupt);});
        rejects([]{tora::compat::readTransportation(std::string(4*1024*1024+1,'x'));});
        std::cout<<"Transportation safety checks passed\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
