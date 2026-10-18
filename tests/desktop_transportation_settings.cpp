#include "application_shell.hpp"
#include "transportation_screens.hpp"
#include <QApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTest>
using namespace tora::desktop;
class TransportationSettingsTest:public QObject {
    Q_OBJECT
private slots:
    void observedFormats_data() {
        QTest::addColumn<QJsonObject>("record");QFile file(QStringLiteral(TORA_SETTINGS_GOLDENS));QVERIFY(file.open(QIODevice::ReadOnly));
        for(const auto value:QJsonDocument::fromJson(file.readAll()).array()) {
            const auto r=value.toObject();QTest::newRow(qPrintable(r["case"].toString()))<<r;
        }
    }
    void observedFormats() {
        QFETCH(QJsonObject,record);const auto input=record["problem"].toObject();const auto settings=record["format"].toArray();
        const auto supply=input["supply"].toArray(),demand=input["demand"].toArray();
        auto p=tora::transportation::Problem::blank(static_cast<std::size_t>(supply.size()),static_cast<std::size_t>(demand.size()));
        p.title=record["title"].toString().toStdString();p.format={settings[2].toInt()==2?tora::core::Notation::Scientific:tora::core::Notation::Decimal,settings[0].toInt(),settings[1].toInt()};
        for(int i=0;i<supply.size();++i){p.supply[static_cast<std::size_t>(i)]=supply[i].toDouble();for(int j=0;j<demand.size();++j)p.costs[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]=input["costs"].toArray()[i].toArray()[j].toDouble();}
        for(int j=0;j<demand.size();++j)p.demand[static_cast<std::size_t>(j)]=demand[j].toDouble();
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);c.load(p);QString error;QVERIFY(c.solve(error));
        QVERIFY(shell.navigate(QStringLiteral("transportation/northwest")));shell.show();QTest::qWait(10);
        auto* grid=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("transport-tableau"));QVERIFY(grid);
        const auto expected=record["expected_initial"].toObject();QCOMPARE(grid->item(0,2)->text(),expected["objective"].toString());
        for(int j=0;j<demand.size();++j)QCOMPARE(grid->item(2,j+3)->text(),expected["v"].toArray()[j].toString());
        for(int i=0;i<supply.size();++i) {
            QCOMPARE(grid->item(3+3*i,2)->text(),expected["u"].toArray()[i].toString());
            for(int j=0;j<demand.size();++j) {
                QCOMPARE(grid->item(3+3*i,j+3)->text(),expected["costs"].toArray()[i].toArray()[j].toString());
                QCOMPARE(grid->item(4+3*i,j+3)->text(),expected["shipments"].toArray()[i].toArray()[j].toString());
                QCOMPARE(grid->item(5+3*i,j+3)->text(),expected["reduced_costs"].toArray()[i].toArray()[j].toString());
            }
        }
        const auto bytes=c.document()->encode();QVERIFY(c.computeFinal(error));QVERIFY(shell.navigate(QStringLiteral("transportation/final")));
        grid=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("transport-final-grid"));QVERIFY(grid);
        const auto rows=record["expected_final"].toObject()["rows"].toArray();QCOMPARE(grid->rowCount(),rows.size()+1);
        for(int i=0;i<rows.size();++i)for(int j=0;j<5;++j)QCOMPARE(grid->item(i+1,j)->text(),rows[i].toArray()[j].toString());
        QCOMPARE(c.document()->encode(),bytes);
    }
    void newControlsAndInvalidFormatPreserveInput() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);
        QVERIFY(shell.navigate(QStringLiteral("transportation/input-mode")));auto* page=shell.currentContent();
        auto* n=page->findChild<QLineEdit*>(QStringLiteral("transport-format-n"));auto* d=page->findChild<QLineEdit*>(QStringLiteral("transport-format-d"));
        auto* sci=page->findChild<QRadioButton*>(QStringLiteral("transport-scientific"));QVERIFY(n);QVERIFY(d);QVERIFY(sci);
        QCOMPARE(n->text(),QStringLiteral("5"));QCOMPARE(d->text(),QStringLiteral("2"));
        sci->click();n->setText(QStringLiteral("2"));d->setText(QStringLiteral("3"));page->findChild<QPushButton*>(QStringLiteral("transport-new"))->click();
        QString error;QVERIFY(c.initialize(QStringLiteral("New scientific"),2,3,error));QCOMPARE(c.problem().format.notation,tora::core::Notation::Scientific);
        QCOMPARE(c.document()->cell(1,1),std::string("0.00e+000"));const auto before=c.document()->encode();
        c.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(shell.navigate(QStringLiteral("transportation/input-mode")));
        int errors=0;c.dialogs().showError=[&](const QString&,const QString&){++errors;};n->setText(QStringLiteral("100"));
        page->findChild<QPushButton*>(QStringLiteral("transport-new"))->click();QCOMPARE(errors,1);QCOMPARE(c.document()->encode(),before);
    }
    void structuralUndoAndReinitializeRetainFormat() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);
        c.beginNew({tora::core::Notation::Scientific,2,3});QString error;QVERIFY(c.initialize(QStringLiteral("Draft"),2,3,error));
        const auto before=c.document()->encode();QVERIFY(c.selectLine(TransportationController::Axis::Destination,1));QVERIFY(c.insertSelection(error));
        QCOMPARE(c.document()->cell(1,3),std::string("0.00e+000"));QVERIFY(c.undo(error));QCOMPARE(c.document()->encode(),before);
        QVERIFY(c.initialize(QStringLiteral("Reinitialized"),3,2,error));QCOMPARE(c.problem().format.n,2);QCOMPARE(c.problem().format.d,3);
        QTemporaryDir temp;const auto file=temp.filePath(QStringLiteral("scientific.txt"));QVERIFY(c.saveTo(file,error));c.loadExample();QVERIFY(c.openFile(file,error));
        QCOMPARE(c.problem().format.notation,tora::core::Notation::Scientific);QCOMPARE(c.problem().format.n,2);
    }
};
int main(int argc,char** argv) {
#ifdef Q_OS_WIN
    qputenv("QT_QPA_FONTDIR",qgetenv("SystemRoot")+"/Fonts");
#endif
    QApplication app(argc,argv);TransportationSettingsTest tests;return QTest::qExec(&tests,argc,argv);
}
#include "desktop_transportation_settings.moc"
