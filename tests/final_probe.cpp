#include <tora/lp/final_solution.hpp>
#include <iostream>
#include <locale>
#include <stdexcept>
using namespace tora::lp;
double number(std::istream& s){std::string t;s>>t;return std::stod(t);}
void quoted(const std::string& t){std::cout<<'"';for(char c:t){if(c=='\n')std::cout<<"\\n";else if(c=='\r')std::cout<<"\\r";else if(c=='\t')std::cout<<"\\t";else{if(c=='"'||c=='\\')std::cout<<'\\';std::cout<<c;}}std::cout<<'"';}
int main(){try{std::cin.imbue(std::locale::classic());std::cout.imbue(std::locale::classic());
        std::size_t n,m;std::string sense;std::cin>>n>>m>>sense;
        auto p=LpProblem::blank(n,m);p.sense=sense=="max"?ObjectiveSense::Maximize:ObjectiveSense::Minimize;
        for(auto& v:p.variables){v.lowerBound=number(std::cin);v.upperBound=number(std::cin);std::cin>>v.unrestricted;}
        for(auto& cost:p.objective)cost=number(std::cin);
        for(auto& r:p.constraints){std::string relation;std::cin>>relation;r.relation=relation=="<="?Relation::LessEqual:relation==">="?Relation::GreaterEqual:Relation::Equal;
            r.rhs=number(std::cin);for(auto& a:r.coefficients)a=number(std::cin);}

        int scientific,nDigits,dDigits;std::cin>>scientific>>nDigits>>dDigits;
        FinalSolutionSession session(p);
        const auto d=session.display({scientific?tora::core::Notation::Scientific:tora::core::Notation::Decimal,nDigits,dDigits}
#ifdef TORA_SENSITIVITY_PROBE
        ,true
#endif
        );
        std::cout<<"{\"heading\":";quoted(d.heading);std::cout<<",\"iteration\":"<<d.iteration<<",\"objective_label\":";quoted(d.objectiveLabel);
        std::cout<<",\"objective_value\":";if(d.objectiveValue)quoted(*d.objectiveValue);else std::cout<<"null";
        std::cout<<",\"message\":";quoted(d.message);std::cout<<",\"sections\":[";
        for(std::size_t k=0;k<d.sections.size();++k){if(k)std::cout<<',';const auto& section=d.sections[k];std::cout<<"{";if(!section.heading.empty()){std::cout<<"\"heading\":";quoted(section.heading);std::cout<<',';}std::cout<<"\"columns\":[";
            for(std::size_t j=0;j<section.columns.size();++j){if(j)std::cout<<',';quoted(section.columns[j]);}std::cout<<"],\"rows\":[";
            for(std::size_t i=0;i<section.rows.size();++i){if(i)std::cout<<',';std::cout<<"{\"label\":";quoted(section.rows[i].label);std::cout<<",\"values\":[";
                for(std::size_t j=0;j<section.rows[i].values.size();++j){if(j)std::cout<<',';quoted(section.rows[i].values[j]);}std::cout<<"]}";}
            std::cout<<"]}";}
        std::cout<<"]}";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
