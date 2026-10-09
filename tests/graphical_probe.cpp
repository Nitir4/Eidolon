#include <tora/lp/graphical.hpp>
#include <iostream>
#include <locale>
#include <iomanip>
#include <stdexcept>
using namespace tora::lp;
static double number(){std::string s;std::cin>>s;return std::stod(s);}
static void text(const std::string& s){std::cout<<'"';for(char c:s){if(c=='"'||c=='\\')std::cout<<'\\';if(c=='\n')std::cout<<"\\n";else std::cout<<c;}std::cout<<'"';}
int main(){try{
    std::cin.imbue(std::locale::classic());std::cout.imbue(std::locale::classic());
    std::size_t n,m;std::string sense;std::cin>>n>>m>>sense;auto p=LpProblem::blank(n,m);p.sense=sense=="max"?ObjectiveSense::Maximize:ObjectiveSense::Minimize;
    for(auto& v:p.variables){v.lowerBound=number();v.upperBound=number();std::cin>>v.unrestricted;}
    for(auto& c:p.objective)c=number();
    for(auto& r:p.constraints){std::string relation;std::cin>>relation;r.relation=relation=="<="?Relation::LessEqual:relation==">="?Relation::GreaterEqual:Relation::Equal;r.rhs=number();for(auto& a:r.coefficients)a=number();}
    int sci,nDigits,d;std::cin>>sci>>nDigits>>d;
    GraphicalSession graph(p,{sci?tora::core::Notation::Scientific:tora::core::Notation::Decimal,nDigits,d});graph.drawAll();const auto& r=graph.result();
    std::cout<<"{\"objective\":";if(r.message.empty())text(r.objective);else std::cout<<"null";
    std::cout<<",\"x1\":";if(r.message.empty())text(r.x1);else std::cout<<"null";
    std::cout<<",\"x2\":";if(r.message.empty())text(r.x2);else std::cout<<"null";
    std::cout<<",\"message\":";text(r.message);
    std::cout<<",\"range_max\":"<<std::setprecision(17)<<graph.rangeMax()<<",\"range_min\":"<<graph.rangeMin()<<",\"grid_step\":"<<graph.gridStep()<<",\"tick_step\":"<<graph.tickStep();
    graph.zoom(1);std::cout<<",\"zoom_in_max\":"<<graph.rangeMax();graph.zoom(-1);std::cout<<",\"zoom_out_max\":"<<graph.rangeMax()<<'}';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
