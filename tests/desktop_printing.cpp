#include "application_shell.hpp"
#include "lp_input_controller.hpp"
#include "lp_simplex_controller.hpp"
#include "lp_graphical_controller.hpp"
#include "lp_final_controller.hpp"
#include "lp_screens.hpp"
#include "lp_printing.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPrinter>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTest>
#include <QPrintDialog>
#include <QTimer>
#include <cstdio>

using namespace tora::desktop;
class DesktopPrinting final : public QObject {
    Q_OBJECT
private slots:
    void cancelWithoutDefault() {
        bool seen=false,rendered=false;QTimer timer;timer.setInterval(10);
        connect(&timer,&QTimer::timeout,[&]{
            if(auto* dialog=qobject_cast<QPrintDialog*>(QApplication::activeModalWidget())){seen=true;dialog->reject();}
        });
        timer.start();
        QCOMPARE(requestLpPrint(nullptr,[&](QPrinter&){rendered=true;return true;},QPrinterInfo()),PrintOutcome::Cancelled);
        QVERIFY(seen);QVERIFY(!rendered);
    }
    void documents_data() {
        QTest::addColumn<QJsonObject>("record");
        QFile file(QStringLiteral(TORA_PRINT_GOLDENS));QVERIFY(file.open(QIODevice::ReadOnly));
        for (const auto& value : QJsonDocument::fromJson(file.readAll()).array()) {
            const auto record=value.toObject();
            QTest::newRow(qPrintable(record["case"].toString()))<<record;
        }
    }
    void documents() {
        QFETCH(QJsonObject,record);
        const auto source=record["problem"].toObject();
        const auto variables=source["variables"].toArray(), constraints=source["constraints"].toArray();
        auto problem=tora::lp::LpProblem::blank(static_cast<std::size_t>(variables.size()),static_cast<std::size_t>(constraints.size()));
        problem.title=source["title"].toString().toStdString();
        problem.sense=source["sense"]==QStringLiteral("min")?tora::lp::ObjectiveSense::Minimize:tora::lp::ObjectiveSense::Maximize;
        for(int j=0;j<variables.size();++j) {
            const auto v=variables[j].toObject();auto& x=problem.variables[static_cast<std::size_t>(j)];
            x.name=v["name"].toString().toStdString();x.lowerBound=std::stod(v["lower"].toString().toStdString());x.upperBound=std::stod(v["upper"].toString().toStdString());x.unrestricted=v["unrestricted"].toBool();
            problem.objective[static_cast<std::size_t>(j)]=std::stod(source["objective"].toArray()[j].toString().toStdString());
        }
        for(int i=0;i<constraints.size();++i) {
            const auto v=constraints[i].toObject();auto& row=problem.constraints[static_cast<std::size_t>(i)];
            row.rhs=std::stod(v["rhs"].toString().toStdString());const auto relation=v["relation"].toString();
            row.relation=relation==QStringLiteral("<=")?tora::lp::Relation::LessEqual:relation==QStringLiteral(">=")?tora::lp::Relation::GreaterEqual:tora::lp::Relation::Equal;
            for(int j=0;j<variables.size();++j)row.coefficients[static_cast<std::size_t>(j)]=std::stod(v["coefficients"].toArray()[j].toString().toStdString());
        }
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));
        auto& input=inputController(shell);input.dialogs().askSave=[] {return SaveChoice::No;};
        QTemporaryDir temp;auto document=tora::compat::ToraLpCodec::create(problem);
        const auto path=temp.filePath(QStringLiteral("input.txt"));
        tora::compat::LpFileStore::saveAs(document,std::filesystem::path(path.toStdString()));QVERIFY(input.openFile(path));
        const auto before=tora::compat::ToraLpCodec::encode(*input.document());
        const auto settings=record["format"].toArray();
        const tora::core::NumberFormat format{settings[0].toInt()?tora::core::Notation::Scientific:tora::core::Notation::Decimal,settings[1].toInt(),settings[2].toInt()};
        QPrinter printer(QPrinter::HighResolution);printer.setResolution(300);
        printer.setOutputFormat(QPrinter::PdfFormat);printer.setPageSize(QPageSize(QPageSize::A4));
        const auto folder=QStringLiteral(TORA_PRINT_PDFS);QVERIFY(QDir().mkpath(folder));
        printer.setOutputFileName(folder+QLatin1Char('/')+record["case"].toString()+QStringLiteral(".pdf"));
        QVERIFY(!QFile::exists(printer.outputFileName()) || QFile::remove(printer.outputFileName()));
        const auto method=record["method"].toString();
        if(method==QStringLiteral("final")) {
            auto& c=finalController(shell);c.notice=[](const QString&,const QString&){};
            QVERIFY(c.request());QVERIFY(c.start(format));
            const auto count=c.session()->solver().history().size();
            if(!record["print_available"].toBool(true)) {
                QVERIFY(!printSummary(printer,c,shell.font()));
                QVERIFY(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("summary-print"))->isHidden());
                QVERIFY(!QFile::exists(printer.outputFileName()));
                QCOMPARE(tora::compat::ToraLpCodec::encode(*input.document()),before);
                return;
            }
            QVERIFY(printSummary(printer,c,shell.font()));
            QCOMPARE(c.session()->solver().history().size(),count);
        } else if(method==QStringLiteral("graphical")) {
            auto& c=graphicalController(shell);c.notice=[](const QString&,const QString&){};
            QVERIFY(c.request());QVERIFY(c.start(format));c.selectRow(static_cast<int>(problem.constraints.size())+4);
            const auto value=c.session()->result().objective;QVERIFY(printGraph(printer,c,shell.font()));
            QCOMPARE(c.session()->result().objective,value);
        } else {
            auto& c=simplexController(shell);c.notice=[](const QString&,const QString&){};
            c.redirect=[](const QString&){return true;};c.singleWarning=[](const QString&){};
            if(method==QStringLiteral("all_slack"))QVERIFY(c.requestAllSlack());
            else if(method==QStringLiteral("m_method")) {QVERIFY(c.requestMMethod());QVERIFY(c.setPenalty(QStringLiteral("100")));}
            else if(method==QStringLiteral("two_phase"))QVERIFY(c.requestTwoPhase());
            else if(method==QStringLiteral("bounded"))QVERIFY(c.requestBounded());
            else if(method==QStringLiteral("dual_simplex"))QVERIFY(c.requestDual());
            else QFAIL("Unknown printing method");
            QVERIFY(c.start(format));c.all();QTRY_VERIFY_WITH_TIMEOUT(!c.busy(),10000);
            const auto count=c.session()->history().size();const auto values=c.session()->history().back().tableau.values().data();
            QVERIFY(printSimplexTrace(printer,c,shell.font()));QCOMPARE(c.session()->history().size(),count);QCOMPARE(c.session()->history().back().tableau.values().data(),values);
        }
        QFile pdf(printer.outputFileName());QVERIFY(pdf.open(QIODevice::ReadOnly));QCOMPARE(pdf.read(5),QByteArray("%PDF-"));
        QCOMPARE(tora::compat::ToraLpCodec::encode(*input.document()),before);
    }
};
int main(int argc,char** argv) {
#ifdef Q_OS_WIN
    qputenv("QT_QPA_FONTDIR",qgetenv("SystemRoot")+"/Fonts");
#endif
    QApplication app(argc,argv);
    if(argc==2 && QByteArray(argv[1])==QByteArray("--probe-default-printer")) {
        const auto info=QPrinterInfo::defaultPrinter();if(info.isNull())return 3;
        bool invoked=false;
        const auto result=requestLpPrint(nullptr,[&](QPrinter& printer){invoked=true;std::printf("Direct default printer callback: %s\n",qPrintable(printer.printerName()));return printer.printerName()==info.printerName();},info);
        return invoked && result==PrintOutcome::Complete?0:4;
    }
    DesktopPrinting tests;return QTest::qExec(&tests,argc,argv);
}
#include "desktop_printing.moc"
