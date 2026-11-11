#include <tora/lp/postoptimal.hpp>
#include <tora/compat/lp_file.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
namespace {
tora::compat::LpDocument read(const char* path) {
    std::ifstream f(path,std::ios::binary);if(!f)throw std::invalid_argument("Cannot read LP fixture");
    return tora::compat::ToraLpCodec::decode(std::string{std::istreambuf_iterator<char>(f),{}});
}
void values(const std::vector<double>& numbers,const tora::core::NumberFormatter& f) {
    std::cout<<'[';for(std::size_t i=0;i<numbers.size();++i){if(i)std::cout<<',';std::cout<<'"'<<f.format(numbers[i])<<'"';}std::cout<<']';
}
void outcome(const tora::lp::PostOptimalOutcome& r,const tora::core::NumberFormatter& f) {
    std::cout<<"{\"objective\":";if(r.objective)std::cout<<'"'<<f.format(*r.objective)<<'"';else std::cout<<"null";
    std::cout<<",\"values\":";values(r.values,f);std::cout<<'}';
}
}
int main(int argc,char** argv) {
    try {
        if(argc!=3)throw std::invalid_argument("Use postoptimal_probe baseline.txt changed.txt");
        const auto base=read(argv[1]),changed=read(argv[2]);std::vector<double> rhs;
        if(base.problem.variables.size()!=changed.problem.variables.size() || base.problem.constraints.size()!=changed.problem.constraints.size())throw std::invalid_argument("Only Cj/RHS may change");
        for(std::size_t i=0;i<base.problem.constraints.size();++i)if(base.problem.constraints[i].coefficients!=changed.problem.constraints[i].coefficients || base.problem.constraints[i].relation!=changed.problem.constraints[i].relation)throw std::invalid_argument("Constraint matrix/relation changed");
        for(std::size_t j=0;j<base.problem.variables.size();++j) {
            const auto& a=base.problem.variables[j];const auto& b=changed.problem.variables[j];
            if(a.lowerBound!=b.lowerBound || a.upperBound!=b.upperBound || a.unrestricted!=b.unrestricted)throw std::invalid_argument("Variable bounds changed");
        }
        for(const auto& row:changed.problem.constraints)rhs.push_back(row.rhs);
        const auto result=tora::lp::analyzeChanges(base.problem,changed.problem.objective,rhs);
        const tora::core::NumberFormatter f(changed.inputFormat);
        std::cout<<"{\"before\":";outcome(result.before,f);std::cout<<",\"after\":";outcome(result.after,f);
        std::cout<<",\"retained\":{\"primal_feasible\":"<<(result.retained.primalFeasible?"true":"false")
            <<",\"dual_feasible\":"<<(result.retained.dualFeasible?"true":"false")<<",\"objective\":\""<<f.format(result.retained.objective)<<"\",\"values\":";
        values(result.retained.values,f);std::cout<<"}}\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
