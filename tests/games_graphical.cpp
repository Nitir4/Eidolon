#include <tora/games/graphical.hpp>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <stdexcept>

using namespace tora::games;
using tora::core::Matrix;
namespace {
void check(bool value){if(!value)throw std::runtime_error("Graphical Game regression failed");}
void near(double actual,double expected){check(std::isfinite(actual) && std::isfinite(expected) && std::abs(actual-expected)<1e-8*(1+std::abs(expected)));}
void verify(const Matrix& a,const GraphicalResult& result) {
    const auto row=result.player==GraphicalPlayer::RowA;
    check(result.lines.size()==(row?a.columns():a.rows()));
    check(!result.envelope.empty());near(result.envelope.front().from,0);near(result.envelope.back().to,1);
    for(std::size_t i=0;i<result.envelope.size();++i) {
        const auto& segment=result.envelope[i];check(segment.from<segment.to && segment.strategy<result.lines.size());
        if(i)near(segment.from,result.envelope[i-1].to);
        for(const auto t:{segment.from,(segment.from+segment.to)/2,segment.to}) {
            const auto payoffs=result.expectedPayoffs(t);
            const auto bound=row?*std::min_element(payoffs.begin(),payoffs.end()):*std::max_element(payoffs.begin(),payoffs.end());
            near(payoffs[segment.strategy],bound);
        }
    }
    const auto& e=result.equilibrium;
    near(std::accumulate(e.rowStrategy.begin(),e.rowStrategy.end(),0.),1);
    near(std::accumulate(e.columnStrategy.begin(),e.columnStrategy.end(),0.),1);
    for(auto p:e.rowStrategy)check(p>=0 && p<=1);
    for(auto p:e.columnStrategy)check(p>=0 && p<=1);
    for(std::size_t j=0;j<a.columns();++j){double v=0;for(std::size_t i=0;i<a.rows();++i)v+=e.rowStrategy[i]*a(i,j);check(v>=e.value-1e-8*(1+std::abs(e.value)));}
    for(std::size_t i=0;i<a.rows();++i){double v=0;for(std::size_t j=0;j<a.columns();++j)v+=e.columnStrategy[j]*a(i,j);check(v<=e.value+1e-8*(1+std::abs(e.value)));}
    near(e.value,solve(a).value); // Independent incremental-LP backend.
    near(row?e.rowStrategy[0]:e.columnStrategy[0],result.probability);
    check(result.optimalInterval[0]<=result.probability && result.probability<=result.optimalInterval[1]);
    for(int sample=0;sample<=100;++sample) {
        const auto values=result.expectedPayoffs(sample/100.);
        const auto v=row?*std::min_element(values.begin(),values.end()):*std::max_element(values.begin(),values.end());
        check(row?v<=e.value+1e-8*(1+std::abs(e.value)):v>=e.value-1e-8*(1+std::abs(e.value)));
    }
}
}
int main() {
    Matrix mixed(2,2,{3,0,1,2});auto r=solveGraphical(mixed);near(r.probability,.25);near(r.equilibrium.value,1.5);
    near(r.lines[0].atZero,1);near(r.lines[0].atOne,3);verify(mixed,r);
    r=solveGraphical(mixed,GraphicalPlayer::ColumnB);near(r.probability,.5);verify(mixed,r);
    Matrix rectangular(2,3,{4,1,-1,0,2,3});r=solveGraphical(rectangular);near(r.probability,.375);near(r.equilibrium.value,1.5);near(r.equilibrium.columnStrategy[1],0);verify(rectangular,r);
    Matrix pure(2,2,{4,3,2,1});r=solveGraphical(pure);near(r.probability,1);near(r.equilibrium.value,3);verify(pure,r);
    Matrix flat(2,3,{-1,1,0,1,-1,0});r=solveGraphical(flat);near(r.probability,.5);verify(flat,r);
    Matrix reversedPure(2,2,{2,1,4,3});r=solveGraphical(reversedPure);near(r.probability,0);verify(reversedPure,r);
    Matrix interval(2,3,{.75,-.25,0,-.25,.75,0});r=solveGraphical(interval);near(r.optimalInterval[0],.25);near(r.optimalInterval[1],.75);verify(interval,r);
    Matrix zero(2,4);r=solveGraphical(zero);near(r.optimalInterval[0],0);near(r.optimalInterval[1],1);verify(zero,r);
    Matrix duplicate(2,3,{3,0,0,1,2,2});r=solveGraphical(duplicate);near(r.probability,.25);verify(duplicate,r);
    std::mt19937 random(20261016);
    for(int sample=0;sample<300;++sample) {
        const bool row=sample%2==0;Matrix a(row?2:1+random()%7,row?1+random()%7:2);
        for(std::size_t i=0;i<a.rows();++i)for(std::size_t j=0;j<a.columns();++j)a(i,j)=int(random()%41)-20;
        r=solveGraphical(a,row?GraphicalPlayer::RowA:GraphicalPlayer::ColumnB);verify(a,r);
    }
    Matrix huge(2,2,{1e308,-1e308,-1e308,1e308});r=solveGraphical(huge);near(r.probability,.5);near(r.equilibrium.value,0);near(r.expectedPayoffs(.5)[0],0);
    Matrix tiny(2,2,{3e-200,0,1e-200,2e-200});r=solveGraphical(tiny);near(r.probability,.25);near(r.equilibrium.value/1e-200,1.5);
    Matrix large(2,100);for(std::size_t j=0;j<100;++j){large(0,j)=int(j%11)-5;large(1,j)=int(j%13)-6;}verify(large,solveGraphical(large));
    Matrix largeColumns(100,2);for(std::size_t i=0;i<100;++i){largeColumns(i,0)=large(0,i);largeColumns(i,1)=large(1,i);}verify(largeColumns,solveGraphical(largeColumns,GraphicalPlayer::ColumnB));
    for(int test=0;test<9;++test) {
        bool rejected=false;try{
            if(test==0)(void)solveGraphical(Matrix(3,3));
            if(test==1)(void)solveGraphical(Matrix(2,101));
            if(test==2)(void)solveGraphical(Matrix(2,1,{NAN,1}));
            if(test==3)(void)solveGraphical(mixed,static_cast<GraphicalPlayer>(99));
            if(test==4)(void)r.expectedPayoffs(-.1);
            if(test==5)(void)r.expectedPayoffs(1.1);
            if(test==6)(void)r.expectedPayoffs(NAN);
            if(test==7)(void)solveGraphical(Matrix(2,1,{INFINITY,0}));
            if(test==8)(void)solveGraphical(Matrix(2,3),GraphicalPlayer::ColumnB);
        }catch(const std::invalid_argument&){rejected=true;}check(rejected);
    }
}
