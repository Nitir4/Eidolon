#include "pdf_text.hpp"
#include "integer_controller.hpp"
#include "ui_font.hpp"
#include <QApplication>
#include <cmath>
#include <QDir>
#include <QComboBox>
#include <QCheckBox>
#include <QTreeWidget>
#include <QFile>
#include <QLineEdit>
#include <QLabel>
#include <QPrinter>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QTableView>
#include <QTableWidget>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTest>
using namespace tora::desktop;
namespace {
// The application supplies F2 consistently on every platform.
constexpr auto tableEditKey=Qt::Key_F2;
}
class DesktopInteger:public QObject {
 Q_OBJECT
private slots:
 void initTestCase(){QApplication::setFont(readableUiFont(QApplication::font()));}
 void pureMixedLimitsAndTrace(){ApplicationShell shell;QVERIFY(shell.registerModule(integerModule()));auto& c=integerController(shell);c.dialogs().showError=[](const QString&,const QString&){};c.dialogs().askSave=[]{return SaveChoice::No;};QVERIFY(c.loadExample());QVERIFY(c.solve());QCOMPARE(c.result()->status,tora::integer::Status::Optimal);QCOMPARE(*c.result()->objective,6.);QVERIFY(c.result()->nodes.size()>1);auto* nodes=shell.currentContent()->findChild<QTableWidget*>("integer-nodes");QVERIFY(nodes);QCOMPARE(nodes->rowCount(),int(c.result()->nodes.size()));QVERIFY(c.setFlag(1,false));QVERIFY(!c.result());QVERIFY(c.solve());QCOMPARE(*c.result()->objective,7.);QVERIFY(c.setFlag(1,true));QVERIFY(c.solve(1));QCOMPARE(c.result()->status,tora::integer::Status::NodeLimit);QVERIFY(!c.result()->objective);}
 void guidedBranchingAndCompletion(){ApplicationShell shell;QVERIFY(shell.registerModule(integerModule()));auto& c=integerController(shell);c.dialogs().showError=[](const QString&,const QString&){};c.dialogs().askSave=[]{return SaveChoice::No;};QVERIFY(c.loadExample());QVERIFY(shell.navigate("integer/input"));QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>("integer-guided"),Qt::LeftButton);QVERIFY(c.guided());QVERIFY(c.guided()->awaitingBranch()==0);auto* variables=shell.currentContent()->findChild<QComboBox*>("integer-branch-variable");QVERIFY(variables);QCOMPARE(variables->count(),2);variables->setCurrentIndex(1);QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>("integer-branch"),Qt::LeftButton);QCOMPARE(c.result()->nodes[0].branchVariable,std::optional<std::size_t>(1));QVERIFY(!c.guided()->awaitingBranch());auto* pending=shell.currentContent()->findChild<QComboBox*>("integer-pending");QCOMPARE(pending->count(),2);pending->setCurrentIndex(0);QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>("integer-next"),Qt::LeftButton);QVERIFY(c.result()->nodes.size()==2);auto* pruning=shell.currentContent()->findChild<QCheckBox*>("integer-bound-pruning");pruning->setChecked(false);QVERIFY(!c.guided()->boundPruning());QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>("integer-complete"),Qt::LeftButton);QVERIFY(c.guided()->finished());QCOMPARE(c.result()->status,tora::integer::Status::Optimal);QCOMPARE(*c.result()->objective,6.);auto* tree=shell.currentContent()->findChild<QTreeWidget*>("integer-search-tree");QVERIFY(tree);QCOMPARE(tree->topLevelItemCount(),1);QVERIFY(tree->topLevelItem(0)->childCount()>0);QVERIFY(c.setFlag(1,false));QVERIFY(!c.guided());QVERIFY(!c.result());}
 void guidedPendingTreeSelection(){
  ApplicationShell shell;QVERIFY(shell.registerModule(integerModule()));auto& c=integerController(shell);
  c.dialogs().showError=[](const QString&,const QString&){};c.dialogs().askSave=[]{return SaveChoice::No;};
  QVERIFY(c.loadExample());QVERIFY(c.startGuided());QVERIFY(c.branchGuided(1));
  auto* tree=shell.currentContent()->findChild<QTreeWidget*>("integer-search-tree");
  auto* pending=shell.currentContent()->findChild<QComboBox*>("integer-pending");
  QVERIFY(tree);QVERIFY(pending);QCOMPARE(tree->topLevelItemCount(),1);
  auto* tabs=shell.currentContent()->findChild<QTabWidget*>("integer-result-tabs");QVERIFY(tabs);tabs->setCurrentIndex(1);
  const auto capture=[&](const QString& name){const auto destination=qEnvironmentVariable("TORA_UI_CAPTURES");if(destination.isEmpty())return;QVERIFY(QDir().mkpath(destination));double scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();if(scale<=0)scale=1;shell.resize(qRound(1366/scale),qRound(768/scale));shell.show();QApplication::processEvents();QVERIFY(shell.grab().save(destination+"/"+name+"-"+qEnvironmentVariable("QT_SCALE_FACTOR","1")+".png"));QVERIFY(tree->grab().save(destination+"/"+name+"-widget-"+qEnvironmentVariable("QT_SCALE_FACTOR","1")+".png"));};
  auto* root=tree->topLevelItem(0);QCOMPARE(root->childCount(),2);
  auto* high=root->child(0);QVERIFY(high->text(0).contains("x2 >= 2.00"));QCOMPARE(high->text(1),QString("Pending LP relaxation"));
  tree->setCurrentItem(high);QCOMPARE(pending->currentIndex(),0);QCOMPARE(c.result()->nodes.size(),std::size_t(1));
  auto* detail=shell.currentContent()->findChild<QLabel*>("integer-node-detail");QVERIFY(detail);QVERIFY(detail->text().contains("x2 >= 2.00"));
  capture("integer-pending-tree");
  QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>("integer-next"),Qt::LeftButton);
  QCOMPARE(c.result()->nodes.size(),std::size_t(2));QCOMPARE(c.result()->nodes[1].incoming->relation,tora::lp::Relation::GreaterEqual);QCOMPARE(*c.result()->nodes[1].bound,5.5);
  QVERIFY(c.branchGuided(0));
  root=tree->topLevelItem(0);QCOMPARE(root->childCount(),2);
  tree->setCurrentItem(root->child(1));QCOMPARE(pending->currentIndex(),0);QVERIFY(detail->text().contains("x2 <= 1.00"));
  QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>("integer-next"),Qt::LeftButton);
  QCOMPARE(c.result()->nodes.size(),std::size_t(3));QCOMPARE(*c.result()->nodes[2].bound,7.25);QVERIFY(c.branchGuided(0));
  root=tree->topLevelItem(0);auto* low=root->child(1);QCOMPARE(low->childCount(),2);
  tree->setCurrentItem(low->child(0));QVERIFY(detail->text().contains("x2 <= 1.00; x1 >= 2.00"));
  capture("integer-nested-pending-tree");
  QVERIFY(c.finishGuided());QCOMPARE(*c.result()->objective,6.);
 }
 void guidedUnrestrictedBounds(){ApplicationShell shell;QVERIFY(shell.registerModule(integerModule()));auto& c=integerController(shell);c.dialogs().showError=[](const QString&,const QString&){};c.dialogs().askSave=[]{return SaveChoice::No;};QVERIFY(c.initialize("Free integer branch",1,2));auto p=tora::lp::LpProblem::blank(1,2);p.objective={1};p.variables[0].unrestricted=true;p.variables[0].lowerBound=-INFINITY;p.constraints={{{1},tora::lp::Relation::LessEqual,2.5},{{1},tora::lp::Relation::GreaterEqual,-2.5}};QVERIFY(c.input().loadDocument(tora::compat::ToraLpCodec::create(p)));QVERIFY(c.startGuided());QVERIFY(c.branchGuided(0));auto* pending=shell.currentContent()->findChild<QComboBox*>("integer-pending");QVERIFY(pending);QCOMPARE(pending->count(),2);QVERIFY(pending->itemText(0).contains("-infinity"));QVERIFY(c.nextGuided(0));QVERIFY(c.finishGuided());QCOMPARE(c.result()->status,tora::integer::Status::Optimal);QCOMPARE(*c.result()->objective,2.);}
 void guidedPendingPrinting(){
  QTemporaryDir dir;QVERIFY(dir.isValid());ApplicationShell shell;QVERIFY(shell.registerModule(integerModule()));
  auto& c=integerController(shell);c.dialogs().showError=[](const QString&,const QString&){};c.dialogs().askSave=[]{return SaveChoice::No;};
  QVERIFY(c.loadExample());QVERIFY(c.startGuided());
  // Theme programming-font ligatures can omit inequality operators in PDF text maps.
  // Use ordinary sans-serif text for this semantic PDF-content regression.
  QFont printFont(QStringLiteral("sans-serif"),12);printFont.setStyleHint(QFont::SansSerif);
  QString pdfError;
  const auto printedText=[&](const QString& name){
   const auto pdf=dir.filePath(name+".pdf");QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);printer.setOutputFileName(pdf);
   if(!printInteger(printer,c,printFont))return QString();
   const auto capture=qEnvironmentVariable("TORA_UI_CAPTURES");
   if(!capture.isEmpty() && (!QDir().mkpath(capture) || !QFile::copy(pdf,capture+"/"+name+"-"+qEnvironmentVariable("QT_SCALE_FACTOR","1")+".pdf")))return QString();
   QString text;
   if(!tora::test::extractPdfText(pdf,text,pdfError))return QString();
   return text.simplified();
  };
  const QString heading="Pending subproblems (LP relaxations not solved)";
  const auto before=printedText("integer-before-branch");QVERIFY2(!before.isEmpty(),qPrintable(pdfError));QVERIFY(!before.contains(heading));
  QVERIFY(before.contains("Awaiting branching choice"));
  QVERIFY(before.contains("User-guided B&B: choose a fractional integer branching variable"));
  QVERIFY(!before.contains("No feasible integer solution"));
  QVERIFY(c.branchGuided(1));
  const auto split=printedText("integer-pending-branches");QVERIFY2(split.contains(heading),qPrintable(pdfError));
  QVERIFY(!split.contains("No feasible integer solution"));
  const auto pending=split.mid(split.indexOf(heading));
  QVERIFY(pending.contains("Pending 1; parent 1; depth 1"));QVERIFY(pending.contains("Pending 2; parent 1; depth 1"));
  QVERIFY(pending.contains("Defining branches: x2 >= 2.00"));QVERIFY(pending.contains("Defining branches: x2 <= 1.00"));
  QVERIFY(pending.contains("Pending LP relaxation: not solved yet"));QVERIFY(!pending.contains("LP bound ="));
  QCOMPARE(c.result()->nodes.size(),std::size_t(1));QCOMPARE(c.guided()->pending().size(),std::size_t(2));
  // Choose the lower x2 child, then split its fractional x1 value.
  QVERIFY(c.nextGuided(1));QVERIFY(c.branchGuided(0));
  const auto nested=printedText("integer-pending-nested");QVERIFY2(nested.contains(heading),qPrintable(pdfError));
  QVERIFY(nested.contains("parent 2; depth 2"));
  QVERIFY(nested.contains("Defining branches: x2 <= 1.00; x1 >= 2.00"));
  QVERIFY(nested.contains("Defining branches: x2 <= 1.00; x1 <= 1.00"));
  QCOMPARE(c.result()->nodes.size(),std::size_t(2));
  QVERIFY(c.finishGuided());
  const auto complete=printedText("integer-pending-completed");QVERIFY2(complete.contains("Integer optimum certified"),qPrintable(pdfError));
  QVERIFY(!complete.contains(heading));QVERIFY(!complete.contains("Pending LP relaxation: not solved yet"));
 }
 void original5000FilesAndGuards(){QTemporaryDir dir;ApplicationShell shell;QVERIFY(shell.registerModule(integerModule()));auto& c=integerController(shell);c.dialogs().showError=[](const QString&,const QString&){};c.dialogs().askSave=[]{return SaveChoice::No;};QVERIFY(c.initialize("Blank original file",2,2));const auto blank=dir.filePath("blank.txt");QVERIFY(c.saveTo(blank));QVERIFY(!c.dirty());QVERIFY(c.loadExample());QVERIFY(c.setFlag(1,false));QVERIFY(c.setFormat({tora::core::Notation::Scientific,7,3}));const auto file=dir.filePath("integer.txt");QVERIFY(c.saveTo(file));const auto before=c.encode();QVERIFY(c.openFile(file,true));QVERIFY(c.readOnly());QCOMPARE(c.encode(),before);QVERIFY(!c.setFlag(0,false));QVERIFY(!c.saveTo(file));QVERIFY(c.saveTo(dir.filePath("copy.txt")));QVERIFY(!c.readOnly());QVERIFY(c.solve());QCOMPARE(*c.result()->objective,7.);QVERIFY(!c.openFile(dir.filePath("missing")));QCOMPARE(c.encode(),before);QVERIFY(c.result());QVERIFY(c.setFlag(0,false));c.dialogs().askSave=[]{return SaveChoice::Cancel;};QVERIFY(!c.loadExample());QVERIFY(!shell.navigate("main"));QVERIFY(!c.flags()[0]);}
 void isolationReadOnlyAndActiveSave(){
  QTemporaryDir dir;ApplicationShell shell;QVERIFY(shell.registerModule(integerModule()));auto& c=integerController(shell);
  c.dialogs().showError=[](const QString&,const QString&){};c.dialogs().askSave=[]{return SaveChoice::No;};
  QVERIFY(c.loadExample());auto& lp=inputController(shell);lp.dialogs().showError=[](const QString&,const QString&){};
  QVERIFY(&lp!=&c.input());QVERIFY(!lp.document());QVERIFY(lp.initialize("Separate LP",1,1));
  QCOMPARE(c.input().document()->problem.variables.size(),std::size_t(2));QCOMPARE(lp.document()->problem.variables.size(),std::size_t(1));
  QVERIFY(shell.navigate("integer/input"));shell.show();
  // Native window exposure and keyboard activation arrive asynchronously on Wayland.
  // Establish actual editing focus before injecting keys into the delegate.
  QVERIFY(QTest::qWaitForWindowExposed(&shell));QVERIFY(QTest::qWaitForWindowActive(&shell));
  auto* grid=shell.currentContent()->findChild<QTableView*>("integer-grid");QVERIFY(grid);
  grid->setCurrentIndex(c.input().grid()->index(1,1));QTest::keyClick(grid,tableEditKey);QApplication::processEvents();
  auto* editor=grid->findChild<QLineEdit*>();QVERIFY(editor);QTRY_VERIFY(editor->hasFocus());
  QTest::keyClick(editor,Qt::Key_A,Qt::ControlModifier);QTest::keyClicks(editor,"4");QCOMPARE(editor->text(),QString("4"));
  QCOMPARE(c.input().document()->problem.objective[0],3.); // Still pending; no Return key was sent.
  const auto file=dir.filePath("active.txt");QVERIFY(c.saveTo(file));QCOMPARE(c.input().document()->problem.objective[0],4.);
  QVERIFY(c.openFile(file,true));QCOMPARE(c.input().document()->problem.objective[0],4.);
  QVERIFY(!(c.input().grid()->flags(c.input().grid()->index(1,1))&Qt::ItemIsEditable));
  QVERIFY(!c.input().grid()->setData(c.input().grid()->index(1,1),"9"));QVERIFY(!c.input().setTitle("Blocked"));
  QVERIFY(c.saveTo(dir.filePath("editable-copy.txt")));QVERIFY(c.input().grid()->flags(c.input().grid()->index(1,1))&Qt::ItemIsEditable);
  QVERIFY(c.input().grid()->setData(c.input().grid()->index(1,1),"5"));QCOMPARE(lp.document()->problem.objective[0],0.);
 }

 void gridStructureFlagsAndUndo(){ApplicationShell shell;QVERIFY(shell.registerModule(integerModule()));auto& c=integerController(shell);c.dialogs().showError=[](const QString&,const QString&){};c.dialogs().askSave=[]{return SaveChoice::No;};QVERIFY(c.loadExample());QVERIFY(c.setFlag(1,false));const auto initial=c.encode();c.selectColumn(1);QVERIFY(c.insertSelection());QCOMPARE(c.flags(),std::vector<bool>({true,true,false}));QVERIFY(c.undo());QCOMPARE(c.encode(),initial);c.selectColumn(2);QVERIFY(c.copySelection());c.selectColumn(1);QVERIFY(c.pasteSelection());QCOMPARE(c.flags(),std::vector<bool>({false,false}));QVERIFY(c.undo());QCOMPARE(c.encode(),initial);c.selectColumn(1);QVERIFY(c.insertSelection(true));c.selectColumn(2);QVERIFY(c.deleteSelection());QCOMPARE(c.flags(),std::vector<bool>({true,false}));QVERIFY(c.undo());QCOMPARE(c.flags(),std::vector<bool>({true,true,false}));}
 void keyboardPrintingAndSizes(){QTemporaryDir dir;ApplicationShell shell;QVERIFY(shell.registerModule(integerModule()));auto& c=integerController(shell);c.dialogs().showError=[](const QString&,const QString&){};c.dialogs().askSave=[]{return SaveChoice::No;};QVERIFY(c.loadExample());QVERIFY(shell.navigate("integer/input"));shell.show();QTest::qWait(10);auto* grid=shell.currentContent()->findChild<QTableView*>("integer-grid");QVERIFY(grid);grid->setCurrentIndex(c.input().grid()->index(1,1));QTest::keyClick(grid,tableEditKey);QApplication::processEvents();auto* editor=grid->findChild<QLineEdit*>();QVERIFY(editor);QTest::keyClick(editor,Qt::Key_A,Qt::ControlModifier);QTest::keyClicks(editor,"4");QTest::keyClick(editor,Qt::Key_Return);QApplication::processEvents();QCOMPARE(c.input().document()->problem.objective[0],4.);QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>("integer-solve"),Qt::LeftButton);QVERIFY(c.result());QCOMPARE(*c.result()->objective,8.);QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);const auto pdf=dir.filePath("integer.pdf");printer.setOutputFileName(pdf);QVERIFY(printInteger(printer,c,shell.font()));QFile f(pdf);QVERIFY(f.open(QIODevice::ReadOnly));QCOMPARE(f.read(5),QByteArray("%PDF-"));const auto extractor=QStandardPaths::findExecutable("pdftotext");if(!extractor.isEmpty()){QProcess process;process.start(extractor,{pdf,"-"});QVERIFY(process.waitForFinished());QCOMPARE(process.exitCode(),0);const auto text=QString::fromUtf8(process.readAllStandardOutput()).normalized(QString::NormalizationForm_KC);QVERIFY(text.contains("Integer optimum certified"));QVERIFY(text.contains("incumbent objective = 8.00"));QVERIFY(text.contains("Branch and bound trace"));}const auto root=qEnvironmentVariable("TORA_UI_CAPTURES");if(!root.isEmpty()){QVERIFY(QDir().mkpath(root));QVERIFY(QFile::copy(pdf,root+"/integer-"+qEnvironmentVariable("QT_SCALE_FACTOR")+".pdf"));}double scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();if(scale<=0)scale=1;for(const auto& route:{QStringLiteral("integer/input"),QStringLiteral("integer/result")}){QVERIFY(shell.navigate(route));for(auto size:{QSize(1366,768),QSize(1920,1080)}){shell.resize(QSize(qRound(size.width()/scale),qRound(size.height()/scale)));QApplication::processEvents();for(auto* b:shell.currentContent()->findChildren<QPushButton*>())QVERIFY(b->width()>=b->minimumSizeHint().width());if(!root.isEmpty())QVERIFY(shell.grab().save(root+"/"+(route.endsWith("input")?"integer-input-":"integer-result-")+QString::number(size.width())+"-"+qEnvironmentVariable("QT_SCALE_FACTOR")+".png"));}}}
};
int main(int argc,char** argv){QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);QApplication app(argc,argv);DesktopInteger test;return QTest::qExec(&test,argc,argv);}
#include "desktop_integer.moc"
