#include "lp_changes_controller.hpp"
#include "lp_input_controller.hpp"
#include "lp_final_controller.hpp"
#include "lp_screens.hpp"
#include "ui_font.hpp"
#include <QTest>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QPrinter>
#include <QProcess>
#include <QPushButton>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTableWidget>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QToolButton>
#include <limits>
using namespace tora::desktop;
namespace {
tora::lp::LpProblem example(){auto p=tora::lp::LpProblem::blank(2,3);p.title="Class changes <baseline>";p.objective={3,2};p.constraints={{{1,1},tora::lp::Relation::LessEqual,4},{{1,0},tora::lp::Relation::LessEqual,2},{{0,1},tora::lp::Relation::LessEqual,3}};return p;}
bool load(ApplicationShell& shell,const tora::lp::LpProblem& p,const QString& path,bool readOnly=false){auto doc=tora::compat::ToraLpCodec::create(p);tora::compat::LpFileStore::saveAs(doc,std::filesystem::path(path.toStdString()));auto& input=inputController(shell);input.dialogs().askSave=[] {return SaveChoice::No;};input.dialogs().showError=[](const QString&,const QString&){};changesController(shell).notice=[](const QString&,const QString&){};return input.openFile(path,readOnly);}
void edit(QTableWidget* table,int row,const QString& text){table->setCurrentCell(row,2);table->editItem(table->item(row,2));QApplication::processEvents();auto* editor=qobject_cast<QLineEdit*>(QApplication::focusWidget());QVERIFY(editor && table->isAncestorOf(editor));QTest::keyClick(editor,Qt::Key_A,Qt::ControlModifier);QTest::keyClicks(editor,text);QTest::keyClick(editor,Qt::Key_Return);QApplication::processEvents();}
void capture(ApplicationShell& shell,const QString& name){const auto root=qEnvironmentVariable("TORA_UI_CAPTURES");if(root.isEmpty())return;QVERIFY(QDir().mkpath(root));QVERIFY(shell.grab().save(root+QStringLiteral("/")+name+QStringLiteral("-")+qEnvironmentVariable("QT_SCALE_FACTOR")+QStringLiteral(".png")));}
}
class DesktopLpChanges:public QObject {
    Q_OBJECT
private slots:
    void baselineTenChangedTwelve_data(){QTest::addColumn<bool>("commitWithReturn");QTest::newRow("return")<<true;QTest::newRow("active-editor")<<false;}
    void baselineTenChangedTwelve(){
        QFETCH(bool,commitWithReturn);
        QTemporaryDir temp;ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();
        QVERIFY(load(shell,example(),temp.filePath(QStringLiteral("baseline.txt"))));
        auto& input=inputController(shell);const auto original=tora::compat::ToraLpCodec::encode(*input.document());
        auto& summary=finalController(shell);QVERIFY(summary.request());QVERIFY(summary.start({}));
        QCOMPARE(*summary.session()->display().objectiveValue,std::string("10.00"));
        auto& c=changesController(shell);QVERIFY(c.request(LpChangeMethod::Fresh));
        auto* costs=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-costs"));QVERIFY(costs);
        QCOMPARE(costs->item(0,1)->text(),QStringLiteral("3"));
        costs->setCurrentCell(0,2);costs->editItem(costs->item(0,2));QApplication::processEvents();
        auto* editor=qobject_cast<QLineEdit*>(QApplication::focusWidget());QVERIFY(editor && costs->isAncestorOf(editor));
        QTest::keyClick(editor,Qt::Key_A,Qt::ControlModifier);QTest::keyClicks(editor,"4");
        if(commitWithReturn)QTest::keyClick(editor,Qt::Key_Return);
        QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("changes-analyze")),Qt::LeftButton);
        QCOMPARE(shell.currentScreen(),QStringLiteral("lp/changes-result"));QVERIFY(c.fresh());
        QCOMPARE(*c.baselineObjective(),10.);QCOMPARE(*c.fresh()->after.objective,12.);
        auto* solution=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-solution"));QVERIFY(solution);
        QCOMPARE(solution->item(0,1)->text(),QStringLiteral("10.00"));QCOMPARE(solution->item(0,2)->text(),QStringLiteral("12.00"));
        QCOMPARE(solution->item(1,1)->text(),QStringLiteral("2.00"));QCOMPARE(solution->item(1,2)->text(),QStringLiteral("2.00"));
        QCOMPARE(solution->item(2,1)->text(),QStringLiteral("2.00"));QCOMPARE(solution->item(2,2)->text(),QStringLiteral("2.00"));
        QCOMPARE(tora::compat::ToraLpCodec::encode(*input.document()),original);
        capture(shell,QStringLiteral("baseline10-changed12-")+QString::fromLatin1(QTest::currentDataTag()));
        QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("changes-back")),Qt::LeftButton);
        edit(costs,0,QStringLiteral("5"));
        QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("changes-analyze")),Qt::LeftButton);
        QCOMPARE(solution->item(0,1)->text(),QStringLiteral("10.00"));QCOMPARE(solution->item(0,2)->text(),QStringLiteral("14.00"));
    }
    void freshDraftApplyAndSave(){
        QTemporaryDir temp;ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();QVERIFY(shell.navigate(QStringLiteral("lp/solve-menu")));
        auto* action=shell.findChild<QAction*>(QStringLiteral("solve-changes"));QVERIFY(action && !action->isEnabled());
        const auto source=temp.filePath(QStringLiteral("baseline.txt"));QVERIFY(load(shell,example(),source,true));auto& input=inputController(shell);const auto original=tora::compat::ToraLpCodec::encode(*input.document());
        auto& c=changesController(shell);action->trigger();QCOMPARE(shell.currentScreen(),QStringLiteral("lp/changes"));QVERIFY(c.baseline());
        auto* costs=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-costs"));QVERIFY(costs);edit(costs,0,QStringLiteral("1"));
        capture(shell,QStringLiteral("fresh-draft"));QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("changes-analyze")),Qt::LeftButton);
        QCOMPARE(shell.currentScreen(),QStringLiteral("lp/changes-result"));QVERIFY(c.fresh());QCOMPARE(*c.fresh()->after.objective,7.);QCOMPARE(tora::compat::ToraLpCodec::encode(*input.document()),original);
        auto* solution=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-solution"));QCOMPARE(solution->item(0,1)->text(),QStringLiteral("10.00"));QCOMPARE(solution->item(0,2)->text(),QStringLiteral("7.00"));QCOMPARE(solution->item(1,2)->text(),QStringLiteral("1.00"));QCOMPARE(solution->item(2,2)->text(),QStringLiteral("3.00"));QVERIFY(!c.fresh()->retained.dualFeasible);
        capture(shell,QStringLiteral("fresh-result"));
        QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("changes-back")),Qt::LeftButton);QCOMPARE(costs->item(0,2)->text(),QStringLiteral("1"));edit(costs,0,QStringLiteral("1.23456789012345"));QVERIFY(!c.hasResult());QVERIFY(!c.apply());
        QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("changes-analyze")),Qt::LeftButton);QVERIFY(c.hasResult());
        QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("changes-apply")),Qt::LeftButton);QCOMPARE(shell.currentScreen(),QStringLiteral("lp/input"));QVERIFY(!c.baseline() && !c.hasResult());QVERIFY(input.document()->dirty());QVERIFY(input.document()->readOnly());QCOMPARE(input.document()->sourcePath().string(),source.toStdString());QCOMPARE(input.document()->problem.objective[0],1.23456789012345);
        const auto saved=temp.filePath(QStringLiteral("changed.txt"));QVERIFY(input.saveTo(saved));QVERIFY(input.openFile(saved));QCOMPARE(input.document()->problem.objective[0],1.23456789012345);QCOMPARE(input.document()->problem.constraints[0].rhs,4.);QVERIFY(!input.document()->dirty());
    }
    void revisedTraceAndPrinting(){
        QTemporaryDir temp;ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();QVERIFY(load(shell,example(),temp.filePath(QStringLiteral("baseline.txt"))));shell.navigate(QStringLiteral("lp/solve-menu"));
        auto* action=shell.findChild<QAction*>(QStringLiteral("solve-dual-revised"));QVERIFY(action && action->isEnabled());action->trigger();auto& c=changesController(shell);QCOMPARE(c.method(),LpChangeMethod::RevisedDual);
        auto* costs=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-costs"));QVERIFY(!(costs->item(0,2)->flags()&Qt::ItemIsEditable));
        auto* rhs=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-rhs"));edit(rhs,0,QStringLiteral("6"));QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("changes-analyze")),Qt::LeftButton);
        QVERIFY(c.revised());QCOMPARE(*c.revised()->objective,12.);QCOMPARE(*c.baselineObjective(),10.);QCOMPARE(inputController(shell).document()->problem.constraints[0].rhs,4.);
        auto* solution=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-solution"));QCOMPARE(solution->item(0,2)->text(),QStringLiteral("12.00"));QCOMPARE(solution->item(1,2)->text(),QStringLiteral("2.00"));QCOMPARE(solution->item(2,2)->text(),QStringLiteral("3.00"));
        QCOMPARE(solution->item(3,2)->text(),QStringLiteral("6.00"));QCOMPARE(solution->item(4,2)->text(),QStringLiteral("6.00"));QCOMPARE(solution->item(5,2)->text(),QStringLiteral("1.00"));QCOMPARE(solution->item(6,2)->text(),QStringLiteral("0.00"));QCOMPARE(solution->item(7,2)->text(),QStringLiteral("0.00"));
        auto* tabs=shell.currentContent()->findChild<QTabWidget*>(QStringLiteral("changes-result-tabs"));tabs->setCurrentIndex(1);auto* step=shell.currentContent()->findChild<QSpinBox*>(QStringLiteral("changes-trace-step"));QVERIFY(step->maximum()>1);step->setValue(1);auto* trace=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-trace"));bool negative=false;for(int row=0;row<trace->rowCount();++row)if(trace->item(row,0)->text()==QStringLiteral("Basic") && trace->item(row,2)->text().toDouble()<0)negative=true;QVERIFY(negative);QVERIFY(shell.currentContent()->findChild<QLabel*>(QStringLiteral("changes-pivot"))->text().contains(QStringLiteral("Leaving:")));
        QApplication::processEvents();QVERIFY(shell.currentContent()->minimumSizeHint().width()<=shell.width());
        capture(shell,QStringLiteral("revised-initial-trace"));step->setValue(step->maximum());capture(shell,QStringLiteral("revised-final-trace"));
        const auto count=c.revised()->canonical.trace.size();QPrinter printer(QPrinter::HighResolution);printer.setOutputFormat(QPrinter::PdfFormat);const auto pdf=temp.filePath(QStringLiteral("changes.pdf"));printer.setOutputFileName(pdf);QVERIFY(printChanges(printer,c,shell.font(),count-1));QCOMPARE(c.revised()->canonical.trace.size(),count);
        QFile file(pdf);QVERIFY(file.open(QIODevice::ReadOnly));QCOMPARE(file.read(5),QByteArray("%PDF-"));QVERIFY(file.size()>1000);const auto extractor=QStandardPaths::findExecutable(QStringLiteral("pdftotext"));
        if(!extractor.isEmpty()){QProcess text;text.start(extractor,{pdf,QStringLiteral("-")});QVERIFY(text.waitForFinished());QCOMPARE(text.exitCode(),0);const auto content=text.readAllStandardOutput();for(const auto& token:QList<QByteArray>{"Baseline model","Changed model","12.00","Selected trace step","Class changes <baseline>"})QVERIFY2(content.contains(token),content.constData());}
        else qInfo("PDF structure checked; optional pdftotext unavailable, content extraction not checked");
        const auto captureRoot=qEnvironmentVariable("TORA_UI_CAPTURES");if(!captureRoot.isEmpty())QVERIFY(QFile::copy(pdf,captureRoot+QStringLiteral("/revised-")+qEnvironmentVariable("QT_SCALE_FACTOR")+QStringLiteral(".pdf")));
        QVERIFY(!c.analyze({1,2},{6,2,3},{}));QVERIFY(!c.hasResult());QVERIFY(!shell.currentContent()->findChild<QPushButton*>(QStringLiteral("changes-apply"))->isEnabled());
    }
    void minimumAndWidePrint(){
        QTemporaryDir temp;ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();
        auto p=tora::lp::LpProblem::blank(2,3);p.sense=tora::lp::ObjectiveSense::Minimize;p.objective={2,3};p.constraints={{{1,1},tora::lp::Relation::GreaterEqual,4},{{1,2},tora::lp::Relation::GreaterEqual,6},{{2,1},tora::lp::Relation::GreaterEqual,5}};
        QVERIFY(load(shell,p,temp.filePath(QStringLiteral("minimum.txt"))));auto& c=changesController(shell);QVERIFY(c.request(LpChangeMethod::RevisedDual));QVERIFY(c.analyze({2,3},{7,6,5},{}));
        auto* solution=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-solution"));QCOMPARE(solution->item(0,2)->text(),QStringLiteral("14.00"));QCOMPARE(solution->item(1,2)->text(),QStringLiteral("7.00"));QCOMPARE(solution->item(2,2)->text(),QStringLiteral("0.00"));QCOMPARE(solution->item(3,2)->text(),QStringLiteral("14.00"));QCOMPARE(solution->item(4,2)->text(),QStringLiteral("0.00"));QCOMPARE(solution->item(5,2)->text(),QStringLiteral("0.00"));QCOMPARE(solution->item(6,2)->text(),QStringLiteral("1.00"));QCOMPARE(solution->item(7,2)->text(),QStringLiteral("9.00"));capture(shell,QStringLiteral("minimum-result"));
        auto wide=tora::lp::LpProblem::blank(100,1);wide.objective[0]=1;wide.constraints[0].coefficients.assign(100,1);wide.constraints[0].rhs=1;QVERIFY(load(shell,wide,temp.filePath(QStringLiteral("wide.txt"))));QVERIFY(c.request(LpChangeMethod::Fresh));QVERIFY(c.analyze(wide.objective,{2},{}));QCOMPARE(*c.fresh()->after.objective,2.);
        shell.currentContent()->findChild<QTabWidget*>(QStringLiteral("changes-result-tabs"))->setCurrentIndex(1);auto* grid=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-trace"));QApplication::processEvents();QVERIFY(grid->columnCount()>100);capture(shell,QStringLiteral("wide-trace"));
        QPrinter printer(QPrinter::HighResolution);printer.setOutputFormat(QPrinter::PdfFormat);printer.setOutputFileName(temp.filePath(QStringLiteral("wide.pdf")));QVERIFY(printChanges(printer,c,shell.font(),c.fresh()->after.trace.size()-1));QFile file(printer.outputFileName());QVERIFY(file.open(QIODevice::ReadOnly));QVERIFY(file.size()>1000);
        const auto extractor=QStandardPaths::findExecutable(QStringLiteral("pdftotext"));if(!extractor.isEmpty()){QProcess process;process.start(extractor,{printer.outputFileName(),QStringLiteral("-")});QVERIFY(process.waitForFinished());QCOMPARE(process.exitCode(),0);const auto text=process.readAllStandardOutput();QVERIFY(text.contains("x100"));QVERIFY(text.contains("Changed model"));QVERIFY(text.contains("Selected trace step"));QVERIFY(text.count('\f')>1);}
        const auto root=qEnvironmentVariable("TORA_UI_CAPTURES");if(!root.isEmpty())QVERIFY(QFile::copy(printer.outputFileName(),root+QStringLiteral("/wide-")+qEnvironmentVariable("QT_SCALE_FACTOR")+QStringLiteral(".pdf")));
    }
    void terminalErrorsAndCancel(){
        QTemporaryDir temp;ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();QVERIFY(load(shell,example(),temp.filePath(QStringLiteral("baseline.txt"))));auto& input=inputController(shell);auto& c=changesController(shell);QString message;c.notice=[&](const QString&,const QString& text){message=text;};
        QVERIFY(c.request(LpChangeMethod::Fresh));QVERIFY(c.analyze({3,2},{-1,2,3},{}));QVERIFY(c.fresh());QCOMPARE(c.fresh()->after.status,tora::lp::SimplexStatus::Infeasible);QVERIFY(!c.fresh()->after.objective);QCOMPARE(shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-solution"))->item(0,2)->text(),QStringLiteral("—"));QVERIFY(shell.currentContent()->findChild<QLabel*>(QStringLiteral("changes-status"))->text().contains(QStringLiteral("infeasible")));QCOMPARE(input.document()->problem.constraints[0].rhs,4.);
        QVERIFY(c.request(LpChangeMethod::RevisedDual));QVERIFY(c.analyze({3,2},{-1,2,3},{}));QCOMPARE(c.revised()->canonical.status,tora::lp::RevisedDualStatus::Infeasible);QVERIFY(!c.revised()->objective);
        QVERIFY(!c.analyze({3,2},{std::numeric_limits<double>::infinity(),2,3},{}));QVERIFY(!message.isEmpty());QVERIFY(!c.hasResult());QCOMPARE(input.document()->problem.constraints[0].rhs,4.);
        shell.navigate(QStringLiteral("lp/changes"));auto* rhs=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-rhs"));rhs->item(0,2)->setText(QStringLiteral("1e999"));QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("changes-analyze")),Qt::LeftButton);QCOMPARE(shell.currentScreen(),QStringLiteral("lp/changes"));QVERIFY(!c.hasResult());
        QVERIFY(shell.navigate(QStringLiteral("lp/input")));input.dialogs().askSave=[] {return SaveChoice::Cancel;};const auto old=c.baseline()->problem.constraints[0].rhs;QVERIFY(!c.request(LpChangeMethod::Fresh));QCOMPARE(shell.currentScreen(),QStringLiteral("lp/input"));QCOMPARE(c.baseline()->problem.constraints[0].rhs,old);
        auto unbounded=tora::lp::LpProblem::blank(2,1);unbounded.objective={-1,-1};unbounded.constraints={{{1,-1},tora::lp::Relation::LessEqual,1}};QVERIFY(load(shell,unbounded,temp.filePath(QStringLiteral("unbounded-change.txt"))));QVERIFY(c.request(LpChangeMethod::Fresh));QVERIFY(c.analyze({2,-1},{1},{}));QCOMPARE(c.fresh()->after.status,tora::lp::SimplexStatus::Unbounded);QVERIFY(!c.fresh()->after.objective);
    }
    void scientificAndLayout(){
        QTemporaryDir temp;ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();QVERIFY(load(shell,example(),temp.filePath(QStringLiteral("baseline.txt"))));auto& c=changesController(shell);QVERIFY(c.request(LpChangeMethod::RevisedDual));auto* rhs=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-rhs"));edit(rhs,0,QStringLiteral("6"));shell.currentContent()->findChild<QComboBox*>(QStringLiteral("changes-notation"))->setCurrentIndex(1);shell.currentContent()->findChild<QSpinBox*>(QStringLiteral("changes-N"))->setValue(5);shell.currentContent()->findChild<QSpinBox*>(QStringLiteral("changes-D"))->setValue(2);QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("changes-analyze")),Qt::LeftButton);QVERIFY(c.revised());QCOMPARE(c.format().notation,tora::core::Notation::Scientific);QCOMPARE(*c.revised()->objective,12.);
        const auto expected=QString::fromStdString(tora::core::NumberFormatter({tora::core::Notation::Scientific,5,2}).format(12));QCOMPARE(shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("changes-solution"))->item(0,2)->text(),expected);
        const double scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();
        for(const auto& route:{QStringLiteral("lp/changes"),QStringLiteral("lp/changes-result")}){QVERIFY(shell.navigate(route));for(auto size:{QSize(1366,768),QSize(1920,1080),QSize(3840,2160)}){shell.resize(QSize(qRound(size.width()/scale),qRound(size.height()/scale)));QApplication::processEvents();for(auto* b:shell.currentContent()->findChildren<QPushButton*>())QVERIFY2(b->width()>=b->minimumSizeHint().width(),qPrintable(b->objectName()));auto* grid=shell.currentContent()->findChild<QTableWidget*>(route.endsWith(QStringLiteral("result"))?QStringLiteral("changes-solution"):QStringLiteral("changes-rhs"));QVERIFY(grid->viewport()->width()>100 && grid->viewport()->height()>80);capture(shell,(route.endsWith(QStringLiteral("result"))?QStringLiteral("result-"):QStringLiteral("draft-"))+QString::number(size.width()));}}
        inputController(shell).beginNew();QVERIFY(!c.baseline() && !c.hasResult());
    }
};
int main(int argc,char** argv){
#ifdef Q_OS_WIN
    qputenv("QT_QPA_FONTDIR",qgetenv("SystemRoot")+"/Fonts");
#endif
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);QApplication app(argc,argv);QApplication::setFont(readableUiFont(app.font()));DesktopLpChanges test;return QTest::qExec(&test,argc,argv);
}
#include "desktop_lp_changes.moc"
