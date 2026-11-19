#include "lp_final_controller.hpp"
#include "lp_printing.hpp"
#include "lp_input_controller.hpp"
#include "ui_font.hpp"
#include "ui_colors.hpp"
#include <QAbstractTableModel>
#include <QApplication>
#include <QHeaderView>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QPainter>
#include <QResizeEvent>
#include <algorithm>

namespace tora::desktop {
LpFinalController::LpFinalController(ApplicationShell& shell):QObject(&shell),shell_(shell) {
    setObjectName(QStringLiteral("lp-final-controller"));
    notice=[&shell](const QString& title,const QString& message){QMessageBox box(QMessageBox::Information,title,message,QMessageBox::Ok,&shell);polishUsableFonts(box);box.exec();};
    connect(&inputController(shell),&LpInputController::changed,this,[this]{requested_=false;session_.reset();emit changed();});
}
bool LpFinalController::request() {
    if(!inputController(shell_).document() || !inputController(shell_).validateForSolve())return false;
    shell_.setProperty("lp-output-method",QStringLiteral("final"));requested_=true;session_.reset();emit changed();
    return shell_.navigate(QStringLiteral("lp/output-format"));
}
bool LpFinalController::start(tora::core::NumberFormat format) {
    if(!requested_ || !inputController(shell_).document())return false;
    try {
        (void)tora::core::NumberFormatter(format);
        auto next=std::make_unique<tora::lp::FinalSolutionSession>(inputController(shell_).document()->problem);
        format_=format;session_=std::move(next);emit changed();return shell_.navigate(QStringLiteral("lp/final"));
    }catch(const std::exception& e){notice(QStringLiteral("Final solution"),QString::fromUtf8(e.what()));return false;}
}
LpFinalController& finalController(ApplicationShell& shell) {
    if(auto* c=shell.findChild<LpFinalController*>(QStringLiteral("lp-final-controller")))return *c;
    return *new LpFinalController(shell);
}
namespace {
struct SummaryRow {QStringList cells;bool header=false;bool title=false;};
class SummaryModel final:public QAbstractTableModel {
public:
    SummaryModel(LpFinalController& c,QObject* parent):QAbstractTableModel(parent),c_(c){connect(&c,&LpFinalController::changed,this,[this]{refresh();});refresh();}
    int rowCount(const QModelIndex& parent={}) const override{return parent.isValid()?0:static_cast<int>(rows_.size());}
    int columnCount(const QModelIndex& parent={}) const override{return parent.isValid()?0:5;}
    QVariant data(const QModelIndex& index,int role=Qt::DisplayRole) const override {
        if(!index.isValid() || index.row()>=rowCount() || index.column()>=columnCount())return {};
        const auto& row=rows_[static_cast<std::size_t>(index.row())];
        if(role==Qt::UserRole)return row.title;
        if(role==Qt::DisplayRole)return index.column()<row.cells.size()?row.cells[index.column()]:QString();
        if(role==Qt::BackgroundRole)return QColor(row.header?colors::header:colors::surface);
        if(role==Qt::ForegroundRole)return QColor(colors::text);
        if(role==Qt::TextAlignmentRole)return int((row.title?Qt::AlignHCenter:index.column()==0?Qt::AlignLeft:Qt::AlignRight)|Qt::AlignVCenter);
        if(role==Qt::FontRole){auto font=usableUiFont(QApplication::font());font.setBold(!row.header);return font;}
        return {};
    }
    Qt::ItemFlags flags(const QModelIndex& index) const override{return index.isValid()?Qt::ItemIsEnabled:Qt::NoItemFlags;}
    void refresh(){beginResetModel();rows_.clear();if(c_.session())for(const auto& s:c_.session()->display(c_.format(),true).sections){if(!s.heading.empty())rows_.push_back({{QString::fromStdString(s.heading)},false,true});SummaryRow header;header.header=true;for(const auto& x:s.columns)header.cells<<QString::fromStdString(x);rows_.push_back(std::move(header));for(const auto& r:s.rows){SummaryRow row;row.cells<<QString::fromStdString(r.label);for(const auto& x:r.values)row.cells<<QString::fromStdString(x);rows_.push_back(std::move(row));}rows_.push_back({{},false});}endResetModel();}
private:
    LpFinalController& c_;std::vector<SummaryRow> rows_;
};
class SummaryView final:public QTableView {
public:
    void fit(){if(!model())return;clearSpans();for(int row=0;row<model()->rowCount();++row)if(model()->index(row,0).data(Qt::UserRole).toBool())setSpan(row,0,1,5);const QFontMetrics metrics(usableUiFont(font()));std::vector<int> widths(5,0);int total=0;
        for(int j=0;j<5;++j){for(int i=0;i<model()->rowCount();++i)widths[static_cast<std::size_t>(j)]=std::max(widths[static_cast<std::size_t>(j)],metrics.horizontalAdvance(model()->index(i,j).data().toString()));widths[static_cast<std::size_t>(j)]+=metrics.averageCharWidth()*3;total+=widths[static_cast<std::size_t>(j)];}
        const int extra=std::max(0,viewport()->width()-total);for(int j=0;j<5;++j)setColumnWidth(j,widths[static_cast<std::size_t>(j)]+extra/5);
        verticalHeader()->setDefaultSectionSize(metrics.height()+metrics.height()/2);
    }
protected:
    void resizeEvent(QResizeEvent* e) override{QTableView::resizeEvent(e);fit();}
};
}
bool printSummary(QPrinter& printer,const LpFinalController& c,const QFont& font) {
    return c.session() && printLpSummary(printer,*c.session(),c.format(),font);
}
QWidget* finalScreen(ApplicationShell& shell) {
    auto& c=finalController(shell);auto* page=new QWidget;auto* layout=new QVBoxLayout(page);
    auto* heading=new QLabel(QStringLiteral("LINEAR PROGRAMMING OUTPUT SUMMARY"));heading->setAlignment(Qt::AlignCenter);auto bold=heading->font();bold.setBold(true);heading->setFont(bold);layout->addWidget(heading);
    const auto label=[layout](const QString& id){auto* l=new QLabel;l->setObjectName(id);l->setWordWrap(true);layout->addWidget(l);return l;};
    auto* title=label(QStringLiteral("summary-title"));auto* iteration=label(QStringLiteral("summary-iteration"));auto* objective=label(QStringLiteral("summary-objective"));auto* message=label(QStringLiteral("summary-message"));
    auto* sensitivityNote=label(QStringLiteral("summary-sensitivity-note"));
    sensitivityNote->setText(QStringLiteral("Sensitivity intervals retain the final basis and variable orientation. Other bases may keep the same point optimal outside these intervals."));
    const auto button=[page](const QString& text,const QString& id,QHBoxLayout* row){auto* b=new QPushButton(text,page);b->setObjectName(id);row->addWidget(b);return b;};
    auto* commands=new QHBoxLayout;commands->addStretch();button(QStringLiteral("Next Iteration"),QStringLiteral("summary-next"),commands)->setEnabled(false);button(QStringLiteral("All Iterations"),QStringLiteral("summary-all"),commands)->setEnabled(false);auto* print=button(QStringLiteral("Write to Printer"),QStringLiteral("summary-print"),commands);commands->addStretch();layout->addLayout(commands);
    auto* view=new SummaryView;view->setObjectName(QStringLiteral("summary-grid"));view->setModel(new SummaryModel(c,view));view->horizontalHeader()->hide();view->verticalHeader()->hide();view->setSelectionMode(QAbstractItemView::NoSelection);view->setShowGrid(false);view->setStyleSheet(QStringLiteral("QTableView {background: white; border: 1px solid #879bb5;}"));layout->addWidget(view,1);
    auto* footer=new QHBoxLayout;footer->addStretch();auto* input=button(QStringLiteral("View/Modify Input Data"),QStringLiteral("summary-view-input"),footer);auto* main=button(QStringLiteral("MAIN Menu"),QStringLiteral("summary-main"),footer);auto* exit=button(QStringLiteral("Exit Eidolon"),QStringLiteral("summary-exit"),footer);footer->addStretch();layout->addLayout(footer);
    const auto update=[&c,title,iteration,objective,message,sensitivityNote,view,print]{tora::lp::SummaryDisplay d;if(c.session())d=c.session()->display(c.format(),true);title->setText(QStringLiteral("Title: ")+legacyText(d.title));iteration->setText(QStringLiteral("Final Iteration No.: ")+QString::number(d.iteration));objective->setText(d.objectiveValue?QString::fromStdString(d.objectiveLabel)+QStringLiteral(" =")+QString::fromStdString(*d.objectiveValue):QString());message->setText(QString::fromStdString(d.message));message->setVisible(!d.message.empty());const bool available=!d.sections.empty();
        sensitivityNote->setVisible(available);
        print->setEnabled(available);print->setVisible(available);view->setVisible(available);
        objective->setVisible(d.objectiveValue.has_value());
        for(const auto& id:{QStringLiteral("summary-next"),QStringLiteral("summary-all")})
            print->parentWidget()->findChild<QPushButton*>(id)->setVisible(available);
        view->fit();};
    QObject::connect(&c,&LpFinalController::changed,page,update);QObject::connect(&shell,&ApplicationShell::displayMetricsChanged,page,update);update();
    QObject::connect(print,&QPushButton::clicked,page,[&shell,&c]{if(requestLpPrint(&shell,[&](QPrinter& p){return printSummary(p,c,shell.font());})==PrintOutcome::Failed)c.notice(QStringLiteral("Printer"),QStringLiteral("Could not complete printing to the selected printer"));});
    QObject::connect(input,&QPushButton::clicked,page,[&shell]{shell.navigate(QStringLiteral("lp/input"));});QObject::connect(main,&QPushButton::clicked,page,[&shell]{shell.navigate(QStringLiteral("main"));});QObject::connect(exit,&QPushButton::clicked,page,[&shell]{shell.close();});return page;
}
} // namespace tora::desktop
