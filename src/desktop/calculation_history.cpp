#include "calculation_history.hpp"
#include <algorithm>
namespace tora::desktop {
namespace {
QString number(double x){return QString::number(x,'g',12);}
QString values(const std::vector<double>& v,std::size_t first=0) {
    QStringList parts;for(std::size_t i=first;i<v.size();++i)parts<<number(v[i]);return parts.join("  ");
}
QString matrix(const core::Matrix& a) {
    QStringList lines;for(std::size_t i=0;i<a.rows();++i){QStringList cells;for(std::size_t j=0;j<a.columns();++j)cells<<number(a(i,j)).rightJustified(14);lines<<cells.join(" ");}return lines.join('\n');
}
}
QString assignmentStepText(const AssignmentData& d,const assignment::Result& r,int index) {
    const auto& s=r.history.at(static_cast<std::size_t>(index));
    QString text=QString("%1\nStep %2: %3\n").arg(d.title).arg(index+1).arg(QString::fromStdString(s.event));
    text+=QString("Original objective: %1; workers=%2, jobs=%3.\n").arg(d.model.sense==assignment::Sense::Minimize?"minimize":"maximize").arg(d.workers.size()).arg(d.jobs.size());
    text+="Workers: "+d.workers.join(" | ")+"\nJobs: "+d.jobs.join(" | ")+"\n";
    text+="Forbidden pairs (never candidate edges): ";for(const auto& p:d.model.forbidden)text+=QString("(%1,%2) ").arg(p.first+1).arg(p.second+1);text+='\n';
    text+="Declared dummy workers: ";for(auto i:d.dummyWorkers)text+=QString::number(i+1)+" ";text+="\nDeclared dummy jobs: ";for(auto j:d.dummyJobs)text+=QString::number(j+1)+" ";text+='\n';
    const auto costs=[&](const core::Matrix& a,bool reduced){QStringList lines;for(std::size_t i=0;i<a.rows();++i){QStringList cells;for(std::size_t j=0;j<a.columns();++j){const bool blocked=std::find(d.model.forbidden.begin(),d.model.forbidden.end(),std::make_pair(i,j))!=d.model.forbidden.end();const auto x=a(i,j)-(reduced?s.rowPotential[i+1]+s.columnPotential[j+1]:0);cells<<(blocked?QString("X"):number(x)).rightJustified(14);}lines<<cells.join(' ');}return lines.join('\n');};
    if(index==0)return text+"Original costs/profits (X = forbidden):\n"+costs(d.model.costs,false);
    text+="Square minimization costs (profits negated; extra rows/columns are zero-cost dummies):\n"+costs(r.transformedCosts,false)+"\n";
    text+=QString("Search root row=%1; selected column=%2; delta=%3\n").arg(s.root).arg(s.column).arg(number(s.delta));
    text+="u: "+values(s.rowPotential,1)+"\nv: "+values(s.columnPotential,1)+"\n";
    if(!s.distance.empty())text+="Distances: "+values(s.distance,1)+"\n";
    text+="Matching (row -> column): ";for(std::size_t j=1;j<s.matched.size();++j)if(s.matched[j])text+=QString("%1 -> %2; ").arg(s.matched[j]).arg(j);text+='\n';
    if(!s.used.empty()){text+="Search tree columns: ";for(std::size_t j=0;j<s.used.size();++j)if(s.used[j])text+=QString::number(j)+" ";text+='\n';}
    text+="Predecessor columns: ";for(std::size_t j=1;j<s.previous.size();++j)text+=QString("%1:%2 ").arg(j).arg(s.previous[j]);
    text+="\nReduced costs c' - u - v (X = forbidden):\n";
    text+=costs(r.transformedCosts,true)+"\n";
    if(index+1==static_cast<int>(r.history.size()))text+=r.status==assignment::Status::Optimal?"Original objective = "+number(r.objective):"Status: INFEASIBLE. No admissible complete assignment exists.";
    return text;
}
QString gameStepText(const GamesDocument& d,const games::Result& r,int index) {
    const auto& s=r.history.at(static_cast<std::size_t>(index));
    QString text=QString::fromStdString(d.title())+QString("\nStep %1: ").arg(index+1)+QString::fromStdString(s.event)+"\n"+QString::fromStdString(s.detail)+"\n";
    text+="Row strategy labels: ";for(const auto& name:d.rowNames())text+=QString::fromStdString(name)+" | ";text+="\nColumn strategy labels: ";for(const auto& name:d.columnNames())text+=QString::fromStdString(name)+" | ";text+='\n';
    if(index==0)text+="Original payoff to maximizing row player A:\n"+matrix(d.payoff())+"\n";
    if(r.scale)text+="scale = "+number(r.scale)+"; shift = "+number(r.shift)+"\n";
    if(s.pivot)text+=QString("Pivot row=%1, entering=%2, leaving=%3, divisor=%4\n").arg(s.pivot->row).arg(QString::fromStdString(s.tableau->columns()[s.pivot->column])).arg(QString::fromStdString(s.tableau->columns()[s.pivot->previousBasicColumn])).arg(number(s.pivot->value));
    if(s.tableau){text+="Basic          ";for(const auto& name:s.tableau->columns())text+=QString::fromStdString(name).rightJustified(14)+" ";text+="           RHS\n";for(std::size_t i=0;i<s.tableau->values().rows();++i){text+=(i?QString::fromStdString(s.tableau->basicLabel(i)):QString("z")).leftJustified(15);for(std::size_t j=0;j<s.tableau->values().columns();++j)text+=number(s.tableau->values()(i,j)).rightJustified(14)+" ";text+='\n';}}
    if(s.calculation.rows())text+="Recorded augmented system:\n"+matrix(s.calculation)+"\n";
    if(!s.rowValues.empty())text+="Row values: "+values(s.rowValues)+"\n";
    if(!s.columnValues.empty())text+="Column values: "+values(s.columnValues)+"\n";
    if(index+1==static_cast<int>(r.history.size()))text+="Original game value = "+number(r.value)+"\nLower guarantee = "+number(r.lowerBound)+"; upper guarantee = "+number(r.upperBound)+"; gap = "+number(r.dualityGap)+"\nStatus: optimal (certified)";
    return text;
}
}
