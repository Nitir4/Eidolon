#include <tora/queuing/transient.hpp>
#include <tora/core/number_formatter.hpp>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
using namespace tora::queuing;
void check(bool b){if(!b)throw std::runtime_error("Queue transient regression failed");}
void near(long double x,long double y,long double eps=1e-11){check(std::abs(x-y)<=eps*(1+std::abs(y)));}
int main(){
 auto r=pureBirth(2);near(r.mean,2);near(probability(r,0),std::exp(-2.));near(probability(r,2),2*std::exp(-2.));near(cumulativeProbability(r,3),19.0/3*std::exp(-2.));
 tora::core::NumberFormatter f({tora::core::Notation::Decimal,5,5});check(f.format(probability(r,0))=="0.13534");check(f.format(probability(r,3))=="0.18045");
 r=pureDeath(1,3);const double e=std::exp(-1.);near(probability(r,0),1-2.5*e);near(probability(r,1),.5*e);near(probability(r,2),e);near(probability(r,3),e);near(r.mean,5.5*e);near(r.mean-3*probability(r,3),2.5*e);check(f.format(r.mean-3*probability(r,3))=="0.91970");
 auto zero=pureDeath(0,3,1);near(probability(zero,3),1);near(zero.mean,3);near(pureBirth(0).mean,0);
 std::mt19937 gen(20261018);for(int sample=0;sample<500;++sample){const double rate=(gen()%2000+1)/100.;const std::size_t n=gen()%30+1;const double time=(gen()%300+1)/100.;const auto birth=pureBirth(rate,time);const auto death=pureDeath(rate,n,time);long double mass=0,mean=0;for(std::size_t k=0;k<birth.probabilities.size();++k){mass+=birth.probabilities[k];mean+=k*birth.probabilities[k];check(birth.cumulative[k]>=0&&birth.cumulative[k]<=1+1e-12);if(k)check(birth.cumulative[k]>=birth.cumulative[k-1]);}near(mass,1);near(mean,rate*time);long double expected=0,p=std::exp(-static_cast<long double>(rate)*time);for(std::size_t k=0;k<n;++k){expected+=(n-k)*p;near(probability(death,n-k),p);p*=static_cast<long double>(rate)*time/(k+1);}near(death.mean,expected);near(std::accumulate(death.probabilities.begin(),death.probabilities.end(),0.L),1);const auto scaled=pureBirth(rate*2,time/2);near(probability(scaled,n),probability(birth,n));}
 r=pureBirth(100000);near(std::accumulate(r.probabilities.begin(),r.probabilities.end(),0.L),1,1e-10);near(r.mean,100000);r=pureDeath(3,9999);near(r.mean,9996);r=pureDeath(100000,3);near(probability(r,0),1,1e-10);near(r.mean,0);
 for(int k=0;k<6;++k){bool rejected=false;try{if(k==0)(void)pureBirth(-1);if(k==1)(void)pureBirth(INFINITY);if(k==2)(void)pureBirth(100001);if(k==3)(void)pureDeath(1,0);if(k==4)(void)pureDeath(1,100001);if(k==5)(void)pureBirth(1,-1);}catch(const std::invalid_argument&){rejected=true;}check(rejected);}
 std::cout<<"Queue transients:500 independent Poisson/censored-death cases, captured values and boundaries passed\n";
}
