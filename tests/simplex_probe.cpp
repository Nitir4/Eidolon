#include <tora/lp/all_slack.hpp>
#ifdef TORA_M_METHOD_PROBE
#include <tora/lp/m_method.hpp>
#endif
#ifdef TORA_TWO_PHASE_PROBE
#include <tora/lp/two_phase.hpp>
#endif
#ifdef TORA_BOUNDED_PROBE
#include <tora/lp/bounded.hpp>
#endif
#ifdef TORA_DUAL_PROBE
#include <tora/lp/dual.hpp>
#include <tora/lp/m_method.hpp>
#include <memory>
#endif
#include <iomanip>
#include <iostream>
#include <locale>
#include <stdexcept>

using namespace tora::lp;
double number(std::istream& stream) {std::string text;stream>>text;return std::stod(text);}
void quoted(const std::string& text) {
    std::cout<<'"';
    for(const char ch:text) {
        if(ch=='\n')std::cout<<"\\n";
        else if(ch=='\r')std::cout<<"\\r";
        else if(ch=='\t')std::cout<<"\\t";
        else {if(ch=='"' || ch=='\\')std::cout<<'\\';std::cout<<ch;}
    }
    std::cout<<'"';
}
int main() {
    try {
        std::cin.imbue(std::locale::classic());std::cout.imbue(std::locale::classic());
        std::size_t n,m;std::string sense;std::cin>>n>>m>>sense;
        auto p=LpProblem::blank(n,m);p.sense=sense=="max"?ObjectiveSense::Maximize:ObjectiveSense::Minimize;
        for(auto& v:p.variables){v.lowerBound=number(std::cin);v.upperBound=number(std::cin);std::cin>>v.unrestricted;}
        for(auto& cost:p.objective)cost=number(std::cin);
        for(auto& r:p.constraints){std::string relation;std::cin>>relation;r.relation=relation=="<="?Relation::LessEqual:relation==">="?Relation::GreaterEqual:Relation::Equal;
            r.rhs=number(std::cin);for(auto& a:r.coefficients)a=number(std::cin);}
#if !defined(TORA_M_METHOD_PROBE) && !defined(TORA_TWO_PHASE_PROBE) && !defined(TORA_BOUNDED_PROBE) && !defined(TORA_DUAL_PROBE)
        if(AllSlackSession::requiresArtificial(p)){std::cout<<"{\"requires_artificial\":true}";return 0;}
#elif defined(TORA_DUAL_PROBE)
        std::string route;std::cin>>route;const double penalty=number(std::cin);
#elif defined(TORA_M_METHOD_PROBE)
        const double penalty=number(std::cin);
#endif
        int scientific,nDigits,dDigits;std::cin>>scientific>>nDigits>>dDigits;
        std::string operation;std::cin>>operation;
#ifdef TORA_DUAL_PROBE
        std::unique_ptr<PrimalSimplexSession> selected;
        if(route=="m")selected=std::make_unique<MMethodSession>(p,penalty);
        else selected=std::make_unique<DualSession>(p);
        auto& solver=*selected;
#elif defined(TORA_M_METHOD_PROBE)
        MMethodSession solver(p,penalty);
#elif defined(TORA_TWO_PHASE_PROBE)
        TwoPhaseSession solver(p);
#elif defined(TORA_BOUNDED_PROBE)
        BoundedSession solver(p);
#else
        AllSlackSession solver(p);
#endif
        if(operation!="initial")solver.solveAll();
        const auto noticeBeforeAlternative=solver.terminalMessage();
        if(operation=="alternative"){std::size_t column;std::cin>>column;solver.selectEntering(column);solver.next();}
        std::cout<<"{\"requires_artificial\":"<<(AllSlackSession::requiresArtificial(p)?"true":"false")<<",\"tables\":[";
        const auto format=tora::core::NumberFormat{scientific?tora::core::Notation::Scientific:tora::core::Notation::Decimal,nDigits,dDigits};
        for(std::size_t k=0;k<solver.history().size();++k){if(k)std::cout<<',';const auto table=solver.display(k,format);
            std::cout<<"{\"heading\":";quoted(table.heading);std::cout<<",\"columns\":[";
            for(std::size_t j=0;j<table.columns.size();++j){if(j)std::cout<<',';quoted(table.columns[j]);}std::cout<<"],\"rows\":[";
            for(std::size_t i=0;i<table.rows.size();++i){if(i)std::cout<<',';std::cout<<"{\"label\":";quoted(table.rows[i].label);std::cout<<",\"values\":[";
                for(std::size_t j=0;j<table.rows[i].values.size();++j){if(j)std::cout<<',';quoted(table.rows[i].values[j]);}std::cout<<"]}";}
            std::cout<<"],\"entering\":";if(table.entering)quoted(*table.entering);else std::cout<<"null";
            std::cout<<",\"leaving\":";if(table.leaving)quoted(*table.leaving);else std::cout<<"null";std::cout<<'}';}
        std::cout<<"],\"message\":";
#ifdef TORA_DUAL_PROBE
        quoted(operation=="alternative" && solver.method()==PrimalMethod::Dual?noticeBeforeAlternative:solver.terminalMessage());
#else
        (void)noticeBeforeAlternative;quoted(solver.terminalMessage());
#endif
        std::cout<<'}';
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
