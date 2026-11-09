#include <tora/games/games.hpp>
#include <tora/games/graphical.hpp>
#include <tora/core/number_formatter.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>

namespace {
// Test-only reader for authenticated 9000 records. Production file editing
// and round-trip compatibility are separate integration work.
std::string line(std::istream& input) {
    std::string text;
    if(!std::getline(input,text))throw std::invalid_argument("Truncated Game fixture");
    if(!text.empty() && text.back()=='\r')text.pop_back();
    return text;
}
int integer(std::istream& input) {
    const auto text=line(input);std::size_t end=0;const auto value=std::stoi(text,&end);
    if(end!=text.size())throw std::invalid_argument("Invalid Game integer");
    return value;
}
std::string quoted(std::istream& input) {
    std::istringstream source(line(input));std::string value;
    if(!(source>>std::quoted(value)) || !(source>>std::ws).eof())throw std::invalid_argument("Invalid quoted Game field");
    return value;
}
void values(const std::vector<double>& numbers,const tora::core::NumberFormatter& formatter) {
    std::cout<<'[';
    for(std::size_t i=0;i<numbers.size();++i){if(i)std::cout<<',';std::cout<<'"'<<formatter.format(numbers[i])<<'"';}
    std::cout<<']';
}
}
int main(int argc,char** argv) {
    try {
        if(argc!=2 && !(argc==3 && std::string(argv[2])=="graphical"))throw std::invalid_argument("Use games_probe fixture.txt [graphical]");
        std::ifstream input(argv[1],std::ios::binary);
        if(!input || integer(input)!=9000)throw std::invalid_argument("Expected original Game record");
        tora::core::NumberFormat settings;settings.n=integer(input);settings.d=integer(input);
        const auto notation=integer(input);
        if(notation!=1 && notation!=2)throw std::invalid_argument("Invalid Game notation");
        settings.notation=notation==1?tora::core::Notation::Decimal:tora::core::Notation::Scientific;
        const auto title=quoted(input);(void)title;
        const auto rows=integer(input),columns=integer(input);
        if(rows<1 || rows>100 || columns<1 || columns>100)throw std::invalid_argument("Invalid Game size");
        tora::core::Matrix payoffs(static_cast<std::size_t>(rows),static_cast<std::size_t>(columns));
        for(int i=0;i<rows;++i)for(int j=0;j<columns;++j) {
            const auto text=quoted(input);std::size_t end=0;const auto value=std::stod(text,&end);
            if(end!=text.size())throw std::invalid_argument("Invalid Game payoff");
            payoffs(static_cast<std::size_t>(i),static_cast<std::size_t>(j))=value;
        }
        if(input.peek()!=std::char_traits<char>::eof())throw std::invalid_argument("Unexpected Game fields");
        const auto graph=argc==3?std::optional<tora::games::GraphicalResult>(tora::games::solveGraphical(payoffs)):std::nullopt;
        const auto result=graph?graph->equilibrium:tora::games::solve(payoffs);
        const tora::core::NumberFormatter formatter(settings);
        std::cout<<"{\"value\":\""<<formatter.format(result.value)<<"\",\"row_strategy\":";values(result.rowStrategy,formatter);
        std::cout<<",\"column_strategy\":";values(result.columnStrategy,formatter);
        if(graph) {
            std::cout<<",\"lines\":[";
            for(std::size_t i=0;i<graph->lines.size();++i) {
                if(i)std::cout<<',';
                const auto& line=graph->lines[i];values({line.atZero,line.atOne-line.atZero},formatter);
            }
            std::cout<<"]}\n";return 0;
        }
        const auto lp=tora::games::rowPlayerLp(payoffs);
        std::cout<<",\"formulation\":{\"objective\":";values(lp.objective,formatter);
        std::cout<<",\"rows\":[";
        for(std::size_t i=0;i<lp.constraints.size();++i) {
            if(i)std::cout<<',';
            std::cout<<'[';const auto& row=lp.constraints[i];
            for(const auto value:row.coefficients)std::cout<<'"'<<formatter.format(value)<<"\",";
            const auto relation=row.relation==tora::lp::Relation::Equal?"=":row.relation==tora::lp::Relation::LessEqual?"<=":">=";
            std::cout<<'"'<<relation<<"\",\""<<formatter.format(row.rhs)<<"\"]";
        }
        std::cout<<"],\"unrestricted\":[";
        for(std::size_t i=0;i<lp.variables.size();++i){if(i)std::cout<<',';std::cout<<'"'<<(lp.variables[i].unrestricted?'y':'n')<<'"';}
        std::cout<<"]}}\n";
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
