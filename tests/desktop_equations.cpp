#include "equations_screens.hpp"
#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QPrinter>
#include <QTableWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
using namespace tora::desktop;
class DesktopEquations:public QObject {
 Q_OBJECT
private slots:
 void originalMatrix1000(){const std::string bytes="1000\r\n5\r\n2\r\n1\r\n\"Inverse\"\r\n2\r\n\"4.00\"\r\n\"7.00\"\r\n\"2.00\"\r\n\"6.00\"\r\n";auto d=EquationsDocument::decode(bytes);QVERIFY(d.matrixOnly());QCOMPARE(d.encode(),bytes);QCOMPARE(d.coefficients()(0,1),7.);QVERIFY_EXCEPTION_THROWN(d.setCell(0,2,"3"),std::invalid_argument);d.setCell(0,0,"5");auto next=EquationsDocument::decode(d.encode());QVERIFY(next.matrixOnly());QCOMPARE(next.coefficients()(0,0),5.);QTemporaryDir dir;const auto path=dir.filePath("inverse.txt");QFile f(path);QVERIFY(f.open(QIODevice::WriteOnly));f.write(bytes.data(),qint64(bytes.size()));f.close();ApplicationShell shell;QVERIFY(shell.registerModule(equationsModule()));auto& c=equationsController(shell);QString e;QVERIFY(c.openFile(path,e));QVERIFY(shell.navigate("equations/input"));auto* grid=shell.currentContent()->findChild<QTableWidget*>("equations-input-grid");QCOMPARE(grid->columnCount(),2);QVERIFY(!c.solve(e));QVERIFY(c.solve(e,true));QVERIFY(c.inverse()->invertible);QVERIFY(std::abs(c.inverse()->inverse(0,0)-.6)<1e-12);QCOMPARE(*c.inverse()->determinant,10.);QVERIFY(c.outputText().contains("ORIGINAL MATRIX"));QVERIFY(c.outputText().contains("Determinant = 10.00"));}
 void quotedTitles(){auto d=EquationsDocument::blank(1,"A \"quoted\" title \\ path");const auto bytes=d.encode();QVERIFY(bytes.find("\"\"quoted\"\"")!=std::string::npos);auto next=EquationsDocument::decode(bytes);QCOMPARE(next.title(),d.title());QCOMPARE(next.encode(),bytes);next.setTitle("Changed \"title\"");QCOMPARE(EquationsDocument::decode(next.encode()).title(),next.title());}
 void originalRecords(){
  const std::string bytes="2000\r\n5\r\n2\r\n1\r\n\"Example\"\r\n2\r\n\"2\"\r\n\"1\"\r\n\"5\"\r\n\"1\"\r\n\"-1\"\r\n\"1\"\r\n";
  auto d=EquationsDocument::decode(bytes);QCOMPARE(d.encode(),bytes);QCOMPARE(d.coefficients()(1,1),-1.);QCOMPARE(d.rhs()[1],1.);d.setCell(0,0,"4.25");auto next=EquationsDocument::decode(d.encode());QCOMPARE(next.coefficients()(0,0),4.25);QVERIFY_EXCEPTION_THROWN(next.setCell(0,0,"nan"),std::invalid_argument);QCOMPARE(next.coefficients()(0,0),4.25);QVERIFY_EXCEPTION_THROWN(EquationsDocument::decode(bytes+"junk"),std::invalid_argument);QVERIFY_EXCEPTION_THROWN(EquationsDocument::decode("2000\n5\n2\n1\n\"\"\n2147483647\n"),std::invalid_argument);
 }
 void solveFilesAndInvalidation(){
  ApplicationShell shell;auto& c=equationsController(shell);QString e;QVERIFY(c.initialize("Known",2,e));const char* values[2][3]={{"2","1","5"},{"1","-1","1"}};for(int i=0;i<2;++i)for(int j=0;j<3;++j)QVERIFY(c.editCell(i,j,values[i][j],e));QVERIFY(c.solve(e));QCOMPARE(c.result()->solution[0],2.);QCOMPARE(c.result()->solution[1],1.);QVERIFY(c.outputText().contains("UNIQUE SOLUTION"));QVERIFY(c.setFormat({tora::core::Notation::Scientific,0,2},e));QVERIFY(c.result());QVERIFY(c.setFormat({},e));QVERIFY(c.solve(e,true));QVERIFY(c.inverse()->invertible);QVERIFY(c.outputText().contains("MATRIX INVERSION"));QVERIFY(c.outputText().contains("Determinant = -3.00"));QVERIFY(!c.inverse()->trace.empty());
  QTemporaryDir dir;auto path=dir.filePath("equations.txt");QVERIFY(c.saveTo(path,e));QVERIFY(c.openFile(path,e,true));QVERIFY(!c.editCell(0,0,"5",e));QVERIFY(!c.saveTo(path,e));QVERIFY(c.saveTo(dir.filePath("copy.txt"),e));QVERIFY(!c.readOnly());QVERIFY(c.solve(e));QVERIFY(!c.openFile(dir.filePath("absent"),e));QVERIFY(c.result());QVERIFY(!c.editCell(0,0,"inf",e));QVERIFY(c.result());QVERIFY(c.editCell(0,0,"5",e));QVERIFY(!c.result());QVERIFY(!c.inverse());
 }
 void originalSolutionInverseAndPdf(){
  // Reviewed original2000 output, REA ev_6078bb7c6b4e0b06bf06127ba7042fc6f731ff37f88f0297ea3c54294e289730:
  // [2,1|5;1,-1|1] -> inverse[.33,.33;.33,-.67], determinant-3, x=(2,1).
  ApplicationShell shell;QVERIFY(shell.registerModule(equationsModule()));auto& c=equationsController(shell);QString error;
  QVERIFY(c.initialize("Observed square equation summary",2,error));
  const char* values[2][3]={{"2","1","5"},{"1","-1","1"}};
  for(int i=0;i<2;++i)for(int j=0;j<3;++j)QVERIFY(c.editCell(i,j,values[i][j],error));
  QVERIFY(c.solve(error));QVERIFY(c.result());QVERIFY(!c.inverse());QVERIFY(c.solutionInverse());QVERIFY(c.solutionInverse()->invertible);
  QCOMPARE(*c.solutionInverse()->determinant,-3.);QVERIFY(std::abs(c.solutionInverse()->inverse(0,0)-1./3)<1e-12);QVERIFY(std::abs(c.solutionInverse()->inverse(1,1)+2./3)<1e-12);
  QVERIFY(c.solutionInverse()->trace.empty());QVERIFY(!c.result()->trace.empty());QCOMPARE(c.result()->trace.front().augmented.columns(),std::size_t(3));
  auto text=c.outputText();QVERIFY(text.contains("ASSOCIATED MATRIX INVERSE"));QVERIFY(text.contains("Determinant = -3.00"));QVERIFY(text.contains("0.33  0.33"));QVERIFY(text.contains("0.33  -0.67"));QVERIFY(text.contains("x1 = 2.00"));QVERIFY(text.contains("x2 = 1.00"));
  QVERIFY(shell.navigate("equations/result"));auto* result=shell.currentContent()->findChild<QPlainTextEdit*>("equations-result");auto* trace=shell.currentContent()->findChild<QPlainTextEdit*>("equations-trace");QVERIFY(result);QVERIFY(trace);QVERIFY(result->toPlainText().contains("ASSOCIATED MATRIX INVERSE"));
  QTemporaryDir temporary;QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);const auto pdf=temporary.filePath("square-summary.pdf");printer.setOutputFileName(pdf);QVERIFY(printEquations(printer,c,trace->toPlainText()));
  const auto extractor=QStandardPaths::findExecutable("pdftotext");if(!extractor.isEmpty()){
   QProcess process;process.start(extractor,{pdf,"-"});QVERIFY(process.waitForFinished());QCOMPARE(process.exitCode(),0);text=QString::fromUtf8(process.readAllStandardOutput());QVERIFY(text.contains("ASSOCIATED MATRIX INVERSE"));QVERIFY(text.contains("Determinant = -3.00"));QVERIFY(text.contains("UNIQUE SOLUTION"));QVERIFY(text.contains("SELECTED ITERATIONS"));
  }
  const auto destination=qEnvironmentVariable("TORA_UI_CAPTURES");if(!destination.isEmpty()){
   QVERIFY(QDir().mkpath(destination));QVERIFY(QFile::copy(pdf,destination+"/equations-solution-inverse-"+qEnvironmentVariable("QT_SCALE_FACTOR","1")+".pdf"));
   double scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();if(scale<=0)scale=1;shell.resize(qRound(1366/scale),qRound(768/scale));shell.show();QApplication::processEvents();QVERIFY(shell.grab().save(destination+"/equations-solution-inverse-"+qEnvironmentVariable("QT_SCALE_FACTOR","1")+".png"));
  }
  QVERIFY(c.setFormat({tora::core::Notation::Scientific,0,2},error));QVERIFY(c.solutionInverse());QVERIFY(c.solutionInverse()->trace.empty());QVERIFY(c.setFormat({},error));
  QVERIFY(c.solve(error,true));QVERIFY(!c.result());QVERIFY(!c.solutionInverse());QVERIFY(c.inverse());QVERIFY(!c.inverse()->trace.empty());QCOMPARE(c.inverse()->trace.front().augmented.columns(),std::size_t(4));
  QVERIFY(c.solve(error));QVERIFY(c.solutionInverse());QVERIFY(!c.editCell(0,0,"nan",error));QVERIFY(c.solutionInverse());QVERIFY(c.editCell(0,0,"3",error));QVERIFY(!c.solutionInverse());QVERIFY(!c.result());
 }
 void associatedInverseNumericalSafeguards(){
  ApplicationShell shell;auto& c=equationsController(shell);QString error;
  QVERIFY(c.initialize("Singular equation summary",2,error));QVERIFY(c.solve(error));QCOMPARE(c.result()->status,tora::equations::Status::Infinite);QVERIFY(!c.solutionInverse());QVERIFY(!c.outputText().contains("ASSOCIATED MATRIX INVERSE"));
  QVERIFY(c.editCell(0,2,"1",error));QVERIFY(c.solve(error));QCOMPARE(c.result()->status,tora::equations::Status::Inconsistent);QVERIFY(!c.solutionInverse());QVERIFY(c.outputText().contains("no solution"));
  // The equation solution0 is representable while entries of A^-1 exceed binary64.
  // Optional associated output must not turn this valid solution into a solve failure.
  QVERIFY(c.initialize("Inverse exceeds output range",2,error));
  const char* values[2][3]={{"1e-300","1e-300","0"},{"1e-300","1.00000000001e-300","0"}};
  for(int i=0;i<2;++i)for(int j=0;j<3;++j)QVERIFY(c.editCell(i,j,values[i][j],error));
  QVERIFY(c.solve(error));QCOMPARE(c.result()->status,tora::equations::Status::Unique);QCOMPARE(c.result()->solution,std::vector<double>({0,0}));QVERIFY(!c.solutionInverse());QVERIFY(c.outputText().contains("Associated matrix inverse unavailable:"));QVERIFY(c.outputText().contains("UNIQUE SOLUTION"));
  c.beginNew();QVERIFY(!c.result());QVERIFY(!c.solutionInverse());QVERIFY(c.outputText().isEmpty());
 }
 void classificationAndPdf(){
  ApplicationShell shell;auto& c=equationsController(shell);QString e;QVERIFY(c.initialize("Singular",2,e));QVERIFY(c.solve(e));QCOMPARE(c.result()->status,tora::equations::Status::Infinite);QVERIFY(c.outputText().contains("INFINITELY MANY"));QVERIFY(c.solve(e,true));QVERIFY(!c.inverse()->invertible);QVERIFY(c.outputText().contains("NO INVERSE"));QVERIFY(c.editCell(0,2,"1",e));QVERIFY(c.solve(e));QCOMPARE(c.result()->status,tora::equations::Status::Inconsistent);QVERIFY(c.outputText().contains("no solution"));QVERIFY(c.setTitle(QString(3000,'W'),e));QVERIFY(c.solve(e));QTemporaryDir dir;QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);auto path=dir.filePath("equations.pdf");printer.setOutputFileName(path);QVERIFY(printEquations(printer,c));QFile pdf(path);QVERIFY(pdf.open(QIODevice::ReadOnly));QVERIFY(pdf.readAll().startsWith("%PDF"));
 }
 void nativeGridTraceAndInvalidText(){
  ApplicationShell shell;QVERIFY(shell.registerModule(equationsModule()));auto& c=equationsController(shell);c.dialogs().showError=[](const QString&,const QString&){};QString e;QVERIFY(c.initialize("Native",2,e));QVERIFY(shell.navigate("equations/input"));shell.resize(800,600);shell.show();QTest::qWait(20);auto* grid=shell.currentContent()->findChild<QTableWidget*>("equations-input-grid");QVERIFY(grid);QCOMPARE(grid->columnCount(),3);grid->item(0,0)->setText("2");grid->item(0,1)->setText("1");grid->item(0,2)->setText("5");grid->item(1,0)->setText("1");grid->item(1,1)->setText("-1");grid->item(1,2)->setText("1");grid->item(0,0)->setText("bad");QVERIFY(!c.solve(e));QVERIFY(!c.result());grid->item(0,0)->setText("2");QVERIFY(c.solve(e));QVERIFY(shell.navigate("equations/result"));auto* trace=shell.currentContent()->findChild<QPlainTextEdit*>("equations-trace");QVERIFY(trace);QVERIFY(trace->toPlainText().contains("Step 1"));auto* next=shell.currentContent()->findChild<QPushButton*>("equations-next");QVERIFY(next->isEnabled());QTest::mouseClick(next,Qt::LeftButton);QVERIFY(trace->toPlainText().contains("Step 2"));auto* all=shell.currentContent()->findChild<QPushButton*>("equations-all");QTest::mouseClick(all,Qt::LeftButton);QVERIFY(trace->toPlainText().contains("Step 1"));QVERIFY(trace->toPlainText().contains("Step 2"));QVERIFY(c.solve(e,true));QVERIFY(trace->isVisible());QVERIFY(trace->toPlainText().contains("Step 1"));QVERIFY(next->isEnabled());QTest::mouseClick(next,Qt::LeftButton);QVERIFY(trace->toPlainText().contains("Step 2"));
 }
};
QTEST_MAIN(DesktopEquations)
#include "desktop_equations.moc"
