#include "application_shell.hpp"
#include "transportation_screens.hpp"
#include "ui_font.hpp"
#include "ui_colors.hpp"
#include <QFontMetrics>
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
#include <QComboBox>
#include <QTabWidget>
#include <QPrinter>
#include "transportation_printing.hpp"
#include "ui_colors.hpp"

using namespace tora::desktop;
class DesktopTransportation: public QObject {
    Q_OBJECT
private slots:
    void unbalancedAllocationSteps() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);
        auto p=tora::transportation::Problem::blank(2,3);p.costs={{2,3,1},{5,4,8}};p.supply={20,30};p.demand={10,25,10};
        for(auto method:{tora::transportation::StartMethod::NorthWest,tora::transportation::StartMethod::LeastCost,tora::transportation::StartMethod::Vogel}) {
            c.load(p);const auto original=c.document()->encode();QString error;QVERIFY2(c.solve(error,method),qPrintable(error));
            QCOMPARE(c.problem().demand.size(),std::size_t(3));QCOMPARE(c.solutionProblem().demand.size(),std::size_t(4));
            QCOMPARE(c.solutionProblem().dummyDestinations,std::vector<std::size_t>{3});
            QVERIFY(shell.navigate(QStringLiteral("transportation/northwest")));shell.resize(1366,768);shell.show();QTest::qWait(20);
            auto* page=shell.currentContent();auto* tabs=page->findChild<QTabWidget*>("transport-phases");QVERIFY(tabs);
            for (int i = 0; i < tabs->count(); ++i) {
                const auto* panel = tabs->widget(i);
                QVERIFY(panel->autoFillBackground());
                QCOMPARE(panel->palette().color(QPalette::Window),QColor(colors::canvas));
            }
            auto* balancing=page->findChild<QLabel*>("transport-balancing");QVERIFY(balancing->text().contains("unused supply"));
            auto* modi=page->findChild<QTableWidget*>("transport-tableau");QCOMPARE(modi->columnCount(),8);
            tabs->setCurrentIndex(1);
            auto* next=page->findChild<QPushButton*>("transport-allocation-next");auto* previous=page->findChild<QPushButton*>("transport-allocation-previous");
            auto* all=page->findChild<QPushButton*>("transport-allocation-all");QVERIFY(next);QVERIFY(previous);QVERIFY(all);
            QVERIFY(!previous->isEnabled());next->click();QVERIFY(previous->isEnabled());previous->click();QVERIFY(!previous->isEnabled());all->click();QVERIFY(!next->isEnabled());
            auto* allocations=page->findChild<QTableWidget*>("transport-allocation-grid");QCOMPARE(allocations->columnCount(),7);
            auto* explanation=page->findChild<QLabel*>("transport-allocation-explanation");QVERIFY(explanation->text().contains("Initial basis complete"));
            QCOMPARE(c.document()->encode(),original);QVERIFY(!c.dirty());
            const auto captures=qEnvironmentVariable("TORA_UI_CAPTURES");
            QTest::qWait(20);
            QVERIFY(explanation->height()>=explanation->heightForWidth(explanation->width()));
            if(!captures.isEmpty()){QVERIFY(QDir().mkpath(captures));QVERIFY(shell.grab().save(captures+QStringLiteral("/transport-allocations-%1.png").arg(int(method))));}
            tabs->setCurrentIndex(0);
            QTemporaryDir dir;QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);printer.setOutputFileName(dir.filePath("unbalanced.pdf"));
            QVERIFY(printTransportationTrace(printer,c,c.trace().size()));QVERIFY(QFileInfo(printer.outputFileName()).size()>0);
            QVERIFY(c.initializeMultiplier({tora::transportation::MultiplierAxis::Destination,3},error));
            QVERIFY(c.computeFinal(error));QCOMPARE(c.finalOutput()->objective,std::string("130.00"));
        }
        p.supply={20,25};p.demand={10,25,15};p.maximize=true;c.load(p);QString error;QVERIFY(c.solve(error));
        QCOMPARE(c.solutionProblem().dummySources,std::vector<std::size_t>{2});
        QVERIFY(shell.navigate(QStringLiteral("transportation/vogel")));auto* page=shell.currentContent();
        QVERIFY(page->findChild<QLabel*>("transport-balancing")->text().contains("unmet demand"));
        auto* tabs=page->findChild<QTabWidget*>("transport-phases");tabs->setCurrentIndex(1);
        page->findChild<QPushButton*>("transport-allocation-all")->click();
        auto* allocations=page->findChild<QTableWidget*>("transport-allocation-grid");QCOMPARE(allocations->rowCount(),6);
        // A new solve restarts the cached allocation page and keeps input unchanged.
        QVERIFY(c.solve(error,tora::transportation::StartMethod::Vogel));
        QVERIFY(page->findChild<QLabel*>("transport-allocation-explanation")->text().startsWith("Allocation 1 of"));
    }
    void manualLoopFeedback() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);
        c.loadExample();QString error;QVERIFY(c.solve(error));QVERIFY(shell.navigate(QStringLiteral("transportation/northwest")));shell.show();QTest::qWait(20);
        auto* grid=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("transport-tableau"));QVERIFY(grid);
        const auto click=[grid](int row,int col) {QTest::mouseClick(grid->viewport(),Qt::LeftButton,Qt::NoModifier,grid->visualItemRect(grid->item(row,col)).center());};
        const auto input=c.document()->encode();
        click(3,5);QCOMPARE(grid->item(4,5)->background().color(),QColor(colors::surface)); // cost row ignored
        click(5,5);QCOMPARE(grid->item(4,5)->background().color(),QColor(colors::surface)); // reduced row ignored
        click(7,3);QCOMPARE(grid->item(7,3)->background().color(),QColor(colors::surface)); // wrong shipment
        click(7,5);QCOMPARE(grid->item(7,5)->background().color(),QColor(colors::header));
        click(4,4);QCOMPARE(grid->item(4,4)->background().color(),QColor(colors::leaving));
        click(4,5);QCOMPARE(grid->item(4,5)->background().color(),QColor(colors::entering));
        click(4,5);QCOMPARE(grid->item(4,5)->background().color(),QColor(colors::entering));
        auto* anchor=shell.currentContent()->findChild<QComboBox*>(QStringLiteral("transport-multiplier-anchor"));QVERIFY(anchor);
        anchor->setFocus();QTest::keyClick(anchor,Qt::Key_End);
        QCOMPARE(grid->item(4,5)->background().color(),QColor(colors::entering));
        QCOMPARE(grid->item(4,4)->background().color(),QColor(colors::leaving));
        QCOMPARE(c.document()->encode(),input);QVERIFY(!c.dirty());QCOMPARE(c.trace()[0].objective,230.0);
        const auto base=qEnvironmentVariable("TORA_UI_CAPTURES");
        for(const auto physical:{QSize(1366,768),QSize(1920,1080),QSize(3840,2160)}) {
            const auto scale=shell.devicePixelRatioF();shell.resize(qRound(physical.width()/scale),qRound(physical.height()/scale));QTest::qWait(20);
            for(auto* button:shell.currentContent()->findChildren<QPushButton*>())QVERIFY(button->width()>=button->minimumSizeHint().width());
            if(!base.isEmpty()) {
                const auto folder=base+QStringLiteral("/transportation-manual");QVERIFY(QDir().mkpath(folder));
                QVERIFY(shell.grab().save(folder+QStringLiteral("/manual-%1x%2-%3.png").arg(physical.width()).arg(physical.height()).arg(scale)));
            }
        }
    }
    void partialManualNextAndReset() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);
        c.loadExample();QString error;QVERIFY(c.solve(error));QVERIFY(shell.navigate(QStringLiteral("transportation/northwest")));shell.show();QTest::qWait(20);
        auto* grid=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("transport-tableau"));QVERIFY(grid);
        const auto click=[grid](int row,int col) {grid->scrollToItem(grid->item(row,col));QTest::qWait(10);QTest::mouseClick(grid->viewport(),Qt::LeftButton,Qt::NoModifier,grid->visualItemRect(grid->item(row,col)).center());};
        click(4,5);
        auto* next=shell.currentContent()->findChild<QPushButton*>(QStringLiteral("transport-next"));QVERIFY(next);next->click();
        QCOMPARE(grid->rowCount(),20);QCOMPARE(grid->item(10,2)->text(),QStringLiteral("170.00"));
        QCOMPARE(grid->item(4,4)->background().color(),QColor(colors::leaving)); // automatic completion
        click(17,3);QCOMPARE(grid->item(17,3)->background().color(),QColor(colors::entering));
        QVERIFY(c.solve(error));QCOMPARE(grid->rowCount(),10);QCOMPARE(grid->item(4,5)->background().color(),QColor(colors::surface));
        QVERIFY(c.solve(error,tora::transportation::StartMethod::LeastCost));
        click(4,5);QCOMPARE(grid->item(4,5)->background().color(),QColor(colors::surface));QVERIFY(!next->isEnabled());
    }
    void manualTieAndZeroFeedback() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);QString error;
        auto p=tora::transportation::Problem::blank(3,3);p.costs={{6,6,2},{1,5,5},{7,1,3}};p.supply={2,9,3};p.demand={1,8,5};c.load(p);QVERIFY(c.solve(error));
        QVERIFY(shell.navigate(QStringLiteral("transportation/northwest")));shell.show();QTest::qWait(20);
        auto* grid=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("transport-tableau"));QVERIFY(grid);
        const auto click=[&grid](int row,int col) {QTest::mouseClick(grid->viewport(),Qt::LeftButton,Qt::NoModifier,grid->visualItemRect(grid->item(row,col)).center());};
        click(7,3);QCOMPARE(grid->item(7,3)->background().color(),QColor(colors::surface)); // tied alternate entering ignored
        click(7,5);QCOMPARE(grid->item(7,5)->background().color(),QColor(colors::header));
        p.costs={{7,0,8},{9,5,3},{9,0,5}};p.supply={2,3,8};p.demand={6,4,3};c.load(p);QVERIFY(c.solve(error));
        click(10,5);QCOMPARE(grid->item(10,5)->background().color(),QColor(colors::header));
        click(7,3);QCOMPARE(grid->item(7,3)->background().color(),QColor(colors::leaving));
        p=tora::transportation::Problem::blank(2,3);p.costs={{2,3,1},{5,4,8}};p.supply={0,50};p.demand={10,25,15};c.load(p);QVERIFY(c.solve(error));
        click(4,3);QCOMPARE(grid->item(4,3)->text(),QStringLiteral("0"));QCOMPARE(grid->item(4,3)->background().color(),QColor(colors::leaving));
        p.costs={{2,2,5},{2,4,1}};p.supply={17,23};p.demand={11,13,16};c.load(p);
        QVERIFY(c.solve(error,tora::transportation::StartMethod::LeastCost));QVERIFY(shell.navigate(QStringLiteral("transportation/leastcost")));
        grid=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("transport-tableau"));QVERIFY(grid);QTest::qWait(10);
        click(7,3);QCOMPARE(grid->item(7,3)->background().color(),QColor(colors::entering));
        click(7,4);QCOMPARE(grid->item(7,4)->background().color(),QColor(colors::leaving));
        p=tora::transportation::Problem::blank(3,3);p.costs={{7,0,8},{9,5,3},{9,0,5}};p.supply={2,3,8};p.demand={6,4,3};c.load(p);
        QVERIFY(c.solve(error,tora::transportation::StartMethod::Vogel));QVERIFY(shell.navigate(QStringLiteral("transportation/vogel")));
        grid=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("transport-tableau"));QVERIFY(grid);QTest::qWait(10);
        click(4,3);QCOMPARE(grid->item(4,3)->background().color(),QColor(colors::entering));
        click(4,4);QCOMPARE(grid->item(4,4)->background().color(),QColor(colors::leaving));
        click(10,5);QCOMPARE(grid->item(10,5)->background().color(),QColor(colors::surface));
    }
    void multiplierAnchorLifecycle() {
        using namespace tora::transportation;
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);
        c.loadExample();QString error;QVERIFY(c.solve(error));
        QVERIFY(shell.navigate(QStringLiteral("transportation/northwest")));shell.show();QTest::qWait(20);
        auto* anchor=shell.currentContent()->findChild<QComboBox*>(QStringLiteral("transport-multiplier-anchor"));QVERIFY(anchor);
        QCOMPARE(anchor->count(),5);QVERIFY(anchor->isEnabled());QCOMPARE(anchor->itemText(4),QStringLiteral("v3=0"));
        const auto bytes=c.document()->encode();QVERIFY(!c.dirty());
        anchor->setFocus();QTest::keyClick(anchor,Qt::Key_End);
        auto* grid=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("transport-tableau"));QVERIFY(grid);
        QCOMPARE(grid->item(3,2)->text(),QStringLiteral("u1=7"));
        QCOMPARE(grid->item(2,5)->text(),QStringLiteral("v3=0"));
        QCOMPARE(grid->item(2,5)->foreground().color(),QColor(Qt::darkRed));
        QVERIFY(grid->item(3,2)->foreground().color()!=QColor(Qt::darkRed));
        QCOMPARE(c.document()->encode(),bytes);QVERIFY(!c.dirty());
        auto* next=shell.currentContent()->findChild<QPushButton*>(QStringLiteral("transport-next"));QVERIFY(next);next->click();
        QVERIFY(!anchor->isEnabled());QCOMPARE(grid->item(13,2)->text(),QStringLiteral("u1=1.00"));
        QCOMPARE(grid->item(12,5)->text(),QStringLiteral("v3=0.00"));
        const auto before=c.trace()[0].u;QVERIFY(!c.initializeMultiplier({MultiplierAxis::Source,0},error));QCOMPARE(c.trace()[0].u,before);
        c.loadExample();QVERIFY(c.solve(error,StartMethod::LeastCost));
        QVERIFY(shell.navigate(QStringLiteral("transportation/leastcost")));
        anchor=shell.currentContent()->findChild<QComboBox*>(QStringLiteral("transport-multiplier-anchor"));QVERIFY(anchor);QVERIFY(anchor->isEnabled());
        anchor->setFocus();QTest::keyClick(anchor,Qt::Key_Home);QTest::keyClick(anchor,Qt::Key_Down);
        QCOMPARE(c.trace()[0].u,(std::vector<double>{-3,0}));
        QVERIFY(!c.initializeMultiplier({MultiplierAxis::Source,999},error));QCOMPARE(c.trace()[0].u,(std::vector<double>{-3,0}));
        QVERIFY(c.solve(error));QCOMPARE(c.multiplierAnchor().index,std::size_t(0));QVERIFY(!c.multipliersRefreshed());
    }
    void multiplierAnchorResolutionPreview() {
        ApplicationShell shell;shell.registerModule(transportationModule());auto& c=transportationController(shell);
        c.loadExample();QString error;QVERIFY(c.solve(error));QVERIFY(shell.navigate(QStringLiteral("transportation/northwest")));shell.show();
        auto* anchor=shell.currentContent()->findChild<QComboBox*>(QStringLiteral("transport-multiplier-anchor"));QVERIFY(anchor);
        anchor->setFocus();QTest::keyClick(anchor,Qt::Key_End);
        for(const auto physical:{QSize(1366,768),QSize(1920,1080),QSize(3840,2160)}) {
            const auto scale=shell.devicePixelRatioF();shell.resize(qRound(physical.width()/scale),qRound(physical.height()/scale));QTest::qWait(20);
            QVERIFY(anchor->width()>=anchor->minimumSizeHint().width());
            for(auto* button:shell.currentContent()->findChildren<QPushButton*>())QVERIFY(button->width()>=button->minimumSizeHint().width());
            const auto base=qEnvironmentVariable("TORA_UI_CAPTURES");
            if(!base.isEmpty()) {
                const auto folder=base+QStringLiteral("/transportation-multipliers");QVERIFY(QDir().mkpath(folder));
                QVERIFY(shell.grab().save(folder+QStringLiteral("/anchor-%1x%2-%3.png").arg(physical.width()).arg(physical.height()).arg(scale)));
            }
        }
    }
    void initTestCase() {
        const auto font=readableUiFont(QApplication::font());
        const QFontMetrics metrics(font);
        for(const auto character:QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-=().,:/'—"))
            QVERIFY2(metrics.inFont(character),qPrintable(QStringLiteral("Missing UI glyph U+%1 in %2")
                .arg(static_cast<unsigned>(character.unicode()),4,16,QLatin1Char('0')).arg(font.toString())));
    }
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
        QVERIFY(inputGrid->editTriggers().testFlag(QAbstractItemView::DoubleClicked));
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
        QCOMPARE(grid->columnCount(),4);QCOMPARE(grid->item(2,2)->text(),QStringLiteral("3"));
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
int main(int argc,char** argv) {
#ifdef Q_OS_WIN
    // The offscreen Windows backend uses FreeType rather than the native
    // Windows font database, so supply the OS fonts just as other UI tests do.
    qputenv("QT_QPA_FONTDIR",qgetenv("SystemRoot")+"/Fonts");
#endif
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc,argv);
    DesktopTransportation test;
    return QTest::qExec(&test,argc,argv);
}
#include "desktop_transportation.moc"
