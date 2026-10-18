#include <tora/assignment/assignment.hpp>
#include <tora/compat/transportation_document.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <stdexcept>
int main(int argc,char** argv) {
    try {
        if(argc!=3)throw std::invalid_argument("Use assignment_probe min|negated-max file");
        const std::string mode=argv[1];if(mode!="min" && mode!="negated-max")throw std::invalid_argument("Invalid mode");
        std::ifstream input(argv[2],std::ios::binary);if(!input)throw std::invalid_argument("Cannot open input");
        const std::string bytes{std::istreambuf_iterator<char>(input),{}};
        const auto doc=tora::compat::TransportationDocument::decode(bytes);const auto& p=doc.problem();
        for(auto quantity:p.supply)if(quantity!=1)throw std::invalid_argument("Expected unit supplies");
        for(auto quantity:p.demand)if(quantity!=1)throw std::invalid_argument("Expected unit demands");
        tora::assignment::Problem model{tora::core::Matrix(p.supply.size(),p.demand.size()),mode=="min"?tora::assignment::Sense::Minimize:tora::assignment::Sense::Maximize};
        for(std::size_t i=0;i<p.supply.size();++i)for(std::size_t j=0;j<p.demand.size();++j)model.costs(i,j)=(mode=="min"?1:-1)*p.costs[i][j];
        const auto r=tora::assignment::solve(model);std::cout<<std::setprecision(17)<<"{\"objective\":"<<r.objective<<",\"pairs\":[";
        bool first=true;for(std::size_t i=0;i<r.rowToColumn.size();++i)if(r.rowToColumn[i]){if(!first)std::cout<<',';first=false;std::cout<<'['<<i<<','<<*r.rowToColumn[i]<<']';}
        std::cout<<"]}\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
