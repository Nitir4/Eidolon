#include "games_screens.hpp"
#include "history_view.hpp"
#include "calculation_history.hpp"
#include <QTabWidget>
#include "screen_widgets.hpp"
#include "ui_font.hpp"
#include "lp_printing.hpp"
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
#include <QSplitter>
#include <QMenuBar>
#include <QMenu>
#include <QLabel>
#include <QComboBox>
#include <QApplication>
#include <QFontMetricsF>
#include <cmath>
#include <memory>
namespace tora::desktop {
void drawGamesGraph(QPainter& p,const QRectF& rect,const games::GraphicalResult& g,double zoom){
 p.save();p.setClipRect(rect);p.fillRect(rect,Qt::white);
 double scale=0;for(const auto& line:g.lines)scale=std::max({scale,std::abs(line.atZero),std::abs(line.atOne)});if(scale==0)scale=1;
 double low=0,high=0;for(const auto& line:g.lines){low=std::min({low,line.atZero/scale,line.atOne/scale});high=std::max({high,line.atZero/scale,line.atOne/scale});}
 const double center=(low+high)/2,span=std::max(0.2,high-low)*1.2/zoom;
 const auto topLabel=QString::number((center+span/2)*scale,'g',5),bottomLabel=QString::number((center-span/2)*scale,'g',5);
 const QFontMetricsF metrics(p.font());const qreal height=metrics.height()+4;
 const qreal yWidth=std::max(metrics.horizontalAdvance(topLabel),metrics.horizontalAdvance(bottomLabel))+8;
 const qreal tickWidth=metrics.horizontalAdvance("0.75")+8;
 const QRectF area=rect.adjusted(yWidth,height/2,-tickWidth/2,-2*height-8);
 if(area.width()<20||area.height()<20){p.restore();return;}
 auto point=[&](double x,double y){return QPointF(area.left()+x*area.width(),area.bottom()-(y/scale-center+span/2)/span*area.height());};
 p.setPen(Qt::black);p.drawRect(area);
 const int divisions=area.width()>=4*tickWidth?4:2;
 for(int i=0;i<=divisions;++i){const double x=double(i)/divisions;p.drawText(QRectF(area.left()+x*area.width()-tickWidth/2,area.bottom()+2,tickWidth,height),Qt::AlignHCenter|Qt::AlignTop|Qt::TextSingleLine,QString::number(x));}
 p.drawText(QRectF(rect.left(),area.top()-height/2,yWidth-4,height),Qt::AlignRight|Qt::AlignVCenter|Qt::TextSingleLine,topLabel);
 p.drawText(QRectF(rect.left(),area.bottom()-height/2,yWidth-4,height),Qt::AlignRight|Qt::AlignVCenter|Qt::TextSingleLine,bottomLabel);
 p.drawText(QRectF(area.left(),area.bottom()+height+4,area.width(),height),Qt::AlignHCenter|Qt::AlignTop|Qt::TextSingleLine,"Probability of A1 (0 to 1)");
 p.save();p.setClipRect(area);int index=0;for(const auto& line:g.lines){p.setPen(QPen(QColor::fromHsv((index++*67)%360,170,190),1));p.drawLine(point(0,line.atZero),point(1,line.atOne));}
 p.setPen(QPen(Qt::black,3));for(const auto& segment:g.envelope){const auto& line=g.lines.at(segment.strategy);auto value=[&](double x){return ((1-x)*(line.atZero/scale)+x*(line.atOne/scale))*scale;};p.drawLine(point(segment.from,value(segment.from)),point(segment.to,value(segment.to)));}
 p.setPen(QPen(Qt::red,1,Qt::DashLine));p.drawLine(QPointF(point(g.probability,0).x(),area.top()),QPointF(point(g.probability,0).x(),area.bottom()));p.restore();p.restore();
}
namespace {
class Graph final:public QWidget{public:GamesController& c;double zoom=1;explicit Graph(GamesController& controller):c(controller){setObjectName("games-graph");setMinimumSize(220,180);}void paintEvent(QPaintEvent*)override{QPainter p(this);if(c.graph())drawGamesGraph(p,rect(),*c.graph(),zoom);}};
void failure(GamesController& c,const QString& e){if(!e.isEmpty())c.dialogs().showError("Games",e);}
void solve(ApplicationShell& shell,bool graphical){auto& c=gamesController(shell);QString e;if(c.solve(e,graphical))shell.navigate(graphical?"games/graphical":"games/result");else failure(c,e);}
QWidget* input(ApplicationShell& shell){auto& c=gamesController(shell);auto* page=new QWidget;auto* layout=new QVBoxLayout(page);auto* form=new QFormLayout;auto* title=new QLineEdit;title->setObjectName("games-title");auto* rows=new QSpinBox;rows->setRange(1,100);rows->setValue(2);rows->setObjectName("games-rows");auto* cols=new QSpinBox;cols->setRange(1,100);cols->setValue(2);cols->setObjectName("games-columns");form->addRow("Title",title);form->addRow("Player A strategies",rows);form->addRow("Player B strategies",cols);auto* notation=new QComboBox;notation->addItems({"Decimal","Scientific"});notation->setObjectName("games-notation");auto* digits=new QSpinBox;digits->setRange(0,core::NumberFormatter::MaximumDigits);digits->setValue(5);digits->setObjectName("games-format-n");auto* decimals=new QSpinBox;decimals->setRange(0,core::NumberFormatter::MaximumDigits);decimals->setValue(2);decimals->setObjectName("games-format-d");form->addRow("Notation",notation);form->addRow("N",digits);form->addRow("D",decimals);layout->addLayout(form);auto* init=new QPushButton("Initialize");init->setObjectName("games-initialize");layout->addWidget(init);auto* grid=new QTableWidget;grid->setObjectName("games-input-grid");grid->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);layout->addWidget(grid,1);auto updating=std::make_shared<bool>(false);auto render=[&c,title,rows,cols,grid,updating,notation,digits,decimals]{*updating=true;if(const auto* d=c.document()){title->setText(QString::fromStdString(d->title()));notation->setCurrentIndex(d->format().notation==core::Notation::Decimal?0:1);digits->setValue(d->format().n);decimals->setValue(d->format().d);rows->setValue(static_cast<int>(d->payoff().rows()));cols->setValue(static_cast<int>(d->payoff().columns()));grid->setRowCount(rows->value());grid->setColumnCount(cols->value());for(int r=0;r<rows->value();++r){grid->setVerticalHeaderItem(r,new QTableWidgetItem(QString::fromStdString(d->rowNames()[static_cast<std::size_t>(r)])));for(int col=0;col<cols->value();++col)grid->setItem(r,col,new QTableWidgetItem(QString::fromStdString(d->cell(r,col))));}for(int col=0;col<cols->value();++col)grid->setHorizontalHeaderItem(col,new QTableWidgetItem(QString::fromStdString(d->columnNames()[static_cast<std::size_t>(col)])));}else{grid->setRowCount(0);grid->setColumnCount(0);title->clear();}grid->setEditTriggers(c.readOnly()?QAbstractItemView::NoEditTriggers:(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed));title->setReadOnly(c.readOnly());notation->setEnabled(!c.readOnly());digits->setEnabled(!c.readOnly());decimals->setEnabled(!c.readOnly());*updating=false;};QObject::connect(&c,&GamesController::inputReset,page,render);QObject::connect(init,&QPushButton::clicked,page,[&c,title,rows,cols,notation,digits,decimals]{QString e;core::NumberFormat f;f.n=digits->value();f.d=decimals->value();f.notation=notation->currentIndex()==0?core::Notation::Decimal:core::Notation::Scientific;if(!c.setFormat(f,e)){failure(c,e);return;}if(!c.initialize(title->text(),rows->value(),cols->value(),e))failure(c,e);});QObject::connect(grid,&QTableWidget::cellChanged,page,[&c,grid,updating](int r,int col){if(*updating)return;QString e;if(!c.editCell(r,col,grid->item(r,col)->text(),e))failure(c,e);});QObject::connect(grid->verticalHeader(),&QHeaderView::sectionClicked,page,[&c](int at){c.selectLine(true,at);});QObject::connect(grid->horizontalHeader(),&QHeaderView::sectionClicked,page,[&c](int at){c.selectLine(false,at);});QObject::connect(grid->verticalHeader(),&QHeaderView::sectionDoubleClicked,page,[&c](int at){c.selectLine(true,at,true);});QObject::connect(grid->horizontalHeader(),&QHeaderView::sectionDoubleClicked,page,[&c](int at){c.selectLine(false,at,true);});auto commit=[&c,title,grid,notation,digits,decimals]{if(auto* focus=QApplication::focusWidget();focus&&grid->isAncestorOf(focus))grid->setFocus();QString e;for(int r=0;r<grid->rowCount();++r)for(int col=0;col<grid->columnCount();++col)if(!c.editCell(r,col,grid->item(r,col)->text(),e))return;if(!c.setTitle(title->text(),e))return;core::NumberFormat f;f.n=digits->value();f.d=decimals->value();f.notation=notation->currentIndex()==0?core::Notation::Decimal:core::Notation::Scientific;c.setFormat(f,e);};QObject::connect(&c,&GamesController::commitRequested,page,commit);auto* bar=new QHBoxLayout;auto* lp=new QPushButton("LP-based");lp->setObjectName("games-lp-based");auto* graphical=new QPushButton("Graphical");graphical->setObjectName("games-graphical");bar->addWidget(lp);bar->addWidget(graphical);layout->addLayout(bar);QObject::connect(lp,&QPushButton::clicked,page,[&shell]{solve(shell,false);});QObject::connect(graphical,&QPushButton::clicked,page,[&shell]{solve(shell,true);});auto eligibility=[&c,lp,graphical,init]{lp->setEnabled(c.document());graphical->setEnabled(c.document()&&c.document()->payoff().rows()==2);init->setEnabled(!c.readOnly());};QObject::connect(&c,&GamesController::changed,page,eligibility);eligibility();render();polishUsableFonts(*page);return page;}
QWidget* output(ApplicationShell& shell) {
    auto& controller = gamesController(shell);
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    auto* tabs = new QTabWidget;
    auto* split = new QSplitter;
    auto* text = new QPlainTextEdit;
    text->setReadOnly(true);
    text->setObjectName("games-result");
    split->addWidget(text);
    auto* graph = new Graph(controller);
    split->addWidget(graph);
    tabs->addTab(split, "Solution / Graph");
    auto* steps = new HistoryView("games");
    steps->setObjectName("games-history");
    tabs->addTab(steps, "Steps / Iterations");
    layout->addWidget(tabs, 1);

    auto* buttons = new QHBoxLayout;
    auto* in = new QPushButton("Zoom In");
    auto* out = new QPushButton("Zoom Out");
    auto* print = new QPushButton("Print");
    in->setObjectName("games-zoom-in");
    out->setObjectName("games-zoom-out");
    print->setObjectName("games-print");
    buttons->addWidget(in);
    buttons->addWidget(out);
    buttons->addWidget(print);
    layout->addLayout(buttons);

    const auto render = [&controller, text, graph, in, out, steps] {
        QStringList titles;
        if (const auto* result = controller.result()) {
            for (std::size_t i = 0; i < result->history.size(); ++i)
                titles << QString::number(i + 1) + ". " + QString::fromStdString(result->history[i].event);
        }
        steps->setHistory(titles, [&controller](int index) {
            return gameStepText(*controller.document(), *controller.result(), index);
        });
        const bool graphical = controller.graph() != nullptr;
        text->setPlainText(controller.outputText());
        graph->setVisible(graphical);
        in->setVisible(graphical);
        out->setVisible(graphical);
        graph->update();
    };
    QObject::connect(&controller, &GamesController::changed, page, render);
    QObject::connect(in, &QPushButton::clicked, page, [graph] {
        graph->zoom = std::min(16., graph->zoom * 1.25);
        graph->update();
    });
    QObject::connect(out, &QPushButton::clicked, page, [graph] {
        graph->zoom = std::max(0.25, graph->zoom / 1.25);
        graph->update();
    });
    QObject::connect(print, &QPushButton::clicked, page, [&controller, &shell] {
        const auto outcome = requestLpPrint(&shell, [&controller](QPrinter& p) { return printGames(p, controller); });
        if (outcome == PrintOutcome::Failed)
            failure(controller, "The Games print job could not be completed.");
    });
    auto* back = new QPushButton("Input Data");
    layout->addWidget(back);
    QObject::connect(back, &QPushButton::clicked, page, [&shell] { shell.navigate("games/input"); });
    render();
    polishUsableFonts(*page);
    return page;
}
void menus(ApplicationShell& shell,QMenuBar& bar){auto& c=gamesController(shell);auto* file=bar.addMenu("File");auto* create=file->addAction("New",&shell,[&shell,&c]{if(c.leaveInput()){c.beginNew();shell.navigate("games/input");}});auto* open=file->addAction("Open…",&shell,[&shell,&c]{if(c.requestOpen())shell.navigate("games/input");});auto* save=file->addAction("Save",&c,[&c]{c.requestSave();});auto* saveAs=file->addAction("Save As…",&c,[&c]{c.requestSave();});create->setShortcut(QKeySequence::New);open->setShortcut(QKeySequence::Open);save->setShortcut(QKeySequence::Save);saveAs->setShortcut(QKeySequence::SaveAs);auto* edit=bar.addMenu("EditGrid");auto* undo=edit->addAction("Undo Last Insert, Paste, Or Delete");auto* copy=edit->addAction("Copy Column or Row");auto* paste=edit->addAction("Paste on Column or Row");auto* erase=edit->addAction("Delete Column or Row");auto* insert=edit->addAction("Insert Column or Row");undo->setShortcut(QKeySequence("Ctrl+U"));copy->setShortcut(QKeySequence("Ctrl+C"));paste->setShortcut(QKeySequence("Ctrl+P"));erase->setShortcut(QKeySequence("Ctrl+D"));insert->setShortcut(QKeySequence("Ctrl+I"));QObject::connect(copy,&QAction::triggered,&c,[&c]{c.copySelection();});const auto operation=[&c](int op){QString e;if(!c.editSelection(op,e))failure(c,e);};QObject::connect(undo,&QAction::triggered,&c,[operation]{operation(0);});QObject::connect(paste,&QAction::triggered,&c,[operation]{operation(1);});QObject::connect(erase,&QAction::triggered,&c,[operation]{operation(2);});QObject::connect(insert,&QAction::triggered,&c,[operation]{operation(3);});auto updateEdit=[&c,undo,copy,paste,erase,insert]{undo->setEnabled(c.canUndo());copy->setEnabled(c.hasSelection());paste->setEnabled(c.canPaste());erase->setEnabled(c.hasSelection()&&!c.readOnly());insert->setEnabled(c.hasSelection()&&!c.readOnly());};QObject::connect(&c,&GamesController::changed,edit,updateEdit);updateEdit();auto* menu=bar.addMenu("Solve Problem");auto* lp=menu->addAction("LP-based",&shell,[&shell]{solve(shell,false);});auto* graph=menu->addAction("Graphical",&shell,[&shell]{solve(shell,true);});auto update=[&c,lp,graph]{lp->setEnabled(c.document());graph->setEnabled(c.document()&&c.document()->payoff().rows()==2);};QObject::connect(&c,&GamesController::changed,menu,update);update();bar.addAction("MAIN Menu",&shell,[&shell]{shell.navigate("main");});}
}
ModuleDescriptor gamesModule(){ModuleDescriptor m{"games","Zero-Sum Games","games/input",{}, {}};m.screens.append({"games/input","ZERO-SUM GAMES",input,menus});m.screens.append({"games/result","ZERO-SUM GAMES",output,menus});m.screens.append({"games/graphical","ZERO-SUM GAMES",output,menus});m.beforeLeave=[](ApplicationShell& shell,const QString&,const QString& to){return to.startsWith("games/")?gamesController(shell).commitInput():gamesController(shell).leaveInput();};return m;}
bool printGames(QPrinter& printer,const GamesController& c){if(!c.result())return false;QPainter p;if(!p.begin(&printer))return false;auto area=printer.pageLayout().paintRectPixels(printer.resolution());auto font=p.font();font.setPointSize(10);p.setFont(font);QString complete=c.outputText();for(std::size_t i=0;i<c.result()->history.size();++i)complete+="\n"+gameStepText(*c.document(),*c.result(),static_cast<int>(i))+"\n";const auto paragraphs=complete.split('\n');const int height=p.fontMetrics().height()+3;int y=area.top();const auto write=[&](const QString& line){if(y+height>area.bottom()){if(!printer.newPage())return false;y=area.top();}p.drawText(area.left(),y+p.fontMetrics().ascent(),line);y+=height;return true;};for(const auto& paragraph:paragraphs){QString line;for(const auto& word:paragraph.split(' ')){const QString candidate=line.isEmpty()?word:line+' '+word;if(!line.isEmpty()&&p.fontMetrics().horizontalAdvance(candidate)>area.width()){if(!write(line))return false;line=word;}else line=candidate;}if(!write(line))return false;}if(c.graph()){if(!printer.newPage())return false;drawGamesGraph(p,area,*c.graph());}return p.end();}
}
