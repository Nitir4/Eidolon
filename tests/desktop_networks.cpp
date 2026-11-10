#include "pdf_text.hpp"
#include "networks_screens.hpp"
#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QPrinter>
#include <QTableWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QDir>
#include <QLineEdit>
using namespace tora::desktop;
namespace {
std::string record(NetworkMode mode){
 const int marker=mode==NetworkMode::MinimalTree?7001:mode==NetworkMode::ShortestRoute?0:7004;
 std::string out=std::to_string(marker)+"\r\n5\r\n2\r\n1\r\n\"Original grid\"\r\n3\r\n";
 const char* mst[4][4]={{"Node Name","A","B","C"},{"A","","1","3"},{"B","1","","2"},{"C","3","2",""}};
 const char* route[4][4]={{"Node Name","A","B","C"},{"A","","2","8"},{"B","infinity","","3"},{"C","infinity","infinity",""}};
 const char* flow[4][4]={{"Node Name","A","B","C"},{"A","","3","2"},{"B","0.00","","4"},{"C","0.00","0.00",""}};
 for(int r=0;r<4;++r)for(int c=0;c<4;++c)out+='"'+std::string(mode==NetworkMode::MinimalTree?mst[r][c]:mode==NetworkMode::ShortestRoute?route[r][c]:flow[r][c])+"\"\r\n";
 return out+(mode==NetworkMode::MinimalTree?"1\r\n":"0\r\n");
}
bool load(NetworksController& c,NetworkMode mode,QTemporaryDir& dir,QString& error,bool ro=false){c.beginNew(mode);QFile f(dir.filePath(QString::number(static_cast<int>(mode))+".txt"));if(!f.open(QIODevice::WriteOnly))return false;auto bytes=record(mode);if(f.write(bytes.data(),bytes.size())!=static_cast<qint64>(bytes.size()))return false;f.close();return c.openFile(f.fileName(),error,ro);}
}
class DesktopNetworks:public QObject{
 Q_OBJECT
private slots:
 void originalFormatsAndEditing(){
  for(auto mode:{NetworkMode::MinimalTree,NetworkMode::ShortestRoute,NetworkMode::MaximalFlow}){auto bytes=record(mode);auto d=NetworksDocument::decode(bytes,mode);QCOMPARE(d.encode(),bytes);QCOMPARE(d.nodes(),3);QCOMPARE(d.nodeName(1),std::string("B"));QCOMPARE(d.graph().directed,mode!=NetworkMode::MinimalTree);d.setCell(1,2," +4.25 ");QCOMPARE(d.graph().edges.front().value,4.25);if(mode==NetworkMode::MinimalTree)QCOMPARE(d.cell(2,1),std::string(" +4.25 "));d.setTitle("A \"quoted\" title \\ path");auto next=NetworksDocument::decode(d.encode(),mode);QCOMPARE(next.title(),d.title());QVERIFY_EXCEPTION_THROWN(next.setCell(1,2,"nan"),std::invalid_argument);QVERIFY_EXCEPTION_THROWN(next.setCell(1,1,"1"),std::invalid_argument);QVERIFY_EXCEPTION_THROWN(NetworksDocument::decode(bytes+"junk",mode),std::invalid_argument);}
  QVERIFY_EXCEPTION_THROWN(NetworksDocument::decode(record(NetworkMode::ShortestRoute),NetworkMode::MinimalTree),std::invalid_argument);
  auto d=NetworksDocument::blank(NetworkMode::MaximalFlow,3);QVERIFY_EXCEPTION_THROWN(d.setCell(1,2,"-1"),std::invalid_argument);d.setCell(0,1,"First");d.setCell(1,2,"7");d.insertNode(1);QCOMPARE(d.nodes(),4);QCOMPARE(d.cell(1,3),std::string("7"));QCOMPARE(d.nodeName(0),std::string("First"));d.eraseNode(1);QCOMPARE(d.cell(1,2),std::string("7"));
 }
 void originalExamplesAndFiles(){
  ApplicationShell shell;auto& c=networksController(shell);QString e;QTemporaryDir dir;
  QVERIFY(load(c,NetworkMode::MinimalTree,dir,e));QVERIFY(c.solve(0,2,e));QCOMPARE(std::get<tora::networks::SpanningForest>(*c.result()).totalWeight,3.);QVERIFY(c.outputText().contains("A -- B"));QCOMPARE(c.iterationText().size(),2);
  QVERIFY(load(c,NetworkMode::ShortestRoute,dir,e));QVERIFY(c.solve(0,2,e,NetworkRouteMethod::Dijkstra));const auto& routes=std::get<tora::networks::ShortestRoutes>(*c.result());QCOMPARE(*routes.routes[2].distance,5.);QCOMPARE(routes.routes[2].nodes,(std::vector<std::size_t>{0,1,2}));QVERIFY(c.outputText().contains("A -> B -> C"));QCOMPARE(c.iterationText().size(),3);QVERIFY(c.solve(0,2,e,NetworkRouteMethod::Floyd));QCOMPARE(std::get<tora::networks::AllPairsRoutes>(*c.result()).trace.size(),std::size_t(4));QCOMPARE(*std::get<tora::networks::AllPairsRoutes>(*c.result()).sources[0].routes[2].distance,5.);QVERIFY(c.solve(0,2,e));QVERIFY(c.outputText().contains("From C"));
  QVERIFY(load(c,NetworkMode::MaximalFlow,dir,e,true));QVERIFY(c.solve(0,2,e));QCOMPARE(std::get<tora::networks::MaximumFlow>(*c.result()).value,5.);QVERIFY(!c.editCell(1,2,"9",e));QVERIFY(!c.saveTo(c.sourcePath(),e));QVERIFY(c.saveTo(dir.filePath("copy.txt"),e));QVERIFY(!c.readOnly());QVERIFY(c.result());QVERIFY(!c.openFile(dir.filePath("absent.txt"),e));QVERIFY(c.result());QVERIFY(c.editCell(1,2,"9",e));QVERIFY(!c.result());QVERIFY(c.selectLine(true,0));QVERIFY(c.copySelection());QVERIFY(c.selectLine(true,1));QVERIFY(c.pasteSelection(e));QCOMPARE(c.document()->cell(2,3),std::string("2"));QVERIFY(c.selectLine(true,0));QVERIFY(c.editSelection(true,e));QCOMPARE(c.document()->nodes(),4);QVERIFY(c.undo(e));QCOMPARE(c.document()->nodes(),3);QVERIFY(!c.canUndo());
  c.dialogs().askSave=[] {return SaveChoice::Cancel;};QVERIFY(!c.leaveInput());c.dialogs().askSave=[] {return SaveChoice::Yes;};c.dialogs().chooseSave=[](const QString&)->std::optional<QString>{return {};};QVERIFY(!c.leaveInput());c.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(c.leaveInput());
 }
 void disconnectedAndNegativeCycle(){
  ApplicationShell shell;auto& c=networksController(shell);QString e;QVERIFY(c.initialize(NetworkMode::MinimalTree,"Disconnected",3,e));QVERIFY(c.solve(1,2,e));QCOMPARE(std::get<tora::networks::SpanningForest>(*c.result()).componentCount,std::size_t(3));QVERIFY(c.outputText().contains("Disconnected"));
  c.beginNew(NetworkMode::ShortestRoute);QVERIFY(c.initialize(NetworkMode::ShortestRoute,"Cycles",3,e));QVERIFY(c.editCell(1,2,"1",e));QVERIFY(c.editCell(2,3,"-3",e));QVERIFY(c.editCell(3,2,"1",e));QVERIFY(c.solve(0,2,e,NetworkRouteMethod::Floyd));QVERIFY(c.outputText().contains("Unbounded below"));QVERIFY(c.solve(1,2,e,NetworkRouteMethod::BellmanFord));QCOMPARE(std::get<tora::networks::ShortestRoutes>(*c.result()).routes[0].status,tora::networks::RouteStatus::Unreachable);
 }
 void nativeKeyboardTraceAndPdf(){
  ApplicationShell shell;QVERIFY(shell.registerModule(networksModule()));auto& c=networksController(shell);c.dialogs().showError=[](const QString&,const QString&){};QString e;QTemporaryDir dir;QVERIFY(load(c,NetworkMode::MinimalTree,dir,e));QVERIFY(shell.navigate("networks/input"));shell.resize(800,600);shell.show();QTest::qWait(20);auto* grid=shell.currentContent()->findChild<QTableWidget*>("networks-input-grid");QVERIFY(grid);QCOMPARE(grid->rowCount(),4);grid->item(1,2)->setText("bad");QVERIFY(!c.solve(0,2,e));QVERIFY(!c.result());QVERIFY(!shell.navigate("main"));grid->item(1,2)->setText("1");grid->setCurrentCell(1,2);grid->editItem(grid->item(1,2));QApplication::processEvents();auto* editor=grid->findChild<QLineEdit*>();QVERIFY(editor);QTest::keyClick(editor,Qt::Key_A,Qt::ControlModifier);QTest::keyClicks(editor,"1.25");QTest::keyClick(editor,Qt::Key_Return);QApplication::processEvents();QCOMPARE(c.document()->cell(1,2),std::string("1.25"));QVERIFY(c.solve(0,2,e));QVERIFY(shell.navigate("networks/result"));auto* trace=shell.currentContent()->findChild<QTableWidget*>("networks-iterations");QVERIFY(trace);QCOMPARE(trace->rowCount(),0);QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>("networks-next"),Qt::LeftButton);QCOMPARE(trace->rowCount(),1);QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>("networks-all"),Qt::LeftButton);QCOMPARE(trace->rowCount(),2);
  const auto artifact=QDir::current().filePath("desktop-networks-native-scale-"+qEnvironmentVariable("QT_SCALE_FACTOR","1"));QVERIFY(shell.grab().save(artifact+".png"));QVERIFY(c.setTitle(QString(3000,'W'),e));QVERIFY(c.solve(0,2,e));QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);printer.setOutputFileName(artifact+".pdf");
  // Keep semantic PDF assertions independent of host programming-font spacing.
  const auto themeFont=QApplication::font();
  QFont printFont(QStringLiteral("sans-serif"),10);printFont.setStyleHint(QFont::SansSerif);
  QApplication::setFont(printFont);
  const bool printed=printNetworks(printer,c);
  QApplication::setFont(themeFont);
  QCOMPARE(QApplication::font(),themeFont);QVERIFY(printed);QFile pdf(artifact+".pdf");QVERIFY(pdf.open(QIODevice::ReadOnly));QVERIFY(pdf.readAll().startsWith("%PDF"));QString text,pdfError;QVERIFY2(tora::test::extractPdfText(artifact+".pdf",text,pdfError),qPrintable(pdfError));QVERIFY(text.contains("Total length = 3.25"));QVERIFY(text.contains("Calculation steps"));
 }
};
QTEST_MAIN(DesktopNetworks)
#include "desktop_networks.moc"
