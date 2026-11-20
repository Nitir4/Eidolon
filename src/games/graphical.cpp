#include <tora/games/graphical.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace tora::games {
namespace {
core::Scalar interpolate(core::Scalar zero,core::Scalar one,core::Scalar p) {
    return (1-p)*zero+p*one;
}
core::Scalar restore(core::Scalar normalized,core::Scalar scale) {
    // A convex combination of scaled finite payoffs lies in [-1,1].
    return std::clamp(normalized,core::Scalar(-1),core::Scalar(1))*scale;
}
}
std::vector<core::Scalar> GraphicalResult::expectedPayoffs(core::Scalar p) const {
    if(!std::isfinite(p) || p<0 || p>1)throw std::invalid_argument("Game probability must lie in [0,1]");
    std::vector<core::Scalar> values;
    for(const auto& line:lines) {
        if(!std::isfinite(line.atZero) || !std::isfinite(line.atOne))throw std::invalid_argument("Game payoffs must be finite");
        const auto scale=std::max(std::abs(line.atZero),std::abs(line.atOne));
        values.push_back(scale==0?0:restore(interpolate(line.atZero/scale,line.atOne/scale,p),scale));
    }
    return values;
}
GraphicalResult solveGraphical(const core::Matrix& a,GraphicalPlayer player,const core::Tolerance& tolerance) {
    if(player!=GraphicalPlayer::RowA && player!=GraphicalPlayer::ColumnB)throw std::invalid_argument("Invalid graphical Game player");
    const bool row=player==GraphicalPlayer::RowA;
    if(a.rows()<1 || a.columns()<1 || a.rows()>100 || a.columns()>100 || (row?a.rows():a.columns())!=2)
        throw std::invalid_argument("Graphical Games require exactly two strategies for the selected player and 1..100 for the opponent");
    core::Scalar scale=0;
    for(const auto x:a.data()) {
        if(!std::isfinite(x))throw std::invalid_argument("Game payoffs must be finite");
        scale=std::max(scale,std::abs(x));
    }
    GraphicalResult result;result.player=player;
    auto& history=result.equilibrium.history;
    history.push_back({"Graphical payoff lines","Selected player's first-strategy probability is p. Opponent strategy lines are (1-p)*atZero+p*atOne; normalize payoffs before intersections.",{},{},{},{}});
    const auto count=row?a.columns():a.rows();std::vector<PayoffLine> normalized;
    for(std::size_t i=0;i<count;++i) {
        const auto zero=row?a(1,i):a(i,1),one=row?a(0,i):a(i,0);
        result.lines.push_back({i,zero,one});normalized.push_back({i,scale==0?0:zero/scale,scale==0?0:one/scale});
    }
    core::Matrix endpoints(count,2);for(std::size_t i=0;i<count;++i){endpoints(i,0)=result.lines[i].atZero;endpoints(i,1)=result.lines[i].atOne;}
    history.back().calculation=std::move(endpoints);
    const auto payoff=[&](std::size_t i,core::Scalar p){return interpolate(normalized[i].atZero,normalized[i].atOne,p);};
    const auto envelope=[&](core::Scalar p) {
        std::size_t active=0;auto value=payoff(0,p);
        for(std::size_t i=1;i<count;++i) {
            const auto candidate=payoff(i,p);
            if(row?candidate<value:candidate>value){value=candidate;active=i;}
        }
        return std::pair<core::Scalar,std::size_t>{value,active};
    };
    // Endpoints and intersections delimit every affine envelope piece.
    // Scaled arithmetic keeps even +/-1e308 endpoint differences finite.
    std::vector<core::Scalar> points{0,1};
    for(std::size_t i=0;i<count;++i)for(std::size_t j=i+1;j<count;++j) {
        const auto zero=normalized[i].atZero-normalized[j].atZero;
        const auto one=normalized[i].atOne-normalized[j].atOne;
        const auto denominator=zero-one;
        if(denominator==0)continue;
        const auto p=zero/denominator;
        if(std::isfinite(p) && p>0 && p<1)points.push_back(p);
    }
    std::sort(points.begin(),points.end());points.erase(std::unique(points.begin(),points.end()),points.end());
    history.push_back({"Envelope breakpoints","Endpoints 0 and 1 plus all finite interior pairwise line intersections; sorted and deduplicated exactly.",{},{},points,{}});
    for(std::size_t i=1;i<points.size();++i) {
        const auto active=envelope((points[i-1]+points[i])/2).second;
        if(!result.envelope.empty() && result.envelope.back().strategy==active)result.envelope.back().to=points[i];
        else result.envelope.push_back({points[i-1],points[i],active});
    }
    auto best=envelope(0).first;
    for(const auto p:points) {
        const auto value=envelope(p).first;
        history.push_back({"Evaluate envelope candidate",row?"Maximize the lower envelope (row player A).":"Minimize the upper envelope (column player B).",{},{},{p},{restore(value,scale)}});
        if(row?value>best:value<best){best=value;result.probability=p;}
    }
    result.optimalInterval={result.probability,result.probability};
    for(const auto p:points)if(envelope(p).first==best) {
        result.optimalInterval[0]=std::min(result.optimalInterval[0],p);
        result.optimalInterval[1]=std::max(result.optimalInterval[1],p);
    }
    const auto p=result.probability;
    std::vector<std::size_t> active;
    for(std::size_t i=0;i<count;++i)if(tolerance.equal(payoff(i,p),best))active.push_back(i);
    const auto slope=[&](std::size_t i){return normalized[i].atOne-normalized[i].atZero;};
    core::Matrix slopes(active.size(),3);for(std::size_t k=0;k<active.size();++k){slopes(k,0)=static_cast<core::Scalar>(active[k]+1);slopes(k,1)=restore(payoff(active[k],p),scale);slopes(k,2)=slope(active[k]);}
    history.push_back({"Recover opposing strategy","Active lines at selected p: columns are strategy index, original payoff, normalized slope. An admissible endpoint slope permits a pure response; otherwise mix negative/positive slopes to zero.",{},{},{p},{},std::move(slopes)});
    // Active slopes bracket zero at an interior optimum. Their zero-slope
    // mixture makes the selected player's two strategies indifferent.
    std::vector<core::Scalar> opponent(count);bool found=false;
    for(const auto i:active) {
        const auto s=slope(i);
        const bool allowed=p==0?(row?s<=0:s>=0):p==1?(row?s>=0:s<=0):s==0;
        if(allowed){opponent[i]=1;found=true;break;}
    }
    if(!found) {
        auto negative=count,positive=count;
        for(const auto i:active){if(slope(i)<0 && negative==count)negative=i;if(slope(i)>0 && positive==count)positive=i;}
        if(negative==count || positive==count)throw std::runtime_error("Graphical Game has no certifiable opposing support");
        const auto weight=slope(positive)/(slope(positive)-slope(negative));
        opponent[negative]=weight;opponent[positive]=1-weight;
    }
    auto& e=result.equilibrium;
    if(row){e.rowStrategy={p,1-p};e.columnStrategy=std::move(opponent);}
    else{e.rowStrategy=std::move(opponent);e.columnStrategy={p,1-p};}
    core::Scalar lower=1,upper=-1;
    for(std::size_t j=0;j<a.columns();++j) {
        core::Scalar value=0;
        for(std::size_t i=0;i<a.rows();++i)value+=e.rowStrategy[i]*(scale==0?0:a(i,j)/scale);
        lower=std::min(lower,value);
    }
    for(std::size_t i=0;i<a.rows();++i) {
        core::Scalar value=0;
        for(std::size_t j=0;j<a.columns();++j)value+=e.columnStrategy[j]*(scale==0?0:a(i,j)/scale);
        upper=std::max(upper,value);
    }
    if(!tolerance.equal(lower,best) || !tolerance.equal(upper,best))throw std::runtime_error("Graphical Game strategies fail the payoff certificate");
    e.value=restore(best,scale);e.lowerBound=restore(std::min(lower,upper),scale);e.upperBound=restore(std::max(lower,upper),scale);
    e.dualityGap=std::max(core::Scalar(0),upper-lower)*scale;
    history.push_back({"Graphical equilibrium","Choose the earliest strictly best candidate. Mix active opposing slopes to zero (or select an admissible endpoint slope), then verify both original payoff guarantees.",{},{},e.rowStrategy,e.columnStrategy});
    std::vector<core::Scalar> rowMin(a.rows(),std::numeric_limits<core::Scalar>::infinity());
    std::vector<core::Scalar> columnMax(a.columns(),-std::numeric_limits<core::Scalar>::infinity());
    for(std::size_t i=0;i<a.rows();++i)for(std::size_t j=0;j<a.columns();++j) {
        rowMin[i]=std::min(rowMin[i],a(i,j));columnMax[j]=std::max(columnMax[j],a(i,j));
    }
    const auto maximin=*std::max_element(rowMin.begin(),rowMin.end()),minimax=*std::min_element(columnMax.begin(),columnMax.end());
    if(maximin==minimax)for(std::size_t i=0;i<a.rows();++i)for(std::size_t j=0;j<a.columns();++j)
        if(rowMin[i]==maximin && columnMax[j]==minimax)e.saddlePoints.emplace_back(i,j);
    return result;
}
} // namespace tora::games
