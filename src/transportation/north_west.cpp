#include <tora/transportation/north_west.hpp>
#include <tora/core/number_formatter.hpp>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <stdexcept>
#include <utility>

namespace tora::transportation {
namespace {
constexpr std::size_t SizeLimit=100;
constexpr double Epsilon=1e-9;
using Edge=std::pair<std::size_t,std::size_t>;
using Basis=std::map<Edge,double>;
void dimensions(std::size_t m,std::size_t n) {
    if(!m || !n || m>SizeLimit || n>SizeLimit)
        throw std::invalid_argument("Enter 1 to 100 sources and destinations (native size limit).");
}
void finite(double value) {
    if(!std::isfinite(value))throw std::invalid_argument("Transportation cells must contain finite numbers.");
}
Tableau snapshot(const Problem& p,const Basis& basis) {
    const auto m=p.supply.size(),n=p.demand.size();
    Tableau t;
    t.shipments.assign(m,std::vector<std::optional<double>>(n));
    t.reducedCosts.assign(m,std::vector<double>(n));
    t.u.assign(m,std::numeric_limits<double>::quiet_NaN());
    t.v.assign(n,std::numeric_limits<double>::quiet_NaN());
    t.u[0]=0;
    for(std::size_t pass=0;pass<m+n;++pass) {
        for(const auto& [edge,value]:basis) {
            (void)value;const auto [i,j]=edge;
            if(std::isfinite(t.u[i]))t.v[j]=p.costs[i][j]-t.u[i];
            else if(std::isfinite(t.v[j]))t.u[i]=p.costs[i][j]-t.v[j];
        }
    }
    for(double value:t.u)finite(value);
    for(double value:t.v)finite(value);
    for(const auto& [edge,value]:basis) {
        const auto [i,j]=edge;t.shipments[i][j]=value;
        t.objective+=value*p.costs[i][j];
    }
    finite(t.objective);
    for(std::size_t i=0;i<m;++i)for(std::size_t j=0;j<n;++j) {
        t.reducedCosts[i][j]=t.u[i]+t.v[j]-p.costs[i][j];
        finite(t.reducedCosts[i][j]);
    }
    return t;
}
Pivot pivot(const Basis& basis,Edge entering,std::size_t m,std::size_t n) {
    std::vector<std::vector<std::size_t>> graph(m+n);
    for(const auto& [edge,value]:basis) {
        (void)value;const auto [i,j]=edge;
        graph[i].push_back(m+j);graph[m+j].push_back(i);
    }
    const auto start=m+entering.second,goal=entering.first;
    std::vector<std::size_t> parent(m+n,m+n);
    std::queue<std::size_t> queue;queue.push(start);parent[start]=start;
    while(!queue.empty() && parent[goal]==m+n) {
        const auto node=queue.front();queue.pop();
        for(auto other:graph[node])if(parent[other]==m+n) {
            parent[other]=node;queue.push(other);
        }
    }
    if(parent[goal]==m+n)throw std::runtime_error("Transportation basis is disconnected.");
    std::vector<std::size_t> path{goal};
    while(path.back()!=start)path.push_back(parent[path.back()]);
    std::reverse(path.begin(),path.end());
    Pivot result{{entering.first,entering.second},{m,n},{{entering.first,entering.second}}};
    double minimum=std::numeric_limits<double>::infinity();
    for(std::size_t k=0;k+1<path.size();++k) {
        const Edge edge{std::min(path[k],path[k+1]),std::max(path[k],path[k+1])-m};
        result.loop.push_back({edge.first,edge.second});
        if(k%2==0) {
            const double value=basis.at(edge);
            const Edge leaving{result.leaving.source,result.leaving.destination};
            if(value<minimum || (value==minimum && edge<leaving)) {
                minimum=value;result.leaving={edge.first,edge.second};
            }
        }
    }
    return result;
}
}

Problem Problem::blank(std::size_t m,std::size_t n) {
    dimensions(m,n);Problem p;
    p.costs.assign(m,std::vector<double>(n));p.supply.resize(m);p.demand.resize(n);
    p.sourceNames.resize(m);p.destinationNames.resize(n);return p;
}
void validate(const Problem& p) {
    const auto m=p.supply.size(),n=p.demand.size();dimensions(m,n);
    if(p.costs.size()!=m || p.sourceNames.size()!=m || p.destinationNames.size()!=n)
        throw std::invalid_argument("Transportation model dimensions are inconsistent.");
    for(const auto& row:p.costs) {
        if(row.size()!=n)throw std::invalid_argument("Transportation cost row has the wrong size.");
        for(double value:row)finite(value);
    }
    for(const auto& values:{p.supply,p.demand})for(double value:values) {
        finite(value);if(value<0)throw std::invalid_argument("Supply and demand must be nonnegative.");
    }
}
std::vector<Tableau> solve(const Problem& p,StartMethod method) {
    if(method!=StartMethod::NorthWest && method!=StartMethod::LeastCost && method!=StartMethod::Vogel)
        throw std::invalid_argument("Unsupported Transportation starting method.");
    validate(p);
    for(const auto& values:{p.supply,p.demand})for(double value:values)
        if(static_cast<double>(static_cast<float>(value))!=value)
            throw std::invalid_argument("This Transportation preview supports exactly representable single-precision quantities. TORA's other fractional-quantity display is still under investigation.");
    const double supply=std::accumulate(p.supply.begin(),p.supply.end(),0.0);
    const double demand=std::accumulate(p.demand.begin(),p.demand.end(),0.0);
    finite(supply);finite(demand);
    if(supply!=demand)throw std::invalid_argument("This Transportation preview requires equal total supply and demand.");
    const auto m=p.supply.size(),n=p.demand.size();
    auto remainingSupply=p.supply,remainingDemand=p.demand;Basis basis;
    if(method==StartMethod::NorthWest) {
        std::size_t i=0,j=0;
        while(i<m && j<n) {
            const double value=std::min(remainingSupply[i],remainingDemand[j]);
            basis[{i,j}]=value;remainingSupply[i]-=value;remainingDemand[j]-=value;
            // Equal exhaustion moves to the next source first, preserving the
            // observed zero basic shipment in that destination.
            if(remainingSupply[i]==0 && i+1<m)++i;
            else if(remainingDemand[j]==0)++j;
            else ++i;
        }
    } else {
        // Remove exactly one exhausted line at each allocation. Equal
        // exhaustion removes the source first unless it is the last source;
        // keeping the destination active preserves the observed basic zero.
        std::vector<bool> activeSources(m,true),activeDestinations(n,true);
        std::size_t sources=m,destinations=n;
        while(sources && destinations) {
            std::optional<Edge> selected;
            if(method==StartMethod::LeastCost) {
                double best=std::numeric_limits<double>::infinity();
                for(std::size_t r=0;r<m;++r)for(std::size_t c=0;c<n;++c)
                    if(activeSources[r] && activeDestinations[c] && (!selected || p.costs[r][c]<best)) {
                        selected=Edge{r,c};best=p.costs[r][c];
                    }
            } else {
                double bestPenalty=-std::numeric_limits<double>::infinity();
                // Source lines precede destination lines on equal penalties;
                // equal minima retain the first cell in that line.
                const auto consider=[&](bool source,std::size_t line) {
                    double first=std::numeric_limits<double>::infinity(),second=first;
                    std::optional<Edge> cell;
                    std::size_t count=0;
                    for(std::size_t k=0;k<(source?n:m);++k) {
                        if(!(source?activeDestinations[k]:activeSources[k]))continue;
                        const Edge edge=source?Edge{line,k}:Edge{k,line};
                        const double cost=p.costs[edge.first][edge.second];++count;
                        if(!cell || cost<first) {second=first;first=cost;cell=edge;}
                        else if(cost<second)second=cost;
                    }
                    // A singleton's remaining allocation is forced. Its
                    // internal TORA penalty sentinel is not observable.
                    const double penalty=count>1?second-first:0;
                    finite(penalty);
                    if(cell && (!selected || penalty>bestPenalty)) {selected=cell;bestPenalty=penalty;}
                };
                for(std::size_t r=0;r<m;++r)if(activeSources[r])consider(true,r);
                for(std::size_t c=0;c<n;++c)if(activeDestinations[c])consider(false,c);
            }
            if(!selected)throw std::runtime_error("No active Transportation allocation cell.");
            const auto [r,c]=*selected;
            const double value=std::min(remainingSupply[r],remainingDemand[c]);
            basis[*selected]=value;remainingSupply[r]-=value;remainingDemand[c]-=value;
            if(remainingSupply[r]==0 && sources>1) {activeSources[r]=false;--sources;}
            else if(remainingDemand[c]==0) {activeDestinations[c]=false;--destinations;}
            else {activeSources[r]=false;--sources;}
        }
    }
    std::vector<Tableau> history;
    std::set<std::vector<Edge>> visited;
    for(std::size_t iteration=0;iteration<1000;++iteration) {
        std::vector<Edge> identity;for(const auto& entry:basis)identity.push_back(entry.first);
        if(!visited.insert(identity).second)throw std::runtime_error("Transportation pivot cycle detected; no optimal result returned.");
        history.push_back(snapshot(p,basis));auto& current=history.back();
        std::optional<Edge> entering;double best=Epsilon;
        for(std::size_t r=0;r<m;++r)for(std::size_t c=0;c<n;++c)
            if(!basis.count({r,c}) && current.reducedCosts[r][c]>best) {
                best=current.reducedCosts[r][c];entering=Edge{r,c};
            }
        if(!entering)return history;
        current.nextPivot=pivot(basis,*entering,m,n);
        const auto& selected=*current.nextPivot;
        const double amount=basis.at({selected.leaving.source,selected.leaving.destination});
        basis[*entering]=0;
        for(std::size_t k=0;k<selected.loop.size();++k) {
            const auto cell=selected.loop[k];auto& value=basis.at({cell.source,cell.destination});
            value+=(k%2==0?amount:-amount);finite(value);
        }
        basis.erase({selected.leaving.source,selected.leaving.destination});
    }
    throw std::runtime_error("Transportation iteration safety limit reached; no optimal result returned.");
}
std::vector<Tableau> solveNorthWest(const Problem& p) { return solve(p,StartMethod::NorthWest); }
std::vector<Tableau> solveLeastCost(const Problem& p) { return solve(p,StartMethod::LeastCost); }
std::vector<Tableau> solveVogel(const Problem& p) { return solve(p,StartMethod::Vogel); }
std::string quantity(double value) {
    finite(value);if(value==0)return "0";
    char buffer[128];const auto result=std::to_chars(buffer,buffer+sizeof(buffer),value);
    if(result.ec!=std::errc{})throw std::runtime_error("Cannot format shipment.");
    return {buffer,result.ptr};
}
Display display(const Problem& p,const Tableau& t,std::size_t iteration) {
    const core::NumberFormatter formatter({core::Notation::Decimal,5,2});
    Display result;result.label="Iter"+std::to_string(iteration);result.objective=formatter.format(t.objective);
    for(std::size_t i=0;i<p.supply.size();++i) {
        result.costs.emplace_back();result.shipments.emplace_back();result.reducedCosts.emplace_back();
        result.u.push_back("u"+std::to_string(i+1)+"="+formatter.format(t.u.at(i)));
        result.supply.push_back(quantity(p.supply[i]));
        for(std::size_t j=0;j<p.demand.size();++j) {
            result.costs.back().push_back(formatter.format(p.costs[i][j]));
            const auto q=t.shipments.at(i).at(j);
            result.shipments.back().push_back(q?quantity(*q):"");
            result.reducedCosts.back().push_back(formatter.format(t.reducedCosts.at(i).at(j)));
        }
    }
    for(std::size_t j=0;j<p.demand.size();++j) {
        result.v.push_back("v"+std::to_string(j+1)+"="+formatter.format(t.v.at(j)));
        result.demand.push_back(quantity(p.demand[j]));
    }
    return result;
}
} // namespace tora::transportation
