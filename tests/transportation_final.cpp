#include <tora/transportation/final_summary.hpp>
#include <functional>
#include <iostream>
#include <stdexcept>
using namespace tora::transportation;
static void check(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
static void rejects(const std::function<void()>& action) {
    try {action();}catch(const std::exception&){return;}
    throw std::runtime_error("Invalid final summary accepted");
}
int main() {
    try {
        auto p=Problem::blank(2,3);p.title="Pilot";p.costs={{2,3,1},{5,4,8}};p.supply={20,30};p.demand={10,25,15};
        const auto original=p;const auto result=finalSummary(p);
        check(result.title=="Pilot" && result.iteration=="1" && result.objective=="150.00","Observed summary header mismatch");
        check(result.rows==std::vector<std::vector<std::string>>({{"S1:","D1:","5","2.00","10.00"},{"S1:","D3:","15","1.00","15.00"},{"S2:","D1:","5","5.00","25.00"},{"S2:","D2:","25","4.00","100.00"}}),"Observed summary rows mismatch");
        check(p.costs==original.costs && p.supply==original.supply && p.demand==original.demand,"Summary mutated problem");
        p.sourceNames={"Alpha","Beta"};p.destinationNames={"Depot","Hub","Port"};
        const auto names=finalSummary(p);check(names.rows[0][0]=="S1: Alpha" && names.rows[0][1]=="D1: Depot","Summary lost aliases");
        p.supply={0,50};const auto zero=finalSummary(p);
        check(zero.rows.size()==4 && zero.rows[0]==std::vector<std::string>({"S1: Alpha","D3: Port","0","1.00","0.00"}),"Zero basic summary row missing");
        for(const auto& [amount,expected]:std::vector<std::pair<double,std::string>>{{0.125,"0"},{0.5,"0"},{0.75,"0"},{0.984375,"0"},{1.125,"1"},{1.5,"2"},{2.5,"3"},{3.5,"4"},{4.5,"5"}}) {
            auto single=Problem::blank(1,1);single.costs={{2}};single.supply={amount};single.demand={amount};
            const auto s=finalSummary(single);check(s.rows[0][2]==expected,"Observed fractional summary amount rule mismatch");
        }
        auto fractional=Problem::blank(1,1);fractional.costs={{2.25}};fractional.supply={0.75};fractional.demand={0.75};
        const auto f=finalSummary(fractional);check(f.rows[0][2]=="0" && f.rows[0][4]=="1.69" && f.objective=="1.69","Contribution was computed from displayed amount");
        p.supply={20,30};p.demand={11,25,15};rejects([&]{finalSummary(p);});
        p.demand={10,25,15};p.costs={{1e308,1e308,1e308},{1e308,1e308,1e308}};rejects([&]{finalSummary(p);});
        std::cout<<"Final summary, zero basis, aliases, fractional display and safety checks passed\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
