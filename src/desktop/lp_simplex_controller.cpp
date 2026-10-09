#include "lp_simplex_controller.hpp"
#include "lp_input_controller.hpp"
#include "ui_font.hpp"
#include <QAbstractTableModel>
#include <QApplication>
#include <QColor>
#include <QTableView>
#include <QHeaderView>
#include <QScrollBar>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QStyle>
#include <QLocale>
#include <cmath>
#include <algorithm>

namespace tora::desktop {
using tora::lp::SimplexStatus;
LpSimplexController::LpSimplexController(ApplicationShell& shell):QObject(&shell),shell_(shell) {
    setObjectName(QStringLiteral("lp-simplex-controller"));
    notice=[&shell](const QString& title,const QString& body){
        QMessageBox box(QMessageBox::Information,title,body,QMessageBox::Ok,&shell);polishUsableFonts(box);box.exec();};
    redirect=[&shell](const QString& body){QMessageBox box(body.startsWith(QStringLiteral("Dual "))?QMessageBox::Critical:QMessageBox::Warning,QStringLiteral("Warning"),body,QMessageBox::Ok|QMessageBox::Cancel,&shell);
        polishUsableFonts(box);return box.exec()==QMessageBox::Ok;};
    singleWarning=[&shell](const QString& body){QMessageBox box(QMessageBox::Critical,QStringLiteral("Warning"),body,QMessageBox::Ok,&shell);polishUsableFonts(box);box.exec();};
    connect(&inputController(shell),&LpInputController::changed,this,[this]{timer_.stop();busy_=false;requested_=false;mPending_=false;session_.reset();emit changed();});
    connect(&shell,&ApplicationShell::screenChanged,this,[this](const QString& screen){if(screen!=QStringLiteral("lp/simplex") && busy_){timer_.stop();busy_=false;emit changed();}});
    timer_.setInterval(0);connect(&timer_,&QTimer::timeout,this,&LpSimplexController::runOne);
}
bool LpSimplexController::requestAllSlack() {
    const auto* doc=inputController(shell_).document();if(!doc)return false;
    if(!inputController(shell_).validateForSolve())return false;
    timer_.stop();busy_=false;requested_=false;mPending_=false;session_.reset();
    requestedMethod_=tora::lp::PrimalMethod::AllSlack;
    try {
        if(tora::lp::AllSlackSession::requiresArtificial(doc->problem)) {
            const bool twoPhase=redirect(QString::fromLatin1(tora::lp::AllSlackSession::redirectWarning()));
            if(twoPhase){requestedMethod_=tora::lp::PrimalMethod::TwoPhase;requested_=true;}
            else {requestedMethod_=tora::lp::PrimalMethod::MMethod;mPending_=true;penalty_=100;}emit changed();
            shell_.navigate(twoPhase?QStringLiteral("lp/output-format"):QStringLiteral("lp/m-value"));return false;
        }
        requested_=true;emit changed();shell_.navigate(QStringLiteral("lp/output-format"));return true;
    }catch(const std::exception& e){notice(QStringLiteral("Input Data Error"),QString::fromUtf8(e.what()));return false;}
}
bool LpSimplexController::requestMMethod() {
    const auto* doc=inputController(shell_).document();if(!doc || !inputController(shell_).validateForSolve())return false;
    timer_.stop();busy_=false;requested_=false;mPending_=false;session_.reset();
    requestedMethod_=tora::lp::PrimalMethod::MMethod;penalty_=100;
    try {
        if(!tora::lp::PrimalSimplexSession::requiresArtificial(doc->problem)) {
            (void)redirect(QString::fromLatin1(tora::lp::MMethodSession::allSlackWarning())); // Captured OK and Cancel both continue.
            requestedMethod_=tora::lp::PrimalMethod::AllSlack;requested_=true;emit changed();shell_.navigate(QStringLiteral("lp/output-format"));return true;
        }
        mPending_=true;emit changed();shell_.navigate(QStringLiteral("lp/m-value"));return true;
    }catch(const std::exception& e){notice(QStringLiteral("Input Data Error"),QString::fromUtf8(e.what()));return false;}
}
bool LpSimplexController::setPenalty(const QString& text) {
    if(!mPending_ || !inputController(shell_).document())return false;
    bool ok=false;auto value=QLocale::c().toDouble(text,&ok);
    if(!ok)value=0; // Captured nonnumeric M="bogus" behaves as zero.
    if(!std::isfinite(value)){notice(QStringLiteral("M-Method"),QStringLiteral("M must be a finite number"));return false;}
    penalty_=value;requested_=true;emit changed();shell_.navigate(QStringLiteral("lp/output-format"));return true;
}
bool LpSimplexController::requestTwoPhase() {
    const auto* doc=inputController(shell_).document();if(!doc || !inputController(shell_).validateForSolve())return false;
    timer_.stop();busy_=false;requested_=false;mPending_=false;session_.reset();
    requestedMethod_=tora::lp::PrimalMethod::TwoPhase;
    try {
        if(!tora::lp::PrimalSimplexSession::requiresArtificial(doc->problem)) {
            (void)redirect(QString::fromLatin1(tora::lp::MMethodSession::allSlackWarning()));
            requestedMethod_=tora::lp::PrimalMethod::AllSlack;
        }
        requested_=true;emit changed();shell_.navigate(QStringLiteral("lp/output-format"));return true;
    }catch(const std::exception& e){notice(QStringLiteral("Input Data Error"),QString::fromUtf8(e.what()));return false;}
}
bool LpSimplexController::requestBounded() {
    const auto* doc=inputController(shell_).document();if(!doc || !inputController(shell_).validateForSolve())return false;
    timer_.stop();busy_=false;requested_=false;mPending_=false;session_.reset();
    try {
        // Bounded reaches output formatting directly, without either shared
        // method's warning. Dispatch after normalization and lower-bound shifts.
        (void)tora::lp::PrimalSimplexSession::requiresArtificial(doc->problem);
        requestedMethod_=tora::lp::PrimalMethod::Bounded;
        requested_=true;emit changed();shell_.navigate(QStringLiteral("lp/output-format"));return true;
    }catch(const std::exception& e){notice(QStringLiteral("Input Data Error"),QString::fromUtf8(e.what()));return false;}
}
bool LpSimplexController::requestDual() {
    const auto* doc=inputController(shell_).document();if(!doc || !inputController(shell_).validateForSolve())return false;
    timer_.stop();busy_=false;requested_=false;mPending_=false;session_.reset();
    try {
        requestedMethod_=tora::lp::PrimalSimplexSession::dualSetup(doc->problem);
        if(requestedMethod_==tora::lp::PrimalMethod::AllSlack)
            singleWarning(QStringLiteral("Dual Simplex not applicable directly. All-slack will be used"));
        else if(requestedMethod_==tora::lp::PrimalMethod::TwoPhase){
            const auto warning=tora::lp::PrimalSimplexSession::dualBoundsFallback(doc->problem)
                ?QStringLiteral("Dual simplex not applicable because a var has upper/lower bound or is unrestricted. Two-phase method will be used; else click CANCEL to use M-method")
                :QStringLiteral("Dual simplex not applicable directly. Two-phase method will be used; else click CANCEL to use M-method");
            if(!redirect(warning)){requestedMethod_=tora::lp::PrimalMethod::MMethod;penalty_=100;mPending_=true;emit changed();shell_.navigate(QStringLiteral("lp/m-value"));return true;}
        }
        requested_=true;emit changed();shell_.navigate(QStringLiteral("lp/output-format"));return true;
    }catch(const std::exception& e){notice(QStringLiteral("Dual Simplex Error"),QString::fromUtf8(e.what()));emit changed();return false;}
}
bool LpSimplexController::start(tora::core::NumberFormat format) {
    if(!requested_ || !inputController(shell_).document())return false;
    try {
        (void)tora::core::NumberFormatter(format);
        std::unique_ptr<tora::lp::PrimalSimplexSession> next;
        if(requestedMethod_==tora::lp::PrimalMethod::Dual)next=std::make_unique<tora::lp::DualSession>(inputController(shell_).document()->problem);
        else if(requestedMethod_==tora::lp::PrimalMethod::Bounded)next=std::make_unique<tora::lp::BoundedSession>(inputController(shell_).document()->problem);
        else if(requestedMethod_==tora::lp::PrimalMethod::MMethod)next=std::make_unique<tora::lp::MMethodSession>(inputController(shell_).document()->problem,penalty_);
        else if(requestedMethod_==tora::lp::PrimalMethod::TwoPhase)next=std::make_unique<tora::lp::TwoPhaseSession>(inputController(shell_).document()->problem);
        else next=std::make_unique<tora::lp::AllSlackSession>(inputController(shell_).document()->problem);
        format_=format;session_=std::move(next);emit changed();shell_.navigate(QStringLiteral("lp/simplex"));terminal();return true;
    }catch(const std::exception& e){notice(QStringLiteral("Simplex"),QString::fromUtf8(e.what()));return false;}
}
bool LpSimplexController::canNext() const {
    return session_ && !busy_ && (session_->status()==SimplexStatus::Running ||
        (session_->status()==SimplexStatus::Alternative && session_->history().back().entering.has_value()));
}
void LpSimplexController::selectCell(int row,int col) {
    if(!session_ || busy_)return;
    const int block=static_cast<int>(session_->problem().constraints.size())+7;
    const int first=static_cast<int>(session_->history().size()-1)*block;
    if(row<first)return;
    const int relative=row-first;
    try {
        if(relative==1 && col>0)session_->selectEntering(static_cast<std::size_t>(col-1));
        else if(col==0 && relative>=3 && relative<block-4)session_->selectLeaving(static_cast<std::size_t>(relative-2));
        else return;
        emit changed();
    }catch(const std::invalid_argument& e){notice(relative==1?QStringLiteral("Entering Variable"):QStringLiteral("Leaving Variable"),QString::fromUtf8(e.what()));}
}
void LpSimplexController::terminal() {
    if(session_ && session_->status()!=SimplexStatus::Running)notice(QString::fromStdString(session_->terminalTitle()),QString::fromStdString(session_->terminalMessage()));
}
void LpSimplexController::next() {
    if(!canNext())return;
    try {
        const bool dualAlternative=session_->method()==tora::lp::PrimalMethod::Dual && session_->status()==SimplexStatus::Alternative;
        session_->next();emit changed();
        if(dualAlternative)return; // TORA appends the alternative table without another notice.
        if(session_ && session_->phaseOneComplete())notice(QStringLiteral("Phase 1"),QStringLiteral("End of Phase 1: Feasible solution found--initiating Phase 2"));
        else terminal();
    }
    catch(const std::exception& e){notice(QStringLiteral("Simplex"),QString::fromUtf8(e.what()));}
}
void LpSimplexController::all() {
    if(!session_ || busy_ || session_->status()!=SimplexStatus::Running)return;
    busy_=true;automaticSteps_=0;emit changed();timer_.start();
}
void LpSimplexController::runOne() {
    if(!session_ || !busy_){timer_.stop();return;}
    try {
        if(++automaticSteps_>1024)throw std::runtime_error("Native simplex iteration safety limit reached; the partial trace is retained");
        session_->next();emit changed();
        if(session_->status()!=SimplexStatus::Running){timer_.stop();busy_=false;emit changed();terminal();}
    }catch(const std::exception& e){timer_.stop();busy_=false;emit changed();notice(QStringLiteral("Simplex"),QString::fromUtf8(e.what()));}
}
LpSimplexController& simplexController(ApplicationShell& shell) {
    if(auto* existing=shell.findChild<LpSimplexController*>(QStringLiteral("lp-simplex-controller")))return *existing;
    return *new LpSimplexController(shell);
}
namespace {
class TraceView final:public QTableView {
public:
    TraceView():labelHost_(new QWidget),labels_(new QTableView(labelHost_)),labelLayout_(new QVBoxLayout(labelHost_)) {
        labelLayout_->setContentsMargins(0,0,0,0);labelLayout_->addWidget(labels_);
        labels_->setObjectName(QStringLiteral("simplex-basic-column"));
        labels_->setFrameShape(QFrame::NoFrame);labels_->setStyleSheet(QStringLiteral("QTableView {border: none; background: #00ffff; gridline-color: white;}"));labels_->verticalHeader()->hide();labels_->horizontalHeader()->hide();
        labels_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);labels_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        labels_->setFocusPolicy(Qt::NoFocus);labels_->setSelectionMode(QAbstractItemView::NoSelection);
        connect(verticalScrollBar(),&QScrollBar::valueChanged,labels_->verticalScrollBar(),&QScrollBar::setValue);
        connect(labels_->verticalScrollBar(),&QScrollBar::valueChanged,verticalScrollBar(),&QScrollBar::setValue);
        connect(labels_,&QTableView::clicked,this,[this](const QModelIndex& index){emit clicked(index);});
    }
    void setModel(QAbstractItemModel* model) override {
        QTableView::setModel(model);labels_->setModel(model);fitColumns();
    }
    QWidget* labelHost() const {return labelHost_;}
    const std::vector<int>& baseWidths() const {return widths_;}
    void setBaseWidths(std::vector<int> widths){widths_=std::move(widths);fitColumns();}
protected:
    void resizeEvent(QResizeEvent* event) override {QTableView::resizeEvent(event);fitColumns();}
private:
    void fitColumns() {
        if(fitting_)return;
        fitting_=true;
        if(widths_.empty()){labelHost_->hide();fitting_=false;return;}
        hideColumn(0);labels_->showColumn(0);
        for(int j=1;j<labels_->model()->columnCount();++j)labels_->hideColumn(j);
        labelHost_->setFixedWidth(widths_[0]);labelHost_->show();
        labels_->setColumnWidth(0,widths_[0]);labels_->verticalHeader()->setDefaultSectionSize(verticalHeader()->defaultSectionSize());
        labelLayout_->setContentsMargins(0,viewport()->y(),0,std::max(0,height()-viewport()->height()-viewport()->y()));
        int total=0;for(std::size_t j=1;j<widths_.size();++j)total+=widths_[j];
        const int count=static_cast<int>(widths_.size())-1,extra=std::max(0,viewport()->width()-total);
        for(int j=1;j<=count;++j)setColumnWidth(j,widths_[static_cast<std::size_t>(j)]+extra/count+(j==count?extra%count:0));
        fitting_=false;
    }
    QWidget* labelHost_;
    QTableView* labels_;
    QVBoxLayout* labelLayout_;
    bool fitting_=false;
    std::vector<int> widths_;
};
class TraceModel final:public QAbstractTableModel {
public:
    TraceModel(LpSimplexController& c,QObject* parent):QAbstractTableModel(parent),c_(c){connect(&c,&LpSimplexController::changed,this,[this]{refresh();});refresh();}
    int rowCount(const QModelIndex& parent={}) const override{return parent.isValid()?0:static_cast<int>(tables_.size())*block_;}
    int columnCount(const QModelIndex& parent={}) const override{return parent.isValid()||tables_.empty()?0:static_cast<int>(tables_[0].columns.size())+1;}
    QVariant data(const QModelIndex& index,int role=Qt::DisplayRole) const override {
        if(!index.isValid()||index.row()>=rowCount()||index.column()>=columnCount())return {};
        const auto& table=tables_[static_cast<std::size_t>(index.row()/block_)];const int row=index.row()%block_,col=index.column();
        bool entering=false,leaving=false;
        if(col>0 && table.entering)entering=table.columns[static_cast<std::size_t>(col-1)]==*table.entering;
        if(row>=2 && row<block_-1 && table.leaving)leaving=table.rows[static_cast<std::size_t>(row-2)].label==*table.leaving;
        bool inverseCell=false;
        if(row>=3 && row<block_-4 && col>0) {
            const auto& name=table.columns[static_cast<std::size_t>(col-1)];
            inverseCell=name.rfind("sx",0)==0 || name.rfind("Rx",0)==0 || (c_.session()->method()==tora::lp::PrimalMethod::Dual && name.rfind("Sx",0)==0);
        }
        if(role==Qt::DisplayRole) {
            if(row==0)return col==0?QString::fromStdString(table.heading):QString();
            if(row==1)return col==0?QStringLiteral("Basic"):QString::fromStdString(table.columns[static_cast<std::size_t>(col-1)]);
            if(row==block_-1)return QString();
            const auto& r=table.rows[static_cast<std::size_t>(row-2)];return QString::fromStdString(col==0?r.label:r.values[static_cast<std::size_t>(col-1)]);
        }
        if(role==Qt::BackgroundRole)return leaving?QColor(Qt::red):entering?QColor(Qt::green):inverseCell?QColor(Qt::white):(col==0 || row<=1?QColor(Qt::cyan):QColor(Qt::blue));
        if(role==Qt::ForegroundRole)return col==0 || row<=1 || inverseCell?QColor(Qt::blue):QColor(Qt::white);
        if(role==Qt::TextAlignmentRole)return int((col==0?Qt::AlignLeft:Qt::AlignRight)|Qt::AlignVCenter);
        if(role==Qt::FontRole){auto font=usableUiFont(QApplication::font());font.setBold(col>0 && row>1);return font;}
        return {};
    }
    Qt::ItemFlags flags(const QModelIndex& index) const override {return index.isValid()?Qt::ItemIsEnabled:Qt::NoItemFlags;}
    void refresh() {
        std::vector<tora::lp::SimplexDisplay> next;int block=0;
        if(c_.session()) {
            block=static_cast<int>(c_.session()->problem().constraints.size())+7;
            const auto count=c_.session()->history().size();
            if(block==block_ && count>=tables_.size())next=tables_;
            const auto first=next.empty()?std::size_t(0):next.size()-1;
            for(std::size_t k=first;k<count;++k){auto display=c_.session()->display(k,c_.format());if(k<next.size())next[k]=std::move(display);else next.push_back(std::move(display));}
        }
        const int oldRows=rowCount(),newRows=static_cast<int>(next.size())*block;
        if(block!=block_ || newRows<oldRows || tables_.empty() || next.empty()) {
            beginResetModel();tables_=std::move(next);block_=block;endResetModel();
        }else if(newRows>oldRows){beginInsertRows({},oldRows,newRows-1);tables_=std::move(next);endInsertRows();if(oldRows)emit dataChanged(index(oldRows-block_,0),index(oldRows-1,columnCount()-1));}
        else {tables_=std::move(next);if(oldRows)emit dataChanged(index(0,0),index(oldRows-1,columnCount()-1));}
    }
private:
    LpSimplexController& c_;int block_=0;std::vector<tora::lp::SimplexDisplay> tables_;
};
void printTrace(ApplicationShell& shell,LpSimplexController& c) {
    if(!c.session())return;
    QPrinter printer(QPrinter::HighResolution);QPrintDialog dialog(&printer,&shell);polishUsableFonts(dialog);if(dialog.exec()!=QDialog::Accepted)return;
    if(!printSimplexTrace(printer,c,shell.font()))c.notice(QStringLiteral("Printer"),QStringLiteral("Could not complete printing to the selected printer"));
}
}
bool printSimplexTrace(QPrinter& printer,const LpSimplexController& c,const QFont& font) {
    try {
    if(!c.session())return false;
    QPainter painter;if(!painter.begin(&printer))return false;
    const auto page=printer.pageLayout().paintRectPixels(printer.resolution());painter.setFont(usableUiFont(font));
    QFontMetrics fm(painter.font(),&printer);const int line=fm.height()+fm.height()/2;int y=line;
    for(std::size_t k=0;k<c.session()->history().size();++k) {
        const auto table=c.session()->display(k,c.format());const int cols=static_cast<int>(table.columns.size())+1;
        std::vector<int> widths(static_cast<std::size_t>(cols),0);
        for(const auto& r:table.rows){widths[0]=std::max(widths[0],fm.horizontalAdvance(QString::fromStdString(r.label)));for(int j=1;j<cols;++j)widths[static_cast<std::size_t>(j)]=std::max(widths[static_cast<std::size_t>(j)],fm.horizontalAdvance(QString::fromStdString(r.values[static_cast<std::size_t>(j-1)])));}
        for(int j=1;j<cols;++j)widths[static_cast<std::size_t>(j)]=std::max(widths[static_cast<std::size_t>(j)],fm.horizontalAdvance(QString::fromStdString(table.columns[static_cast<std::size_t>(j-1)])));
        int total=0;for(auto& width:widths){width+=2*fm.averageCharWidth();total+=width;}
        const double factor=std::min(1.0,static_cast<double>(page.width())/total);painter.save();painter.scale(factor,factor);
        const auto draw=[&](const QStringList& texts){if((y+line)*factor>page.height()){painter.restore();if(!printer.newPage())throw std::runtime_error("Printer rejected a new page");painter.save();painter.scale(factor,factor);y=line;}int x=0;for(int j=0;j<texts.size();++j){painter.drawText(QRect(x,y-line,widths[static_cast<std::size_t>(j)],line),Qt::AlignLeft|Qt::AlignVCenter,texts[j]);x+=widths[static_cast<std::size_t>(j)];}y+=line;};
        draw({QString::fromStdString(table.heading)});QStringList header{QStringLiteral("Basic")};for(const auto& col:table.columns)header<<QString::fromStdString(col);draw(header);
        for(const auto& row:table.rows){QStringList texts{QString::fromStdString(row.label)};for(const auto& value:row.values)texts<<QString::fromStdString(value);draw(texts);}y+=line;painter.restore();
    }
    return painter.end();
    }catch(const std::exception&){return false;}
}
QWidget* simplexScreen(ApplicationShell& shell) {
    auto& c=simplexController(shell);auto* page=new QWidget;auto* outer=new QHBoxLayout(page);
    outer->addStretch(1);auto* body=new QWidget;outer->addWidget(body,8);outer->addStretch(1);auto* layout=new QVBoxLayout(body);
    auto* heading=new QLabel(QStringLiteral("SIMPLEX TABLEAU - (Starting All-Slack Method)"));heading->setObjectName(QStringLiteral("simplex-method"));heading->setAlignment(Qt::AlignCenter);auto bold=heading->font();bold.setBold(true);heading->setFont(bold);layout->addWidget(heading);
    auto* title=new QLabel;title->setObjectName(QStringLiteral("simplex-title"));layout->addWidget(title);
    auto* instructions=new QLabel(QStringLiteral("Steps for generating NEXT tableau from CURRENT one:\n"
        "1. ENTERING variable: Click a NONBASIC variable (if correct, column turns green)\n"
        "2. LEAVING variable: Click a BASIC variable (if correct, row turns red)\n"
        "3. Click command button NEXT ITERATION (or ALL ITERATIONS) -- This step may be executed without Steps 1 and/or 2."));instructions->setWordWrap(true);layout->addWidget(instructions);
    auto* commands=new QHBoxLayout;commands->addStretch();
    const auto button=[page](const QString& text,const QString& id,QHBoxLayout* row){auto* b=new QPushButton(text,page);b->setObjectName(id);row->addWidget(b);return b;};
    auto* next=button(QStringLiteral("Next Iteration"),QStringLiteral("simplex-next"),commands);
    auto* all=button(QStringLiteral("All Iterations"),QStringLiteral("simplex-all"),commands);
    auto* print=button(QStringLiteral("Write to Printer"),QStringLiteral("simplex-print"),commands);commands->addStretch();layout->addLayout(commands);
    auto* view=new TraceView;view->setObjectName(QStringLiteral("simplex-grid"));view->setModel(new TraceModel(c,view));view->verticalHeader()->hide();view->horizontalHeader()->hide();view->setSelectionMode(QAbstractItemView::NoSelection);
    view->setStyleSheet(QStringLiteral("QTableView {background: #00ffff; gridline-color: white; border: 1px solid white;}"));
    auto* gridArea=new QWidget(page);auto* gridLayout=new QHBoxLayout(gridArea);gridLayout->setContentsMargins(0,0,0,0);gridLayout->setSpacing(0);
    gridLayout->addWidget(view->labelHost());gridLayout->addWidget(view,1);layout->addWidget(gridArea,1);
    auto* footer=new QHBoxLayout;footer->addStretch();auto* input=button(QStringLiteral("View/Modify Input Data"),QStringLiteral("simplex-view-input"),footer);
    auto* main=button(QStringLiteral("MAIN Menu"),QStringLiteral("simplex-main"),footer);auto* exit=button(QStringLiteral("Exit TORA"),QStringLiteral("simplex-exit"),footer);footer->addStretch();layout->addLayout(footer);
    const auto update=[&c,heading,title,next,all,print,view] {
        const auto method=c.session()?c.session()->method():tora::lp::PrimalMethod::AllSlack;
        heading->setText(method==tora::lp::PrimalMethod::Dual?QStringLiteral("SIMPLEX TABLEAU - (Dual Simplex Method)"):method==tora::lp::PrimalMethod::MMethod?QStringLiteral("SIMPLEX TABLEAU - (M-Method)"):method==tora::lp::PrimalMethod::TwoPhase?QStringLiteral("SIMPLEX TABLEAU - (Two-Phase Method)"):QStringLiteral("SIMPLEX TABLEAU - (Starting All-Slack Method)"));
        title->setText(c.session()?QStringLiteral("Title: ")+legacyText(c.session()->problem().title)+(c.session()->problem().sense==tora::lp::ObjectiveSense::Maximize?QStringLiteral(" (Maximize)"):QStringLiteral(" (Minimize)")):QString());
        next->setEnabled(c.canNext());all->setEnabled(c.session() && !c.busy() && c.session()->status()==SimplexStatus::Running);print->setEnabled(c.session() && !c.busy());
        const QFontMetrics fm(view->font());view->verticalHeader()->setDefaultSectionSize(fm.height()+fm.height()/2);
        const int rows=view->model()->rowCount(),previous=view->property("simplex-measured-rows").toInt();
        const bool sameFont=view->property("simplex-measured-font").value<QFont>()==view->font();
        const int first=sameFont && rows>=previous?previous:0;
        std::vector<int> widths;
        for(int j=0;j<view->model()->columnCount();++j){int width=first && static_cast<std::size_t>(j)<view->baseWidths().size()?view->baseWidths()[static_cast<std::size_t>(j)]-2*fm.averageCharWidth():0;for(int i=first;i<rows;++i){const auto index=view->model()->index(i,j);const QFontMetrics cellMetrics(view->model()->data(index,Qt::FontRole).value<QFont>());width=std::max(width,cellMetrics.horizontalAdvance(view->model()->data(index).toString()));}width+=2*fm.averageCharWidth();widths.push_back(width);}
        view->setProperty("simplex-measured-rows",rows);view->setProperty("simplex-measured-font",view->font());
        view->setBaseWidths(std::move(widths));
    };
    QObject::connect(&c,&LpSimplexController::changed,page,update);QObject::connect(&shell,&ApplicationShell::displayMetricsChanged,page,update);
    QObject::connect(view,&QTableView::clicked,page,[&c](const QModelIndex& at){c.selectCell(at.row(),at.column());});
    QObject::connect(next,&QPushButton::clicked,&c,&LpSimplexController::next);QObject::connect(all,&QPushButton::clicked,&c,&LpSimplexController::all);
    QObject::connect(print,&QPushButton::clicked,page,[&shell,&c]{printTrace(shell,c);});
    QObject::connect(input,&QPushButton::clicked,&shell,[&shell]{shell.navigate(QStringLiteral("lp/input"));});QObject::connect(main,&QPushButton::clicked,&shell,[&shell]{shell.navigate(QStringLiteral("main"));});QObject::connect(exit,&QPushButton::clicked,&shell,&QWidget::close);
    update();return page;
}
} // namespace tora::desktop
