#include "lp_changes_controller.hpp"
#include "lp_input_controller.hpp"
#include "lp_screens.hpp"
#include "pdf_text.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QPrinter>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QTemporaryDir>
#include <QTest>

using namespace tora::desktop;
namespace {
QStringList headings(const QString& text) {
    QStringList result;
    auto matches=QRegularExpression(QStringLiteral("(Baseline|Changed) trace step (\\d+) of (\\d+)"))
        .globalMatch(text.simplified());
    while(matches.hasNext()){const auto m=matches.next();result<<m.captured(0);}
    return result;
}
QByteArray read(const QString& path){QFile f(path);return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();}
std::vector<double> numbers(const QString& text){std::vector<double> values;for(const auto& x:text.split(','))values.push_back(x.toDouble());return values;}
}
class ChangesExport final:public QObject {
    Q_OBJECT
private slots:
    void completeHistory_data() {
        QTest::addColumn<bool>("revised");QTest::addColumn<QString>("costs");QTest::addColumn<QString>("rhs");
        QTest::addColumn<double>("objective");QTest::addColumn<QString>("solution");
        QTest::newRow("Fresh-12-Q1")<<false<<QString("4,2")<<QString("4,2,3")<<12.<<QString("2,2");
        QTest::newRow("Fresh-12-Q2")<<false<<QString("3,1")<<QString("4,2,3")<<8.<<QString("2,2");
        QTest::newRow("Fresh-13-Q1")<<false<<QString("1,2")<<QString("4,2,3")<<7.<<QString("1,3");
        QTest::newRow("Fresh-13-Q2")<<false<<QString("3,4")<<QString("4,2,3")<<15.<<QString("1,3");
        for(bool revised:{false,true}) {
            const auto prefix=revised?QByteArray("Revised-"):QByteArray("Fresh-");
            QTest::newRow((prefix+"14-Q1").constData())<<revised<<QString("3,2")<<QString("3,2,3")<<8.<<QString("2,1");
            QTest::newRow((prefix+"14-Q2").constData())<<revised<<QString("3,2")<<QString("4.5,2,3")<<11.<<QString("2,2.5");
            QTest::newRow((prefix+"15-Q1").constData())<<revised<<QString("3,2")<<QString("6,2,3")<<12.<<QString("2,3");
            QTest::newRow((prefix+"15-Q2").constData())<<revised<<QString("3,2")<<QString("1,2,3")<<3.<<QString("1,0");
        }
    }
    void completeHistory() {
        QFETCH(bool,revised);QFETCH(QString,costs);QFETCH(QString,rhs);QFETCH(double,objective);QFETCH(QString,solution);
        QTemporaryDir temp;ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));
        auto base=tora::lp::LpProblem::blank(2,3);base.title="Export <baseline>";base.objective={3,2};
        base.constraints={{{1,1},tora::lp::Relation::LessEqual,4},{{1,0},tora::lp::Relation::LessEqual,2},{{0,1},tora::lp::Relation::LessEqual,3}};
        auto document=tora::compat::ToraLpCodec::create(base);auto& input=inputController(shell);
        input.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(input.loadDocument(document));
        auto& c=changesController(shell);c.notice=[](const QString&,const QString&){};
        QVERIFY(c.request(revised?LpChangeMethod::RevisedDual:LpChangeMethod::Fresh));QVERIFY(c.analyze(numbers(costs),numbers(rhs),{}));
        QCOMPARE(*c.baselineObjective(),10.);QCOMPARE(c.baselineValues(),std::vector<double>({2,2}));
        QCOMPARE(revised?*c.revised()->objective:*c.fresh()->after.objective,objective);
        QCOMPARE(revised?c.revised()->values:c.fresh()->after.values,numbers(solution));
        const auto before=tora::compat::ToraLpCodec::encode(*input.document());
        const auto changed=revised?c.revised()->values:c.fresh()->after.values;
        const auto count=revised?c.revised()->canonical.trace.size():c.fresh()->after.trace.size();
        const auto baselineCount=revised?0:c.fresh()->before.trace.size();
        QStringList expected;
        for(std::size_t i=0;i<baselineCount;++i)expected<<QStringLiteral("Baseline trace step %1 of %2").arg(i+1).arg(baselineCount);
        for(std::size_t i=0;i<count;++i)expected<<QStringLiteral("Changed trace step %1 of %2").arg(i+1).arg(count);
        auto* selected=shell.currentContent()->findChild<QSpinBox*>("changes-trace-step");QVERIFY(selected);
        QCOMPARE(shell.currentContent()->findChild<QPushButton*>("changes-print")->text(),QString("Print all steps"));
        QVERIFY(shell.currentContent()->findChild<QPushButton*>("changes-print-selected")->text().contains("selected step"));
        QString fullText,pdfText;
        for(int selection:{1,static_cast<int>(count)}) {
            selected->setValue(selection);
            const auto stem=temp.filePath(QString("history-%1").arg(selection));
            QVERIFY(exportChangesHistory(stem+".txt",c));const auto content=QString::fromUtf8(read(stem+".txt"));
            QCOMPARE(headings(content),expected);QVERIFY(!content.contains("Selected trace step"));
            if(!fullText.isEmpty())QCOMPARE(content,fullText);
            fullText=content;
            if(revised)QVERIFY(content.contains("no baseline iteration history is stored"));
            else {
                // Compare every recorded row, including the baseline tail when
                // the changed solve has fewer steps than the baseline.
                int offset=0;
                for(const auto* history:{&c.fresh()->before.trace,&c.fresh()->after.trace})for(const auto& step:*history){
                    const auto start=content.indexOf(expected[offset]);const auto end=offset+1<expected.size()?content.indexOf(expected[offset+1]):content.size();
                    const auto block=content.mid(start,end-start);const auto& t=step.tableau;
                    for(std::size_t i=0;i<t.values().rows();++i){QStringList cells{i?QString::fromStdString(t.basicLabel(i)):QString("z")};for(std::size_t j=0;j<t.values().columns();++j)cells<<QString::fromStdString(tora::core::NumberFormatter(c.format()).format(t.values()(i,j)));QVERIFY2(block.contains(cells.join("; ")),qPrintable(block));}
                    ++offset;
                }
            }
            QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);printer.setOutputFileName(stem+".pdf");
            QVERIFY(printChanges(printer,c,shell.font()));QString extracted,error;QVERIFY2(tora::test::extractPdfText(stem+".pdf",extracted,error),qPrintable(error));
            QCOMPARE(headings(extracted),expected);QVERIFY(!extracted.contains("Selected trace step"));
            auto compactPdf=extracted;compactPdf.remove(QRegularExpression("\\s+"));
            for(auto line:content.split('\n'))if(line.contains(';')){
                line.remove(QRegularExpression("\\s+"));QVERIFY2(compactPdf.contains(line),qPrintable(line));
            }
            if(!pdfText.isEmpty())QCOMPARE(extracted,pdfText);
            pdfText=extracted;
            QCOMPARE(selected->value(),selection);QCOMPARE(tora::compat::ToraLpCodec::encode(*input.document()),before);
            QCOMPARE(revised?c.revised()->values:c.fresh()->after.values,changed);
            QCOMPARE(revised?c.revised()->canonical.trace.size():c.fresh()->after.trace.size(),count);
            const auto root=qEnvironmentVariable("TORA_EXPORT_FIX_ARTIFACTS");
            if(!root.isEmpty()){QVERIFY(QDir().mkpath(root));for(const auto& suffix:{QString(".pdf"),QString(".txt")}){const auto target=root+"/"+QTest::currentDataTag()+QString("-selection%1").arg(selection)+suffix;QFile::remove(target);QVERIFY(QFile::copy(stem+suffix,target));}}
        }
        const auto path=temp.filePath("selected.pdf");QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);printer.setOutputFileName(path);
        QVERIFY(printChanges(printer,c,shell.font(),count-1));QString extracted,error;QVERIFY2(tora::test::extractPdfText(path,extracted,error),qPrintable(error));
        QVERIFY(extracted.contains(QString("Selected trace step %1 of %2").arg(count).arg(count)));QVERIFY(headings(extracted).isEmpty());
        QVERIFY(!printChanges(printer,c,shell.font(),count));
        c.clearResult();QVERIFY(!exportChangesHistory(temp.filePath("empty.txt"),c));QVERIFY(!printChanges(printer,c,shell.font()));
        QVERIFY(!shell.currentContent()->findChild<QPushButton*>("changes-export-all")->isEnabled());
    }
};
int main(int argc,char** argv) {
#ifdef Q_OS_WIN
    qputenv("QT_QPA_FONTDIR",qgetenv("SystemRoot")+"/Fonts");
#endif
    QApplication app(argc,argv);ChangesExport test;return QTest::qExec(&test,argc,argv);
}
#include "desktop_changes_export.moc"
