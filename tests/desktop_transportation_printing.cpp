#include "application_shell.hpp"
#include "transportation_screens.hpp"
#include "transportation_printing.hpp"
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

using namespace tora::desktop;
namespace {
tora::transportation::Problem problem(const QJsonObject& record) {
    const auto input=record["problem"].toObject();
    const auto supply=input["supply"].toArray(),demand=input["demand"].toArray();
    auto p=tora::transportation::Problem::blank(static_cast<std::size_t>(supply.size()),static_cast<std::size_t>(demand.size()));
    p.title=record["title"].toString().toStdString();
    for(int i=0;i<supply.size();++i) {
        p.supply[static_cast<std::size_t>(i)]=supply[i].toDouble();
        for(int j=0;j<demand.size();++j)p.costs[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]=input["costs"].toArray()[i].toArray()[j].toDouble();
    }
    for(int j=0;j<demand.size();++j)p.demand[static_cast<std::size_t>(j)]=demand[j].toDouble();
    for(int i=0;i<input["source_names"].toArray().size();++i)p.sourceNames[static_cast<std::size_t>(i)]=input["source_names"].toArray()[i].toString().toStdString();
    for(int j=0;j<input["destination_names"].toArray().size();++j)p.destinationNames[static_cast<std::size_t>(j)]=input["destination_names"].toArray()[j].toString().toStdString();
    return p;
}
void configurePdf(QPrinter& printer,const QString& filename) {
    printer.setResolution(300);printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize(QPageSize::A4));printer.setOutputFileName(filename);
}
}
class TransportationPrintingTest:public QObject {
    Q_OBJECT
private slots:
    void documents_data() {
        QTest::addColumn<QJsonObject>("record");QFile file(QStringLiteral(TORA_TRANSPORT_PRINT_GOLDENS));QVERIFY(file.open(QIODevice::ReadOnly));
        for(const auto value:QJsonDocument::fromJson(file.readAll()).array()) {
            const auto r=value.toObject();QTest::newRow(qPrintable(r["case"].toString()))<<r;
        }
    }
    void documents() {
        QFETCH(QJsonObject,record);
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);c.load(problem(record));
        QString error;const auto method=record["method"].toString();
        if(method==QStringLiteral("final"))QVERIFY2(c.computeFinal(error),qPrintable(error));
        else {
            QVERIFY2(c.solve(error,method==QStringLiteral("vogel")?tora::transportation::StartMethod::Vogel:
                method==QStringLiteral("leastcost")?tora::transportation::StartMethod::LeastCost:tora::transportation::StartMethod::NorthWest),qPrintable(error));
            const auto anchor=record["anchor"].toObject();
            if(!anchor.isEmpty())QVERIFY(c.initializeMultiplier({tora::transportation::MultiplierAxis::Destination,static_cast<std::size_t>(anchor["index"].toInt())},error));
        }
        const auto bytes=c.document()->encode();const auto trace=c.trace();
        const auto final=c.finalOutput()?std::optional<tora::transportation::FinalSummary>(*c.finalOutput()):std::nullopt;
        const auto folder=QStringLiteral(TORA_TRANSPORT_PRINT_PDFS);QVERIFY(QDir().mkpath(folder));
        const auto jobs=record["jobs"].toArray();
        for(int i=0;i<jobs.size();++i) {
            QPrinter printer(QPrinter::HighResolution);configurePdf(printer,folder+QLatin1Char('/')+record["case"].toString()+QStringLiteral("-%1.pdf").arg(i+1));
            if(QFile::exists(printer.outputFileName()))QVERIFY(QFile::remove(printer.outputFileName()));
            const auto role=jobs[i].toObject()["role"].toString();
            const bool ok=role==QStringLiteral("input")?printTransportationInput(printer,c.problem(),true):
                role==QStringLiteral("summary")?printTransportationSummary(printer,*c.finalOutput()):
                printTransportationTrace(printer,c,static_cast<std::size_t>(record["shown"].toInt()));
            QVERIFY(ok);QFile pdf(printer.outputFileName());QVERIFY(pdf.open(QIODevice::ReadOnly));QCOMPARE(pdf.read(5),QByteArray("%PDF-"));
        }
        QCOMPARE(c.document()->encode(),bytes);QCOMPARE(c.trace().size(),trace.size());
        for(std::size_t i=0;i<trace.size();++i) {
            QCOMPARE(c.trace()[i].shipments,trace[i].shipments);QCOMPARE(c.trace()[i].u,trace[i].u);
            QCOMPARE(c.trace()[i].v,trace[i].v);QCOMPARE(c.trace()[i].reducedCosts,trace[i].reducedCosts);QCOMPARE(c.trace()[i].objective,trace[i].objective);
        }
        if(final){QVERIFY(c.finalOutput());QCOMPARE(c.finalOutput()->rows,final->rows);QCOMPARE(c.finalOutput()->objective,final->objective);}
    }
    void finalJobBoundaryAndCancellation() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);c.loadExample();QString error;QVERIFY(c.computeFinal(error));
        QTemporaryDir temp;int selections=0;
        const auto select=[&](QPrinter& printer,QWidget*) {configurePdf(printer,temp.filePath(QStringLiteral("job-%1.pdf").arg(++selections)));return true;};
        QCOMPARE(requestTransportationFinalPrint(nullptr,c,QPrinterInfo(),select),PrintOutcome::Complete);QCOMPARE(selections,2);
        QVERIFY(QFile::exists(temp.filePath(QStringLiteral("job-1.pdf"))));QVERIFY(QFile::exists(temp.filePath(QStringLiteral("job-2.pdf"))));
        selections=0;
        QCOMPARE(requestTransportationFinalPrint(nullptr,c,QPrinterInfo(),[&](QPrinter&,QWidget*){++selections;return false;}),PrintOutcome::Cancelled);QCOMPARE(selections,1);
        selections=0;
        QCOMPARE(requestTransportationFinalPrint(nullptr,c,QPrinterInfo(),[&](QPrinter& printer,QWidget*) {
            if(++selections==2)return false;configurePdf(printer,temp.filePath(QStringLiteral("partial-input.pdf")));return true;
        }),PrintOutcome::Cancelled);QCOMPARE(selections,2);QVERIFY(QFile::exists(temp.filePath(QStringLiteral("partial-input.pdf"))));
        QCOMPARE(c.finalOutput()->objective,std::string("150.00"));
    }
    void unavailableAndInvalidShownCount() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);int calls=0;
        QCOMPARE(requestTransportationFinalPrint(nullptr,c,QPrinterInfo(),[&](QPrinter&,QWidget*){++calls;return true;}),PrintOutcome::Failed);QCOMPARE(calls,0);
        c.loadExample();QString error;QVERIFY(c.solve(error));QTemporaryDir temp;
        QPrinter printer;configurePdf(printer,temp.filePath(QStringLiteral("invalid.pdf")));
        QVERIFY(!printTransportationTrace(printer,c,0));QVERIFY(!printTransportationTrace(printer,c,c.trace().size()+1));QVERIFY(!QFile::exists(printer.outputFileName()));
    }
    void nativeCommands() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);
        QVERIFY(shell.navigate(QStringLiteral("transportation/final")));
        auto* command=shell.currentContent()->findChild<QPushButton*>(QStringLiteral("transport-final-print"));QVERIFY(command);QVERIFY(!command->isEnabled());
        c.loadExample();QString error;QVERIFY(c.computeFinal(error));QVERIFY(command->isEnabled());
        QVERIFY(c.solve(error));QVERIFY(shell.navigate(QStringLiteral("transportation/northwest")));
        command=shell.currentContent()->findChild<QPushButton*>(QStringLiteral("transport-print"));QVERIFY(command);QVERIFY(command->isEnabled());
        c.beginNew();QVERIFY(!command->isEnabled());
    }
    void invalidOutputAndSummary() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);c.loadExample();QString error;QVERIFY(c.computeFinal(error));
        const auto bytes=c.document()->encode();QTemporaryDir temp;
        QPrinter unavailable;configurePdf(unavailable,temp.filePath(QStringLiteral("missing/print.pdf")));
        QVERIFY(!printTransportationInput(unavailable,c.problem(),true));
        QPrinter printer;configurePdf(printer,temp.filePath(QStringLiteral("invalid-summary.pdf")));
        QVERIFY(!printTransportationSummary(printer,{}));
        auto malformed=*c.finalOutput();malformed.rows.front().pop_back();
        QVERIFY(!printTransportationSummary(printer,malformed));QVERIFY(!QFile::exists(printer.outputFileName()));
        QCOMPARE(c.document()->encode(),bytes);QCOMPARE(c.finalOutput()->objective,std::string("150.00"));
    }
};
int main(int argc,char** argv) {
#ifdef Q_OS_WIN
    qputenv("QT_QPA_FONTDIR",qgetenv("SystemRoot")+"/Fonts");
#endif
    QApplication app(argc,argv);TransportationPrintingTest tests;return QTest::qExec(&tests,argc,argv);
}
#include "desktop_transportation_printing.moc"
