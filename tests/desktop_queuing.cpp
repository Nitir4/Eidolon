#include "queuing_screens.hpp"
#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QPrinter>
#include <QTableWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QAction>
#include <QHeaderView>
#include <QApplication>
#include <QDir>
#include <cmath>
using namespace tora::desktop;
class DesktopQueuing:public QObject {
 Q_OBJECT
private slots:
 void quotedTitles(){auto d=QueuingDocument::blank("A \"quoted\" title \\ path");const auto bytes=d.encode();QVERIFY(bytes.find("\"\"quoted\"\"")!=std::string::npos);auto next=QueuingDocument::decode(bytes);QCOMPARE(next.title(),d.title());QCOMPARE(next.encode(),bytes);next.setTitle("Changed \"title\"");QCOMPARE(QueuingDocument::decode(next.encode()).title(),next.title());}
 void originalRecords(){const std::string bytes="6000\r\n5\r\n2\r\n1\r\n\"\"\r\n1\r\n\"2.00\"\r\n\"3.00\"\r\n\"1\"\r\n\"3\"\r\n\"3\"\r\n";auto d=QueuingDocument::decode(bytes);QCOMPARE(d.encode(),bytes);QCOMPARE(d.model(0).arrivalRate,2.);QCOMPARE(*d.model(0).sourceLimit,std::size_t(3));d.setCell(0,0,"4.25");auto next=QueuingDocument::decode(d.encode());QCOMPARE(next.model(0).arrivalRate,4.25);QVERIFY_EXCEPTION_THROWN(next.setCell(0,0,"nan"),std::invalid_argument);QCOMPARE(next.model(0).arrivalRate,4.25);QVERIFY_EXCEPTION_THROWN(QueuingDocument::decode(bytes+"junk"),std::invalid_argument);next.setCell(0,3,"infinity");QVERIFY_EXCEPTION_THROWN(next.model(0),std::invalid_argument);QVERIFY_EXCEPTION_THROWN(QueuingDocument::decode("6000\n5\n2\n1\n\"\"\n2147483647\n"),std::invalid_argument);}
 void scenarioDocumentEdits(){auto d=QueuingDocument::blank();d.setCell(0,0,"2.00");const auto initial=d.encode();d.insertScenario(0);QCOMPARE(d.cell(1,0),std::string("2.00"));QCOMPARE(d.cell(0,0),std::string("0"));d.pasteScenario(0,d.scenario(1));QCOMPARE(d.cell(0,0),std::string("2.00"));auto invalid=d.scenario(0);invalid[0]="4";invalid[4]="invalid";const auto before=d.encode();QVERIFY_EXCEPTION_THROWN(d.pasteScenario(0,invalid),std::invalid_argument);QCOMPARE(d.encode(),before);d.deleteScenario(0);QCOMPARE(d.encode(),initial);while(d.count()<100)d.addScenario();QVERIFY_EXCEPTION_THROWN(d.insertScenario(0),std::invalid_argument);QCOMPARE(d.count(),std::size_t(100));QVERIFY_EXCEPTION_THROWN(d.insertScenario(101),std::invalid_argument);}
 void scenarioControllerEdits(){ApplicationShell shell;auto& c=queuingController(shell);QString e;QVERIFY(c.initialize("Grid",e));QVERIFY(!c.selectScenario(-1));QVERIFY(!c.selectScenario(1));QVERIFY(!c.copySelection());QVERIFY(c.editCell(0,0,"2.00",e));QVERIFY(c.editCell(0,1,"3.00",e));QVERIFY(c.selectScenario(0));QVERIFY(c.copySelection());QVERIFY(c.editSelection(3,e));QCOMPARE(c.document()->count(),std::size_t(2));QCOMPARE(c.document()->cell(0,0),std::string("2.00"));QCOMPARE(c.document()->cell(1,0),std::string("0"));QVERIFY(c.canUndo());QVERIFY(c.selectScenario(1));QVERIFY(c.canPaste());QVERIFY(c.editSelection(1,e));QCOMPARE(c.document()->cell(1,0),std::string("2.00"));QVERIFY(c.editSelection(0,e));QCOMPARE(c.document()->cell(1,0),std::string("0"));QVERIFY(!c.canUndo());QVERIFY(!c.editSelection(0,e));QVERIFY(c.selectScenario(0,true));QVERIFY(c.editSelection(3,e));QCOMPARE(c.document()->cell(0,0),std::string("0"));QCOMPARE(c.document()->cell(1,0),std::string("2.00"));QVERIFY(c.editSelection(0,e));QCOMPARE(c.document()->count(),std::size_t(2));QVERIFY(c.selectScenario(1));QVERIFY(c.editSelection(2,e));QCOMPARE(c.document()->count(),std::size_t(1));QVERIFY(c.editSelection(0,e));QCOMPARE(c.document()->count(),std::size_t(2));QVERIFY(c.editCell(0,0,"4",e));QVERIFY(!c.canUndo());QTemporaryDir dir;const auto path=dir.filePath("grid.txt");QVERIFY(c.saveTo(path,e));QVERIFY(c.openFile(path,e,true));QVERIFY(c.selectScenario(0));QVERIFY(c.copySelection());QVERIFY(!c.canPaste());QVERIFY(!c.editSelection(2,e));QVERIFY(!c.editSelection(3,e));QVERIFY(!c.editSelection(1,e));QCOMPARE(c.document()->count(),std::size_t(2));QVERIFY(c.saveTo(dir.filePath("copy.txt"),e));QVERIFY(c.selectScenario(1));QVERIFY(c.editSelection(1,e));QCOMPARE(c.document()->cell(1,0),std::string("4"));}
 void scenarioNativeEdits(){
  ApplicationShell shell;QVERIFY(shell.registerModule(queuingModule()));auto& c=queuingController(shell);
  c.dialogs().showError=[](const QString&,const QString&){};QString e;
  QVERIFY(c.initialize("Grid UI",e));QVERIFY(c.editCell(0,0,"2",e));QVERIFY(c.editCell(0,1,"3",e));
  QVERIFY(shell.navigate("queuing/input"));shell.resize(800,600);shell.show();
  // Window shortcuts require actual activation; a fixed delay races Wayland focus.
  QVERIFY(QTest::qWaitForWindowExposed(&shell));QVERIFY(QTest::qWaitForWindowActive(&shell));
  auto* grid=shell.currentContent()->findChild<QTableWidget*>("queuing-input-grid");QVERIFY(grid);
  auto* copy=shell.findChild<QAction*>("queuing-copy-action");auto* insert=shell.findChild<QAction*>("queuing-insert-action");auto* undo=shell.findChild<QAction*>("queuing-undo-action");
  QVERIFY(copy);QVERIFY(insert);QVERIFY(undo);QVERIFY(!copy->isEnabled());
  const auto header=[grid](int row){return QPoint(grid->verticalHeader()->width()/2,grid->verticalHeader()->sectionViewportPosition(row)+grid->verticalHeader()->sectionSize(row)/2);};
  QTest::mouseClick(grid->verticalHeader()->viewport(),Qt::LeftButton,Qt::NoModifier,header(0));QVERIFY(copy->isEnabled());
  copy->trigger();insert->trigger();QCOMPARE(grid->rowCount(),2);QCOMPARE(grid->item(1,0)->text(),QString("0"));QVERIFY(undo->isEnabled());
  undo->trigger();QCOMPARE(grid->rowCount(),1);
  QTest::mouseDClick(grid->verticalHeader()->viewport(),Qt::LeftButton,Qt::NoModifier,header(0));insert->trigger();
  QCOMPARE(grid->rowCount(),2);QCOMPARE(grid->item(0,0)->text(),QString("0"));QCOMPARE(grid->item(1,0)->text(),QString("2"));QVERIFY(c.canUndo());
  grid->item(0,0)->setText("bad");QVERIFY(c.selectScenario(0));QVERIFY(!c.editSelection(2,e));QVERIFY(!c.copySelection());
  QCOMPARE(c.document()->count(),std::size_t(2));QCOMPARE(grid->item(0,0)->text(),QString("bad"));
  grid->item(0,0)->setText("4");QVERIFY(!c.canUndo());
  QTest::mouseClick(grid->verticalHeader()->viewport(),Qt::LeftButton,Qt::NoModifier,header(1));
  QCOMPARE(grid->currentRow(),1);QVERIFY(insert->isEnabled());QVERIFY(c.hasSelection());QVERIFY(shell.isActiveWindow());
  auto* keyboardTarget=QApplication::focusWidget();QVERIFY(keyboardTarget);QVERIFY(shell.isAncestorOf(keyboardTarget));
  QSignalSpy insertion(insert,&QAction::triggered),undoing(undo,&QAction::triggered);
  QTest::keyClick(keyboardTarget,Qt::Key_I,Qt::ControlModifier);QApplication::processEvents();
  QCOMPARE(insertion.count(),1);QCOMPARE(grid->rowCount(),3);QCOMPARE(c.document()->count(),std::size_t(3));
  keyboardTarget=QApplication::focusWidget();QVERIFY(keyboardTarget);QVERIFY(shell.isAncestorOf(keyboardTarget));
  QTest::keyClick(keyboardTarget,Qt::Key_U,Qt::ControlModifier);QApplication::processEvents();
  QCOMPARE(undoing.count(),1);QCOMPARE(grid->rowCount(),2);QCOMPARE(c.document()->count(),std::size_t(2));
 }

 void baselineFiniteAndFiles(){ApplicationShell shell;auto& c=queuingController(shell);QString e;QVERIFY(c.initialize("Baseline",e));QVERIFY(c.editCell(0,0,"2",e));QVERIFY(c.editCell(0,1,"3",e));QVERIFY(c.solve(e));QCOMPARE(c.results().size(),std::size_t(1));QVERIFY(std::abs(c.results()[0].metrics->meanSystem-2)<1e-12);QVERIFY(c.outputText().contains("Ls = 2.00"));QVERIFY(c.setFormat({tora::core::Notation::Scientific,0,2},e));QVERIFY(!c.results().empty());QVERIFY(c.setFormat({},e));QVERIFY(c.addScenario(e));QVERIFY(c.editCell(1,0,"2",e));QVERIFY(c.editCell(1,1,"3",e));QVERIFY(c.editCell(1,3,"3",e));QVERIFY(c.editCell(1,4,"3",e));QVERIFY(c.solve(e));QVERIFY(std::abs(c.results()[1].probabilities[0]-9.0/67)<1e-12);QVERIFY(c.outputText(-1).contains("COMPARATIVE ANALYSIS"));QVERIFY(c.outputText(1).contains("Raw rho/c"));QVERIFY(c.outputText(1).contains("Actual server utilization"));QTemporaryDir dir;auto path=dir.filePath("queue.txt");QVERIFY(c.saveTo(path,e));QVERIFY(c.openFile(path,e,true));QVERIFY(!c.editCell(0,0,"5",e));QVERIFY(!c.saveTo(path,e));QVERIFY(c.saveTo(dir.filePath("copy.txt"),e));QVERIFY(!c.readOnly());QVERIFY(c.solve(e));QVERIFY(!c.openFile(dir.filePath("absent"),e));QCOMPARE(c.results().size(),std::size_t(2));QVERIFY(c.deleteScenario(0,e));QVERIFY(c.results().empty());QVERIFY(!c.deleteScenario(0,e));}
 void invalidModelsAndStability(){ApplicationShell shell;auto& c=queuingController(shell);QString e;QVERIFY(c.initialize("Validation",e));QVERIFY(!c.solve(e));QVERIFY(c.editCell(0,0,"2",e));QVERIFY(c.editCell(0,1,"3",e));QVERIFY(c.editCell(0,4,"3",e));QVERIFY(!c.solve(e));QVERIFY(e.contains("System Limit exceeds Source Limit"));QVERIFY(c.editCell(0,3,"3",e));QVERIFY(c.solve(e));QVERIFY(c.editCell(0,4,"infinity",e));QVERIFY(c.editCell(0,3,"infinity",e));QVERIFY(c.editCell(0,0,"4",e));QVERIFY(c.solve(e));QCOMPARE(c.results()[0].status,tora::queuing::Status::Unstable);QVERIFY(c.outputText().contains("NO STATIONARY DISTRIBUTION"));QVERIFY(c.editCell(0,0,"0",e));QVERIFY(c.solve(e));QVERIFY(c.outputText().contains("PURE DEATH TRANSIENT"));QVERIFY(c.transients()[0]);QVERIFY(std::abs(c.transients()[0]->mean-9996)<1e-9);QVERIFY(c.editCell(0,0,"2",e));QVERIFY(c.editCell(0,1,"0",e));QVERIFY(c.editCell(0,3,"3",e));QVERIFY(c.solve(e));QVERIFY(c.results()[0].metrics->absorbingNoService);QVERIFY(c.outputText().contains("PURE BIRTH TRANSIENT"));QVERIFY(c.transients()[0]);QCOMPARE(c.transients()[0]->mean,2.);}
 void capturedBoundaryProfiles(){ApplicationShell shell;auto& c=queuingController(shell);QString e;QVERIFY(c.initialize("Original transient",e));QVERIFY(c.editCell(0,0,"0",e));QVERIFY(c.editCell(0,1,"1",e));QVERIFY(c.editCell(0,3,"3",e));QVERIFY(c.editCell(0,4,"3",e));QVERIFY(c.solve(e));QVERIFY(c.outputText().contains("Mean population = 2.02"));QVERIFY(c.outputText().contains("Partial mean excluding state N = 0.92"));QVERIFY(c.setFormat({tora::core::Notation::Decimal,5,5},e));QVERIFY(c.outputText().contains("0.08030"));QVERIFY(c.outputText().contains("0.36788"));QVERIFY(c.outputText().contains("0.91970"));QVERIFY(c.editCell(0,2,"2",e));QVERIFY(c.solve(e));QVERIFY(std::abs(c.transients()[0]->mean-5.5/std::exp(1.))<1e-12);QVERIFY(c.editCell(0,0,"2",e));QVERIFY(c.editCell(0,1,"0",e));QVERIFY(c.solve(e));QVERIFY(c.outputText().contains("Mean population = 2.00000"));QVERIFY(c.outputText().contains("0.13534"));QVERIFY(c.outputText().contains("0.85712"));QVERIFY(c.outputText(-1).contains("PURE BIRTH"));}
 void originalDeathCapComparison(){
  ApplicationShell shell;QVERIFY(shell.registerModule(queuingModule()));
  auto& c=queuingController(shell);QString e;
  QVERIFY(c.initialize("Requested and original profiles",e));
  QVERIFY(c.editCell(0,0,"0",e));QVERIFY(c.editCell(0,1,"3",e));
  QVERIFY(c.editCell(0,3,"3",e));
  QVERIFY(c.setFormat({tora::core::Notation::Decimal,5,5},e));
  for(const auto initial:{499,500,501,1000,9999}){
   QVERIFY(c.editCell(0,4,QString::number(initial),e));QVERIFY(c.solve(e));
   QVERIFY(c.transients()[0]);QVERIFY(c.originalComparisons()[0]);
   const auto& requested=*c.transients()[0];const auto& legacy=*c.originalComparisons()[0];
   QCOMPARE(requested.initialPopulation,std::size_t(initial));
   QVERIFY(std::abs(requested.mean-(initial-3.))<1e-9);
   QCOMPARE(legacy.initialPopulation,std::min<std::size_t>(initial,500));
   const double partial=legacy.mean-legacy.initialPopulation*tora::queuing::probability(legacy,legacy.initialPopulation);
   QVERIFY(std::abs(partial-(legacy.initialPopulation-3.-legacy.initialPopulation*std::exp(-3.)))<1e-9);
   const auto text=c.outputText();
   QVERIFY(text.contains("REQUESTED MODEL TRANSIENT"));
   QVERIFY(text.contains("ORIGINAL TORA COMPARISON (independent calculation)"));
   QVERIFY(text.contains("REQUESTED MODEL STATE PROBABILITIES"));
   QVERIFY(text.contains("before the original probability cutoff"));
   QVERIFY(text.contains("not the full mean population"));
  }
  QVERIFY(c.outputText().contains("Mean population = 9996.00000"));
  QVERIFY(c.outputText().contains("Capped-profile true mean = 497.00000"));
  QVERIFY(c.outputText().contains("Partial mean excluding capped terminal state = 472.10647"));
  QVERIFY(shell.navigate("queuing/result"));shell.resize(1000,800);shell.show();QTest::qWait(20);
  auto* notes = shell.currentContent()->findChild<QPlainTextEdit*>("queuing-model-notes");
  auto* states = shell.currentContent()->findChild<QTableWidget*>("queuing-state-probabilities");
  auto* tabs = shell.currentContent()->findChild<QTabWidget*>("queuing-result-tabs");
  QVERIFY(notes && states && tabs);
  QVERIFY(notes->toPlainText().contains("stops summing after the first probability below 0.00001"));
  QCOMPARE(states->item(0,0)->text(),QStringLiteral("0"));
  QVERIFY2(shell.currentContent()->minimumSizeHint().width()<=600,"Queue result controls force horizontal scrolling");
  tabs->setCurrentIndex(1);
  shell.resize(683,600);QApplication::processEvents();
  auto* first=shell.currentContent()->findChild<QSpinBox*>("queuing-first-state");
  auto* count=shell.currentContent()->findChild<QSpinBox*>("queuing-state-count");
  auto* print=shell.currentContent()->findChild<QPushButton*>("queuing-print");
  QVERIFY(first);QVERIFY(count);QVERIFY(print);
  const auto* page=shell.currentContent();
  for(auto* control:{static_cast<QWidget*>(first),static_cast<QWidget*>(count),static_cast<QWidget*>(print)}){
   const auto right=control->mapTo(page,QPoint(control->width(),0)).x();
   QVERIFY(right<=page->width());
  }
  const auto capture=qEnvironmentVariable("TORA_UI_CAPTURES");
  if(!capture.isEmpty()){
   QVERIFY(QDir().mkpath(capture));const auto suffix=qEnvironmentVariable("QT_SCALE_FACTOR","1");
   QVERIFY(shell.grab().save(capture+"/queue-original-cap-"+suffix+".png"));
   QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);
   printer.setOutputFileName(capture+"/queue-original-cap-"+suffix+".pdf");
   QVERIFY(printQueuing(printer,c,0,9990,10));
  }
  QVERIFY(c.outputText(0,9998,2).contains("9999     0.04979     1.00000"));
  QVERIFY(c.outputText(-1).contains("Effective initial population = 500"));
  QVERIFY(c.editCell(0,4,"infinity",e));QVERIFY(c.solve(e));
  QCOMPARE(c.transients()[0]->initialPopulation,std::size_t(9999));
  QCOMPARE(c.originalComparisons()[0]->initialPopulation,std::size_t(500));
  QVERIFY(c.outputText().contains("clone reference N=9999"));
  QVERIFY(!c.outputText().contains("observed original initial-population default"));
  QVERIFY(c.editCell(0,1,"1",e));QVERIFY(c.solve(e));
  QVERIFY(!c.outputText().contains("numerical discrepancy remains"));
  QVERIFY(c.outputText().contains("Partial mean excluding capped terminal state = 315.06028"));
  QVERIFY(c.editCell(0,2,"2",e));QVERIFY(c.solve(e));
  QVERIFY(!c.outputText().contains("Original displayed Ls ="));
  QVERIFY(c.editCell(0,1,"2",e));QVERIFY(c.solve(e));
  QVERIFY(!c.outputText().contains("Original displayed Ls ="));
  QVERIFY(c.editCell(0,0,"2",e));QVERIFY(c.originalComparisons().empty());
  QVERIFY(c.editCell(0,1,"0",e));QVERIFY(c.solve(e));
  QVERIFY(!c.originalComparisons()[0]);
  c.beginNew();QVERIFY(c.originalComparisons().empty());
 }
 void nativeStatesAndPdf(){ApplicationShell shell;QVERIFY(shell.registerModule(queuingModule()));auto& c=queuingController(shell);c.dialogs().showError=[](const QString&,const QString&){};QString e;QVERIFY(c.initialize("Native",e));QVERIFY(shell.navigate("queuing/input"));shell.resize(800,600);shell.show();QTest::qWait(20);auto* grid=shell.currentContent()->findChild<QTableWidget*>("queuing-input-grid");QVERIFY(grid);QCOMPARE(grid->columnCount(),5);grid->item(0,0)->setText("2");grid->item(0,1)->setText("bad");QVERIFY(!c.solve(e));grid->item(0,1)->setText("3");QVERIFY(c.solve(e));QVERIFY(shell.navigate("queuing/result"));auto* page = shell.currentContent();
  auto* states = page->findChild<QTableWidget*>("queuing-state-probabilities");
  auto* metrics = page->findChild<QTableWidget*>("queuing-result");
  auto* tabs = page->findChild<QTabWidget*>("queuing-result-tabs");
  QVERIFY(states && metrics && tabs);
  QCOMPARE(states->horizontalHeaderItem(2)->text(),QStringLiteral("Cumulative probability (Pn)"));
  QCOMPARE(states->item(0,2)->text(),QStringLiteral("0.33"));
  auto* first = page->findChild<QSpinBox*>("queuing-first-state");
  QVERIFY(first);
  first->setValue(20);
  QCOMPARE(states->item(0,0)->text(),QStringLiteral("20"));
  QVERIFY(c.addScenario(e));
  grid->item(1,0)->setText("1");
  grid->item(1,1)->setText("3");
  QVERIFY2(c.solve(e),qPrintable(e));
  auto* choices = page->findChild<QComboBox*>("queuing-scenario");
  QVERIFY(choices);
  choices->setCurrentIndex(choices->count()-1);
  QCOMPARE(metrics->columnCount(),3);
  QCOMPARE(metrics->horizontalHeaderItem(1)->text(),QStringLiteral("Scenario 1"));
  QCOMPARE(metrics->horizontalHeaderItem(2)->text(),QStringLiteral("Scenario 2"));
  QCOMPARE(metrics->item(1,2)->text(),QStringLiteral("1"));
  QVERIFY(!tabs->isTabVisible(1));
  QTemporaryDir dir;QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);auto path=dir.filePath("queue.pdf");printer.setOutputFileName(path);QVERIFY(printQueuing(printer,c,-1));QFile pdf(path);QVERIFY(pdf.open(QIODevice::ReadOnly));QVERIFY(pdf.readAll().startsWith("%PDF"));}
};
QTEST_MAIN(DesktopQueuing)
#include "desktop_queuing.moc"
