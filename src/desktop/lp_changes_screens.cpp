#include "lp_changes_controller.hpp"
#include "lp_input_controller.hpp"
#include "lp_printing.hpp"
#include "ui_font.hpp"
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QTextDocument>
#include <QPrinter>
#include <QVBoxLayout>
#include <cmath>
#include <algorithm>
#include <QTextOption>
#include <QFileDialog>
#include <QSaveFile>

namespace tora::desktop {
namespace {
QString name(const tora::lp::LpProblem& p,std::size_t j){return QString::fromStdString(p.variableLabel(j));}
QString number(double x,tora::core::NumberFormat f){return QString::fromStdString(tora::core::NumberFormatter(f).format(x));}
QString precise(double x){return QString::number(x,'g',17);}
QString methodName(const LpChangesController& c){return c.method()==LpChangeMethod::Fresh?QStringLiteral("Change Cj / RHS"):QStringLiteral("Dual Revised — RHS changes");}
QLabel* label(QVBoxLayout* layout,const QString& id){auto* text=new QLabel;text->setObjectName(id);text->setWordWrap(true);layout->addWidget(text);return text;}
QTableWidget* table(const QString& id,const QStringList& headers,QWidget* parent=nullptr) {
    auto* t=new QTableWidget(0,headers.size(),parent);t->setObjectName(id);t->setHorizontalHeaderLabels(headers);t->verticalHeader()->hide();
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);t->horizontalHeader()->setStretchLastSection(true);
    t->setAlternatingRowColors(true);t->setSelectionBehavior(QAbstractItemView::SelectItems);return t;
}
void cell(QTableWidget* t,int row,int col,const QString& text,bool editable=false) {
    auto* item=new QTableWidgetItem(text);item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable|(editable?Qt::ItemIsEditable:Qt::NoItemFlags));
    item->setTextAlignment((col?Qt::AlignRight:Qt::AlignLeft)|Qt::AlignVCenter);t->setItem(row,col,item);
}
QPushButton* command(QHBoxLayout* row,const QString& text,const QString& id){auto* b=new QPushButton(text);b->setObjectName(id);row->addWidget(b);return b;}
QPushButton* command(QGridLayout* grid,const QString& text,const QString& id,int row,int column){auto* b=new QPushButton(text);b->setObjectName(id);grid->addWidget(b,row,column);return b;}
QString status(const LpChangesController& c) {
    if(c.fresh())switch(c.fresh()->after.status) {
        case tora::lp::SimplexStatus::Optimal:return QStringLiteral("Optimal");
        case tora::lp::SimplexStatus::Alternative:return QStringLiteral("Optimal — alternative solutions exist");
        case tora::lp::SimplexStatus::Infeasible:return QStringLiteral("Changed model is infeasible");
        case tora::lp::SimplexStatus::Unbounded:return QStringLiteral("Changed model is unbounded");
        case tora::lp::SimplexStatus::Running:return QStringLiteral("Iteration limit reached");
    }
    if(c.revised())switch(c.revised()->canonical.status) {
        case tora::lp::RevisedDualStatus::Optimal:return QStringLiteral("Optimal");
        case tora::lp::RevisedDualStatus::Infeasible:return QStringLiteral("Changed model is infeasible");
        case tora::lp::RevisedDualStatus::Limit:return QStringLiteral("Iteration limit reached");
    }
    return QStringLiteral("No current result. Analyze the changes first.");
}
std::optional<double> objective(const LpChangesController& c){return c.fresh()?c.fresh()->after.objective:c.revised()?c.revised()->objective:std::optional<double>{};}
std::vector<double> values(const LpChangesController& c){return c.fresh()?c.fresh()->after.values:c.revised()?c.revised()->values:std::vector<double>{};}
QString diagnosis(const LpChangesController& c) {
    if(c.fresh())return QStringLiteral("Retained basis: primal %1; dual %2. Changed model solved afresh.")
        .arg(c.fresh()->retained.primalFeasible?QStringLiteral("feasible"):QStringLiteral("infeasible"),c.fresh()->retained.dualFeasible?QStringLiteral("feasible"):QStringLiteral("infeasible"));
    if(c.revised())return QStringLiteral("Baseline optimum reused; changed RHS solved by Dual Revised. Trace shows canonical variables.");
    return {};
}
std::size_t traceCount(const LpChangesController& c){return c.fresh()?c.fresh()->after.trace.size():c.revised()?c.revised()->canonical.trace.size():0;}
QString canonicalLabel(const tora::lp::RevisedDualModel& model,std::size_t column) {
    for(std::size_t j=0;j<model.variables.size();++j){const auto& map=model.variables[j];
        if(map.positiveColumn==column)return name(model.original,j)+(map.negativeColumn?QStringLiteral("+"):map.shift!=0?QStringLiteral(" (shifted)"):QString());
        if(map.negativeColumn && *map.negativeColumn==column)return name(model.original,j)+QStringLiteral("-");
    }
    for(std::size_t i=0;i<model.rowSlacks.size();++i)if(model.rowSlacks[i] && *model.rowSlacks[i]==column)return QStringLiteral("slack row %1").arg(i+1);
    return QStringLiteral("column %1").arg(column+1);
}
struct TraceDisplay {QString pivot;QStringList headers;QList<QStringList> rows;};
TraceDisplay trace(const LpChangesController& c,std::size_t selected,bool baseline=false) {
    TraceDisplay out;if(!c.hasResult())return out;
    if(c.fresh()) {
        const auto& history=baseline?c.fresh()->before.trace:c.fresh()->after.trace;
        if(selected>=history.size())return out;
        const auto& step=history[selected];const auto& t=step.tableau;out.headers<<QStringLiteral("Basic");
        for(const auto& col:t.columns())out.headers<<QString::fromStdString(col);
        out.headers<<QStringLiteral("RHS");
        for(std::size_t i=0;i<t.values().rows();++i){QStringList row{i?QString::fromStdString(t.basicLabel(i)):QStringLiteral("z")};for(std::size_t j=0;j<t.values().columns();++j)row<<number(t.values()(i,j),c.format());out.rows<<row;}
        out.pivot=(baseline?QStringLiteral("Baseline solve"):QStringLiteral("Changed-model solve"))
            +QStringLiteral(" — canonical tableau; phase %1").arg(step.phase);
    }else {
        if(selected>=traceCount(c))return out;
        const auto model=tora::lp::makeDualRevisedModel(*c.changedProblem());const auto& step=c.revised()->canonical.trace[selected];
        out.headers={QStringLiteral("Entry"),QStringLiteral("Variable / constraint"),QStringLiteral("Value"),QStringLiteral("Reduced cost")};
        for(std::size_t i=0;i<step.basis.size();++i)out.rows<<QStringList{QStringLiteral("Basic"),canonicalLabel(model,step.basis[i]),number(step.basicValues[i],c.format()),number(step.reducedCosts[step.basis[i]],c.format())};
        for(std::size_t i=0;i<step.dual.size();++i)out.rows<<QStringList{QStringLiteral("Dual price"),QStringLiteral("row %1").arg(i+1),number(step.dual[i],c.format()),QString()};
        for(std::size_t j=0;j<step.reducedCosts.size();++j)if(std::find(step.basis.begin(),step.basis.end(),j)==step.basis.end())out.rows<<QStringList{QStringLiteral("Nonbasic"),canonicalLabel(model,j),number(0,c.format()),number(step.reducedCosts[j],c.format())};
        if(selected+1==traceCount(c) && c.revised()->canonical.status==tora::lp::RevisedDualStatus::Infeasible)
            for(std::size_t i=0;i<c.revised()->canonical.infeasibilityWitness.size();++i)out.rows<<QStringList{QStringLiteral("Infeasibility witness"),QStringLiteral("row %1").arg(i+1),number(c.revised()->canonical.infeasibilityWitness[i],c.format()),QString()};
        out.pivot=step.leavingRow && step.enteringColumn?QStringLiteral("Leaving: %1. Entering: %2.").arg(canonicalLabel(model,step.basis[*step.leavingRow]),canonicalLabel(model,*step.enteringColumn)):QStringLiteral("No pivot — ")+status(c);
    }
    return out;
}
QList<QStringList> solutionRows(const LpChangesController& c) {
    QList<QStringList> rows;if(!c.baseline())return rows;
    const auto after=values(c);const auto z=objective(c);const auto before=c.baselineObjective();
    rows<<QStringList{QStringLiteral("Objective"),before?number(*before,c.format()):QStringLiteral("—"),z?number(*z,c.format()):QStringLiteral("—")};
    const auto& p=c.baseline()->problem;
    for(std::size_t j=0;j<p.variables.size();++j)rows<<QStringList{name(p,j),j<c.baselineValues().size()?number(c.baselineValues()[j],c.format()):QStringLiteral("—"),j<after.size()?number(after[j],c.format()):QStringLiteral("—")};
    if(c.changedProblem()) {
        const auto& changed=*c.changedProblem();
        for(std::size_t j=0;j<p.variables.size();++j)rows<<QStringList{QStringLiteral("Objective contribution ")+name(p,j),j<c.baselineValues().size()?number(p.objective[j]*c.baselineValues()[j],c.format()):QStringLiteral("—"),j<after.size()?number(changed.objective[j]*after[j],c.format()):QStringLiteral("—")};
        for(std::size_t i=0;i<p.constraints.size();++i) {
            const auto gap=[&](const tora::lp::LpProblem& model,const std::vector<double>& x){if(x.size()!=model.variables.size())return QStringLiteral("—");const auto& r=model.constraints[i];double lhs=0;for(std::size_t j=0;j<x.size();++j)lhs+=r.coefficients[j]*x[j];return number(r.relation==tora::lp::Relation::GreaterEqual?lhs-r.rhs:r.rhs-lhs,c.format());};
            rows<<QStringList{QStringLiteral("Constraint %1 slack / surplus").arg(i+1),gap(p,c.baselineValues()),gap(changed,after)};
        }
    }
    return rows;
}
void fill(QTableWidget* t,const QList<QStringList>& rows){t->setRowCount(static_cast<int>(rows.size()));for(int i=0;i<rows.size();++i)for(int j=0;j<rows[i].size();++j)cell(t,i,j,rows[i][j]);}
}
QWidget* changesScreen(ApplicationShell& shell) {
    auto& c=changesController(shell);auto* page=new QWidget;auto* layout=new QVBoxLayout(page);
    auto* heading=label(layout,QStringLiteral("changes-heading"));auto* help=label(layout,QStringLiteral("changes-help"));
    auto* tabs=new QTabWidget;tabs->setObjectName(QStringLiteral("changes-tabs"));
    auto* costs=table(QStringLiteral("changes-costs"),{QStringLiteral("Variable"),QStringLiteral("Baseline Cj"),QStringLiteral("Changed Cj")});
    auto* rhs=table(QStringLiteral("changes-rhs"),{QStringLiteral("Constraint"),QStringLiteral("Baseline RHS"),QStringLiteral("Changed RHS")});
    tabs->addTab(costs,QStringLiteral("Objective coefficients"));tabs->addTab(rhs,QStringLiteral("Constraint RHS"));layout->addWidget(tabs,1);
    auto* format=new QGroupBox(QStringLiteral("Output format"));auto* form=new QHBoxLayout(format);
    auto* notation=new QComboBox;notation->setObjectName(QStringLiteral("changes-notation"));notation->addItems({QStringLiteral("Decimal"),QStringLiteral("Scientific")});notation->setAccessibleName(QStringLiteral("Output notation"));form->addWidget(notation);
    auto* n=new QSpinBox;auto* d=new QSpinBox;n->setObjectName(QStringLiteral("changes-N"));d->setObjectName(QStringLiteral("changes-D"));n->setRange(0,99);d->setRange(0,99);
    n->setAccessibleName(QStringLiteral("Output N"));d->setAccessibleName(QStringLiteral("Output D"));form->addWidget(new QLabel(QStringLiteral("N:")));form->addWidget(n);form->addWidget(new QLabel(QStringLiteral("D:")));form->addWidget(d);form->addStretch();layout->addWidget(format);
    auto* buttons=new QHBoxLayout;auto* analyze=command(buttons,QStringLiteral("Analyze changes"),QStringLiteral("changes-analyze"));auto* input=command(buttons,QStringLiteral("View input"),QStringLiteral("changes-input"));auto* main=command(buttons,QStringLiteral("MAIN Menu"),QStringLiteral("changes-main"));layout->addLayout(buttons);
    const auto refresh=[&c,heading,help,costs,rhs,n,d,notation,analyze,tabs]{
        const QSignalBlocker bc(costs),br(rhs),bn(n),bd(d),bf(notation);heading->setText(methodName(c));
        const bool revised=c.method()==LpChangeMethod::RevisedDual;tabs->setCurrentIndex(revised?1:0);
        help->setText(revised?QStringLiteral("Change constraint RHS values. Objective coefficients stay fixed. Input remains unchanged until you use the changed model."):QStringLiteral("Change objective coefficients or constraint RHS values. Input remains unchanged until you use the changed model."));
        costs->setRowCount(0);rhs->setRowCount(0);analyze->setEnabled(c.baseline()!=nullptr);
        if(!c.baseline())return;
        const auto& p=c.baseline()->problem;costs->setRowCount(static_cast<int>(p.variables.size()));rhs->setRowCount(static_cast<int>(p.constraints.size()));
        for(std::size_t j=0;j<p.variables.size();++j){cell(costs,static_cast<int>(j),0,name(p,j));cell(costs,static_cast<int>(j),1,precise(p.objective[j]));cell(costs,static_cast<int>(j),2,precise(p.objective[j]),!revised);}
        for(std::size_t i=0;i<p.constraints.size();++i){cell(rhs,static_cast<int>(i),0,QString::number(i+1));cell(rhs,static_cast<int>(i),1,precise(p.constraints[i].rhs));cell(rhs,static_cast<int>(i),2,precise(p.constraints[i].rhs),true);}
        notation->setCurrentIndex(c.format().notation==tora::core::Notation::Scientific?1:0);n->setValue(c.format().n);d->setValue(c.format().d);
    };
    QObject::connect(&c,&LpChangesController::baselineChanged,page,refresh);refresh();
    QObject::connect(costs,&QTableWidget::itemChanged,page,[&c]{c.clearResult();});QObject::connect(rhs,&QTableWidget::itemChanged,page,[&c]{c.clearResult();});
    for(auto* spin:{n,d})QObject::connect(spin,&QSpinBox::valueChanged,page,[&c]{c.clearResult();});
    QObject::connect(notation,&QComboBox::currentIndexChanged,page,[&c]{c.clearResult();});
    QObject::connect(analyze,&QPushButton::clicked,page,[&c,costs,rhs,n,d,notation,analyze]{
        analyze->setFocus();std::vector<double> objective,rhsValues;
        for(auto* grid:{costs,rhs})for(int i=0;i<grid->rowCount();++i){bool ok=false;const double x=grid->item(i,2)->text().trimmed().toDouble(&ok);if(!ok || !std::isfinite(x)){c.clearResult();grid->setCurrentCell(i,2);c.notice(QStringLiteral("LP changes"),QStringLiteral("Enter a finite number in %1 row %2").arg(grid==costs?QStringLiteral("objective"):QStringLiteral("RHS")).arg(i+1));return;}(grid==costs?objective:rhsValues).push_back(x);}
        c.analyze(objective,rhsValues,{notation->currentIndex()?tora::core::Notation::Scientific:tora::core::Notation::Decimal,n->value(),d->value()});
    });
    QObject::connect(input,&QPushButton::clicked,page,[&shell]{shell.navigate(QStringLiteral("lp/input"));});QObject::connect(main,&QPushButton::clicked,page,[&shell]{shell.navigate(QStringLiteral("main"));});return page;
}
QWidget* changesResultScreen(ApplicationShell& shell) {
    auto& c=changesController(shell);auto* page=new QWidget;auto* layout=new QVBoxLayout(page);
    auto* heading=label(layout,QStringLiteral("changes-result-heading"));auto* state=label(layout,QStringLiteral("changes-status"));auto* retained=label(layout,QStringLiteral("changes-diagnosis"));
    auto* tabs=new QTabWidget;auto* solution=table(QStringLiteral("changes-solution"),{QStringLiteral("Entry"),QStringLiteral("Baseline"),QStringLiteral("Changed")});tabs->addTab(solution,QStringLiteral("Solution"));
    auto* tracePage=new QWidget;auto* traceLayout=new QVBoxLayout(tracePage);auto* selection=new QHBoxLayout;
    selection->addWidget(new QLabel(QStringLiteral("Trace step:")));auto* step=new QSpinBox;step->setObjectName(QStringLiteral("changes-trace-step"));step->setAccessibleName(QStringLiteral("Trace step"));selection->addWidget(step);selection->addStretch();traceLayout->addLayout(selection);
    auto* pivot=label(traceLayout,QStringLiteral("changes-pivot"));auto* grid=table(QStringLiteral("changes-trace"),{});traceLayout->addWidget(grid,1);tabs->addTab(tracePage,QStringLiteral("Trace"));tabs->setObjectName(QStringLiteral("changes-result-tabs"));layout->addWidget(tabs,1);
    auto* row=new QGridLayout;
    auto* apply=command(row,QStringLiteral("Use changed model"),QStringLiteral("changes-apply"),0,0);
    auto* print=command(row,QStringLiteral("Print all steps"),QStringLiteral("changes-print"),0,1);
    auto* exportAll=command(row,QStringLiteral("Export all steps (TXT)"),QStringLiteral("changes-export-all"),1,0);
    auto* printSelected=command(row,QStringLiteral("Print selected step"),QStringLiteral("changes-print-selected"),1,1);
    auto* back=command(row,QStringLiteral("Edit changes"),QStringLiteral("changes-back"),2,0);
    auto* input=command(row,QStringLiteral("View input"),QStringLiteral("changes-result-input"),2,1);layout->addLayout(row);
    const auto refreshTrace=[&c,step,pivot,grid]{const auto view=trace(c,static_cast<std::size_t>(std::max(0,step->value()-1)));pivot->setText(view.pivot);grid->clear();grid->setColumnCount(static_cast<int>(view.headers.size()));grid->setHorizontalHeaderLabels(view.headers);fill(grid,view.rows);};
    const auto refresh=[&c,heading,state,retained,solution,step,apply,print,printSelected,exportAll,back,refreshTrace]{heading->setText(methodName(c));state->setText(c.hasResult()?QStringLiteral("Changed model: ")+status(c):status(c));retained->setText(diagnosis(c));fill(solution,solutionRows(c));const QSignalBlocker blocked(step);const auto count=traceCount(c);step->setRange(1,static_cast<int>(std::max(std::size_t(1),count)));step->setValue(static_cast<int>(std::max(std::size_t(1),count)));step->setEnabled(count>0);apply->setEnabled(c.hasResult());print->setEnabled(c.hasResult());printSelected->setEnabled(c.hasResult());exportAll->setEnabled(c.hasResult());back->setEnabled(c.baseline()!=nullptr);refreshTrace();};
    QObject::connect(step,&QSpinBox::valueChanged,page,[refreshTrace]{refreshTrace();});QObject::connect(&c,&LpChangesController::resultChanged,page,refresh);QObject::connect(&c,&LpChangesController::baselineChanged,page,refresh);refresh();
    QObject::connect(apply,&QPushButton::clicked,page,[&c]{c.apply();});QObject::connect(back,&QPushButton::clicked,page,[&shell]{shell.navigate(QStringLiteral("lp/changes"));});QObject::connect(input,&QPushButton::clicked,page,[&shell]{shell.navigate(QStringLiteral("lp/input"));});
    QObject::connect(print,&QPushButton::clicked,page,[&shell,&c]{if(requestLpPrint(&shell,[&](QPrinter& printer){return printChanges(printer,c,shell.font());})==PrintOutcome::Failed)c.notice(QStringLiteral("Printer"),QStringLiteral("Could not complete printing to the selected printer"));});
    QObject::connect(printSelected,&QPushButton::clicked,page,[&shell,&c,step]{if(requestLpPrint(&shell,[&](QPrinter& printer){return printChanges(printer,c,shell.font(),static_cast<std::size_t>(step->value()-1));})==PrintOutcome::Failed)c.notice(QStringLiteral("Printer"),QStringLiteral("Could not complete printing to the selected printer"));});
    QObject::connect(exportAll,&QPushButton::clicked,page,[&shell,&c]{
        const auto path=QFileDialog::getSaveFileName(&shell,QStringLiteral("Export all recorded steps"),{},QStringLiteral("Text Files (*.txt)"));
        if(!path.isEmpty() && !exportChangesHistory(path,c))c.notice(QStringLiteral("Export history"),QStringLiteral("Could not export all recorded steps"));
    });return page;
}
namespace {
QString changesReport(const LpChangesController& c,std::optional<std::size_t> selected) {
    if(!c.hasResult() || !c.baseline() || !c.changedProblem() || traceCount(c)==0 ||
       (selected && *selected>=traceCount(c)))return {};
        QString html=QStringLiteral("<h1>Linear Programming — Class extensions</h1>");
        const auto line=[&html](const QString& text){html+=QStringLiteral("<p>")+text.toHtmlEscaped()+QStringLiteral("</p>");};
        line(methodName(c));line(QStringLiteral("Title: ")+legacyText(c.baseline()->problem.title));line(status(c));line(diagnosis(c));
        const auto dataset=[&](const tora::lp::LpProblem& p,const QString& heading){html+=QStringLiteral("<h2>")+heading+QStringLiteral("</h2>");QStringList costs;for(std::size_t j=0;j<p.objective.size();++j)costs<<precise(p.objective[j])+QStringLiteral(" ")+name(p,j);line((p.sense==tora::lp::ObjectiveSense::Maximize?QStringLiteral("Maximize: "):QStringLiteral("Minimize: "))+costs.join(QStringLiteral(" + ")));
            for(std::size_t i=0;i<p.constraints.size();++i){const auto& r=p.constraints[i];QStringList terms;for(std::size_t j=0;j<r.coefficients.size();++j)terms<<precise(r.coefficients[j])+QStringLiteral(" ")+name(p,j);line(QStringLiteral("Constraint %1: ").arg(i+1)+terms.join(QStringLiteral(" + "))+(r.relation==tora::lp::Relation::LessEqual?QStringLiteral(" <= "):r.relation==tora::lp::Relation::GreaterEqual?QStringLiteral(" >= "):QStringLiteral(" = "))+precise(r.rhs));}
            for(std::size_t j=0;j<p.variables.size();++j){const auto& v=p.variables[j];line(name(p,j)+(v.unrestricted?QStringLiteral(": unrestricted"):QStringLiteral(": lower ")+QString::fromStdString(tora::core::NumberFormatter(c.format()).formatBound(v.lowerBound))+QStringLiteral(", upper ")+QString::fromStdString(tora::core::NumberFormatter(c.format()).formatBound(v.upperBound))));}};
        dataset(c.baseline()->problem,QStringLiteral("Baseline model"));dataset(*c.changedProblem(),QStringLiteral("Changed model"));html+=QStringLiteral("<h2>Solution (baseline; changed)</h2>");
        for(const auto& row:solutionRows(c))line(row.join(QStringLiteral("; ")));
        const auto append=[&](std::size_t index,const QString& heading,bool baseline=false){
            const auto view=trace(c,index,baseline);html+=QStringLiteral("<h2>")+heading+QStringLiteral("</h2>");
            line(view.pivot);line(view.headers.join(QStringLiteral("; ")));
            for(const auto& row:view.rows) {
                if(view.headers.size()<=8){line(row.join(QStringLiteral("; ")));continue;}
                QStringList labeled;for(int j=1;j<row.size();++j)labeled<<view.headers[j]+QStringLiteral(" = ")+row[j];
                line(row.front()+QStringLiteral(": ")+labeled.join(QStringLiteral("; ")));
            }
        };
        if(selected)append(*selected,QStringLiteral("Selected trace step %1 of %2").arg(*selected+1).arg(traceCount(c)));
        else {
            html+=QStringLiteral("<h2>All recorded steps</h2>");
            if(c.fresh()) {
                const auto count=c.fresh()->before.trace.size();
                for(std::size_t i=0;i<count;++i)append(i,QStringLiteral("Baseline trace step %1 of %2").arg(i+1).arg(count),true);
            }else line(QStringLiteral("Baseline optimum reused; no baseline iteration history is stored in this result."));
            for(std::size_t i=0;i<traceCount(c);++i)append(i,QStringLiteral("Changed trace step %1 of %2").arg(i+1).arg(traceCount(c)));
        }
        return html;
}
}
QString changesHistoryText(const LpChangesController& c) {
    QTextDocument document;document.setHtml(changesReport(c,{}));return document.toPlainText();
}
bool exportChangesHistory(const QString& path,const LpChangesController& c) {
    const auto bytes=changesHistoryText(c).toUtf8();if(bytes.isEmpty())return false;
    QSaveFile file(path);return file.open(QIODevice::WriteOnly) && file.write(bytes)==bytes.size() && file.commit();
}
bool printChanges(QPrinter& printer,const LpChangesController& c,const QFont& font,std::optional<std::size_t> traceStep) {
    try {
        const auto html=changesReport(c,traceStep);if(html.isEmpty())return false;
        QTextDocument document;
        // A screen monospace font may lack a bold face; Qt simulates it with
        // overlapping text paint operations, duplicating extracted headings.
        const auto screenFont=usableUiFont(font);
        QFont printFont(QStringLiteral("sans-serif"),12);
        if(screenFont.pointSizeF()>0)printFont.setPointSizeF(screenFont.pointSizeF());
        document.setDefaultFont(printFont);
        auto option=document.defaultTextOption();option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);document.setDefaultTextOption(option);
        document.setHtml(html);document.print(&printer);return printer.printerState()!=QPrinter::Error && printer.printerState()!=QPrinter::Aborted;
    }catch(const std::exception&){return false;}
}
}
