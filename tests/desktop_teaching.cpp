#include "assignment_controller.hpp"
#include "games_screens.hpp"
#include "transportation_screens.hpp"
#include "lp_final_controller.hpp"
#include "lp_screens.hpp"
#include "model_json.hpp"
#include "history_view.hpp"
#include <QTest>
#include <QFile>
#include <QDir>
#include <QTemporaryDir>
#include <QComboBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QTabWidget>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPrinter>
using namespace tora::desktop;
namespace {
QString example(const QString& name){return QStringLiteral(TORA_TEACHING_EXAMPLES)+"/"+name;}
QByteArray read(const QString& path){QFile f(path);if(!f.open(QIODevice::ReadOnly))throw std::runtime_error("Cannot read test file");return f.readAll();}
void write(const QString& path,const QByteArray& bytes){QFile f(path);if(!f.open(QIODevice::WriteOnly)||f.write(bytes)!=bytes.size())throw std::runtime_error("Cannot write test file");}
}
class DesktopTeaching final:public QObject {
 Q_OBJECT
private slots:
 void originalDummyMappings_data(){QTest::addColumn<QString>("name");QTest::addColumn<bool>("transport");QTest::addColumn<bool>("worker");QTest::newRow("TP17-Q1")<<QString("17-Q1.transportation.txt")<<true<<false;QTest::newRow("TP17-Q2")<<QString("17-Q2.transportation.txt")<<true<<false;QTest::newRow("TP18-Q1")<<QString("18-Q1.transportation.txt")<<true<<true;QTest::newRow("TP18-Q2")<<QString("18-Q2.transportation.txt")<<true<<true;QTest::newRow("AP25-Q1")<<QString("25-Q1.assignment.txt")<<false<<true;QTest::newRow("AP25-Q2")<<QString("25-Q2.assignment.txt")<<false<<false;QTest::newRow("AP29-Q1")<<QString("29-Q1.assignment.txt")<<false<<false;QTest::newRow("AP29-Q2")<<QString("29-Q2.assignment.txt")<<false<<false;}
 void originalDummyMappings(){QFETCH(QString,name);QFETCH(bool,transport);QFETCH(bool,worker);if(transport){const auto d=tora::compat::TransportationDocument::decode(read(example(name)).toStdString());const auto& p=d.problem();const auto& mapped=worker?p.dummySources:p.dummyDestinations;const auto& other=worker?p.dummyDestinations:p.dummySources;QCOMPARE(mapped,std::vector<std::size_t>{2});QVERIFY(other.empty());QCOMPARE(worker?p.sourceNames[2]:p.destinationNames[2],std::string("DUMMY"));}else{const auto d=decodeAssignment(read(example(name)));const auto& mapped=worker?d.dummyWorkers:d.dummyJobs;const auto& other=worker?d.dummyJobs:d.dummyWorkers;QCOMPARE(mapped,std::vector<std::size_t>{2});QVERIFY(other.empty());QCOMPARE(encodeAssignment(decodeAssignment(encodeAssignmentText(d))),encodeAssignment(d));}}
 void assignmentPairs_data(){QTest::addColumn<QString>("name");QTest::addColumn<double>("objective");QTest::addColumn<bool>("feasible");QTest::newRow("26-Q1")<<QString("26-Q1")<<7.<<true;QTest::newRow("26-Q2")<<QString("26-Q2")<<5.<<true;QTest::newRow("28-Q1")<<QString("28-Q1")<<0.<<false;QTest::newRow("28-Q2")<<QString("28-Q2")<<0.<<false;QTest::newRow("rectangle")<<QString("rectangular-max-negative")<<-5.<<true;QTest::newRow("dummy")<<QString("explicit-dummy")<<3.<<true;QTest::newRow("penalty")<<QString("finite-penalty-legacy")<<2000.<<true;}
 void assignmentPairs(){
    QFETCH(QString,name);QFETCH(double,objective);QFETCH(bool,feasible);
    const auto json=decodeAssignment(read(example(name+".assignment.json")));const auto txt=decodeAssignment(read(example(name+".assignment.txt")));
    QCOMPARE(encodeAssignment(json),encodeAssignment(txt));QCOMPARE(encodeAssignment(decodeAssignment(encodeAssignmentText(json))),encodeAssignment(json));
    ApplicationShell shell;QVERIFY(shell.registerModule(assignmentModule()));auto& c=assignmentController(shell);c.dialogs().askSave=[] {return SaveChoice::No;};c.dialogs().showError=[](const QString&,const QString&){};
    QVERIFY(c.openFile(example(name+".assignment.txt")));QVERIFY(c.solve());
    QCOMPARE(c.result()->status==tora::assignment::Status::Optimal,feasible);if(feasible)QCOMPARE(c.result()->objective,objective);
    else {QVERIFY(c.result()->rowToColumn.empty());auto* pairs=shell.currentContent()->findChild<QTableWidget*>("assignment-pairs");QCOMPARE(pairs->rowCount(),0);}
    if(name=="26-Q1")QCOMPARE(c.result()->paddedAssignment,std::vector<std::size_t>({0,1,2}));
    if(name=="26-Q2")QCOMPARE(c.result()->paddedAssignment,std::vector<std::size_t>({1,2,0}));
 }
 void assignmentLpMapping(){
    const auto d=decodeAssignment(read(example("26-Q1.assignment.json")));auto p=tora::lp::LpProblem::blank(9,6+d.model.forbidden.size());p.sense=tora::lp::ObjectiveSense::Minimize;p.objective=d.model.costs.data();
    for(std::size_t k=0;k<6;++k){auto& r=p.constraints[k];r.relation=tora::lp::Relation::Equal;r.rhs=1;for(std::size_t j=0;j<9;++j)r.coefficients[j]=k<3?(j/3==k):(j%3==k-3);}
    for(std::size_t k=0;k<d.model.forbidden.size();++k){const auto pair=d.model.forbidden[k];auto& r=p.constraints[6+k];r.relation=tora::lp::Relation::Equal;r.coefficients[pair.first*3+pair.second]=1;}
    const auto convert=[&]{return decodeAssignment(QByteArray::fromStdString(tora::compat::ToraLpCodec::encode(tora::compat::ToraLpCodec::create(p))));};
    const auto imported=convert();QVERIFY(!imported.mappingMessage.isEmpty());QCOMPARE(imported.model.forbidden,d.model.forbidden);QCOMPARE(tora::assignment::solve(imported.model).objective,7.);
    p.constraints[0].rhs=2;QVERIFY_EXCEPTION_THROWN(convert(),std::invalid_argument);p.constraints[0].rhs=1;p.constraints[6].relation=tora::lp::Relation::GreaterEqual;QVERIFY_EXCEPTION_THROWN(convert(),std::invalid_argument);
    p=tora::lp::LpProblem::blank(2,1);p.objective={3,2};p.constraints[0]={{1,1},tora::lp::Relation::LessEqual,4};QVERIFY_EXCEPTION_THROWN(convert(),std::invalid_argument);
 }
 void lpTransportationGamePairs(){
    const auto lp=decodeLpJson(read(example("lp.json")));const auto txt=tora::compat::ToraLpCodec::decode(read(example("lp.txt")).toStdString());QCOMPARE(encodeLpJson(lp),encodeLpJson(txt));
    auto exact=QJsonDocument::fromJson(encodeLpJson(lp)).object();exact["objective"]=QJsonArray{1.23456789012345,2.};auto vars=exact["variables"].toArray();auto y=vars[1].toObject();y["lower"]=0.125;vars[1]=y;exact["variables"]=vars;
    const auto precise=decodeLpJson(QJsonDocument(exact).toJson());QCOMPARE(precise.problem.objective[0],1.23456789012345);QCOMPARE(precise.problem.variables[1].lowerBound,0.125);QCOMPARE(encodeLpJson(tora::compat::ToraLpCodec::decode(tora::compat::ToraLpCodec::encode(precise))),encodeLpJson(precise));
    const auto transport=decodeTransportationJson(read(example("transportation.json")));const auto transportTxt=tora::compat::TransportationDocument::decode(read(example("transportation.txt")).toStdString());QCOMPARE(encodeTransportationJson(transport),encodeTransportationJson(transportTxt));
    const auto maxPair=decodeTransportationJson(read(example("transportation-max-dummy.json")));QCOMPARE(encodeTransportationJson(maxPair),encodeTransportationJson(tora::compat::TransportationDocument::decode(read(example("transportation-max-dummy.txt")).toStdString())));QCOMPARE(tora::transportation::solveLeastCost(maxPair.problem()).back().objective,20.);
    const auto game=decodeGameJson(read(example("game.json")));const auto gameTxt=GamesDocument::decode(read(example("game.txt")).toStdString());QCOMPARE(encodeGameJson(game),encodeGameJson(gameTxt));QCOMPARE(encodeGameJson(GamesDocument::decode(game.encode())),encodeGameJson(game));
    ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));auto& input=inputController(shell);input.dialogs().showError=[](const QString&,const QString&){};QVERIFY(input.openFile(example("lp.json"),true));QVERIFY(input.document()->readOnly());auto& final=finalController(shell);QVERIFY(final.request());QVERIFY(final.start({}));QCOMPARE(*final.session()->display().objectiveValue,std::string("-1.00"));
    QTemporaryDir dir;QVERIFY(!input.saveTo(example("lp.json")));QVERIFY(input.saveTo(dir.filePath("copy.json")));QVERIFY(!input.document()->readOnly());
    auto& tc=transportationController(shell);QString error;QVERIFY(tc.openFile(example("transportation.json"),error));QVERIFY(tc.computeFinal(error));QVERIFY(tc.finalOutput());
    QCOMPARE(tc.finalOutput()->objective,std::string("18.00"));
    auto maximum=transport.problem();maximum.maximize=true;maximum.dummyDestinations={1};const auto maxDoc=tora::compat::TransportationDocument::create(maximum);
    const auto maxTxt=tora::compat::TransportationDocument::decode(maxDoc.encode());QCOMPARE(encodeTransportationJson(maxTxt),encodeTransportationJson(maxDoc));
    QCOMPARE(tora::transportation::solveLeastCost(maxTxt.problem()).back().objective,34.);
    auto edited=maxTxt;edited.insert(tora::compat::TransportationDocument::Axis::Destination,0);QCOMPARE(edited.problem().dummyDestinations,std::vector<std::size_t>{2});edited.erase(tora::compat::TransportationDocument::Axis::Destination,0);QCOMPARE(encodeTransportationJson(edited),encodeTransportationJson(maxDoc));
    auto& gc=gamesController(shell);QVERIFY(gc.openFile(example("game.json"),error));QVERIFY(gc.solve(error));QCOMPARE(gc.result()->value,0.);QCOMPARE(gc.result()->rowStrategy[0],.5);
    QVERIFY(gc.solve(error,true));
    QVERIFY(gc.outputText().contains("Heads A = 0.50"));
    QVERIFY(gc.outputText().contains("Tails B: 1.00 - 2.00 p"));
    gc.beginNew({tora::core::Notation::Scientific,4,2});
    QVERIFY(gc.initialize("Extreme finite game",2,2,error));
    QVERIFY(gc.editCell(0,0,"1e308",error));
    QVERIFY(gc.editCell(0,1,"-1e308",error));
    QVERIFY(gc.editCell(1,0,"-1e308",error));
    QVERIFY(gc.editCell(1,1,"1e308",error));
    QVERIFY(gc.solve(error,true));
    QCOMPARE(gc.result()->value,0.);
    QVERIFY(gc.outputText().contains("(1 - p) × (-1.0000e+308) + p × (1.0000e+308)"));
    QVERIFY(!gc.outputText().contains("infinity"));
 }
 void malformedIsTransactional(){
    QTemporaryDir dir;const auto bad=dir.filePath("bad.json");ApplicationShell shell;QString error;
    auto legacy=QJsonDocument::fromJson(read(example("finite-penalty-legacy.assignment.json"))).object();legacy.remove("forbidden");legacy.remove("dummyWorkers");legacy.remove("dummyJobs");legacy["title"]="Forbidden-looking costs are finite penalties";const auto penalty=decodeAssignment(QJsonDocument(legacy).toJson());QVERIFY(penalty.model.forbidden.empty());QCOMPARE(tora::assignment::solve(penalty.model).objective,2000.);
    auto& ac=assignmentController(shell);ac.dialogs().askSave=[] {return SaveChoice::No;};ac.dialogs().showError=[](const QString&,const QString&){};QVERIFY(ac.openFile(example("26-Q1.assignment.json")));const auto assignmentBefore=encodeAssignment(*ac.data());auto a=QJsonDocument::fromJson(assignmentBefore).object();a["forbidden"]=QJsonArray{QJsonArray{0,1}};write(bad,QJsonDocument(a).toJson());QVERIFY(!ac.openFile(bad));QCOMPARE(encodeAssignment(*ac.data()),assignmentBefore);
    auto& lp=inputController(shell);lp.dialogs().showError=[](const QString&,const QString&){};QVERIFY(lp.openFile(example("lp.json")));const auto lpBefore=encodeLpJson(*lp.document());auto l=QJsonDocument::fromJson(lpBefore).object();l["objective"]=QJsonArray{1};write(bad,QJsonDocument(l).toJson());QVERIFY(!lp.openFile(bad));QCOMPARE(encodeLpJson(*lp.document()),lpBefore);
    auto& tc=transportationController(shell);QVERIFY(tc.openFile(example("transportation.json"),error));const auto tBefore=encodeTransportationJson(*tc.document());auto t=QJsonDocument::fromJson(tBefore).object();t["supply"]=QJsonArray{-1,6};write(bad,QJsonDocument(t).toJson());QVERIFY(!tc.openFile(bad,error));QCOMPARE(encodeTransportationJson(*tc.document()),tBefore);
    auto& gc=gamesController(shell);QVERIFY(gc.openFile(example("game.json"),error));QVERIFY(gc.solve(error));const auto gBefore=encodeGameJson(*gc.document());auto g=QJsonDocument::fromJson(gBefore).object();g["payoff"]=QJsonArray{QJsonArray{1,2},QJsonArray{3}};write(bad,QJsonDocument(g).toJson());QVERIFY(!gc.openFile(bad,error));QCOMPARE(encodeGameJson(*gc.document()),gBefore);QVERIFY(gc.result());
    write(bad,"{ invalid");QVERIFY_EXCEPTION_THROWN(decodeAssignment(read(bad)),std::invalid_argument);QVERIFY_EXCEPTION_THROWN(decodeLpJson(read(bad)),std::invalid_argument);QVERIFY_EXCEPTION_THROWN(decodeTransportationJson(read(bad)),std::invalid_argument);QVERIFY_EXCEPTION_THROWN(decodeGameJson(read(bad)),std::invalid_argument);
 }
 void navigationAndExport(){
    ApplicationShell shell;QVERIFY(shell.registerModule(assignmentModule()));QVERIFY(shell.registerModule(gamesModule()));shell.resize(1200,850);shell.show();QTemporaryDir dir;
    auto& ac=assignmentController(shell);ac.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(ac.openFile(example("26-Q2.assignment.txt")));QVERIFY(ac.solve());const auto aBefore=encodeAssignment(*ac.data());const auto aObjective=ac.result()->objective;const auto aSteps=ac.result()->history.size();
    const auto exercise=[&](const QString& prefix){auto* view=shell.currentContent()->findChild<HistoryView*>(prefix+"-history");QVERIFY(view);auto* tabs=shell.currentContent()->findChild<QTabWidget*>();QVERIFY(tabs);tabs->setCurrentWidget(view);auto* selector=view->findChild<QComboBox*>(prefix+"-step-selector");QVERIFY(selector&&selector->count()>2);auto* text=view->findChild<QPlainTextEdit*>(prefix+"-step-text");QVERIFY(text);const auto first=text->toPlainText();QTest::mouseClick(view->findChild<QPushButton*>(prefix+"-step-next"),Qt::LeftButton);QCOMPARE(selector->currentIndex(),1);QVERIFY(text->toPlainText()!=first);QTest::mouseClick(view->findChild<QPushButton*>(prefix+"-step-previous"),Qt::LeftButton);QCOMPARE(text->toPlainText(),first);selector->setCurrentIndex(selector->count()-1);QVERIFY(!view->findChild<QPushButton*>(prefix+"-step-next")->isEnabled());QTest::mouseClick(view->findChild<QPushButton*>(prefix+"-step-all"),Qt::LeftButton);QCOMPARE(text->toPlainText(),view->completeText());const auto path=dir.filePath(prefix+".txt");QVERIFY(view->exportTo(path));QCOMPARE(QString::fromUtf8(read(path)),view->completeText());QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);const auto pdf=dir.filePath(prefix+".pdf");printer.setOutputFileName(pdf);QVERIFY(view->printTo(printer));QVERIFY(read(pdf).startsWith("%PDF-"));
        const auto root=qEnvironmentVariable("TORA_UI_CAPTURES");if(!root.isEmpty()){QVERIFY(QDir().mkpath(root));selector->setCurrentIndex(prefix=="assignment"?5:3);QApplication::processEvents();QVERIFY(shell.grab().save(root+"/"+prefix+"-steps.png"));QVERIFY(QFile::copy(pdf,root+"/"+prefix+"-history.pdf"));}
    };
    exercise("assignment");QCOMPARE(encodeAssignment(*ac.data()),aBefore);QCOMPARE(ac.result()->objective,aObjective);QCOMPARE(ac.result()->history.size(),aSteps);
    auto& gc=gamesController(shell);QString error;QVERIFY(gc.openFile(example("game.json"),error));QVERIFY(gc.solve(error));QVERIFY(shell.navigate("games/result"));const auto gBefore=encodeGameJson(*gc.document());const auto gSteps=gc.result()->history.size();exercise("games");QCOMPARE(encodeGameJson(*gc.document()),gBefore);QCOMPARE(gc.result()->value,0.);QCOMPARE(gc.result()->history.size(),gSteps);
 }
};
QTEST_MAIN(DesktopTeaching)
#include "desktop_teaching.moc"
