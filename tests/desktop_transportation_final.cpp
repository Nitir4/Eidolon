#include "application_shell.hpp"
#include "transportation_screens.hpp"
#include "ui_font.hpp"
#include <QAction>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTest>
using namespace tora::desktop;
class TransportationFinalTest:public QObject {
    Q_OBJECT
private slots:
    void initTestCase() {
        const auto font=readableUiFont(QApplication::font());
        for(const auto c:QStringLiteral("S1:D3: 0.125 -70.00 minimum cost Alpha"))QVERIFY(QFontMetrics(font).inFont(c));
    }
    void observedSummaries_data() {
        QTest::addColumn<QByteArray>("record");QFile file(QStringLiteral(TORA_FINAL_GOLDENS));QVERIFY(file.open(QIODevice::ReadOnly));
        for(const auto value:QJsonDocument::fromJson(file.readAll()).array()) {
            const auto r=value.toObject();QTest::newRow(r["case"].toString().toUtf8().constData())<<QJsonDocument(r).toJson(QJsonDocument::Compact);
        }
    }
    void observedSummaries() {
        QFETCH(QByteArray,record);const auto r=QJsonDocument::fromJson(record).object(),input=r["problem"].toObject(),expected=r["expected"].toObject();
        const auto supply=input["supply"].toArray(),demand=input["demand"].toArray();
        auto p=tora::transportation::Problem::blank(static_cast<std::size_t>(supply.size()),static_cast<std::size_t>(demand.size()));p.title=r["title"].toString().toStdString();
        for(int i=0;i<supply.size();++i){p.supply[static_cast<std::size_t>(i)]=supply[i].toDouble();for(int j=0;j<demand.size();++j)p.costs[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]=input["costs"].toArray()[i].toArray()[j].toDouble();}
        for(int j=0;j<demand.size();++j)p.demand[static_cast<std::size_t>(j)]=demand[j].toDouble();
        for(int i=0;i<input["source_names"].toArray().size();++i)p.sourceNames[static_cast<std::size_t>(i)]=input["source_names"].toArray()[i].toString().toStdString();
        for(int j=0;j<input["destination_names"].toArray().size();++j)p.destinationNames[static_cast<std::size_t>(j)]=input["destination_names"].toArray()[j].toString().toStdString();
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);c.load(std::move(p));QString error;QVERIFY2(c.computeFinal(error),qPrintable(error));
        QVERIFY(shell.navigate(QStringLiteral("transportation/final")));shell.show();QTest::qWait(10);
        auto* page=shell.currentContent();
        QCOMPARE(page->findChild<QLabel*>(QStringLiteral("transport-final-title"))->text(),QStringLiteral("Title: ")+r["title"].toString());
        QCOMPARE(page->findChild<QLabel*>(QStringLiteral("transport-final-iteration"))->text(),QStringLiteral("Final Iteration No.: ")+expected["iteration"].toString());
        QCOMPARE(page->findChild<QLabel*>(QStringLiteral("transport-final-objective"))->text(),QStringLiteral("Objective Value (minimum cost) =")+expected["objective"].toString());
        auto* grid=page->findChild<QTableWidget*>(QStringLiteral("transport-final-grid"));QVERIFY(grid);const auto rows=expected["rows"].toArray();
        QCOMPARE(grid->rowCount(),rows.size()+1);QCOMPARE(grid->columnCount(),5);
        const QStringList headers{QStringLiteral("From"),QStringLiteral("To"),QStringLiteral("Amt Shipped"),QStringLiteral("Obj Coeff"),QStringLiteral("Obj Contrib")};
        for(int col=0;col<5;++col)QCOMPARE(grid->item(0,col)->text(),headers[col]);
        for(int row=0;row<rows.size();++row)for(int col=0;col<5;++col)QCOMPARE(grid->item(row+1,col)->text(),rows[row].toArray()[col].toString());
        QVERIFY(!page->findChild<QPushButton*>(QStringLiteral("transport-final-next"))->isEnabled());
        QVERIFY(!page->findChild<QPushButton*>(QStringLiteral("transport-final-all"))->isEnabled());
    }
    void controllerPreservationAndInvalidation() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);c.loadExample();QString error;QVERIFY(c.solve(error,tora::transportation::StartMethod::Vogel));
        const auto bytes=c.document()->encode();const auto traceCount=c.trace().size();QVERIFY(c.computeFinal(error));QVERIFY(c.finalOutput());
        QCOMPARE(c.method(),tora::transportation::StartMethod::Vogel);QCOMPARE(c.trace().size(),traceCount);QCOMPARE(c.document()->encode(),bytes);QVERIFY(!c.dirty());
        const auto rows=c.finalOutput()->rows;
        const auto connection=QObject::connect(&c,&TransportationController::commitRequested,&shell,[&]{QString ignored;c.editCell(1,1,QStringLiteral("nan"),ignored);});
        QVERIFY(!c.computeFinal(error));QCOMPARE(c.finalOutput()->rows,rows);QObject::disconnect(connection);
        QVERIFY(!c.openFile(QStringLiteral("/path/that/does/not/exist.txt"),error));QCOMPARE(c.finalOutput()->rows,rows);
        QVERIFY(c.editCell(1,1,QStringLiteral("3"),error));QVERIFY(!c.finalOutput());
        c.loadExample();QVERIFY(c.computeFinal(error));c.beginNew();QVERIFY(!c.finalOutput());QVERIFY(!c.computeFinal(error));
    }
    void directMenuCancellation() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);c.loadExample();QString error;
        QVERIFY(shell.navigate(QStringLiteral("transportation/input")));shell.show();QVERIFY(c.setTitle(QStringLiteral("Unsaved"),error));
        c.dialogs().askSave=[] {return SaveChoice::Cancel;};
        auto* final=shell.findChild<QAction*>(QStringLiteral("transport-final-action"));QVERIFY(final);final->trigger();
        QCOMPARE(shell.currentScreen(),QStringLiteral("transportation/input"));QVERIFY(!c.finalOutput());QVERIFY(c.trace().empty());
        c.dialogs().askSave=[] {return SaveChoice::No;};final->trigger();QCOMPARE(shell.currentScreen(),QStringLiteral("transportation/final"));QVERIFY(c.finalOutput());
    }
    void resolutionPreview() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);c.loadExample();QString error;QVERIFY(c.computeFinal(error));
        QVERIFY(shell.navigate(QStringLiteral("transportation/final")));shell.show();
        for(const auto physical:{QSize(1366,768),QSize(1920,1080),QSize(3840,2160)}) {
            const auto scale=shell.devicePixelRatioF();shell.resize(qRound(physical.width()/scale),qRound(physical.height()/scale));QTest::qWait(20);
            for(auto* b:shell.currentContent()->findChildren<QPushButton*>())QVERIFY(b->width()>=b->minimumSizeHint().width());
            auto* grid=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("transport-final-grid"));QVERIFY(grid);
            for(int row=0;row<grid->rowCount();++row)for(int col=0;col<grid->columnCount();++col)QVERIFY(grid->columnWidth(col)>grid->fontMetrics().horizontalAdvance(grid->item(row,col)->text()));
            const auto base=qEnvironmentVariable("TORA_UI_CAPTURES");if(!base.isEmpty()) {const auto folder=base+QStringLiteral("/transportation-final");QVERIFY(QDir().mkpath(folder));QVERIFY(shell.grab().save(folder+QStringLiteral("/final-%1x%2-%3.png").arg(physical.width()).arg(physical.height()).arg(scale)));}
        }
    }
};
int main(int argc,char** argv) {
#ifdef Q_OS_WIN
    qputenv("QT_QPA_FONTDIR",qgetenv("SystemRoot")+"/Fonts");
#endif
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc,argv);TransportationFinalTest test;return QTest::qExec(&test,argc,argv);
}
#include "desktop_transportation_final.moc"
