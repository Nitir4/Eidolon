#include "assignment_controller.hpp"
#include "ui_font.hpp"
#include <tora/compat/transportation_document.hpp>
#include <QTest>
#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLineEdit>
#include <QProcess>
#include <QPrinter>
#include <QPushButton>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTableView>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QDir>
using namespace tora::desktop;
namespace {
// The application supplies F2 consistently on every platform.
constexpr auto tableEditKey=Qt::Key_F2;
}
class DesktopAssignment:public QObject {
 Q_OBJECT
private slots:
 void initTestCase(){QApplication::setFont(readableUiFont(QApplication::font()));}
 void minMaxAndRectangular(){ApplicationShell shell;QVERIFY(shell.registerModule(assignmentModule()));auto& c=assignmentController(shell);c.dialogs().askSave=[] {return SaveChoice::No;};c.dialogs().showError=[](const QString&,const QString&){};QVERIFY(c.loadExample());QVERIFY(c.solve());QCOMPARE(c.result()->objective,9.);QCOMPARE(*c.result()->rowToColumn[0],std::size_t(1));QCOMPARE(*c.result()->rowToColumn[1],std::size_t(0));QCOMPARE(*c.result()->rowToColumn[2],std::size_t(2));auto* pairs=shell.currentContent()->findChild<QTableWidget*>("assignment-pairs");QVERIFY(pairs);QCOMPARE(pairs->item(0,1)->text(),QStringLiteral("Job 2"));QVERIFY(c.loadExample(true));QVERIFY(c.solve());QCOMPARE(c.result()->objective,35.);QCOMPARE(*c.result()->rowToColumn[0],std::size_t(0));QCOMPARE(*c.result()->rowToColumn[1],std::size_t(2));QCOMPARE(*c.result()->rowToColumn[2],std::size_t(1));QVERIFY(c.initialize("Rectangle",3,2));QVERIFY(c.setCost(0,0,"1"));QVERIFY(c.setCost(0,1,"5"));QVERIFY(c.setCost(1,0,"4"));QVERIFY(c.setCost(1,1,"2"));QVERIFY(c.setCost(2,0,"10"));QVERIFY(c.setCost(2,1,"10"));QVERIFY(c.solve());QCOMPARE(c.result()->objective,3.);QVERIFY(!c.result()->rowToColumn[2]);QVERIFY(!c.setCost(0,0,"nan"));QVERIFY(c.result());QVERIFY(c.setCost(0,0,"1.25"));QVERIFY(!c.result());}
 void filesTransactionAndFormats(){QTemporaryDir dir;ApplicationShell shell;QVERIFY(shell.registerModule(assignmentModule()));auto& c=assignmentController(shell);c.dialogs().askSave=[] {return SaveChoice::No;};c.dialogs().showError=[](const QString&,const QString&){};QVERIFY(c.loadExample());QVERIFY(c.setName(true,0,QString::fromUtf8("Worker α <quoted>")));QVERIFY(c.setMetadata(QString::fromUtf8("Assignment α \\\"title"),tora::assignment::Sense::Minimize,{tora::core::Notation::Scientific,7,3}));QVERIFY(c.setCost(0,0,"1.23456789012345"));auto path=dir.filePath("matrix.assignment.json");QVERIFY(c.saveTo(path));QVERIFY(!c.dirty());QVERIFY(c.openFile(path,true));QVERIFY(c.readOnly());QVERIFY(!c.setCost(0,0,"2"));QVERIFY(!c.saveTo(path));QVERIFY(c.saveTo(dir.filePath("copy.json")));QVERIFY(!c.readOnly());QCOMPARE(c.data()->model.costs(0,0),1.23456789012345);QCOMPARE(c.data()->workers[0],QString::fromUtf8("Worker α <quoted>"));QCOMPARE(c.data()->format.n,7);QVERIFY(c.solve());const auto before=encodeAssignment(*c.data());QVERIFY(!c.openFile(dir.filePath("missing")));QCOMPARE(encodeAssignment(*c.data()),before);QVERIFY(c.result());QJsonObject bad=QJsonDocument::fromJson(before).object();bad["version"]=2;QVERIFY_EXCEPTION_THROWN(decodeAssignment(QJsonDocument(bad).toJson()),std::invalid_argument);bad=QJsonDocument::fromJson(before).object();bad["costs"]=QJsonArray{QJsonArray{QJsonValue::Null}};QVERIFY_EXCEPTION_THROWN(decodeAssignment(QJsonDocument(bad).toJson()),std::invalid_argument);QVERIFY(c.setCost(0,0,"3"));c.dialogs().askSave=[] {return SaveChoice::Cancel;};QVERIFY(!c.initialize("Canceled",1,1));QCOMPARE(c.data()->model.costs.rows(),std::size_t(3));QVERIFY(!shell.navigate("main"));}
 void importOriginalEquivalent(){QTemporaryDir dir;ApplicationShell shell;QVERIFY(shell.registerModule(assignmentModule()));auto& c=assignmentController(shell);c.dialogs().askSave=[] {return SaveChoice::No;};c.dialogs().showError=[](const QString&,const QString&){};auto p=tora::transportation::Problem::blank(3,3);p.costs={{9,2,7},{6,4,3},{5,8,1}};p.supply={1,1,1};p.demand={1,1,1};const auto file=dir.filePath("unit.txt");const auto write=[&]{QFile f(file);QVERIFY(f.open(QIODevice::WriteOnly));const auto bytes=tora::compat::TransportationDocument::create(p).encode();QCOMPARE(f.write(QByteArray::fromStdString(bytes)),qint64(bytes.size()));};write();QVERIFY(c.importTransportation(file));QVERIFY(c.solve());QCOMPARE(c.result()->objective,9.);p.costs={{-10,-5,-13},{-3,-9,-18},{-10,-7,-2}};write();QVERIFY(c.importTransportation(file,true));QVERIFY(c.solve());QCOMPARE(c.result()->objective,35.);const auto old=encodeAssignment(*c.data());p.supply[0]=2;write();QVERIFY(!c.importTransportation(file));QCOMPARE(encodeAssignment(*c.data()),old);QVERIFY(c.result());}
 void keyboardLayoutAndPrinting(){QTemporaryDir dir;ApplicationShell shell;QVERIFY(shell.registerModule(assignmentModule()));auto& c=assignmentController(shell);c.dialogs().askSave=[] {return SaveChoice::No;};c.dialogs().showError=[](const QString&,const QString&){};QVERIFY(c.loadExample());QVERIFY(shell.navigate("assignment/input"));shell.show();QTest::qWait(10);auto* grid=shell.currentContent()->findChild<QTableView*>("assignment-grid");QVERIFY(grid);const auto index=c.grid()->index(0,0);grid->setCurrentIndex(index);QTest::keyClick(grid,tableEditKey);QApplication::processEvents();auto* editor=grid->findChild<QLineEdit*>();QVERIFY(editor);QTest::keyClick(editor,Qt::Key_A,Qt::ControlModifier);QTest::keyClicks(editor,"9.125");QTest::keyClick(editor,Qt::Key_Return);QApplication::processEvents();QCOMPARE(c.data()->model.costs(0,0),9.125);QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>("assignment-solve"),Qt::LeftButton);QVERIFY(c.result());QCOMPARE(c.result()->objective,9.);QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);const auto pdf=dir.filePath("assignment.pdf");printer.setOutputFileName(pdf);QVERIFY(printAssignment(printer,c,shell.font()));QFile f(pdf);QVERIFY(f.open(QIODevice::ReadOnly));QCOMPARE(f.read(5),QByteArray("%PDF-"));const auto extractor=QStandardPaths::findExecutable("pdftotext");if(!extractor.isEmpty()){QProcess process;process.start(extractor,{pdf,"-"});QVERIFY(process.waitForFinished());QCOMPARE(process.exitCode(),0);const auto text=QString::fromUtf8(process.readAllStandardOutput()).normalized(QString::NormalizationForm_KC);QVERIFY(text.contains("Minimum cost = 9.00"));QVERIFY(text.contains("Worker 1 -> Job 2"));QVERIFY(text.contains("certificate"));}
    const auto root=qEnvironmentVariable("TORA_UI_CAPTURES");if(!root.isEmpty()){QVERIFY(QDir().mkpath(root));QVERIFY(QFile::copy(pdf,root+"/assignment-keyboard-"+qEnvironmentVariable("QT_SCALE_FACTOR","1")+".pdf"));}
    double scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();if(scale<=0)scale=1;
    for(const auto& route:{QStringLiteral("assignment/input"),QStringLiteral("assignment/result")}){QVERIFY(shell.navigate(route));for(auto size:{QSize(1366,768),QSize(1920,1080)}){shell.resize(QSize(qRound(size.width()/scale),qRound(size.height()/scale)));QApplication::processEvents();for(auto* button:shell.currentContent()->findChildren<QPushButton*>())QVERIFY(button->width()>=button->minimumSizeHint().width());if(!root.isEmpty())QVERIFY(shell.grab().save(root+"/"+(route.endsWith("input")?"assignment-input-":"assignment-result-")+QString::number(size.width())+"-"+qEnvironmentVariable("QT_SCALE_FACTOR")+".png"));}}
 }
};
int main(int argc,char** argv){
#ifdef Q_OS_WIN
 qputenv("QT_QPA_FONTDIR",qgetenv("SystemRoot")+"/Fonts");
#endif
 QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);QApplication app(argc,argv);DesktopAssignment test;return QTest::qExec(&test,argc,argv);
}
#include "desktop_assignment.moc"
