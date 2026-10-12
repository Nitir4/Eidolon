#include "application_shell.hpp"
#include "transportation_screens.hpp"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTest>
#include <QTimer>
#include <QTemporaryDir>
#include <QDir>
#include <QScreen>
#include <QResizeEvent>

using namespace tora::desktop;
class DesktopTransportation: public QObject {
    Q_OBJECT
private slots:
    void goldenNativeTableaus_data() {
        QTest::addColumn<QByteArray>("record");
        for(const auto& method:{QStringLiteral("northwest"),QStringLiteral("leastcost"),QStringLiteral("vogel")}) {
            QFile file(method==QStringLiteral("northwest")?QStringLiteral(TORA_TRANSPORT_GOLDENS):(method==QStringLiteral("leastcost")?QStringLiteral(TORA_LEASTCOST_GOLDENS):QStringLiteral(TORA_VOGEL_GOLDENS)));
            QVERIFY(file.open(QIODevice::ReadOnly));
            for(const auto& value:QJsonDocument::fromJson(file.readAll()).array()) {
                auto object=value.toObject();object["method"]=method;
                QTest::newRow((method+QLatin1Char('/')+object["case"].toString()).toUtf8().constData())<<QJsonDocument(object).toJson(QJsonDocument::Compact);
            }
        }
    }
    void goldenNativeTableaus() {
        QFETCH(QByteArray,record);const auto object=QJsonDocument::fromJson(record).object();
        const auto input=object["problem"].toObject();const auto expected=object["tables"].toArray();
        const auto supply=input["supply"].toArray(),demand=input["demand"].toArray();
        auto p=tora::transportation::Problem::blank(static_cast<std::size_t>(supply.size()),static_cast<std::size_t>(demand.size()));
        for(int i=0;i<supply.size();++i) {
            p.supply[static_cast<std::size_t>(i)]=supply[i].toDouble();
            for(int j=0;j<demand.size();++j)p.costs[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]=input["costs"].toArray()[i].toArray()[j].toDouble();
        }
        for(int j=0;j<demand.size();++j)p.demand[static_cast<std::size_t>(j)]=demand[j].toDouble();
        ApplicationShell shell;QVERIFY(shell.registerModule(transportationModule()));
        auto& controller=transportationController(shell);controller.load(std::move(p));
        QVERIFY(shell.navigate(QStringLiteral("transportation/input")));
        auto* inputGrid=shell.findChild<QTableWidget*>(QStringLiteral("transport-input-grid"));QVERIFY(inputGrid);
        QCOMPARE(inputGrid->editTriggers(),QAbstractItemView::NoEditTriggers);
        const auto method=object["method"].toString();
        QString error;QVERIFY2(controller.solve(error,method==QStringLiteral("vogel")?tora::transportation::StartMethod::Vogel:method==QStringLiteral("leastcost")?tora::transportation::StartMethod::LeastCost:tora::transportation::StartMethod::NorthWest),qPrintable(error));
        QVERIFY(shell.navigate(QStringLiteral("transportation/")+method));shell.show();
        auto* heading=shell.currentContent()->findChild<QLabel*>(QStringLiteral("transport-method-heading"));QVERIFY(heading);
        QCOMPARE(heading->text(),method==QStringLiteral("vogel")?QStringLiteral("TRANSPORTATION TABLEAU - (Vogel's Method)"):method==QStringLiteral("leastcost")?QStringLiteral("TRANSPORTATION TABLEAU - (Least Cost Method)"):QStringLiteral("TRANSPORTATION TABLEAU - (North-West Corner Method)"));
        auto* all=shell.findChild<QPushButton*>(QStringLiteral("transport-all"));QVERIFY(all);
        auto* next=shell.findChild<QPushButton*>(QStringLiteral("transport-next"));QVERIFY(next);
        if(expected.size()>1) {
            QTimer::singleShot(0,[] {for(auto* widget:QApplication::topLevelWidgets())if(auto* box=qobject_cast<QMessageBox*>(widget))box->accept();});
            all->click();
        }
        QVERIFY(!all->isEnabled());QVERIFY(!next->isEnabled());
        auto* grid=shell.findChild<QTableWidget*>(QStringLiteral("transport-tableau"));QVERIFY(grid);
        const int height=3*supply.size()+4;
        QCOMPARE(grid->rowCount(),height*expected.size());
        for(int k=0;k<expected.size();++k) {
            const auto e=expected[k].toObject();const int base=k*height;
            QCOMPARE(grid->item(base,0)->text(),QStringLiteral("Iter %1").arg(k+1));
            QCOMPARE(grid->item(base,2)->text(),e["objective"].toString());
            for(int j=0;j<demand.size();++j) {
                QCOMPARE(grid->item(base+2,j+3)->text(),e["v"].toArray()[j].toString());
                QCOMPARE(grid->item(base+height-1,j+3)->text(),e["demand"].toArray()[j].toString());
            }
            for(int i=0;i<supply.size();++i) {
                const int row=base+3+3*i;
                QCOMPARE(grid->item(row,2)->text(),e["u"].toArray()[i].toString());
                QCOMPARE(grid->item(row,demand.size()+3)->text(),e["supply"].toArray()[i].toString());
                for(int j=0;j<demand.size();++j)for(int offset=0;offset<3;++offset) {
                    const char* key=offset==0?"costs":offset==1?"shipments":"reduced_costs";
                    QCOMPARE(grid->item(row+offset,j+3)->text(),e[key].toArray()[i].toArray()[j].toString());
                }
            }
        }
        QVERIFY(grid->font().pointSizeF()>=12);
        for(int row=0;row<grid->rowCount();++row)for(int col=0;col<grid->columnCount();++col) {
            const auto* item=grid->item(row,col);if(!item)continue;
            const QFontMetrics metrics(item->data(Qt::FontRole).isValid()?item->font():grid->font());
            QVERIFY2(grid->columnWidth(col)>metrics.horizontalAdvance(item->text()),qPrintable(item->text()));
        }
        QVERIFY(shell.navigate(QStringLiteral("main")));
        auto* button=shell.findChild<QPushButton*>(QStringLiteral("Transportation model"));QVERIFY(button);QVERIFY(button->isEnabled());
    }
    void failedSolvePreservesInput() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);
        auto p=tora::transportation::Problem::blank(2,3);p.supply={0.3,0.7};p.demand={0.2,0.3,0.5};c.load(p);
        QString error;QVERIFY(!c.solve(error));QVERIFY(!error.isEmpty());QCOMPARE(c.problem().supply[0],0.3);QVERIFY(c.trace().empty());
    }
    void nextAndReload() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);
        c.loadExample();QString error;QVERIFY(c.solve(error));
        QVERIFY(shell.navigate(QStringLiteral("transportation/northwest")));shell.show();
        auto* next=shell.findChild<QPushButton*>(QStringLiteral("transport-next"));QVERIFY(next);
        auto* grid=shell.findChild<QTableWidget*>(QStringLiteral("transport-tableau"));QVERIFY(grid);
        QCOMPARE(grid->rowCount(),10);QVERIFY(next->isEnabled());
        next->click();QCOMPARE(grid->rowCount(),20);QVERIFY(next->isEnabled());
        QTimer::singleShot(0,[]{for(auto* w:QApplication::topLevelWidgets())if(auto* b=qobject_cast<QMessageBox*>(w))b->accept();});
        next->click();QCOMPARE(grid->rowCount(),30);QVERIFY(!next->isEnabled());
        QVERIFY(shell.navigate(QStringLiteral("transportation/input")));
        auto* input=shell.findChild<QTableWidget*>(QStringLiteral("transport-input-grid"));QVERIFY(input);
        input->setCurrentCell(2,2);input->setFocus();QTest::keyClick(input,Qt::Key_Right);QCOMPARE(input->currentColumn(),3);
        QTest::keyClick(input,Qt::Key_Down);QCOMPARE(input->currentRow(),3);
        auto p=tora::transportation::Problem::blank(1,1);p.costs={{3}};p.supply={4};p.demand={4};c.load(p);
        QCOMPARE(input->rowCount(),4);QVERIFY(c.solve(error));
        QVERIFY(shell.navigate(QStringLiteral("transportation/northwest")));QCOMPARE(grid->rowCount(),7);QVERIFY(!next->isEnabled());
    }
    void failedOpenPreservesModelAndTrace() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);
        c.loadExample();QString error;QVERIFY(c.solve(error));
        QTemporaryDir directory;QVERIFY(directory.isValid());
        const auto path=directory.filePath(QStringLiteral("bad.txt"));QFile file(path);QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("4000\r\n");file.close();
        QVERIFY(!c.openFile(path,error));QVERIFY(!error.isEmpty());QCOMPARE(c.problem().supply[0],20.0);QCOMPARE(c.trace().size(),std::size_t(3));
        QVERIFY(!c.openFile(directory.filePath(QStringLiteral("missing.txt")),error));QVERIFY(!error.isEmpty());
        QCOMPARE(c.trace().back().objective,150.0);
    }
    void resizeDuringColumnShrink() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);c.loadExample();
        QVERIFY(shell.navigate(QStringLiteral("transportation/input")));shell.show();
        auto* grid=shell.findChild<QTableWidget*>(QStringLiteral("transport-input-grid"));QVERIFY(grid);
        // Windows can resize synchronously between dimension replacement and
        // the new destination-column configuration. Isolate that ordering.
        grid->setColumnCount(4);
        QResizeEvent event(grid->viewport()->size(),grid->viewport()->size());
        try {QApplication::sendEvent(grid->viewport(),&event);}catch(const std::exception& e){QFAIL(e.what());}
        auto p=tora::transportation::Problem::blank(1,1);p.supply={4};p.demand={4};p.costs={{3}};c.load(p);
        QCOMPARE(grid->columnCount(),4);QCOMPARE(grid->item(2,2)->text(),QStringLiteral("3.00"));
    }
    void switchStartingMethods() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);
        c.loadExample();QVERIFY(shell.navigate(QStringLiteral("transportation/solve-menu")));shell.show();
        auto* least=shell.findChild<QPushButton*>(QStringLiteral("transport-leastcost-start"));QVERIFY(least);
        QTimer::singleShot(0,[]{for(auto* w:QApplication::topLevelWidgets())if(auto* box=qobject_cast<QMessageBox*>(w))box->accept();});
        least->click();
        QCOMPARE(shell.currentScreen(),QStringLiteral("transportation/leastcost"));
        QCOMPARE(c.method(),tora::transportation::StartMethod::LeastCost);QCOMPARE(c.trace().size(),std::size_t(1));
        QCOMPARE(c.trace().front().objective,150.0);
        QVERIFY(shell.navigate(QStringLiteral("transportation/solve-menu")));
        auto* north=shell.findChild<QPushButton*>(QStringLiteral("transport-northwest-start"));QVERIFY(north);north->click();
        QCOMPARE(shell.currentScreen(),QStringLiteral("transportation/northwest"));
        QCOMPARE(c.method(),tora::transportation::StartMethod::NorthWest);QCOMPARE(c.trace().size(),std::size_t(3));
        QCOMPARE(c.trace().front().objective,230.0);
        QVERIFY(shell.navigate(QStringLiteral("transportation/solve-menu")));
        auto* vogel=shell.findChild<QPushButton*>(QStringLiteral("transport-vogel-start"));QVERIFY(vogel);
        QTimer::singleShot(0,[]{for(auto* w:QApplication::topLevelWidgets())if(auto* box=qobject_cast<QMessageBox*>(w))box->accept();});
        vogel->click();
        QCOMPARE(shell.currentScreen(),QStringLiteral("transportation/vogel"));
        QCOMPARE(c.method(),tora::transportation::StartMethod::Vogel);QCOMPARE(c.trace().size(),std::size_t(1));
        QString error;QVERIFY(!c.solve(error,static_cast<tora::transportation::StartMethod>(999)));
        QCOMPARE(c.method(),tora::transportation::StartMethod::Vogel);QCOMPARE(c.trace().size(),std::size_t(1));
    }
    void initialOptimalNotice_data() {
        QTest::addColumn<QString>("method");
        for(const auto& method:{QStringLiteral("northwest"),QStringLiteral("leastcost"),QStringLiteral("vogel")})
            QTest::newRow(qPrintable(method))<<method;
    }
    void initialOptimalNotice() {
        QFETCH(QString,method);ApplicationShell shell;shell.registerModule(transportationModule());
        auto& c=transportationController(shell);c.loadExample();
        if(method==QStringLiteral("northwest")) {auto p=c.problem();p.costs={{1,1,1},{1,1,1}};c.load(std::move(p));}
        QVERIFY(shell.navigate(QStringLiteral("transportation/solve-menu")));shell.show();
        bool seen=false;QString title,message;QMessageBox::Icon icon=QMessageBox::NoIcon;
        QMessageBox::StandardButtons buttons=QMessageBox::NoButton;
        QTimer notice;notice.setSingleShot(true);
        QObject::connect(&notice,&QTimer::timeout,&shell,[&] {
            for(auto* widget:QApplication::topLevelWidgets())if(auto* box=qobject_cast<QMessageBox*>(widget)) {
                seen=true;title=box->windowTitle();message=box->text();icon=box->icon();buttons=box->standardButtons();box->accept();
            }
        });
        auto* start=shell.findChild<QPushButton*>(QStringLiteral("transport-")+method+QStringLiteral("-start"));
        QVERIFY(start);notice.start(0);start->click();notice.stop();
        QVERIFY2(seen,"Missing observed initial optimal-tableau notice");
#ifdef Q_OS_MACOS
        // QMessageBox ignores titles on macOS by documented platform design.
        QVERIFY(title.isEmpty());
#else
        QCOMPARE(title,QStringLiteral("Optimal Tableau"));
#endif
        QCOMPARE(message,QStringLiteral("Iteration 1 is optimal"));
        QCOMPARE(icon,QMessageBox::Information);QCOMPARE(buttons,QMessageBox::StandardButtons(QMessageBox::Ok));
        QCOMPARE(c.trace().size(),std::size_t(1));
        auto* next=shell.currentContent()->findChild<QPushButton*>(QStringLiteral("transport-next"));QVERIFY(next);QVERIFY(!next->isEnabled());
    }
    void resolutionPreview_data() {
        QTest::addColumn<QSize>("physical");QTest::addColumn<QString>("method");
        for(const auto& method:{QStringLiteral("northwest"),QStringLiteral("leastcost"),QStringLiteral("vogel")})
            for(const auto& physical:{QSize(1366,768),QSize(1920,1080),QSize(3840,2160)})
                QTest::newRow(QStringLiteral("%1-%2x%3").arg(method).arg(physical.width()).arg(physical.height()).toUtf8().constData())<<physical<<method;
    }
    void resolutionPreview() {
        QFETCH(QSize,physical);QFETCH(QString,method);ApplicationShell shell;shell.registerModule(transportationModule());
        auto& c=transportationController(shell);c.loadExample();QString error;
        QVERIFY(c.solve(error,method==QStringLiteral("vogel")?tora::transportation::StartMethod::Vogel:method==QStringLiteral("leastcost")?tora::transportation::StartMethod::LeastCost:tora::transportation::StartMethod::NorthWest));
        const auto scale=shell.devicePixelRatioF();const QSize logical(qRound(physical.width()/scale),qRound(physical.height()/scale));
        shell.resize(logical);QVERIFY(shell.navigate(QStringLiteral("transportation/")+method));shell.show();QTest::qWait(30);
        QCOMPARE(shell.size(),logical);
        auto* grid=shell.findChild<QTableWidget*>(QStringLiteral("transport-tableau"));QVERIFY(grid);QVERIFY(grid->isVisible());
        for(auto* button:shell.currentContent()->findChildren<QPushButton*>())QVERIFY(button->width()>=button->minimumSizeHint().width());
        const auto base=qEnvironmentVariable("TORA_UI_CAPTURES");
        if(!base.isEmpty()) {
            const auto directory=base+QStringLiteral("/transportation");QVERIFY(QDir().mkpath(directory));
            QVERIFY(shell.grab().save(directory+QStringLiteral("/%1-%2x%3-%4.png").arg(method).arg(physical.width()).arg(physical.height()).arg(scale)));
        }
    }
};
QTEST_MAIN(DesktopTransportation)
#include "desktop_transportation.moc"
