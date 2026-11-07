#include <tora/queuing/queuing.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace tora::queuing {
namespace {
double narrow(long double value){if(!std::isfinite(value)||value>std::numeric_limits<double>::max()||value<0)throw std::runtime_error("Queuing metric exceeds binary64 output range");return static_cast<double>(value);}
}
Result solve(const Model& model){
 if(!std::isfinite(model.arrivalRate)||model.arrivalRate<0||!std::isfinite(model.serviceRate)||model.serviceRate<0||(model.arrivalRate==0&&model.serviceRate==0)||model.servers<1||model.servers>10000||(model.systemLimit&&(*model.systemLimit<1||*model.systemLimit>100000))||(model.sourceLimit&&(*model.sourceLimit<1||*model.sourceLimit>100000)))throw std::invalid_argument("Invalid queuing rates, servers, capacity, or source limit");
 const long double lambda=model.arrivalRate,mu=model.serviceRate;
 Result out;
 if(model.systemLimit)out.capacity=model.systemLimit;
 if(model.sourceLimit)out.capacity=out.capacity?std::min(*out.capacity,*model.sourceLimit):*model.sourceLimit;
 if(mu==0){
  if(!out.capacity)return out;
  out.status=Status::Stationary;out.metrics=Metrics{};auto& metrics=*out.metrics;
  const auto cap=*out.capacity;out.probabilities.assign(cap+1,0);out.probabilities[cap]=1;
  metrics.absorbingNoService=true;metrics.probabilityEmpty=0;metrics.probabilityFull=1;
  metrics.meanSystem=static_cast<double>(cap);metrics.meanBusyServers=static_cast<double>(std::min(cap,model.servers));metrics.meanQueue=static_cast<double>(cap-std::min(cap,model.servers));metrics.utilization=metrics.meanBusyServers/model.servers;
  const long double attempts=lambda*(model.sourceLimit?static_cast<long double>(*model.sourceLimit-cap):1.0L);
  metrics.attemptedArrivalRate=metrics.rejectedArrivalRate=narrow(attempts);metrics.blockingProbability=attempts>0?1:0;metrics.arrivalBlockingUndefined=attempts==0;
  return out;
 }
 const long double rho=lambda/(static_cast<long double>(model.servers)*mu);
 if(!out.capacity&&rho>=1)return out;
 out.status=Status::Stationary;out.metrics=Metrics{};auto& metrics=*out.metrics;
 if(rho<=std::numeric_limits<double>::max())metrics.offeredLoadPerServer=static_cast<double>(rho);
 const std::size_t last=out.capacity?*out.capacity:model.servers;
 out.probabilities.assign(last+1,0);
 if(lambda==0){out.probabilities[0]=1;metrics.zeroArrivalLimit=true;metrics.meanSystemTime=narrow(1/mu);return out;}
 std::vector<long double> logs(last+1),p(last+1);
 auto birth=[&](std::size_t n){return lambda*(model.sourceLimit?static_cast<long double>(*model.sourceLimit-n):1.0L);};
 auto death=[&](std::size_t n){return mu*static_cast<long double>(std::min(n,model.servers));};
 long double largest=0;
 for(std::size_t n=1;n<=last;++n){logs[n]=logs[n-1]+std::log(birth(n-1))-std::log(death(n));largest=std::max(largest,logs[n]);}
 // Include the whole geometric tail in normalization, rather than truncating it.
 long double logTail=0;
 if(!out.capacity){logTail=logs[last]-std::log1p(-rho);largest=std::max(largest,logTail);}
 long double normalizer=0;
 for(std::size_t n=0;n<=last;++n){p[n]=std::exp(logs[n]-largest);if(out.capacity||n<last)normalizer+=p[n];}
 if(!out.capacity)normalizer+=std::exp(logTail-largest);
 for(std::size_t n=0;n<=last;++n){p[n]/=normalizer;out.probabilities[n]=narrow(p[n]);}
 long double system=0,queue=0,busy=0,accepted=0,attempted=0,rejected=0,wait=0;
 if(out.capacity){
  for(std::size_t n=0;n<=last;++n){system+=p[n]*n;queue+=p[n]*(n>model.servers?n-model.servers:0);busy+=p[n]*std::min(n,model.servers);const auto attempts=p[n]*birth(n);attempted+=attempts;if(n<last){accepted+=attempts;if(n>=model.servers)wait+=attempts;}else rejected+=attempts;}
  accepted=mu*busy;attempted=accepted+rejected;
  metrics.probabilityFull=narrow(p[last]);
 }else{
  const long double mass=std::exp(logTail-largest)/normalizer;out.tailRatio=narrow(rho);out.logTailRatio=static_cast<double>(std::log(rho));out.tailMass=narrow(mass);
  queue=mass*rho/(1-rho);busy=lambda/mu;system=queue+busy;accepted=attempted=lambda;wait=mass*lambda;
 }
 metrics.probabilityEmpty=out.probabilities[0];metrics.meanSystem=narrow(system);metrics.meanQueue=narrow(queue);metrics.meanBusyServers=narrow(busy);metrics.utilization=narrow(busy/model.servers);metrics.throughput=narrow(accepted);metrics.attemptedArrivalRate=narrow(attempted);metrics.rejectedArrivalRate=narrow(rejected);metrics.probabilityWait=narrow(std::min(1.0L,wait/accepted));metrics.blockingProbability=narrow(rejected/attempted);metrics.meanQueueTime=narrow(queue/accepted);
 // Little's law loses a representable service time when lambda/mu underflows
 // on platforms where long double is binary64 (MSVC and Apple Silicon).
 // Every admitted customer receives one mean service time, so W = Wq + 1/mu.
 metrics.meanSystemTime=narrow(queue/accepted+1/mu);
 long double residual=0;
 for(std::size_t n=0;n<last;++n){const long double left=p[n]*birth(n),right=p[n+1]*death(n+1),scale=std::max(left,right);if(scale)residual=std::max(residual,std::abs(left-right)/scale);}
 metrics.maximumRelativeBalanceResidual=narrow(residual);
 return out;
}
double probability(const Result& result,std::size_t state){
 if(result.status!=Status::Stationary||result.probabilities.empty())throw std::invalid_argument("No stationary probabilities for unstable queue");
 if(state<result.probabilities.size())return result.probabilities[state];
 if(result.capacity)return 0;
 if(result.tailRatio==0)return 0;
 return result.probabilities.back()*std::exp(result.logTailRatio*static_cast<double>(state-(result.probabilities.size()-1)));
}
}
