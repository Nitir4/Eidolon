#include <tora/transportation/north_west.hpp>
#include <tora/compat/transportation_reader.hpp>
#include <iostream>
#include <locale>
#include <fstream>
#include <iterator>
#include <string>

static void quote(const std::string& value) {
    std::cout << '"';
    for (char c : value) { if (c=='"' || c=='\\') std::cout << '\\'; std::cout << c; }
    std::cout << '"';
}
static void array(const std::vector<std::string>& values) {
    std::cout << '[';
    for (std::size_t i=0;i<values.size();++i) {if(i)std::cout<<',';quote(values[i]);}
    std::cout << ']';
}
static void matrix(const std::vector<std::vector<std::string>>& values) {
    std::cout << '[';
    for (std::size_t i=0;i<values.size();++i) {if(i)std::cout<<',';array(values[i]);}
    std::cout << ']';
}
int main(int argc,char** argv) {
    try {
        std::cin.imbue(std::locale::classic());
        tora::transportation::Problem problem;
        auto method=tora::transportation::StartMethod::NorthWest;
        std::string filename;
        for(int i=1;i<argc;++i) {
            const std::string arg=argv[i];
            if(arg=="--file" && i+1<argc)filename=argv[++i];
            else if(arg=="--method" && i+1<argc) {
                const std::string name=argv[++i];
                if(name=="leastcost")method=tora::transportation::StartMethod::LeastCost;
                else if(name=="vogel")method=tora::transportation::StartMethod::Vogel;
                else if(name!="northwest")throw std::invalid_argument("Unknown method");
            } else throw std::invalid_argument("Unknown or incomplete probe option");
        }
        if(!filename.empty()) {
            std::ifstream file(filename,std::ios::binary);
            if(!file)throw std::runtime_error("Cannot open probe file");
            const std::string bytes{std::istreambuf_iterator<char>(file),{}};
            problem=tora::compat::readTransportation(bytes);
        }else {
        std::size_t m=0,n=0;std::cin>>m>>n;
        problem=tora::transportation::Problem::blank(m,n);
        for (auto& row:problem.costs)for(auto& value:row)std::cin>>value;
        for(auto& value:problem.supply)std::cin>>value;
        for(auto& value:problem.demand)std::cin>>value;
        if(!std::cin)throw std::runtime_error("Invalid probe input");
        }
        const auto trace=tora::transportation::solve(problem,method);
        std::cout<<'[';
        for(std::size_t k=0;k<trace.size();++k) {
            if(k)std::cout<<',';
            const auto display=tora::transportation::display(problem,trace[k],k+1);
            std::cout<<"{\"label\":";quote(display.label);
            std::cout<<",\"objective\":";quote(display.objective);
            std::cout<<",\"u\":";array(display.u);std::cout<<",\"v\":";array(display.v);
            std::cout<<",\"costs\":";matrix(display.costs);
            std::cout<<",\"shipments\":";matrix(display.shipments);
            std::cout<<",\"reduced_costs\":";matrix(display.reducedCosts);
            std::cout<<",\"supply\":";array(display.supply);
            std::cout<<",\"demand\":";array(display.demand);std::cout<<'}';
        }
        std::cout<<']';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
