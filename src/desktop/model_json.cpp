#include "model_json.hpp"
#include "lp_input_controller.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <cmath>
#include <algorithm>
namespace tora::desktop {
namespace {
QJsonObject object(const QByteArray& bytes,const QString& format,const QStringList& keys) {
    if(bytes.size()>4*1024*1024)throw std::invalid_argument("Model file exceeds 4 MiB");
    QJsonParseError error;const auto doc=QJsonDocument::fromJson(bytes,&error);
    if(error.error!=QJsonParseError::NoError||!doc.isObject())throw std::invalid_argument("Invalid model JSON object");
    const auto o=doc.object();if(o["format"]!=format||o["version"]!=1)throw std::invalid_argument("Unsupported native model format/version");
    for(auto i=o.begin();i!=o.end();++i)if(!keys.contains(i.key())&&i.key()!="format"&&i.key()!="version")throw std::invalid_argument(("Unsupported model field: "+i.key()).toStdString());
    return o;
}
QString text(const QJsonValue& x){if(!x.isString())throw std::invalid_argument("Expected a model string");return x.toString();}
double number(const QJsonValue& x,bool bound=false) {
    if(bound&&x.isString()){if(x=="infinity")return INFINITY;if(x=="-infinity")return -INFINITY;}
    if(!x.isDouble()||!std::isfinite(x.toDouble()))throw std::invalid_argument("Expected a finite model number (or an explicit infinity bound)");
    return x.toDouble();
}
QJsonValue bound(double x){return std::isinf(x)?QJsonValue(x<0?"-infinity":"infinity"):QJsonValue(x);}
QJsonArray array(const QJsonValue& v){if(!v.isArray())throw std::invalid_argument("Expected a model array");return v.toArray();}
core::NumberFormat format(const QJsonValue& v) {
    const auto a=array(v);if(a.size()!=3)throw std::invalid_argument("numberFormat needs notation,N,D");
    for(const auto& x:a)if(!x.isDouble()||x.toDouble()!=std::floor(x.toDouble())||x.toDouble()<0||x.toDouble()>99)throw std::invalid_argument("Invalid numberFormat digits");
    if(a[0].toInt()>1)throw std::invalid_argument("Invalid numberFormat notation");
    core::NumberFormat f{a[0].toInt()?core::Notation::Scientific:core::Notation::Decimal,a[1].toInt(),a[2].toInt()};(void)core::NumberFormatter(f);return f;
}
QJsonArray format(core::NumberFormat f){return {f.notation==core::Notation::Scientific?1:0,f.n,f.d};}
QJsonArray numbers(const std::vector<double>& v){QJsonArray a;for(auto x:v)a.append(x);return a;}
std::vector<double> numbers(const QJsonValue& v){std::vector<double> out;for(const auto& x:array(v))out.push_back(number(x));return out;}
QJsonArray strings(const std::vector<std::string>& v){QJsonArray a;for(const auto& x:v)a.append(QString::fromStdString(x));return a;}
std::vector<std::string> strings(const QJsonValue& v){std::vector<std::string> out;for(const auto& x:array(v))out.push_back(text(x).toStdString());return out;}
std::string legacy(const QJsonValue& v) {
    // Round-trip the existing legacy boundary rather than silently replacing Unicode.
    const auto s=text(v);std::string out;
    for(const auto ch:s){bool found=false;for(int b=0;b<256;++b)if(legacyText(std::string(1,static_cast<char>(b)))==QString(ch)){out+=static_cast<char>(b);found=true;break;}if(!found)throw std::invalid_argument("LP labels must be representable in the existing Windows-1252 TXT profile");}
    return out;
}
}
compat::LpDocument decodeLpJson(const QByteArray& bytes) {
    const auto o=object(bytes,"EidolonLP",{"title","sense","numberFormat","variables","objective","constraints"});
    const auto vars=array(o["variables"]),rows=array(o["constraints"]);
    if(vars.isEmpty()||vars.size()>100||rows.isEmpty()||rows.size()>100)throw std::invalid_argument("LP dimensions must be 1..100");
    auto p=lp::LpProblem::blank(static_cast<std::size_t>(vars.size()),static_cast<std::size_t>(rows.size()));p.title=legacy(o["title"]);
    if(o["sense"]!="min"&&o["sense"]!="max")throw std::invalid_argument("LP sense must be min or max");
    p.sense=o["sense"]=="min"?lp::ObjectiveSense::Minimize:lp::ObjectiveSense::Maximize;
    p.objective=numbers(o["objective"]);if(p.objective.size()!=p.variables.size())throw std::invalid_argument("LP objective size mismatch");
    for(int j=0;j<vars.size();++j){if(!vars[j].isObject())throw std::invalid_argument("Invalid LP variable");const auto v=vars[j].toObject();for(auto it=v.begin();it!=v.end();++it)if(!QStringList{"name","lower","upper","unrestricted"}.contains(it.key()))throw std::invalid_argument("Unsupported LP variable field");auto& x=p.variables[static_cast<std::size_t>(j)];x.name=legacy(v["name"]);x.lowerBound=number(v["lower"],true);x.upperBound=number(v["upper"],true);if(!v["unrestricted"].isBool())throw std::invalid_argument("unrestricted must be boolean");x.unrestricted=v["unrestricted"].toBool();}
    for(int i=0;i<rows.size();++i){if(!rows[i].isObject())throw std::invalid_argument("Invalid LP row");const auto r=rows[i].toObject();for(auto it=r.begin();it!=r.end();++it)if(!QStringList{"coefficients","relation","rhs"}.contains(it.key()))throw std::invalid_argument("Unsupported LP constraint field");auto& c=p.constraints[static_cast<std::size_t>(i)];c.coefficients=numbers(r["coefficients"]);if(c.coefficients.size()!=p.variables.size())throw std::invalid_argument("LP coefficient size mismatch");c.rhs=number(r["rhs"]);if(r["relation"]!="<="&&r["relation"]!=">="&&r["relation"]!="=")throw std::invalid_argument("Invalid LP relation");c.relation=r["relation"]=="<="?lp::Relation::LessEqual:r["relation"]==">="?lp::Relation::GreaterEqual:lp::Relation::Equal;}
    auto d=compat::ToraLpCodec::create(std::move(p),format(o["numberFormat"]));
    // Seed exact numeric lexemes through the existing parser. Input format
    // controls display precision, not the precision of imported model data.
    const auto entered=d.problem;auto cells=compat::ToraLpCodec::gridText(d);
    const auto n=entered.variables.size(),m=entered.constraints.size();
    const auto spelling=[](double x){return std::isinf(x)?std::string(x<0?"-infinity":"infinity"):QString::number(x,'g',17).toStdString();};
    for(std::size_t j=0;j<n;++j){cells[n+2+j]=spelling(entered.objective[j]);cells[(m+2)*(n+2)+j]=spelling(entered.variables[j].lowerBound);cells[(m+3)*(n+2)+j]=spelling(entered.variables[j].upperBound);}
    for(std::size_t i=0;i<m;++i){for(std::size_t j=0;j<n;++j)cells[(i+2)*(n+2)+j]=spelling(entered.constraints[i].coefficients[j]);cells[(i+2)*(n+2)+n+1]=spelling(entered.constraints[i].rhs);}
    compat::ToraLpCodec::setGridText(d,cells);
    for(std::size_t j=0;j<n;++j){const auto& a=entered.variables[j];const auto& b=d.problem.variables[j];if(a.lowerBound!=b.lowerBound||a.upperBound!=b.upperBound||a.unrestricted!=b.unrestricted)throw std::invalid_argument("LP bounds/unrestricted flag cannot be preserved by the existing editor; use -infinity/infinity for a free variable");}
    return d;
}
QByteArray encodeLpJson(const compat::LpDocument& d) {
    const auto& p=d.problem;QJsonArray variables,rows;
    for(const auto& v:p.variables)variables.append(QJsonObject{{"name",legacyText(v.name)},{"lower",bound(v.lowerBound)},{"upper",bound(v.upperBound)},{"unrestricted",v.unrestricted}});
    for(const auto& c:p.constraints)rows.append(QJsonObject{{"coefficients",numbers(c.coefficients)},{"relation",c.relation==lp::Relation::Equal?"=":c.relation==lp::Relation::LessEqual?"<=":">="},{"rhs",c.rhs}});
    return QJsonDocument(QJsonObject{{"format","EidolonLP"},{"version",1},{"title",legacyText(p.title)},{"sense",p.sense==lp::ObjectiveSense::Minimize?"min":"max"},{"numberFormat",format(d.inputFormat)},{"variables",variables},{"objective",numbers(p.objective)},{"constraints",rows}}).toJson();
}
compat::TransportationDocument decodeTransportationJson(const QByteArray& bytes) {
    const auto o=object(bytes,"EidolonTransportation",{"title","sense","numberFormat","sources","destinations","supply","demand","costs","dummySources","dummyDestinations"});
    if(o["sense"]!="min"&&o["sense"]!="max")throw std::invalid_argument("Transportation sense must be min or max");
    const auto supply=numbers(o["supply"]),demand=numbers(o["demand"]);if(supply.empty()||demand.empty()||supply.size()>100||demand.size()>100)throw std::invalid_argument("Transportation dimensions must be 1..100");
    auto p=transportation::Problem::blank(supply.size(),demand.size());p.maximize=o["sense"]=="max";p.title=text(o["title"]).toStdString();p.format=format(o["numberFormat"]);p.supply=supply;p.demand=demand;p.sourceNames=strings(o["sources"]);p.destinationNames=strings(o["destinations"]);
    for(bool source:{true,false}){const auto key=source?"dummySources":"dummyDestinations";auto& indices=source?p.dummySources:p.dummyDestinations;const auto limit=source?supply.size():demand.size();if(o.contains(key))for(const auto& v:array(o[key])){const auto x=number(v);if(x!=std::floor(x)||x<1||x>static_cast<double>(limit))throw std::invalid_argument("Dummy index outside Transportation model");const auto index=static_cast<std::size_t>(x-1);if(std::find(indices.begin(),indices.end(),index)!=indices.end())throw std::invalid_argument("Duplicate dummy index");indices.push_back(index);}}
    const auto rows=array(o["costs"]);p.costs.clear();for(const auto& r:rows)p.costs.push_back(numbers(r));return compat::TransportationDocument::create(std::move(p));
}
QByteArray encodeTransportationJson(const compat::TransportationDocument& d) {
    const auto& p=d.problem();QJsonArray dummySources,dummyDestinations;for(auto i:p.dummySources)dummySources.append(int(i+1));for(auto j:p.dummyDestinations)dummyDestinations.append(int(j+1));QJsonArray costs;for(const auto& row:p.costs)costs.append(numbers(row));
    return QJsonDocument(QJsonObject{{"format","EidolonTransportation"},{"version",1},{"title",QString::fromStdString(p.title)},{"sense",p.maximize?"max":"min"},{"dummySources",dummySources},{"dummyDestinations",dummyDestinations},{"numberFormat",format(p.format)},{"sources",strings(p.sourceNames)},{"destinations",strings(p.destinationNames)},{"supply",numbers(p.supply)},{"demand",numbers(p.demand)},{"costs",costs}}).toJson();
}
GamesDocument decodeGameJson(const QByteArray& bytes) {
    const auto o=object(bytes,"EidolonGame",{"title","numberFormat","payoff","rowNames","columnNames"});const auto a=array(o["payoff"]);
    if(a.isEmpty()||a.size()>100||!a[0].isArray()||a[0].toArray().isEmpty()||a[0].toArray().size()>100)throw std::invalid_argument("Game dimensions must be 1..100");
    const int columns=a[0].toArray().size();auto d=GamesDocument::blank(a.size(),columns,text(o["title"]).toStdString(),format(o["numberFormat"]));
    for(int i=0;i<a.size();++i){const auto r=array(a[i]);if(r.size()!=columns)throw std::invalid_argument("Game payoff width mismatch");for(int j=0;j<columns;++j)d.setCell(i,j,QString::number(number(r[j]),'g',17).toStdString());}
    if(o.contains("rowNames")||o.contains("columnNames"))d.setNames(strings(o["rowNames"]),strings(o["columnNames"]));
    return d;
}
QByteArray encodeGameJson(const GamesDocument& d) {
    QJsonArray payoff;for(std::size_t i=0;i<d.payoff().rows();++i){QJsonArray row;for(std::size_t j=0;j<d.payoff().columns();++j)row.append(d.payoff()(i,j));payoff.append(row);}
    return QJsonDocument(QJsonObject{{"format","EidolonGame"},{"version",1},{"title",QString::fromStdString(d.title())},{"numberFormat",format(d.format())},{"payoff",payoff},{"rowNames",strings(d.rowNames())},{"columnNames",strings(d.columnNames())}}).toJson();
}
}
