#include "networks_screens.hpp"
#include "ui_font.hpp"
#include "lp_printing.hpp"
#include "result_tables.hpp"
#include <QAbstractTableModel>
#include <QGroupBox>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QApplication>
#include <QLabel>
#include <QTextLayout>
#include <QTextOption>
#include <QPainter>
#include <QPrinter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QMenuBar>
#include <QMenu>
#include <memory>
namespace tora::desktop {
namespace {
void failure(NetworksController& c,const QString& e){if(!e.isEmpty())c.dialogs().showError("Network models",e);}
void solve(ApplicationShell& shell,int source,int sink,NetworkRouteMethod method){auto& c=networksController(shell);QString e;if(c.solve(source,sink,e,method))shell.navigate("networks/result");else failure(c,e);}
QWidget* input(ApplicationShell& shell){auto& c=networksController(shell);auto* page=new QWidget;auto* layout=new QVBoxLayout(page);auto* form=new QFormLayout;auto* mode=new QComboBox;mode->addItems({"Minimal Spanning Tree","Shortest Route","Maximal Flow"});mode->setObjectName("networks-mode");auto* title=new QLineEdit;title->setObjectName("networks-title");auto* count=new QSpinBox;count->setRange(1,100);count->setValue(3);count->setObjectName("networks-nodes");form->addRow("Network model",mode);form->addRow("Title",title);form->addRow("No. of Nodes",count);auto* notation=new QComboBox;notation->addItems({"Decimal","Scientific"});auto* n=new QSpinBox;n->setRange(0,99);n->setValue(5);auto* d=new QSpinBox;d->setRange(0,99);d->setValue(2);auto* format=new QHBoxLayout;format->addWidget(notation);format->addWidget(n);format->addWidget(d);form->addRow("Output format (N / D)",format);layout->addLayout(form);auto* init=new QPushButton("Initialize");init->setObjectName("networks-initialize");layout->addWidget(init);auto* symmetry=new QCheckBox("Check here if network is symmetrical");symmetry->setObjectName("networks-symmetry");layout->addWidget(symmetry);auto* grid=new QTableWidget;grid->setObjectName("networks-input-grid");grid->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);layout->addWidget(grid,1);auto* sources=new QHBoxLayout;auto* source=new QComboBox;source->setObjectName("networks-source");auto* sink=new QComboBox;sink->setObjectName("networks-sink");sources->addWidget(new QLabel("Starting / source node"));sources->addWidget(source);sources->addWidget(new QLabel("Sink node"));sources->addWidget(sink);layout->addLayout(sources);auto* method=new QComboBox;method->setObjectName("networks-route-method");method->addItems({"Shortest routes (all pairs)","Dijkstra iterations","Floyd iterations","Bellman-Ford iterations (negative-edge extension)"});layout->addWidget(method);auto* run=new QPushButton("Solve Problem");run->setObjectName("networks-solve");layout->addWidget(run);auto updating=std::make_shared<bool>(false);auto render=[&c,mode,title,count,notation,n,d,symmetry,grid,source,sink,run,init,updating,method]{*updating=true;const auto* doc=c.document();mode->setCurrentIndex(static_cast<int>(c.mode()));if(doc){title->setText(QString::fromStdString(doc->title()));count->setValue(doc->nodes());notation->setCurrentIndex(doc->format().notation==core::Notation::Decimal?0:1);n->setValue(doc->format().n);d->setValue(doc->format().d);symmetry->setChecked(doc->symmetrical());grid->setRowCount(doc->nodes()+1);grid->setColumnCount(doc->nodes()+1);const int oldSource=source->currentIndex(),oldSink=sink->currentIndex();source->clear();sink->clear();for(int i=0;i<doc->nodes();++i){auto name=QString::fromStdString(doc->nodeName(i));source->addItem(name);sink->addItem(name);}source->setCurrentIndex(oldSource<0?0:std::min(oldSource,doc->nodes()-1));sink->setCurrentIndex(oldSink<0?doc->nodes()-1:std::min(oldSink,doc->nodes()-1));for(int r=0;r<=doc->nodes();++r)for(int col=0;col<=doc->nodes();++col){auto* item=new QTableWidgetItem(QString::fromStdString(doc->cell(r,col)));if(r==col)item->setFlags(item->flags()&~Qt::ItemIsEditable);grid->setItem(r,col,item);}}else{grid->setRowCount(0);grid->setColumnCount(0);source->clear();sink->clear();title->clear();}title->setReadOnly(c.readOnly());grid->setEditTriggers(c.readOnly()?QAbstractItemView::NoEditTriggers:(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed));for(auto* w:QList<QWidget*>{init,mode,count,notation,n,d,symmetry})w->setEnabled(!c.readOnly());run->setEnabled(doc);sink->setEnabled(c.mode()==NetworkMode::MaximalFlow);method->setVisible(c.mode()==NetworkMode::ShortestRoute);*updating=false;};QObject::connect(&c,&NetworksController::inputReset,page,render);QObject::connect(init,&QPushButton::clicked,page,[&c,mode,title,count,notation,n,d]{core::NumberFormat f;f.n=n->value();f.d=d->value();f.notation=notation->currentIndex()==0?core::Notation::Decimal:core::Notation::Scientific;QString e;if(!c.setFormat(f,e)||!c.initialize(static_cast<NetworkMode>(mode->currentIndex()),title->text(),count->value(),e))failure(c,e);});QObject::connect(mode,qOverload<int>(&QComboBox::activated),page,[&c,updating,mode](int index){if(*updating)return;if(c.leaveInput())c.beginNew(static_cast<NetworkMode>(index));else { *updating=true;mode->setCurrentIndex(static_cast<int>(c.mode()));*updating=false; }});QObject::connect(symmetry,&QCheckBox::clicked,page,[&c](bool value){QString e;if(!c.setSymmetrical(value,e))failure(c,e);});QObject::connect(grid,&QTableWidget::cellClicked,page,[&c](int r,int col){if(r==0&&col>0)c.selectLine(false,col-1);else if(col==0&&r>0)c.selectLine(true,r-1);});QObject::connect(grid,&QTableWidget::cellDoubleClicked,page,[&c](int r,int col){if(r==0&&col>0)c.selectLine(false,col-1,true);else if(col==0&&r>0)c.selectLine(true,r-1,true);});QObject::connect(grid->horizontalHeader(),&QHeaderView::sectionClicked,page,[&c](int i){if(i>0)c.selectLine(false,i-1);});QObject::connect(grid->verticalHeader(),&QHeaderView::sectionClicked,page,[&c](int i){if(i>0)c.selectLine(true,i-1);});QObject::connect(grid,&QTableWidget::cellChanged,page,[&c,grid,updating,source,sink](int r,int col){if(*updating)return;QString e;if(!c.editCell(r,col,grid->item(r,col)->text(),e)){failure(c,e);return;}*updating=true;if(c.document()&&(r==0||col==0||c.document()->symmetrical()))grid->item(col,r)->setText(QString::fromStdString(c.document()->cell(col,r)));if(r==0||col==0){const int node=std::max(r,col)-1;if(node>=0&&c.document()){const auto label=QString::fromStdString(c.document()->nodeName(node));source->setItemText(node,label);sink->setItemText(node,label);}}*updating=false;});QObject::connect(&c,&NetworksController::commitRequested,page,[&c,title,notation,n,d,grid]{if(auto* focus=QApplication::focusWidget();focus&&grid->isAncestorOf(focus))grid->setFocus();QString e;for(int r=0;r<grid->rowCount();++r)for(int col=0;col<grid->columnCount();++col)if(!c.editCell(r,col,grid->item(r,col)->text(),e))return;if(!c.setTitle(title->text(),e))return;core::NumberFormat f;f.n=n->value();f.d=d->value();f.notation=notation->currentIndex()==0?core::Notation::Decimal:core::Notation::Scientific;c.setFormat(f,e);});QObject::connect(run,&QPushButton::clicked,page,[&shell,source,sink,method]{solve(shell,source->currentIndex(),sink->currentIndex(),static_cast<NetworkRouteMethod>(method->currentIndex()));});render();polishUsableFonts(*page);return page;}
QWidget* output(ApplicationShell& shell){auto& c=networksController(shell);auto* page=new QWidget;auto* layout=new QVBoxLayout(page);auto* text=new QPlainTextEdit;text->setObjectName("networks-result");text->setReadOnly(true);layout->addWidget(text,1);auto* trace=new QTableWidget;trace->setObjectName("networks-iterations");trace->setColumnCount(2);trace->setHorizontalHeaderLabels({"Iteration","Calculation"});trace->horizontalHeader()->setStretchLastSection(true);trace->setEditTriggers(QAbstractItemView::NoEditTriggers);layout->addWidget(trace,1);auto* bar=new QHBoxLayout;auto* next=new QPushButton("Next Iteration");next->setObjectName("networks-next");auto* all=new QPushButton("All Iterations");all->setObjectName("networks-all");auto* print=new QPushButton("Write to Printer");auto* back=new QPushButton("View/Modify Input Data");bar->addWidget(next);bar->addWidget(all);bar->addWidget(print);bar->addWidget(back);layout->addLayout(bar);auto steps=std::make_shared<QStringList>();auto shown=std::make_shared<int>(0);auto render=[&c,text,trace,steps,shown,next,all]{text->setPlainText(c.outputText());*steps=c.iterationText();*shown=0;trace->setRowCount(0);trace->setVisible(!steps->empty());next->setVisible(!steps->empty());all->setVisible(!steps->empty());next->setEnabled(!steps->empty());all->setEnabled(!steps->empty());};auto advance=[trace,steps,shown,next,all](bool complete){int stop=complete?steps->size():std::min(static_cast<int>(steps->size()),*shown+1);for(;*shown<stop;++*shown){trace->insertRow(*shown);trace->setItem(*shown,0,new QTableWidgetItem(QString::number(*shown+1)));trace->setItem(*shown,1,new QTableWidgetItem(steps->at(*shown)));trace->resizeRowToContents(*shown);}next->setEnabled(*shown<steps->size());all->setEnabled(*shown<steps->size());};QObject::connect(&c,&NetworksController::changed,page,render);QObject::connect(next,&QPushButton::clicked,page,[advance]{advance(false);});QObject::connect(all,&QPushButton::clicked,page,[advance]{advance(true);});QObject::connect(back,&QPushButton::clicked,page,[&shell]{shell.navigate("networks/input");});QObject::connect(print,&QPushButton::clicked,page,[&c,&shell]{auto outcome=requestLpPrint(&shell,[&c](QPrinter& p){return printNetworks(p,c);});if(outcome==PrintOutcome::Failed)failure(c,"The Network print job could not be completed.");});render();polishUsableFonts(*page);return page;}
const networks::AllPairsRoutes* floydResult(const NetworksController& controller)
{
    return controller.result() && controller.routeMethod() == NetworkRouteMethod::Floyd
        ? std::get_if<networks::AllPairsRoutes>(controller.result()) : nullptr;
}

QString nodeName(const NetworksController& controller, std::size_t node)
{
    return QString::fromStdString(controller.document()->nodeName(static_cast<int>(node)));
}

QString floydStepLabel(const NetworksController& controller, const networks::FloydIteration& step, int index)
{
    if (step.initial) return QStringLiteral("Step 0: Initial matrices");
    return QStringLiteral("Step %1: Allow node %2").arg(index).arg(nodeName(controller,step.intermediate));
}

class FloydMatrixModel final : public QAbstractTableModel {
public:
    FloydMatrixModel(const NetworksController& controller, bool nextNodes, QObject* parent)
        : QAbstractTableModel(parent), controller_(controller), nextNodes_(nextNodes) {}

    void show(int selected)
    {
        beginResetModel();
        const auto* result = floydResult(controller_);
        const int nodeCount = result ? static_cast<int>(result->sources.size()) : 0;
        names_.clear();
        for (int node = 0; node < nodeCount; ++node) names_ << nodeName(controller_,node);
        stepCount_ = result ? static_cast<int>(result->trace.size()) : 0;
        all_ = selected == stepCount_;
        selected_ = selected < 0 ? 0 : selected;
        if (result) format_ = core::NumberFormatter(controller_.document()->format());
        endResetModel();
    }

    int rowCount(const QModelIndex& parent = {}) const override
    {
        return parent.isValid() ? 0 : static_cast<int>(names_.size()) * (all_ ? stepCount_ : std::min(1,stepCount_));
    }
    int columnCount(const QModelIndex& parent = {}) const override
    {
        return parent.isValid() || !stepCount_ ? 0 : static_cast<int>(names_.size()) + (all_ ? 1 : 0);
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (role != Qt::DisplayRole && role != Qt::ToolTipRole) return {};
        if (orientation == Qt::Vertical) return names_.isEmpty() ? QString{} : names_.value(section % names_.size());
        if (all_ && section == 0) return QStringLiteral("Iteration");
        return names_.value(section - (all_ ? 1 : 0));
    }
    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid() || names_.isEmpty()) return {};
        const bool stepLabel = all_ && index.column() == 0;
        if (role == Qt::TextAlignmentRole)
            return static_cast<int>((stepLabel ? Qt::AlignLeft : Qt::AlignRight) | Qt::AlignVCenter);
        if (role != Qt::DisplayRole && role != Qt::ToolTipRole) return {};
        const auto* result = floydResult(controller_);
        const int size = static_cast<int>(names_.size());
        const int step = all_ ? index.row() / size : selected_;
        if (!result || step >= static_cast<int>(result->trace.size())) return {};
        const auto& iteration = result->trace[step];
        const int row = index.row() % size;
        if (stepLabel) return row == 0 ? floydStepLabel(controller_,iteration,step) : QString{};
        const auto offset = static_cast<std::size_t>(row * size + index.column() - (all_ ? 1 : 0));
        if (offset >= iteration.distance.size() || offset >= iteration.next.size()) return {};
        if (nextNodes_)
            return iteration.next[offset] ? names_.value(static_cast<int>(*iteration.next[offset])) : QStringLiteral("None");
        return iteration.distance[offset] ? QString::fromStdString(format_.format(*iteration.distance[offset]))
                                          : QStringLiteral("Unreachable");
    }

private:
    const NetworksController& controller_;
    const bool nextNodes_;
    QStringList names_;
    core::NumberFormatter format_;
    int selected_ = 0;
    int stepCount_ = 0;
    bool all_ = false;
};

void menus(ApplicationShell& shell,QMenuBar& bar){auto& c=networksController(shell);auto* file=bar.addMenu("File");file->addAction("New",&shell,[&shell,&c]{if(c.leaveInput()){c.beginNew(c.mode());shell.navigate("networks/input");}})->setShortcut(QKeySequence::New);file->addAction("Open…",&shell,[&shell,&c]{if(c.requestOpen())shell.navigate("networks/input");})->setShortcut(QKeySequence::Open);file->addAction("Save As…",&c,[&c]{c.requestSave();})->setShortcut(QKeySequence::SaveAs);auto* edit=bar.addMenu("EditGrid");edit->addAction("Copy",&c,[&c]{if(!c.copySelection())failure(c,"Select a node row or column heading first.");})->setShortcut(QKeySequence::Copy);edit->addAction("Paste",&c,[&c]{QString e;if(!c.pasteSelection(e))failure(c,e);})->setShortcut(QKeySequence::Paste);edit->addAction("Insert Node (single click after; double click before)",&c,[&c]{QString e;if(!c.editSelection(true,e))failure(c,e);});edit->addAction("Delete Node",&c,[&c]{QString e;if(!c.editSelection(false,e))failure(c,e);});edit->addAction("Undo Last Insert Or Delete",&c,[&c]{QString e;if(!c.undo(e))failure(c,e);})->setShortcut(QKeySequence::Undo);bar.addAction("MAIN Menu",&shell,[&shell]{shell.navigate("main");});}
}
ModuleDescriptor networksModule(){ModuleDescriptor m{"networks","Network models","networks/input",{}, {}};m.screens.append({"networks/input","NETWORK MODELS",input,menus});m.screens.append({"networks/result","NETWORK MODELS",output,menus});m.beforeLeave=[](ApplicationShell& shell,const QString&,const QString& to){return to.startsWith("networks/")?networksController(shell).commitInput():networksController(shell).leaveInput();};return m;}
void drawNetwork(QPainter& painter,const QRectF& rect,const NetworksController& c){painter.drawText(rect,Qt::TextWordWrap,c.outputText());}
bool printNetworks(QPrinter& printer,const NetworksController& c){
 if(!c.result())return false;
 QPainter painter;if(!painter.begin(&printer))return false;
 const auto area=printer.pageLayout().paintRectPixels(printer.resolution());auto font=painter.font();font.setPointSize(10);painter.setFont(font);qreal y=area.top();
 const auto content=c.outputText()+"\nCalculation steps\n"+c.iterationText().join('\n');
 for(const auto& paragraph:content.split('\n')){QTextLayout layout(paragraph.isEmpty()?QString(" "):paragraph,font,&printer);QTextOption option;option.setWrapMode(QTextOption::WrapAnywhere);layout.setTextOption(option);layout.beginLayout();
  while(true){auto line=layout.createLine();if(!line.isValid())break;line.setLineWidth(area.width());if(y+line.height()>area.bottom()){if(!printer.newPage())return false;y=area.top();}line.draw(&painter,QPointF(area.left(),y));y+=line.height()+3;}layout.endLayout();
 }return painter.end();
}
}
