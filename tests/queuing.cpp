#include <tora/queuing/queuing.hpp>
#include <cmath>
#include <numeric>
#include <random>
#include <stdexcept>
#include <sstream>
using namespace tora::queuing;
namespace {
void check(bool v){if(!v)throw std::runtime_error("Queuing regression failed");}
void near(double a,double b){if(!std::isfinite(a)||std::abs(a-b)>=1e-9*(1+std::abs(b))){std::ostringstream message;message<<"Actual "<<a<<" expected "<<b;throw std::runtime_error(message.str());}}
void certificate(const Model& m,const Result& r){check(r.status==Status::Stationary&&r.metrics);const auto& v=*r.metrics;near(v.meanSystem,v.meanQueue+v.meanBusyServers);near(v.throughput,m.serviceRate*v.meanBusyServers);near(v.meanSystemTime,v.meanQueueTime+1/m.serviceRate);near(v.attemptedArrivalRate,v.throughput+v.rejectedArrivalRate);check(v.probabilityWait>=0&&v.probabilityWait<=1);check(v.utilization>=0&&v.utilization<=1+1e-12);check(v.maximumRelativeBalanceResidual<1e-10);if(r.capacity){near(std::accumulate(r.probabilities.begin(),r.probabilities.end(),0.),1);for(std::size_t i=0;i<*r.capacity;++i){double birth=m.arrivalRate*(m.sourceLimit?*m.sourceLimit-i:1);near(probability(r,i)*birth,probability(r,i+1)*m.serviceRate*std::min(i+1,m.servers));}}else{double sum=0;for(std::size_t i=0;i<m.servers;++i)sum+=r.probabilities[i];near(sum+r.tailMass,1);}}
}
int main(){
 Model m{2,3,1,{},{}};auto r=solve(m);certificate(m,r);auto v=*r.metrics;near(v.probabilityEmpty,1.0/3);near(v.meanSystem,2);near(v.meanQueue,4.0/3);near(v.meanSystemTime,1);near(v.meanQueueTime,2.0/3);near(probability(r,1),2.0/9);near(probability(r,2),4.0/27);
 m={3,2,2,{},{}};r=solve(m);certificate(m,r);v=*r.metrics;near(v.probabilityEmpty,1.0/7);near(v.probabilityWait,9.0/14);near(v.meanQueue,27.0/14);near(v.meanSystem,24.0/7);
 m={2,2,1,4,{}};r=solve(m);certificate(m,r);v=*r.metrics;for(auto p:r.probabilities)near(p,.2);near(v.meanSystem,2);near(v.meanQueue,1.2);near(v.throughput,1.6);near(v.blockingProbability,.2);
 m={2,3,1,1,{}};r=solve(m);certificate(m,r);near(r.metrics->blockingProbability,.4);near(r.metrics->probabilityWait,0);near(r.metrics->meanSystemTime,1.0/3);
 // Three independent finite-source users, each arriving at rate 2 and served at 3.
 // With three servers, occupied users have Binomial(3, 2/5) distribution.
 m={2,3,3,{},3};r=solve(m);certificate(m,r);near(probability(r,0),.216);near(probability(r,1),.432);near(probability(r,2),.288);near(probability(r,3),.064);near(r.metrics->meanSystem,1.2);near(r.metrics->throughput,3.6);near(r.metrics->meanQueue,0);near(r.metrics->blockingProbability,0);
 m={2,3,1,{},3};r=solve(m);certificate(m,r);near(probability(r,0),9.0/67);near(probability(r,1),18.0/67);near(probability(r,2),24.0/67);near(probability(r,3),16.0/67);near(r.metrics->throughput,174.0/67);
 m={2,3,1,1,3};r=solve(m);certificate(m,r);near(r.metrics->probabilityFull,2.0/3);near(r.metrics->blockingProbability,4.0/7);near(r.metrics->throughput,2);
 m={0,3,2,{},{}};r=solve(m);certificate(m,r);check(r.metrics->zeroArrivalLimit);near(r.metrics->meanSystemTime,1.0/3);near(probability(r,300),0);
 m={2,0,1,3,{}};r=solve(m);check(r.status==Status::Stationary&&r.metrics->absorbingNoService);near(probability(r,3),1);near(r.metrics->meanSystem,3);near(r.metrics->meanQueue,2);near(r.metrics->throughput,0);near(r.metrics->rejectedArrivalRate,2);near(r.metrics->blockingProbability,1);
 m={2,0,1,{},3};r=solve(m);check(r.metrics->absorbingNoService&&r.metrics->arrivalBlockingUndefined);near(probability(r,3),1);near(r.metrics->attemptedArrivalRate,0);
 m={2,0,1,{},{}};r=solve(m);check(r.status==Status::Unstable);
 m={0,0,1,3,{}};bool bothZero=false;try{(void)solve(m);}catch(const std::invalid_argument&){bothZero=true;}check(bothZero);
 m={3,3,1,{},{}};r=solve(m);check(r.status==Status::Unstable&&!r.metrics);bool rejected=false;try{(void)probability(r,0);}catch(const std::invalid_argument&){rejected=true;}check(rejected);
 std::mt19937 rng(20261016);
 for(int sample=0;sample<300;++sample){m.arrivalRate=(1+rng()%100)/7.;m.serviceRate=(1+rng()%100)/11.;m.servers=1+rng()%10;m.systemLimit=1+rng()%40;if(sample%2)m.sourceLimit=1+rng()%40;else m.sourceLimit.reset();r=solve(m);certificate(m,r);Model changed=m;changed.arrivalRate*=13;changed.serviceRate*=13;auto scaled=solve(changed);near(scaled.metrics->meanSystem,r.metrics->meanSystem);near(scaled.metrics->meanQueue,r.metrics->meanQueue);near(scaled.metrics->meanSystemTime,r.metrics->meanSystemTime/13);}
 m={1e300,1e-300,1,100,{}};r=solve(m);near(r.metrics->meanSystem,100);near(r.metrics->throughput/1e-300,1);check(std::isfinite(r.metrics->meanSystemTime));
 // The occupancy lambda/mu is below binary64 range, while the service time
 // remains representable. This must work without an extended long double.
 for(auto servers:{std::size_t{1},std::size_t{2},std::size_t{10000}}){
  m={1e-300,1e300,servers,{},{}};r=solve(m);near(r.metrics->meanSystem,0);
  near(r.metrics->meanSystemTime/1e-300,1);near(r.metrics->meanQueueTime,0);
  near(r.metrics->throughput/1e-300,1);
 }
 m={1-1e-12,1,1,{},{}};r=solve(m);check(r.metrics->meanSystem>9e11);certificate(m,r);
 for(int k=0;k<7;++k){m={2,3,1,{},{}};if(k==0)m.arrivalRate=-1;if(k==1)m.serviceRate=-1;if(k==2)m.servers=0;if(k==3)m.systemLimit=0;if(k==4)m.sourceLimit=100001;if(k==5)m.arrivalRate=NAN;if(k==6)m.serviceRate=INFINITY;bool caught=false;try{(void)solve(m);}catch(const std::invalid_argument&){caught=true;}check(caught);}
}
