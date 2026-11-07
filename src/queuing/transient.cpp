#include <tora/queuing/transient.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
namespace tora::queuing {
namespace {
struct Poisson {std::vector<long double> p;long double omitted=0;};
Poisson poisson(double rate,double time){
    if(!std::isfinite(rate)||rate<0||!std::isfinite(time)||time<0)
        throw std::invalid_argument("Transient rate and time must be finite and nonnegative");
    const long double mean=static_cast<long double>(rate)*time;
    if(mean>100000)throw std::invalid_argument("Transient rate*time exceeds the 100000 profile");
    Poisson out;
    if(mean==0){out.p={1};return out;}
    const auto mode=static_cast<std::size_t>(std::floor(mean));
    const auto last=static_cast<std::size_t>(std::ceil(mean+16*std::sqrt(mean)+100));
    out.p.assign(last+1,0);
    out.p[mode]=std::exp(-mean+mode*std::log(mean)-std::lgamma(static_cast<long double>(mode)+1));
    for(std::size_t k=mode;k>0;--k)out.p[k-1]=out.p[k]*k/mean;
    for(std::size_t k=mode+1;k<=last;++k)out.p[k]=out.p[k-1]*mean/k;
    const long double ratio=mean/(last+1);
    out.omitted=out.p.back()*ratio/(1-ratio);
    return out;
}
void finish(TransientResult& out){long double cumulative=0;out.cumulative.reserve(out.probabilities.size());for(auto p:out.probabilities){cumulative+=p;out.cumulative.push_back(static_cast<double>(std::min(1.0L,cumulative)));}}
}
TransientResult pureBirth(double rate,double time){auto p=poisson(rate,time);TransientResult out;out.time=time;out.mean=rate*time;out.omittedMass=static_cast<double>(p.omitted);out.probabilities.reserve(p.p.size());for(auto v:p.p)out.probabilities.push_back(static_cast<double>(v));finish(out);return out;}
TransientResult pureDeath(double rate,std::size_t initial,double time){if(initial<1||initial>100000)throw std::invalid_argument("Pure death initial population must be1..100000");auto p=poisson(rate,time);TransientResult out;out.kind=TransientKind::PureDeath;out.time=time;out.initialPopulation=initial;out.probabilities.assign(initial+1,0);long double tail=0;for(std::size_t k=initial;k<p.p.size();++k)tail+=p.p[k];out.probabilities[0]=static_cast<double>(std::min(1.0L,tail));long double mean=0;for(std::size_t n=1;n<=initial;++n){const auto k=initial-n;const auto value=k<p.p.size()?p.p[k]:0;out.probabilities[n]=static_cast<double>(value);mean+=n*value;}out.mean=static_cast<double>(mean);out.omittedMass=static_cast<double>(p.omitted);finish(out);return out;}
double probability(const TransientResult& r,std::size_t state){return state<r.probabilities.size()?r.probabilities[state]:0;}
double cumulativeProbability(const TransientResult& r,std::size_t state){return r.cumulative.empty()?0:state<r.cumulative.size()?r.cumulative[state]:r.cumulative.back();}
}
