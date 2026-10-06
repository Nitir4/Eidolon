#include <tora/lp/problem.hpp>
#include <tora/lp/validation.hpp>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace tora::lp;
void require(bool condition, const std::string& message) { if (!condition) throw std::runtime_error(message); }
Scalar readNumber(std::istream& in) {
    std::string token; in >> token;
    if (token == "inf") return std::numeric_limits<Scalar>::infinity();
    std::istringstream value(token); value.imbue(std::locale::classic());
    Scalar result = 0; value >> result;
    require(!value.fail() && value.peek() == std::char_traits<char>::eof(), "Bad original fixture number");
    return result;
}
// Reader for our original fixture grammar only. It cannot open TORA files.
std::vector<LpProblem> readFixtures(const std::string& path) {
    std::ifstream input(path); input.imbue(std::locale::classic());
    require(input.good(), "Cannot read original model transcriptions");
    std::vector<LpProblem> cases; std::string word;
    while (input >> word) {
        if (word.front() == '#') { std::getline(input, word); continue; }
        require(word == "case", "Fixture case marker");
        std::string id, title, sense; std::size_t n=0,m=0;
        input >> std::quoted(id) >> std::quoted(title) >> sense >> n >> m;
        require(sense == "max" || sense == "min", "Fixture direction");
        auto model=LpProblem::blank(n,m); model.title=title;
        model.sense=sense=="max" ? ObjectiveSense::Maximize : ObjectiveSense::Minimize;
        for (auto& variable:model.variables) {
            input >> word;require(word=="variable", "Fixture variable marker");
            input >> std::quoted(variable.name);variable.lowerBound=readNumber(input);variable.upperBound=readNumber(input);
            input >> word;require(word=="y" || word=="n", "Fixture sign flag");variable.unrestricted=word=="y";
        }
        input >> word;require(word=="objective", "Fixture objective marker");
        for(auto& coefficient:model.objective)coefficient=readNumber(input);
        for(auto& row:model.constraints) {
            input >> word;require(word=="constraint", "Fixture constraint marker");
            input >> word;require(word=="<=" || word==">=" || word=="=", "Fixture relation");
            row.relation=word=="<="?Relation::LessEqual:(word==">="?Relation::GreaterEqual:Relation::Equal);
            row.rhs=readNumber(input);for(auto& coefficient:row.coefficients)coefficient=readNumber(input);
        }
        input >> word;require(word=="end" && !input.fail(), "Complete fixture record");
        const auto before=model;
        const auto report=LpValidator::validate(model);
        require(report.valid() && report.issues.empty(), id+": captured input rejected");
        require(model.title==before.title && model.sense==before.sense && model.objective==before.objective, "Validation changed original objective/title");
        for(std::size_t j=0;j<n;++j) {
            require(model.variableLabel(j)=="x"+std::to_string(j+1), "Variable column order");
            require(model.variables[j].name==before.variables[j].name && model.variables[j].lowerBound==before.variables[j].lowerBound && model.variables[j].upperBound==before.variables[j].upperBound && model.variables[j].unrestricted==before.variables[j].unrestricted, "Validation changed bounds/names/signs");
        }
        for(std::size_t i=0;i<m;++i) {
            require(model.constraintLabel(i)=="Constr "+std::to_string(i+1), "Constraint order");
            require(model.constraints[i].coefficients==before.constraints[i].coefficients && model.constraints[i].relation==before.constraints[i].relation && model.constraints[i].rhs==before.constraints[i].rhs, "Validation normalized a constraint");
        }
        cases.push_back(model);
    }
    return cases;
}
}
int main(int argc,char** argv) {
    try {
        require(argc==2, "Pass original model fixture file");
        const auto cases=readFixtures(argv[1]);require(cases.size()==22, "All 22 reference models required");
        const auto& normal=cases.at(0);
        require(normal.objective==std::vector<Scalar>({3,2}) && normal.constraints[0].rhs==4, "Normal source fields");
        require(LpValidator::validate(cases.at(2)).valid(), "Infeasibility is solver status, not malformed input");
        require(LpValidator::validate(cases.at(3)).valid(), "Unboundedness is solver status");
        require(cases.at(5).constraints[0].relation==Relation::Equal, "Equality retained");
        require(cases.at(7).sense==ObjectiveSense::Minimize, "Minimization retained");
        require(cases.at(11).constraints[0].rhs==-1 && cases.at(12).objective[1]==-1, "Negative RHS/cost retained");
        require(cases.at(18).variables[0].name=="decision" && cases.at(18).variableLabel(0)=="x1", "Name alias separate from x1 header");
        require(cases.at(19).variables[0].unrestricted && cases.at(19).variables[0].lowerBound==-1 && cases.at(19).constraints[1].rhs==-4, "Unrestricted data not transformed");
        auto blank=LpProblem::blank(2,3);
        require(blank.title.empty() && blank.sense==ObjectiveSense::Maximize && blank.objective==std::vector<Scalar>({0,0}), "New-problem objective defaults");
        for(const auto& v:blank.variables)require(v.name.empty() && v.lowerBound==0 && std::isinf(v.upperBound) && v.upperBound>0 && !v.unrestricted, "New-problem bound/sign defaults");
        for(const auto& r:blank.constraints)require(r.coefficients==std::vector<Scalar>({0,0}) && r.rhs==0 && r.relation==Relation::LessEqual, "New constraint defaults");
        require(LpValidator::validate(LpProblem::blank(100,100)).valid(), "Observed 100/100 dimensions");
        auto copy=normal;copy.variables[0].name="edited";copy.objective[0]=-99;copy.constraints[0].coefficients[0]=77;
        require(normal.variables[0].name.empty() && normal.objective[0]==3 && normal.constraints[0].coefficients[0]==1, "Independent document snapshots");
        bool rejected=false;try{(void)normal.variableLabel(normal.variables.size());}catch(const std::out_of_range&){rejected=true;}
        require(rejected,"Variable labels check index before numbering");
        rejected=false;try{(void)normal.constraintLabel(normal.constraints.size());}catch(const std::out_of_range&){rejected=true;}
        require(rejected,"Constraint labels check index before numbering");
        std::cout<<cases.size()<<" observed LP models preserve direction, ordered coefficients, relations, RHS, aliases and bounds/signs\n";
    } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
