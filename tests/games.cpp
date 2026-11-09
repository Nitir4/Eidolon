#include <tora/games/games.hpp>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <stdexcept>
#include <sstream>
#include <iomanip>
using tora::core::Matrix;
using namespace tora::games;
namespace {
void require(bool v,int line){if(!v)throw std::runtime_error("Game backend regression failed at line "+std::to_string(line));}
#define check(value) require(static_cast<bool>(value),__LINE__)
void near(double a,double b){if(!std::isfinite(a) || !std::isfinite(b) || std::abs(a-b)>=1e-8*(1+std::abs(b))){std::ostringstream s;s<<std::setprecision(17)<<"Actual "<<a<<" expected "<<b;throw std::runtime_error(s.str());}}
void certificate(const Matrix& a,const Result& r) {
    check(r.rowStrategy.size()==a.rows() && r.columnStrategy.size()==a.columns());
    near(std::accumulate(r.rowStrategy.begin(),r.rowStrategy.end(),0.),1);
    near(std::accumulate(r.columnStrategy.begin(),r.columnStrategy.end(),0.),1);
    for(auto x:r.rowStrategy)check(x>=-1e-10);
    for(auto x:r.columnStrategy)check(x>=-1e-10);
    for(std::size_t j=0;j<a.columns();++j){double value=0;for(std::size_t i=0;i<a.rows();++i)value+=r.rowStrategy[i]*a(i,j);check(value>=r.value-1e-8*(1+std::abs(r.value)));}
    for(std::size_t i=0;i<a.rows();++i){double value=0;for(std::size_t j=0;j<a.columns();++j)value+=r.columnStrategy[j]*a(i,j);check(value<=r.value+1e-8*(1+std::abs(r.value)));}
    check(r.lowerBound<=r.upperBound+1e-8);near(r.value,(r.lowerBound+r.upperBound)/2);
}
}
int main() {
    Matrix pennies(2,2,{1,-1,-1,1});auto r=solve(pennies);near(r.value,0);near(r.rowStrategy[0],.5);near(r.columnStrategy[0],.5);check(r.saddlePoints.empty());certificate(pennies,r);
    Matrix asymmetric(2,2,{3,0,1,2});r=solve(asymmetric);near(r.value,1.5);near(r.rowStrategy[0],.25);near(r.columnStrategy[0],.5);certificate(asymmetric,r);
    const auto formulation=rowPlayerLp(asymmetric);check(formulation.variables.size()==3 && formulation.constraints.size()==3);
    check(formulation.variables[0].unrestricted && !formulation.variables[1].unrestricted && formulation.objective==std::vector<double>({1,0,0}));
    check(formulation.constraints[0].coefficients==std::vector<double>({1,-3,-1}));
    check(formulation.constraints[1].coefficients==std::vector<double>({1,0,-2}));
    check(formulation.constraints[2].coefficients==std::vector<double>({0,1,1}) && formulation.constraints[2].relation==tora::lp::Relation::Equal && formulation.constraints[2].rhs==1);
    Matrix pure(2,2,{4,3,2,1});r=solve(pure);near(r.value,3);check(r.rowStrategy[0]==1 && r.columnStrategy[1]==1 && r.saddlePoints.size()==1);certificate(pure,r);
    Matrix zero(3,2);r=solve(zero);check(r.saddlePoints.size()==6);certificate(zero,r);
    Matrix dominated(3,3,{3,0,0,1,2,2,-2,-1,-1});r=solve(dominated);near(r.value,1.5);near(r.rowStrategy[2],0);certificate(dominated,r);
    std::mt19937 random(20261015);
    for(int sample=0;sample<200;++sample) {
        Matrix a(2,2);for(std::size_t i=0;i<2;++i)for(std::size_t j=0;j<2;++j)a(i,j)=int(random()%31)-15;
        const auto result=solve(a);certificate(a,result);
        if(result.saddlePoints.empty()) {
            const double denominator=a(0,0)-a(0,1)-a(1,0)+a(1,1);
            near(result.value,(a(0,0)*a(1,1)-a(0,1)*a(1,0))/denominator);
            near(result.rowStrategy[0],(a(1,1)-a(1,0))/denominator);
            near(result.columnStrategy[0],(a(1,1)-a(0,1))/denominator);
        }
    }
    for(int sample=0;sample<100;++sample) {
        Matrix a(1+random()%5,1+random()%5),shifted(a.rows(),a.columns()),opposite(a.columns(),a.rows());
        for(std::size_t i=0;i<a.rows();++i)for(std::size_t j=0;j<a.columns();++j){a(i,j)=int(random()%41)-20;shifted(i,j)=3*a(i,j)+7;opposite(j,i)=-a(i,j);}
        const auto result=solve(a);certificate(a,result);near(solve(shifted).value,3*result.value+7);near(solve(opposite).value,-result.value);
    }
    Matrix huge(2,2,{1e308,-1e308,-1e308,1e308});r=solve(huge);near(r.rowStrategy[0],.5);near(r.value,0);
    Matrix tiny(2,2,{3e-200,0,1e-200,2e-200});r=solve(tiny);near(r.value/1e-200,1.5);
    Matrix symmetric(100,100);for(std::size_t i=0;i<100;++i)for(std::size_t j=0;j<100;++j)symmetric(i,j)=i==j?1:-1;
    r=solve(symmetric);near(r.value,-.98);certificate(symmetric,r);
    for(int k=0;k<4;++k) {
        bool rejected=false;try{if(k==0)(void)solve({});if(k==1)(void)solve(Matrix(101,1));if(k==2)(void)solve(Matrix(1,1,{INFINITY}));if(k==3)(void)solve(Matrix(1,1,{NAN}));}catch(const std::invalid_argument&){rejected=true;}check(rejected);
    }
    bool limited=false;try{(void)solve(pennies,tora::core::Tolerance{},0);}catch(const std::runtime_error&){limited=true;}check(limited);
    bool invalidFormulation=false;try{(void)rowPlayerLp(Matrix(1,1,{NAN}));}catch(const std::invalid_argument&){invalidFormulation=true;}check(invalidFormulation);
}
