#include "assignment_controller.hpp"
#include <tora/compat/lp_file.hpp>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
namespace tora::desktop {
namespace {
void expect(std::istream& in,const char* wanted){std::string token;if(!(in>>token)||token!=wanted)throw std::invalid_argument(std::string("Assignment TXT: expected ")+wanted);}
int count(std::istream& in,int maximum=100){int n=-1;if(!(in>>n)||n<0||n>maximum)throw std::invalid_argument("Invalid Assignment TXT count");return n;}
QJsonArray names(std::istream& in){QJsonArray a;const auto n=count(in);for(int i=0;i<n;++i){in>>std::ws;if(in.peek()!='"')throw std::invalid_argument("Assignment TXT labels must be quoted");std::string s;if(!(in>>std::quoted(s)))throw std::invalid_argument("Invalid Assignment TXT label");a.append(QString::fromUtf8(s.data(),static_cast<qsizetype>(s.size())));}return a;}
}
QByteArray encodeAssignmentText(const AssignmentData& d) {
    const auto object=QJsonDocument::fromJson(encodeAssignment(d)).object();
    std::ostringstream out;out<<std::setprecision(17)<<"EIDOLON ASSIGNMENT 1\ntitle "<<std::quoted(d.title.toUtf8().toStdString())<<"\nsense "<<(d.model.sense==assignment::Sense::Minimize?"min":"max")<<"\nnumberFormat "<<(d.format.notation==core::Notation::Scientific?1:0)<<' '<<d.format.n<<' '<<d.format.d<<'\n';
    for(const auto& key:{"workers","jobs"}){const auto a=object[key].toArray();out<<key<<' '<<a.size()<<'\n';for(const auto& item:a)out<<std::quoted(item.toString().toUtf8().toStdString())<<'\n';}
    for(const auto& key:{"dummyWorkers","dummyJobs"}){const auto a=object[key].toArray();out<<key<<' '<<a.size();for(const auto& x:a)out<<' '<<x.toInt();out<<'\n';}
    out<<"forbidden "<<d.model.forbidden.size()<<'\n';for(const auto& p:d.model.forbidden)out<<p.first+1<<' '<<p.second+1<<'\n';
    out<<"costs "<<d.model.costs.rows()<<' '<<d.model.costs.columns()<<'\n';for(std::size_t i=0;i<d.model.costs.rows();++i){for(std::size_t j=0;j<d.model.costs.columns();++j)out<<d.model.costs(i,j)<<' ';out<<'\n';}out<<"end\n";
    return QByteArray::fromStdString(out.str());
}
AssignmentData decodeAssignmentText(const QByteArray& bytes) {
    if(bytes.size()>4*1024*1024)throw std::invalid_argument("Assignment file exceeds 4 MiB");
    std::istringstream in(bytes.toStdString());expect(in,"EIDOLON");expect(in,"ASSIGNMENT");expect(in,"1");expect(in,"title");
    in>>std::ws;if(in.peek()!='"')throw std::invalid_argument("Assignment TXT title must be quoted");std::string title;if(!(in>>std::quoted(title)))throw std::invalid_argument("Invalid Assignment title");
    expect(in,"sense");std::string sense;in>>sense;expect(in,"numberFormat");const auto notation=count(in,1),n=count(in,99),d=count(in,99);
    QJsonObject o{{"format","ToraCloneAssignment"},{"version",1},{"title",QString::fromUtf8(title.data(),static_cast<qsizetype>(title.size()))},{"sense",QString::fromStdString(sense)},{"numberFormat",QJsonArray{notation,n,d}}};
    expect(in,"workers");o["workers"]=names(in);expect(in,"jobs");o["jobs"]=names(in);
    for(const auto& key:{"dummyWorkers","dummyJobs"}){expect(in,key);const auto size=count(in);QJsonArray a;for(int i=0;i<size;++i)a.append(count(in));o[key]=a;}
    expect(in,"forbidden");const auto blocked=count(in,10000);QJsonArray forbidden;for(int i=0;i<blocked;++i){const auto row=count(in),col=count(in);forbidden.append(QJsonArray{row,col});}o["forbidden"]=forbidden;
    expect(in,"costs");const auto rows=count(in),cols=count(in);QJsonArray costs;
    for(int i=0;i<rows;++i){QJsonArray row;for(int j=0;j<cols;++j){double x;if(!(in>>x)||!std::isfinite(x))throw std::invalid_argument("Invalid Assignment TXT cost");row.append(x);}costs.append(row);}o["costs"]=costs;
    expect(in,"end");in>>std::ws;if(!in.eof())throw std::invalid_argument("Unexpected Assignment TXT fields");
    return decodeAssignment(QJsonDocument(o).toJson());
}
AssignmentData decodeAssignmentLp(const QByteArray& bytes) {
    const auto doc=compat::ToraLpCodec::decode(bytes.toStdString());const auto& p=doc.problem;
    const auto reject=[] {throw std::invalid_argument("LP-to-Assignment mapping not validated. Require row-major x_ij variables, ordered worker/job unit equalities, and optional individual x_ij=0 restrictions. Use native Assignment JSON/TXT for ambiguous mappings or dummies.");};
    const auto size=p.variables.size();if(p.constraints.empty())reject();
    const auto& first=p.constraints.front();std::size_t columns=0;
    while(columns<size&&first.coefficients[columns]==1)++columns;
    if(!columns||size%columns)reject();
    const auto rows=size/columns;
    if(rows!=columns||!rows||rows>100||columns>100||p.constraints.size()<rows+columns)reject();
    for(const auto& v:p.variables)if(v.unrestricted||v.lowerBound!=0||(v.upperBound!=1&&v.upperBound!=std::numeric_limits<double>::infinity()))reject();
    for(std::size_t k=0;k<rows+columns;++k){const auto& r=p.constraints[k];if(r.relation!=lp::Relation::Equal||r.rhs!=1)reject();for(std::size_t j=0;j<size;++j){const auto expected=k<rows?(j/columns==k):(j%columns==k-rows);if(r.coefficients[j]!=(expected?1:0))reject();}}
    AssignmentData d;d.title=legacyText(p.title);d.format=doc.inputFormat;d.model.costs=core::Matrix(rows,columns,p.objective);d.model.sense=p.sense==lp::ObjectiveSense::Minimize?assignment::Sense::Minimize:assignment::Sense::Maximize;
    for(std::size_t k=rows+columns;k<p.constraints.size();++k){const auto& r=p.constraints[k];if(r.relation!=lp::Relation::Equal||r.rhs!=0)reject();std::optional<std::size_t> fixed;for(std::size_t j=0;j<size;++j)if(r.coefficients[j]!=0){if(fixed||r.coefficients[j]!=1)reject();fixed=j;}if(!fixed)reject();d.model.forbidden.emplace_back(*fixed/columns,*fixed%columns);}
    for(std::size_t i=0;i<rows;++i)d.workers<<QString("W%1").arg(i+1);
    for(std::size_t j=0;j<columns;++j)d.jobs<<QString("J%1").arg(j+1);
    d.mappingMessage=QString("Validated LP mapping: x(k) -> W(1+floor((k-1)/%1)), J(1+(k-1) mod %1). First %2 equalities are workers; next %1 are jobs. Individual zero-fixing equalities are forbidden pairs. No dummy metadata inferred.").arg(columns).arg(rows);
    return d;
}
}
