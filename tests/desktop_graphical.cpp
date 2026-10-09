#include "lp_graphical_controller.hpp"
#include "lp_input_controller.hpp"
#include "lp_simplex_controller.hpp"
#include "lp_screens.hpp"
#include <QTest>
#include <QApplication>
#include <QListWidget>
#include <QPushButton>
#include <QAction>
#include <QLabel>
#include <QDir>
#include <QTemporaryDir>
#include <QPrinter>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
using namespace tora::desktop;
class DesktopGraphical:public QObject {
    Q_OBJECT
private slots:
    void unsupportedDimensionsAndBounds(){
        for(int kind=0;kind<5;++kind){
            const auto n=kind==0?1:kind==1?3:2;auto problem=tora::lp::LpProblem::blank(static_cast<std::size_t>(n),1);
            std::fill(problem.objective.begin(),problem.objective.end(),1);std::fill(problem.constraints[0].coefficients.begin(),problem.constraints[0].coefficients.end(),1);problem.constraints[0].rhs=4;
            if(kind==2)problem.variables[0].upperBound=1;
            if(kind==3)problem.variables[0].lowerBound=1;
            if(kind==4)problem.variables[0].unrestricted=true;
            QTemporaryDir temp;auto doc=tora::compat::ToraLpCodec::create(problem);const auto file=temp.filePath(QStringLiteral("problem.txt"));tora::compat::LpFileStore::saveAs(doc,std::filesystem::path(file.toStdString()));
            ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();auto& in=inputController(shell);in.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(in.openFile(file));QVERIFY(shell.navigate(QStringLiteral("lp/solve-menu")));
            auto* action=shell.findChild<QAction*>(QStringLiteral("solve-graphical"));QVERIFY(action);QCOMPARE(action->isEnabled(),kind>=2);
            auto& c=graphicalController(shell);QString title,body;c.notice=[&](const QString& t,const QString& b){title=t;body=b;};QVERIFY(!c.request());QVERIFY(!c.session() && !c.canStart());
            if(kind>=2){QCOMPARE(title,QStringLiteral("Graphical LP"));QCOMPARE(body,QString::fromLatin1(tora::lp::GraphicalSession::boundsError()));}else QVERIFY(body.isEmpty());
        }
    }
    void menuManualZoomPrintingAndLifecycle(){
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();shell.navigate(QStringLiteral("lp/solve-menu"));
        auto* action=shell.findChild<QAction*>(QStringLiteral("solve-graphical"));QVERIFY(action && !action->isEnabled());
        auto& in=inputController(shell);in.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/01_normal.txt")));
        shell.navigate(QStringLiteral("lp/solve-menu"));action=shell.findChild<QAction*>(QStringLiteral("solve-graphical"));QVERIFY(action && action->isEnabled());
        auto& c=graphicalController(shell);QString title,body;c.notice=[&](const QString& t,const QString& b){title=t;body=b;};action->trigger();QCOMPARE(shell.currentScreen(),QStringLiteral("lp/output-format"));
        QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("output-go")),Qt::LeftButton);QCOMPARE(shell.currentScreen(),QStringLiteral("lp/graphical"));
        auto* rows=shell.currentContent()->findChild<QListWidget*>(QStringLiteral("graph-problem"));auto* plot=shell.currentContent()->findChild<GraphPlot*>(QStringLiteral("graph-plot"));QVERIFY(rows && plot);
        c.selectRow(1);QCOMPARE(title,QStringLiteral("Graph Obj Function"));QCOMPARE(body,QStringLiteral("Must graph all constraints prior to objective function"));QVERIFY(!c.session()->objectiveVisible());
        c.selectRow(3);c.selectRow(3);c.selectRow(4);c.selectRow(5);c.selectRow(1);QCOMPARE(c.session()->result().objective,std::string("10.00"));
        c.zoom(1);QCOMPARE(c.session()->rangeMax(),4.0);c.zoom(-1);QCOMPARE(c.session()->rangeMax(),5.0);
        const auto scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();
        for(const auto size:{QSize(1366,768),QSize(1920,1080),QSize(3840,2160)}){
            const QSize logical(qRound(size.width()/scale),qRound(size.height()/scale));shell.resize(logical);QApplication::processEvents();QCOMPARE(shell.size(),logical);const auto area=plot->plotRect();QVERIFY(area.width()>100);QVERIFY(qAbs(area.width()-area.height())<0.01);
            const auto origin=plot->mapPoint({0,0});QVERIFY(qAbs((origin.x()-area.left())/area.width()-1.0/6)<1e-9);
            const auto optimum=plot->mapPoint({2,2});QVERIFY(qAbs((optimum.x()-area.left())/area.width()-0.5)<1e-9);
            for(auto* b:shell.currentContent()->findChildren<QPushButton*>())QVERIFY(b->width()>=b->minimumSizeHint().width());
            if(!qEnvironmentVariable("TORA_UI_CAPTURES").isEmpty()){
                const auto root=qEnvironmentVariable("TORA_UI_CAPTURES")+QStringLiteral("/graphical");QVERIFY(QDir().mkpath(root));QVERIFY(shell.grab().save(root+QStringLiteral("/normal-%1x%2-%3.png").arg(size.width()).arg(size.height()).arg(scale)));
            }
        }
        QTemporaryDir temp;QPrinter printer(QPrinter::HighResolution);printer.setOutputFormat(QPrinter::PdfFormat);printer.setOutputFileName(temp.filePath(QStringLiteral("graph.pdf")));QVERIFY(printGraph(printer,c,shell.font()));QVERIFY(QFile(printer.outputFileName()).size()>1000);QCOMPARE(c.session()->result().objective,std::string("10.00"));
        c.selectRow(6);QVERIFY(!c.session()->objectiveVisible());c.selectRow(7);QVERIFY(c.session()->objectiveVisible());
        QVERIFY(simplexController(shell).requestAllSlack());QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("output-go")),Qt::LeftButton);QCOMPARE(shell.currentScreen(),QStringLiteral("lp/simplex"));
        QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/02_degenerate.txt")));QVERIFY(!c.session() && !c.canStart());
    }
    void goldenResultsInNativePanels_data(){QTest::addColumn<QByteArray>("record");QFile f(QStringLiteral(TORA_GRAPHICAL_GOLDENS));QVERIFY(f.open(QIODevice::ReadOnly));for(const auto& v:QJsonDocument::fromJson(f.readAll()).array()){const auto r=v.toObject();QTest::newRow(r["case"].toString().toUtf8().constData())<<QJsonDocument(r).toJson(QJsonDocument::Compact);}}
    void goldenResultsInNativePanels(){
        QFETCH(QByteArray,record);const auto r=QJsonDocument::fromJson(record).object(),p=r["problem"].toObject();const auto vars=p["variables"].toArray(),constraints=p["constraints"].toArray();
        auto problem=tora::lp::LpProblem::blank(static_cast<std::size_t>(vars.size()),static_cast<std::size_t>(constraints.size()));problem.title=p["title"].toString().toStdString();problem.sense=p["sense"].toString()==QStringLiteral("max")?tora::lp::ObjectiveSense::Maximize:tora::lp::ObjectiveSense::Minimize;
        for(int j=0;j<vars.size();++j){const auto v=vars[j].toObject();auto& target=problem.variables[static_cast<std::size_t>(j)];target.lowerBound=std::stod(v["lower"].toString().toStdString());target.upperBound=std::stod(v["upper"].toString().toStdString());target.unrestricted=v["unrestricted"].toBool();problem.objective[static_cast<std::size_t>(j)]=std::stod(p["objective"].toArray()[j].toString().toStdString());}
        for(int i=0;i<constraints.size();++i){const auto c=constraints[i].toObject();auto& target=problem.constraints[static_cast<std::size_t>(i)];target.rhs=std::stod(c["rhs"].toString().toStdString());target.relation=c["relation"].toString()==QStringLiteral("<=")?tora::lp::Relation::LessEqual:c["relation"].toString()==QStringLiteral(">=")?tora::lp::Relation::GreaterEqual:tora::lp::Relation::Equal;for(int j=0;j<vars.size();++j)target.coefficients[static_cast<std::size_t>(j)]=std::stod(c["coefficients"].toArray()[j].toString().toStdString());}
        QTemporaryDir temp;auto doc=tora::compat::ToraLpCodec::create(problem);const auto file=temp.filePath(QStringLiteral("problem.txt"));tora::compat::LpFileStore::saveAs(doc,std::filesystem::path(file.toStdString()));
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();auto& in=inputController(shell);in.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(in.openFile(file));auto& c=graphicalController(shell);QString message;c.notice=[&](const QString&,const QString& b){message=b;};QVERIFY(c.request());const auto fmt=r["format"].toArray();QVERIFY(c.start({fmt[0].toInt()?tora::core::Notation::Scientific:tora::core::Notation::Decimal,fmt[1].toInt(),fmt[2].toInt()}));c.selectRow(static_cast<int>(problem.constraints.size())+4);
        const auto expected=r["expected"].toObject();const auto* panel=shell.currentContent()->findChild<QLabel*>(QStringLiteral("graph-result"));QVERIFY(panel);const auto body=panel->text();
        if(expected["message"].toString().isEmpty()){
            QVERIFY(body.contains(QStringLiteral("Objective value = ")+expected["objective"].toString()));QVERIFY(body.contains(QStringLiteral("x1 = ")+expected["x1"].toString()));QVERIFY(body.contains(QStringLiteral("x2 = ")+expected["x2"].toString()));
        }else QVERIFY(body.contains(expected["message"].toString()));
        QCOMPARE(c.session()->rangeMax(),expected["range_max"].toDouble());QVERIFY(message.isEmpty());
        const auto scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();const QSize logical(qRound(1920/scale),qRound(1080/scale));shell.resize(logical);QApplication::processEvents();QCOMPARE(shell.size(),logical);
        auto* plot=shell.currentContent()->findChild<GraphPlot*>(QStringLiteral("graph-plot"));QVERIFY(plot);const auto area=plot->plotRect();const auto bitmap=plot->grab().toImage();const auto dpr=bitmap.devicePixelRatio();
        if(!qEnvironmentVariable("TORA_UI_CAPTURES").isEmpty()){
            const auto root=qEnvironmentVariable("TORA_UI_CAPTURES")+QStringLiteral("/graphical");QVERIFY(QDir().mkpath(root));QVERIFY(shell.grab().save(root+QStringLiteral("/%1-%2.png").arg(r["case"].toString()).arg(scale)));
        }
        for(const auto& value:r["color_samples"].toArray()){
            const auto sample=value.toObject();const auto rgb=sample["rgb"].toArray();const QColor color(rgb[0].toInt(),rgb[1].toInt(),rgb[2].toInt());
            const auto at=QPointF(area.left()+area.width()*sample["x"].toDouble(),area.top()+area.height()*sample["y"].toDouble())*dpr;
            const auto radius=qCeil((area.width()*2/398+QFontMetrics(plot->font()).height()/4.0)*dpr);bool found=false;
            for(int y=std::max(0,qRound(at.y())-radius);y<std::min(bitmap.height(),qRound(at.y())+radius+1) && !found;++y)
                for(int x=std::max(0,qRound(at.x())-radius);x<std::min(bitmap.width(),qRound(at.x())+radius+1);++x)if(bitmap.pixelColor(x,y)==color){found=true;break;}
            QVERIFY2(found,qPrintable(r["case"].toString()+QStringLiteral(" ")+sample["label"].toString()+QStringLiteral(" captured color missing")));
        }
    }
};
QTEST_MAIN(DesktopGraphical)
#include "desktop_graphical.moc"
