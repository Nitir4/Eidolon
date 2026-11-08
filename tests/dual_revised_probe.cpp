#include <tora/lp/dual_revised_model.hpp>
#include <tora/compat/lp_file.hpp>
#include <tora/core/number_formatter.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
void values(const std::vector<double>& v,const tora::core::NumberFormatter& f) {
    std::cout<<'[';for(std::size_t j=0;j<v.size();++j){if(j)std::cout<<',';std::cout<<'"'<<f.format(v[j])<<'"';}std::cout<<']';
}
int main(int argc,char** argv){try {
    if(argc<3)throw std::invalid_argument("Use dual_revised_probe changed.txt baseline_rhs...");
    std::ifstream file(argv[1],std::ios::binary);if(!file)throw std::invalid_argument("Cannot read changed LP");
    const auto document=tora::compat::ToraLpCodec::decode(std::string{std::istreambuf_iterator<char>(file),{}});
    const auto& changed=document.problem;auto baseline=changed;std::vector<double> rhs;
    if(static_cast<std::size_t>(argc-2)!=baseline.constraints.size())throw std::invalid_argument("Baseline RHS count mismatch");
    for(std::size_t i=0;i<baseline.constraints.size();++i){baseline.constraints[i].rhs=std::stod(argv[i+2]);rhs.push_back(changed.constraints[i].rhs);}
    const auto r=tora::lp::reoptimizeDualRevisedRhs(baseline,rhs);
    if(!r.objective)throw std::runtime_error("Expected finite revised-dual optimum");
    const tora::core::NumberFormatter f(document.inputFormat);
    std::cout<<"{\"objective\":\""<<f.format(*r.objective)<<"\",\"values\":";values(r.values,f);
    std::vector<double> contributions,gaps;
    for(std::size_t j=0;j<r.values.size();++j)contributions.push_back(changed.objective[j]*r.values[j]);
    for(const auto& row:changed.constraints){double lhs=0;for(std::size_t j=0;j<r.values.size();++j)lhs+=row.coefficients[j]*r.values[j];gaps.push_back(row.relation==tora::lp::Relation::GreaterEqual?lhs-row.rhs:row.rhs-lhs);}
    std::cout<<",\"contributions\":";values(contributions,f);std::cout<<",\"gaps\":";values(gaps,f);std::cout<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
