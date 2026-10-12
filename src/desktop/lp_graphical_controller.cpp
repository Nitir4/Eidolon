#include "lp_graphical_controller.hpp"
#include "lp_printing.hpp"
#include "lp_input_controller.hpp"
#include "ui_font.hpp"
#include <QApplication>
#include <QMessageBox>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QListWidget>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QVariant>
#include <cmath>
#include <algorithm>

namespace tora::desktop {
LpGraphicalController::LpGraphicalController(ApplicationShell& shell):QObject(&shell),shell_(shell){
    setObjectName(QStringLiteral("lp-graphical-controller"));
    notice=[&shell](const QString& title,const QString& text){QMessageBox box(QMessageBox::Critical,title,text,QMessageBox::Ok,&shell);polishUsableFonts(box);box.exec();};
    connect(&inputController(shell),&LpInputController::changed,this,[this]{session_.reset();requested_=false;emit changed();});
}
bool LpGraphicalController::request(){
    const auto* doc=inputController(shell_).document();if(!doc || !inputController(shell_).validateForSolve())return false;
    if(!tora::lp::GraphicalSession::eligible(doc->problem))return false;
    if(!tora::lp::GraphicalSession::boundsSupported(doc->problem)){
        notice(QStringLiteral("Graphical LP"),QString::fromLatin1(tora::lp::GraphicalSession::boundsError()));return false;
    }
    shell_.setProperty("lp-output-method",QStringLiteral("graphical"));requested_=true;session_.reset();emit changed();shell_.navigate(QStringLiteral("lp/output-format"));return true;
}
bool LpGraphicalController::canStart() const {return requested_ && shell_.property("lp-output-method").toString()==QStringLiteral("graphical");}
bool LpGraphicalController::start(tora::core::NumberFormat format){
    if(!canStart() || !inputController(shell_).document())return false;
    try{session_=std::make_unique<tora::lp::GraphicalSession>(inputController(shell_).document()->problem,format);emit changed();shell_.navigate(QStringLiteral("lp/graphical"));return true;}
    catch(const std::exception& e){notice(QStringLiteral("Graphical LP"),QString::fromUtf8(e.what()));return false;}
}
void LpGraphicalController::selectRow(int row){
    if(!session_)return;
    try{
        const int count=static_cast<int>(session_->problem().constraints.size());
        if(row==1){if(!session_->drawObjective())notice(QStringLiteral("Graph Obj Function"),QString::fromStdString(session_->notice()));}
        else if(row>=3 && row<count+3)session_->selectConstraint(static_cast<std::size_t>(row-3));
        else if(row==count+3)session_->resetSignRestrictions();
        else if(row==count+4)session_->drawAll();
        emit changed();
    }catch(const std::exception& e){notice(QStringLiteral("Graphical LP"),QString::fromUtf8(e.what()));}
}
void LpGraphicalController::zoom(int direction){if(session_ && session_->zoom(direction))emit changed();}
LpGraphicalController& graphicalController(ApplicationShell& shell){if(auto* c=shell.findChild<LpGraphicalController*>(QStringLiteral("lp-graphical-controller")))return *c;return *new LpGraphicalController(shell);}
namespace {
QPointF mapped(tora::lp::GraphPoint point,const QRectF& area,double maximum,double minimum){const auto unit=area.width()/(maximum-minimum);return {area.left()+(point.x-minimum)*unit,area.bottom()-(point.y-minimum)*area.height()/(maximum-minimum)};}
void drawPlot(QPainter& painter,const QRectF& area,const tora::lp::GraphicalSession& graph,const QFont& font,bool printing=false){
    painter.save();painter.setFont(usableUiFont(font));
    if(printing){auto paperFont=painter.font();paperFont.setBold(true);painter.setFont(paperFont);}
    const QFontMetricsF metrics(painter.font(),painter.device());
    painter.setClipRect(printing?area.adjusted(-metrics.height(),-metrics.height(),metrics.height()*2,metrics.height()*2):area);
    painter.fillRect(area,Qt::white);
    const double maximum=graph.rangeMax();const auto origin=mapped({0,0},area,maximum,graph.rangeMin());
    const auto sample=graph.gridStep();painter.setPen(Qt::NoPen);painter.setBrush(printing?QColor(Qt::black):QColor(128,128,128));
    const auto dotWidth=metrics.height()*2/13,dotHeight=metrics.height()*3/13;
    for(double x=sample;x<maximum;x+=sample)for(double y=sample;y<maximum;y+=sample)if(graph.contains({x,y})){
        const auto point=mapped({x,y},area,maximum,graph.rangeMin());painter.drawRect(QRectF(point.x()-dotWidth/2,point.y()-dotHeight/2,dotWidth,dotHeight));
    }
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(printing?Qt::black:Qt::blue,std::max(1.0,metrics.height()/20)));painter.drawLine(QPointF(area.left(),origin.y()),QPointF(area.right(),origin.y()));painter.drawLine(QPointF(origin.x(),area.top()),QPointF(origin.x(),area.bottom()));
    const auto label=[&](const QString& text,QPointF at){painter.drawText(at+QPointF(metrics.averageCharWidth()/3,metrics.ascent()),text);};
    label(QString::number(graph.rangeMin(),'g',12),{area.left(),origin.y()});label(QStringLiteral("0"),origin);
    if(printing){
        painter.setBrush(Qt::black);
        const auto radius=metrics.height()/8;
        for(const auto& point:{QPointF(area.left(),origin.y()),origin,QPointF(origin.x(),area.bottom())})painter.drawEllipse(point,radius,radius);
        label(QString::number(graph.rangeMin(),'g',12),{origin.x(),area.bottom()});
    }
    for(double v=graph.tickStep();v<=maximum;v+=graph.tickStep()){
        const auto point=mapped({v,v},area,maximum,graph.rangeMin());label(QString::number(v,'g',12),{point.x(),origin.y()});label(QString::number(v,'g',12),{origin.x(),point.y()});
        if(printing){const auto radius=metrics.height()/8;painter.drawEllipse(QPointF(point.x(),origin.y()),radius,radius);painter.drawEllipse(QPointF(origin.x(),point.y()),radius,radius);}
    }
    painter.setBrush(Qt::NoBrush);
    label(QStringLiteral("x1"),{area.left()+area.width()*0.75,origin.y()+metrics.height()});label(QStringLiteral("x2"),{origin.x()-metrics.horizontalAdvance(QStringLiteral("x2"))-metrics.averageCharWidth(),area.top()+metrics.height()*2});
    const auto line=[&](const tora::lp::GraphSegment& segment,QColor color,double width,const QString& name){
        const auto a=mapped(segment.first,area,maximum,graph.rangeMin()),b=mapped(segment.second,area,maximum,graph.rangeMin());painter.setPen(QPen(color,width));painter.drawLine(a,b);
        auto anchor=(a.x()<b.x()?a:b);
        // Rising boundaries start their label in the negative-axis band,
        // as captured for x1-x2>=1; descending ones start at the left edge.
        if(a.x()!=b.x() && a.y()!=b.y()){
            const auto band=mapped({0,graph.rangeMin()*0.6},area,maximum,graph.rangeMin()).y();
            if(anchor.y()>band){const auto fraction=(band-a.y())/(b.y()-a.y());anchor=a+(b-a)*fraction;}
        }
        if(std::abs(segment.first.x-segment.second.x)<1e-10)anchor=mapped({segment.first.x,graph.rangeMin()*0.6},area,maximum,graph.rangeMin());
        else if(std::abs(segment.first.y-segment.second.y)<1e-10)anchor={area.left()+metrics.averageCharWidth()/2,a.y()};
        anchor.setX(std::clamp(anchor.x(),area.left()+metrics.averageCharWidth(),area.right()-metrics.horizontalAdvance(name)));anchor.setY(std::clamp(anchor.y(),area.top(),area.bottom()-metrics.height()));
        const QRectF tag(anchor,QSizeF(metrics.horizontalAdvance(name),metrics.height()));painter.fillRect(tag,Qt::yellow);painter.setPen(printing?Qt::black:Qt::blue);painter.drawText(tag,Qt::AlignLeft|Qt::AlignVCenter,name);
    };
    if(graph.objectiveVisible()){
        if(auto segment=graph.objectiveSegment())line(*segment,printing?Qt::black:Qt::blue,printing?metrics.height()/10:std::max(3.0,metrics.height()/2),QStringLiteral("Obj"));
    }
    for(std::size_t i=0;i<graph.visibleConstraints().size();++i)if(graph.visibleConstraints()[i])if(auto segment=graph.constraintSegment(i))line(*segment,printing?Qt::black:Qt::red,printing?metrics.height()/40:std::max(2.0,metrics.height()/8),QStringLiteral("( %1)").arg(i+1));
    if(graph.objectiveVisible()){

        const auto point=mapped(graph.result().point,area,maximum,graph.rangeMin());painter.setPen(printing?Qt::black:Qt::blue);painter.setBrush(printing?Qt::black:Qt::blue);const auto radius=metrics.height()/(printing?2.5:1.8);painter.drawEllipse(point,radius,radius);
        painter.drawText(point+QPointF(metrics.height()*1.5,-metrics.height()/3),QString::fromStdString(graph.result().markerLabel));
    }
    painter.restore();painter.save();painter.setPen(Qt::white);painter.drawRect(area);painter.restore();
}
QString resultText(const tora::lp::GraphicalSession* graph){
    if(!graph || !graph->objectiveVisible())return {};
    const auto& r=graph->result();if(!r.message.empty())return QString::fromStdString(r.message);
    return QStringLiteral("Objective value = %1\nx1 = %2\nx2 = %3").arg(QString::fromStdString(r.objective),QString::fromStdString(r.x1),QString::fromStdString(r.x2));
}
}
GraphPlot::GraphPlot(LpGraphicalController& c,QWidget* parent):QWidget(parent),controller_(c){setObjectName(QStringLiteral("graph-plot"));setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);setMinimumSize(QFontMetrics(font()).height()*6,QFontMetrics(font()).height()*6);connect(&c,&LpGraphicalController::changed,this,qOverload<>(&QWidget::update));}
QRectF GraphPlot::plotRect() const {const auto margin=QFontMetricsF(font()).height()/4;const auto side=std::max(0.0,std::min(width(),height())-2*margin);return {(width()-side)/2,(height()-side)/2,side,side};}
QPointF GraphPlot::mapPoint(tora::lp::GraphPoint p) const {return mapped(p,plotRect(),controller_.session()?controller_.session()->rangeMax():6,controller_.session()?controller_.session()->rangeMin():-1);}
void GraphPlot::paintEvent(QPaintEvent*){QPainter p(this);if(controller_.session())drawPlot(p,plotRect(),*controller_.session(),font());}
bool printGraph(QPrinter& printer,const LpGraphicalController& c,const QFont& font){
    return c.session() && printLpGraph(printer,*c.session(),font,[&](QPainter& painter,const QRectF& area,const QFont& paperFont){
        drawPlot(painter,area,*c.session(),paperFont,true);
    });
}
QWidget* graphicalScreen(ApplicationShell& shell){
    auto* page=new QWidget;auto* outer=new QVBoxLayout(page);auto* heading=new QLabel(QStringLiteral("GRAPHICAL LINEAR PROGRAMMING SOLUTION"));heading->setObjectName(QStringLiteral("graph-heading"));heading->setAlignment(Qt::AlignCenter);outer->addWidget(heading);
    auto& c=graphicalController(shell);auto* body=new QHBoxLayout;auto* left=new QVBoxLayout;auto* instruction=new QLabel(QStringLiteral("To graph the LP below, click constraints\none at a time, then click objective function"));instruction->setWordWrap(true);left->addWidget(instruction);
    auto* rows=new QListWidget;rows->setObjectName(QStringLiteral("graph-problem"));rows->setStyleSheet(QStringLiteral("QListWidget {background: white; color: #172b4d; border: 1px solid #879bb5;} QListWidget::item:selected {background: #225ea8; color: white;}"));left->addWidget(rows,1);
    auto* solution=new QGroupBox(QStringLiteral("Optimal Solution"));auto* solutionLayout=new QVBoxLayout(solution);auto* result=new QLabel;result->setObjectName(QStringLiteral("graph-result"));result->setWordWrap(true);result->setTextFormat(Qt::PlainText);solutionLayout->addWidget(result);left->addWidget(solution);
    body->addLayout(left,2);auto* right=new QVBoxLayout;auto* controls=new QHBoxLayout;controls->addStretch();auto* zoomIn=new QPushButton(QStringLiteral("Zoom In"));zoomIn->setObjectName(QStringLiteral("graph-zoom-in"));auto* zoomOut=new QPushButton(QStringLiteral("Zoom Out"));zoomOut->setObjectName(QStringLiteral("graph-zoom-out"));auto* print=new QPushButton(QStringLiteral("Print Graph"));print->setObjectName(QStringLiteral("graph-print"));for(auto* b:{zoomIn,zoomOut,print})controls->addWidget(b);controls->addStretch();right->addLayout(controls);auto* plot=new GraphPlot(c);right->addWidget(plot,1);body->addLayout(right,3);outer->addLayout(body,1);
    auto* footer=new QHBoxLayout;footer->addStretch();auto* input=new QPushButton(QStringLiteral("View/Modify Input Data"));input->setObjectName(QStringLiteral("graph-view-input"));auto* main=new QPushButton(QStringLiteral("MAIN Menu"));auto* exit=new QPushButton(QStringLiteral("Exit TORA"));for(auto* b:{input,main,exit})footer->addWidget(b);footer->addStretch();outer->addLayout(footer);
    QObject::connect(input,&QPushButton::clicked,&shell,[&shell]{shell.navigate(QStringLiteral("lp/input"));});QObject::connect(main,&QPushButton::clicked,&shell,[&shell]{shell.navigate(QStringLiteral("main"));});QObject::connect(exit,&QPushButton::clicked,&shell,&QWidget::close);
    QObject::connect(rows,&QListWidget::itemClicked,page,[&c,rows](QListWidgetItem* item){c.selectRow(rows->row(item));});QObject::connect(rows,&QListWidget::itemActivated,page,[&c,rows](QListWidgetItem* item){c.selectRow(rows->row(item));});
    QObject::connect(zoomIn,&QPushButton::clicked,page,[&c]{c.zoom(1);});QObject::connect(zoomOut,&QPushButton::clicked,page,[&c]{c.zoom(-1);});
    QObject::connect(print,&QPushButton::clicked,page,[&c,&shell]{if(requestLpPrint(&shell,[&](QPrinter& printer){return printGraph(printer,c,shell.font());})==PrintOutcome::Failed)c.notice(QStringLiteral("Print Graph"),QStringLiteral("The graph could not be printed"));});
    const auto refresh=[&c,rows,result,zoomIn,zoomOut,print]{
        const auto* graph=c.session();QStringList values;if(graph)for(const auto& row:graph->expressionLines())values.append(legacyText(row));
        bool same=rows->count()==values.size();if(same)for(int i=0;i<rows->count();++i)if(rows->item(i)->text()!=values[i]){same=false;break;}if(!same){rows->clear();rows->addItems(values);const auto h=QFontMetrics(rows->font()).height();for(int i=0;i<rows->count();++i)rows->item(i)->setSizeHint(QSize(0,(i==0 || i==1 || i==rows->count()-2)?2*h:h));if(graph)rows->setCurrentRow(static_cast<int>(graph->problem().constraints.size())+3);}result->setText(resultText(graph));
        const bool enabled=graph && std::any_of(graph->visibleConstraints().begin(),graph->visibleConstraints().end(),[](bool b){return b;});zoomIn->setEnabled(enabled);zoomOut->setEnabled(enabled);print->setEnabled(graph!=nullptr);
    };QObject::connect(&c,&LpGraphicalController::changed,page,refresh);refresh();return page;
}
} // namespace tora::desktop
