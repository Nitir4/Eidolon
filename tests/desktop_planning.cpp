#include "pdf_text.hpp"
#include "planning_screens.hpp"
#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QPrinter>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QTabWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QDir>
#include <QLineEdit>
#include <QApplication>
#include <QScrollArea>
#include <QScrollBar>
using namespace tora::desktop;
class DesktopPlanning:public QObject {
Q_OBJECT
private slots:
 void originalFileCompatibility(){
 const std::string cpm="8001\r\n5\r\n2\r\n1\r\n\"CPM compatibility\"\r\n7\r\n4\r\n\"1\"\r\n\"3\"\r\n\"A\"\r\n\"4.00\"\r\n\"3\"\r\n\"7\"\r\n\"B\"\r\n\"6.00\"\r\n\"0\"\r\n\"\"\r\n\"\"\r\n\"\"\r\n";
 auto doc=PlanningDocument::decode(cpm);QCOMPARE(doc.encode(),cpm);QCOMPARE(doc.problem().activities.size(),std::size_t(2));QCOMPARE(tora::planning::solve(doc.problem()).duration,10.);
 const std::string pert="8002\r\n5\r\n2\r\n1\r\n\"PERT compatibility\"\r\n7\r\n4\r\n\"1\"\r\n\"3\"\r\n\"A\"\r\n\"1.00\"\r\n\"3.00\"\r\n\"5.00\"\r\n\"3\"\r\n\"7\"\r\n\"B\"\r\n\"2.00\"\r\n\"4.00\"\r\n\"6.00\"\r\n\"0\"\r\n\"\"\r\n\"\"\r\n\"\"\r\n\"\"\r\n\"\"\r\n";
 doc=PlanningDocument::decode(pert);QCOMPARE(doc.encode(),pert);QCOMPARE(tora::planning::solve(doc.problem()).duration,7.);auto model=doc.problem();model.activities[0].mostLikely=3.123456789123;model.activities[0].symbol="A \"quoted\" \\";doc.update("Unicode Ω \"quoted\"",model,{tora::core::Notation::Scientific,4,3});auto round=PlanningDocument::decode(doc.encode());QCOMPARE(round.problem().activities[0].mostLikely,3.123456789123);QCOMPARE(round.problem().activities[0].symbol,model.activities[0].symbol);QCOMPARE(round.title(),doc.title());QCOMPARE(round.format().notation,tora::core::Notation::Scientific);QVERIFY_THROWS_EXCEPTION(std::invalid_argument,PlanningDocument::decode(pert+"unexpected\r\n"));auto truncated=pert.substr(0,pert.size()-4);QVERIFY_THROWS_EXCEPTION(std::invalid_argument,PlanningDocument::decode(truncated));QVERIFY_THROWS_EXCEPTION(std::invalid_argument,PlanningDocument::decode("8001\r\n5\r\n2\r\n1\r\n\"x\"\r\n2\r\n1000000\r\n"));
 }

 void filesSolveAndAtomicFailure(){ApplicationShell shell;auto& c=planningController(shell);QString e;tora::planning::Problem p;p.activities={{1,2,"A",3},{2,3,"B",5},{1,3,"C",2}};QVERIFY(c.setProblem("Plan",p,e));QVERIFY(c.solve(e));QCOMPARE(c.result()->duration,8.);QVERIFY(c.outputText().contains("CRITICAL"));QTemporaryDir dir;auto path=dir.filePath("planning.json");QVERIFY(c.saveTo(path,e));QVERIFY(!c.dirty());QVERIFY(c.openFile(path,e,true));QVERIFY(c.readOnly());QVERIFY(!c.setProblem("Other",p,e));QVERIFY(!c.saveTo(path,e));QVERIFY(c.saveTo(dir.filePath("copy.json"),e));QVERIFY(!c.readOnly());QVERIFY(c.solve(e));QVERIFY(!c.openFile(dir.filePath("absent"),e));QCOMPARE(c.result()->duration,8.);QFile invalid(dir.filePath("invalid.json"));QVERIFY(invalid.open(QIODevice::WriteOnly));invalid.write("{\"format\":\"tora-clone-planning-v1\",\"method\":\"CPM\",\"title\":\"\",\"activities\":[{\"from\":1.5}]} ");invalid.close();QVERIFY(!c.openFile(invalid.fileName(),e));QCOMPARE(c.result()->duration,8.);p.activities[0].duration=4;QVERIFY(c.setProblem("Plan",p,e));QVERIFY(!c.result());QVERIFY(c.solve(e));QCOMPARE(c.result()->duration,9.);}
 void editingFormattingAndCancellation(){ApplicationShell shell;QVERIFY(shell.registerModule(planningModule()));auto& c=planningController(shell);c.dialogs().showError=[](const QString&,const QString&){};c.dialogs().askSave=[]{return SaveChoice::Cancel;};tora::planning::Problem p;p.activities={{1,2,"A",3},{2,3,"B",5}};QString e;QVERIFY(c.setProblem("Native",p,e));QVERIFY(shell.navigate("planning/input"));shell.resize(800,600);shell.show();QTest::qWait(20);auto* grid=shell.currentContent()->findChild<QTableWidget*>("planning-input-grid");QVERIFY(grid);grid->setCurrentCell(0,0);emit c.gridEditRequested(0);grid->setCurrentCell(1,0);emit c.gridEditRequested(1);QCOMPARE(grid->item(1,2)->text(),QString("A"));emit c.gridEditRequested(4);QCOMPARE(grid->item(1,2)->text(),QString("B"));emit c.gridEditRequested(2);QCOMPARE(grid->rowCount(),3);emit c.gridEditRequested(4);QCOMPARE(grid->rowCount(),2);auto* notation=shell.currentContent()->findChild<QComboBox*>("planning-notation");auto* n=shell.currentContent()->findChild<QSpinBox*>("planning-format-n");auto* d=shell.currentContent()->findChild<QSpinBox*>("planning-format-d");notation->setCurrentIndex(1);n->setValue(3);d->setValue(2);QVERIFY(c.solve(e));QVERIFY(c.outputText().contains("Project duration = 8.000e+00"));QVERIFY(!shell.navigate("main"));QCOMPARE(shell.currentScreen(),QString("planning/input"));QCOMPARE(c.result()->duration,8.);c.dialogs().askSave=[]{return SaveChoice::No;};QVERIFY(shell.navigate("main"));}
 void nativeInputValidationAndPdf(){ApplicationShell shell;QVERIFY(shell.registerModule(planningModule()));auto& c=planningController(shell);c.dialogs().showError=[](const QString&,const QString&){};c.dialogs().askSave=[] {return SaveChoice::No;};QString e;tora::planning::Problem p;p.method=tora::planning::Method::Pert;p.activities={{1,2,"A",0,1,3,5}};QVERIFY(c.setProblem("Native",p,e));QVERIFY(shell.navigate("planning/input"));shell.resize(800,600);shell.show();QTest::qWait(20);auto* grid=shell.currentContent()->findChild<QTableWidget*>("planning-input-grid");QVERIFY(grid);QCOMPARE(grid->rowCount(),1);grid->item(0,4)->setText("bad");QVERIFY(!c.solve(e));QVERIFY(!c.result());QCOMPARE(c.problem().activities[0].optimistic,1.);grid->item(0,4)->setText("1");QVERIFY(c.solve(e));QCOMPARE(c.result()->duration,3.);QVERIFY(shell.navigate("planning/result"));auto* graph=shell.currentContent()->findChild<QGraphicsView*>("planning-network");QVERIFY(graph);QVERIFY(!graph->scene()->items().isEmpty());auto* tabs=shell.currentContent()->findChild<QTabWidget*>("planning-output-tabs");QVERIFY(tabs);tabs->setCurrentIndex(1);QTest::qWait(20);const QString artifactDir=QCoreApplication::applicationDirPath()+"/planning-native-scale-"+qEnvironmentVariable("QT_SCALE_FACTOR","1");QVERIFY(QDir().mkpath(artifactDir));shell.grab().save(artifactDir+"/network.png");tabs->setCurrentIndex(0);shell.grab().save(artifactDir+"/calculations.png");QTemporaryDir dir;QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);auto path=artifactDir+"/planning.pdf";printer.setOutputFileName(path);
  // Keep semantic PDF assertions independent of host programming-font spacing.
  const auto themeFont=QApplication::font();
  QFont printFont(QStringLiteral("sans-serif"),10);printFont.setStyleHint(QFont::SansSerif);
  QApplication::setFont(printFont);
  const bool printed=printPlanning(printer,c);
  QApplication::setFont(themeFont);
  QCOMPARE(QApplication::font(),themeFont);QVERIFY(printed);QFile pdf(path);QVERIFY(pdf.open(QIODevice::ReadOnly));QVERIFY(pdf.readAll().startsWith("%PDF"));QString text,pdfError;QVERIFY2(tora::test::extractPdfText(path,text,pdfError),qPrintable(pdfError));QVERIFY(text.contains("Project duration = 3.00"));QVERIFY(text.contains("Forward event 2"));QVERIFY(text.contains("Path standard deviation"));QVERIFY(shell.navigate("planning/input"));grid=shell.currentContent()->findChild<QTableWidget*>("planning-input-grid");auto* add=shell.currentContent()->findChild<QPushButton*>("planning-add");QTest::mouseClick(add,Qt::LeftButton);QCOMPARE(grid->rowCount(),2);grid->item(1,0)->setText("2");grid->item(1,1)->setText("1");QVERIFY(!c.solve(e));QVERIFY(e.contains("cycle"));}
 void scheduleChartExperimentsAndPrint(){
  ApplicationShell shell;QVERIFY(shell.registerModule(planningModule()));auto& c=planningController(shell);c.dialogs().showError=[](const QString&,const QString&){};c.dialogs().askSave=[] {return SaveChoice::No;};
  QString error;tora::planning::Problem p;p.activities={{1,2,"A",3},{2,4,"B",5},{1,3,"C",2},{3,4,"D",2}};
  QVERIFY(c.setProblem("Schedule oracle",p,error));QVERIFY(c.solve(error));QVERIFY(shell.navigate("planning/result"));shell.resize(800,600);shell.show();
  auto* tabs=shell.currentContent()->findChild<QTabWidget*>("planning-output-tabs");QVERIFY(tabs);QCOMPARE(tabs->count(),3);tabs->setCurrentIndex(2);QTest::qWait(20);
  auto* chart=tabs->findChild<QGraphicsView*>("planning-schedule");auto* activity=tabs->findChild<QComboBox*>("planning-schedule-activity");auto* shift=tabs->findChild<QDoubleSpinBox*>("planning-schedule-shift");auto* apply=tabs->findChild<QPushButton*>("planning-schedule-apply");auto* reset=tabs->findChild<QPushButton*>("planning-schedule-reset");QVERIFY(chart&&activity&&shift&&apply&&reset);QVERIFY(!chart->scene()->items().isEmpty());
  activity->setCurrentIndex(2);QCOMPARE(shift->minimum(),0.);QCOMPARE(shift->maximum(),4.);shift->setValue(3);QTest::mouseClick(apply,Qt::LeftButton);
  QCOMPARE(c.schedule()->starts[2],3.);QCOMPARE(c.schedule()->starts[3],5.);QCOMPARE(c.schedule()->completion,8.);QCOMPARE(shift->minimum(),-3.);QCOMPARE(shift->maximum(),1.);
  QVERIFY(!c.shiftActivity(2,2,error));QCOMPARE(c.schedule()->starts[2],3.);QVERIFY(!c.shiftActivity(3,-3,error));QVERIFY(error.contains("predecessor"));QCOMPARE(c.schedule()->starts[3],5.);
  const auto scale=chart->transform().m11();QTest::mouseClick(tabs->findChild<QPushButton*>("planning-schedule-zoom-in"),Qt::LeftButton);QVERIFY(chart->transform().m11()>scale);QTest::mouseClick(tabs->findChild<QPushButton*>("planning-schedule-zoom-out"),Qt::LeftButton);QVERIFY(qAbs(chart->transform().m11()-scale)<1e-8);
  const auto artifactDir=QCoreApplication::applicationDirPath()+"/planning-native-scale-"+qEnvironmentVariable("QT_SCALE_FACTOR","1");QVERIFY(QDir().mkpath(artifactDir));QVERIFY(shell.grab().save(artifactDir+"/schedule.png"));
  QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);const auto path=artifactDir+"/schedule.pdf";printer.setOutputFileName(path);QVERIFY(printPlanningSchedule(printer,c));QString text,pdfError;QVERIFY2(tora::test::extractPdfText(path,text,pdfError),qPrintable(pdfError));QVERIFY(text.contains("Schedule oracle"));QVERIFY(text.contains("critical"));
  QTest::mouseClick(reset,Qt::LeftButton);QCOMPARE(c.schedule()->starts[2],0.);QCOMPARE(c.schedule()->starts[3],2.);QCOMPARE(c.result()->duration,8.);QCOMPARE(c.problem().activities[2].duration,2.);
 }

 void chartFitsCompactTileWithLargeText(){
  struct RestoreFont {
   QFont old = QApplication::font();
   ~RestoreFont(){QApplication::setFont(old);}
  } restore;
  auto font = restore.old;
  font.setPixelSize(30);
  QApplication::setFont(font);
  ApplicationShell shell;
  QVERIFY(shell.registerModule(planningModule()));
  auto& controller = planningController(shell);
  tora::planning::Problem project;
  project.activities = {{1,2,"A",3},{2,4,"B",5},{1,3,"C",2},{3,4,"D",2}};
  QString error;
  QVERIFY(controller.setProblem("Compact CPM chart",project,error));
  QVERIFY(controller.solve(error));
  QVERIFY(shell.navigate("planning/result"));
  shell.resize(683,384);
  shell.show();
  auto* content = shell.currentContent();
  auto* tabs = content->findChild<QTabWidget*>("planning-output-tabs");
  QVERIFY(tabs);
  tabs->setCurrentIndex(2);
  auto* scroll = qobject_cast<QScrollArea*>(content->parentWidget()->parentWidget());
  QVERIFY(scroll);
  QTRY_COMPARE(scroll->horizontalScrollBar()->maximum(),0);
  QCOMPARE(shell.width(),683);
 }

 void cachedOutputSwitchesPlanningMethod(){
  ApplicationShell shell;QVERIFY(shell.registerModule(planningModule()));auto& c=planningController(shell);c.dialogs().askSave=[] {return SaveChoice::No;};QString error;
  tora::planning::Problem pert;pert.method=tora::planning::Method::Pert;pert.activities={{1,2,"PERT A",0,1,3,5}};QVERIFY(c.setProblem("PERT",pert,error));QVERIFY(c.solve(error));QVERIFY(shell.navigate("planning/result"));shell.show();QTest::qWait(20);
  auto* page=shell.currentContent();auto* tabs=page->findChild<QTabWidget*>("planning-output-tabs");QVERIFY(tabs);QVERIFY(!tabs->isTabVisible(2));QVERIFY(page->findChild<QDoubleSpinBox*>("planning-deadline")->isVisible());
  tora::planning::Problem cpm;cpm.activities={{1,2,"CPM B",4}};QVERIFY(c.setProblem("CPM",cpm,error));QVERIFY(c.solve(error));QVERIFY(shell.navigate("planning/result"));QCOMPARE(shell.currentContent(),page);QVERIFY(tabs->isTabVisible(2));QVERIFY(!page->findChild<QDoubleSpinBox*>("planning-deadline")->isVisible());QVERIFY(tabs->findChild<QComboBox*>("planning-schedule-activity")->itemText(0).contains("CPM B"));
  pert.activities = {{1,2,"New PERT project",0,19,20,21}};
  QVERIFY(c.setProblem("Different PERT project",pert,error));QVERIFY(c.solve(error));
  QVERIFY(!tabs->isTabVisible(2));
  auto* deadline = page->findChild<QDoubleSpinBox*>("planning-deadline");
  auto* probability = page->findChild<QLabel*>("planning-probability");
  QVERIFY(deadline && deadline->isVisible() && probability);
  QCOMPARE(deadline->value(),20.);
  QCOMPARE(probability->text(),QStringLiteral("0.5"));
  deadline->setValue(21.);
  QVERIFY(c.setFormat({tora::core::Notation::Decimal,5,3},error));
  QCOMPARE(deadline->value(),21.);
 }

};
QTEST_MAIN(DesktopPlanning)
#include "desktop_planning.moc"
