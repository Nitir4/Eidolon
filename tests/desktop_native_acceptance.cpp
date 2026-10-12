#include "application_shell.hpp"
#include "lp_input_controller.hpp"
#include "lp_simplex_controller.hpp"
#include "lp_graphical_controller.hpp"
#include "lp_final_controller.hpp"
#include "lp_printing.hpp"
#include "lp_screens.hpp"
#include "ui_font.hpp"
#include "transportation_screens.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QDialogButtonBox>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QScreen>
#include <QStyleFactory>
#include <QTest>
#include <QTableWidget>
#include <QTimer>
#include <stdexcept>
using namespace tora::desktop;

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QApplication::setFont(usableUiFont(app.font()));
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({QStringLiteral("problem"), QStringLiteral("Saved problem to display."), QStringLiteral("path")});
    parser.addOption({QStringLiteral("output"), QStringLiteral("Private capture/report directory."), QStringLiteral("path")});
    parser.addOption({QStringLiteral("print-dialog"), QStringLiteral("Capture and cancel the real Linux Qt print dialog; sends no print job.")});
    parser.addOption({QStringLiteral("fullscreen"), QStringLiteral("Temporarily display each view fullscreen on the connected monitor.")});
    parser.addOption({QStringLiteral("transportation"), QStringLiteral("Also check native Transportation North-west, Least-cost and Vogel methods.")});
    parser.process(app);
    try {
        require(parser.isSet(QStringLiteral("problem")) && parser.isSet(QStringLiteral("output")), "--problem and --output are required");
        require(app.platformName() != QStringLiteral("offscreen") && app.platformName() != QStringLiteral("minimal"), "This check requires an actual desktop backend");
        const auto output=parser.value(QStringLiteral("output"));
        require(QDir().mkpath(output), "Cannot create output directory");
        ApplicationShell shell;
        require(shell.registerModule(lpModule()), "LP module registration failed");
        require(shell.registerModule(transportationModule()), "Transportation module registration failed");
        if(parser.isSet(QStringLiteral("fullscreen"))) shell.showFullScreen();
        else shell.showMaximized();
        QTest::qWait(200);
        QJsonArray captures;
        const auto capture=[&](const QString& name, const QString& route) {
            QTest::qWait(150);
            require(shell.currentScreen()==route,"Capture route does not match its label");
            const auto* screen=shell.screen();
            require(screen!=nullptr,"Window has no display");
            const auto bounds=shell.isFullScreen()?screen->geometry():screen->availableGeometry();
            require(shell.width()<=bounds.width() && shell.height()<=bounds.height(), "Window exceeds the actual available display area");
            for(auto* button:shell.currentContent()->findChildren<QPushButton*>())
                require(button->width()>=button->minimumSizeHint().width(), "Button is narrower than its text/layout minimum");
            require(shell.grab().save(output+QLatin1Char('/')+name+QStringLiteral(".png")), "Cannot save capture");
            captures.append(QJsonObject{{QStringLiteral("view"),name},{QStringLiteral("width"),shell.width()},
                                      {QStringLiteral("height"),shell.height()},{QStringLiteral("dpr"),shell.devicePixelRatioF()}});
        };
        capture(QStringLiteral("welcome"),QStringLiteral("welcome"));
        require(shell.navigate(QStringLiteral("main")),"Main menu navigation failed");
        capture(QStringLiteral("main"),QStringLiteral("main"));
        auto& input=inputController(shell);
        input.dialogs().askSave=[] {return SaveChoice::No;};
        require(input.openFile(parser.value(QStringLiteral("problem"))), "Saved model could not be opened");
        require(shell.navigate(QStringLiteral("lp/input")),"Input navigation failed");
        capture(QStringLiteral("input"),QStringLiteral("lp/input"));
        const auto before=tora::compat::ToraLpCodec::encode(*input.document());
        auto& simplex=simplexController(shell);
        simplex.notice=[](const QString&,const QString&){};
        require(simplex.requestAllSlack() && simplex.start({tora::core::Notation::Decimal,5,2}), "Simplex start failed");
        simplex.all();capture(QStringLiteral("simplex"),QStringLiteral("lp/simplex"));
        auto& graph=graphicalController(shell);
        graph.notice=[](const QString&,const QString&){};
        require(graph.request() && graph.start({tora::core::Notation::Decimal,5,2}), "Graphical start failed");
        graph.selectRow(static_cast<int>(graph.session()->problem().constraints.size())+4);
        capture(QStringLiteral("graphical"),QStringLiteral("lp/graphical"));
        auto& summary=finalController(shell);
        summary.notice=[](const QString&,const QString&){};
        require(summary.request() && summary.start({tora::core::Notation::Decimal,5,2}), "Final solution start failed");
        capture(QStringLiteral("final-sensitivity"),QStringLiteral("lp/final"));
        if (parser.isSet(QStringLiteral("transportation"))) {
            require(shell.navigate(QStringLiteral("transportation/input-mode")),"Transportation entry failed");
            capture(QStringLiteral("transportation-entry"),QStringLiteral("transportation/input-mode"));
            auto& transport=transportationController(shell);transport.loadExample();
            require(shell.navigate(QStringLiteral("transportation/input")),"Transportation input failed");
            capture(QStringLiteral("transportation-input"),QStringLiteral("transportation/input"));
            QString error;require(transport.solve(error),"Transportation solve failed");
            require(shell.navigate(QStringLiteral("transportation/northwest")),"Transportation iteration route failed");
            capture(QStringLiteral("transportation-initial"),QStringLiteral("transportation/northwest"));
            auto* all=shell.findChild<QPushButton*>(QStringLiteral("transport-all"));require(all && all->isEnabled(),"Transportation All missing");
            QTimer::singleShot(0,[]{for(auto* widget:QApplication::topLevelWidgets())if(auto* box=qobject_cast<QMessageBox*>(widget))box->accept();});
            all->click();require(!all->isEnabled(),"Transportation did not terminate");
            require(transport.trace().size()==3 && transport.trace().back().objective==150,"Transportation pilot mismatch");
            capture(QStringLiteral("transportation-optimal"),QStringLiteral("transportation/northwest"));
            auto tie=tora::transportation::Problem::blank(2,3);
            tie.title="Least-cost tie example";tie.costs={{2,2,5},{2,4,1}};tie.supply={17,23};tie.demand={11,13,16};
            transport.load(std::move(tie));require(transport.solve(error,tora::transportation::StartMethod::LeastCost),"Least-cost solve failed");
            require(shell.navigate(QStringLiteral("transportation/leastcost")),"Least-cost route failed");
            require(transport.trace().size()==2 && transport.trace().front().objective==78,"Least-cost initial mismatch");
            capture(QStringLiteral("transportation-leastcost-initial"),QStringLiteral("transportation/leastcost"));
            auto* leastAll=shell.currentContent()->findChild<QPushButton*>(QStringLiteral("transport-all"));
            require(leastAll && leastAll->isEnabled(),"Least-cost All missing");
            QTimer::singleShot(0,[]{for(auto* widget:QApplication::topLevelWidgets())if(auto* box=qobject_cast<QMessageBox*>(widget))box->accept();});
            leastAll->click();require(!leastAll->isEnabled(),"Least-cost did not terminate");
            require(transport.trace().back().objective==64,"Least-cost terminal mismatch");
            capture(QStringLiteral("transportation-leastcost-optimal"),QStringLiteral("transportation/leastcost"));
            auto vogel=tora::transportation::Problem::blank(3,3);
            vogel.title="Vogel degenerate improvement example";
            vogel.costs={{7,0,8},{9,5,3},{9,0,5}};vogel.supply={2,3,8};vogel.demand={6,4,3};
            transport.load(std::move(vogel));require(transport.solve(error,tora::transportation::StartMethod::Vogel),"Vogel solve failed");
            require(shell.navigate(QStringLiteral("transportation/vogel")),"Vogel route failed");
            require(transport.trace().size()==2 && transport.trace().front().objective==63,"Vogel initial mismatch");
            capture(QStringLiteral("transportation-vogel-initial"),QStringLiteral("transportation/vogel"));
            auto* vogelAll=shell.currentContent()->findChild<QPushButton*>(QStringLiteral("transport-all"));
            require(vogelAll && vogelAll->isEnabled(),"Vogel All missing");
            QTimer::singleShot(0,[]{for(auto* widget:QApplication::topLevelWidgets())if(auto* box=qobject_cast<QMessageBox*>(widget))box->accept();});
            vogelAll->click();require(!vogelAll->isEnabled(),"Vogel did not terminate");
            require(transport.trace().back().objective==59,"Vogel terminal mismatch");
            capture(QStringLiteral("transportation-vogel-optimal"),QStringLiteral("transportation/vogel"));
            require(shell.navigate(QStringLiteral("transportation/input")),"Editable input route failed");
            require(transport.initialize(QStringLiteral("Native Transportation input"),2,3,error),"New Transportation grid failed");
            capture(QStringLiteral("transportation-input-new"),QStringLiteral("transportation/input"));
            auto* inputGrid=shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("transport-input-grid"));
            require(inputGrid && inputGrid->item(2,2),"Editable Transportation grid missing");
            inputGrid->setCurrentCell(2,2);inputGrid->editItem(inputGrid->item(2,2));QTest::qWait(30);
            auto* editor=inputGrid->findChild<QLineEdit*>();require(editor!=nullptr,"Native grid editor missing");
            editor->setText(QStringLiteral("9.876"));
            const auto savedPath=output+QStringLiteral("/transportation-native.txt");
            transport.dialogs().chooseSave=[savedPath](const QString&)->std::optional<QString>{return savedPath;};
            require(transport.requestSave(),"Native active-editor save failed");
            require(transport.problem().costs[0][0]==9.88 && !transport.dirty(),"Native edit/save state mismatch");
            capture(QStringLiteral("transportation-input-saved"),QStringLiteral("transportation/input"));
            const auto savedDocument=transport.document()->encode();
            require(transport.openFile(savedPath,error),"Native Transportation reopen failed");
            require(transport.document()->encode()==savedDocument,"Native Transportation reopen changed saved bytes");
            capture(QStringLiteral("transportation-input-reopened"),QStringLiteral("transportation/input"));
        }
        QString panel=QStringLiteral("not requested");
        if(parser.isSet(QStringLiteral("print-dialog"))) {
            require(app.platformName()==QStringLiteral("wayland") || app.platformName()==QStringLiteral("xcb"), "Linux dialog automation requires Wayland or X11");
            bool seen=false, rendered=false, saved=false;
            QTimer cancel;
            cancel.setInterval(100);
            QObject::connect(&cancel,&QTimer::timeout,&app,[&] {
                for(auto* widget:QApplication::topLevelWidgets()) {
                    auto* dialog=qobject_cast<QPrintDialog*>(widget);
                    if(!dialog || !dialog->isVisible()) continue;
                    seen=true;saved=dialog->grab().save(output+QStringLiteral("/print-dialog.png"));
                    dialog->reject();cancel.stop();
                }
            });
            QTimer timeout;
            timeout.setSingleShot(true);
            QObject::connect(&timeout,&QTimer::timeout,&app,[] {QApplication::exit(3);});
            cancel.start();timeout.start(10000);
            const auto result=requestLpPrint(&shell,[&](QPrinter&){rendered=true;return false;},QPrinterInfo());
            timeout.stop();cancel.stop();
            require(seen && saved && !rendered && result==PrintOutcome::Cancelled, "Real print dialog cancellation failed");
            const auto pdf=QDir(output).absoluteFilePath(QStringLiteral("native-summary.pdf"));
            require(!QFile::exists(pdf), "Refusing to overwrite existing acceptance PDF");
            bool selected=false;
            QTimer choosePdf;
            choosePdf.setInterval(100);
            QObject::connect(&choosePdf,&QTimer::timeout,&app,[&] {
                for(auto* widget:QApplication::topLevelWidgets()) {
                    auto* dialog=qobject_cast<QPrintDialog*>(widget);
                    if(!dialog || !dialog->isVisible()) continue;
                    auto* filename=dialog->findChild<QLineEdit*>(QStringLiteral("filename"));
                    auto* buttons=dialog->findChild<QDialogButtonBox*>();
                    if(!filename || !buttons || !filename->isVisible()) {dialog->reject();choosePdf.stop();return;}
                    filename->setText(pdf);
                    auto* print=buttons->button(QDialogButtonBox::Ok);
                    if(!print || !print->isEnabled()) {dialog->reject();choosePdf.stop();return;}
                    selected=dialog->grab().save(output+QStringLiteral("/print-to-pdf.png"));
                    choosePdf.stop();print->click();
                }
            });
            choosePdf.start();timeout.start(10000);
            const auto printed=requestLpPrint(&shell,[&](QPrinter& printer){
                if(printer.outputFormat()!=QPrinter::PdfFormat || printer.outputFileName()!=pdf) return false;
                return printSummary(printer,summary,shell.font());
            },QPrinterInfo());
            timeout.stop();choosePdf.stop();
            require(selected && printed==PrintOutcome::Complete,"Real Linux PDF print selection failed");
            QFile document(pdf);require(document.open(QIODevice::ReadOnly) && document.read(5)==QByteArray("%PDF-"),"Native print output is not PDF");
            panel=QStringLiteral("real dialog cancellation and PDF selection passed; no physical print job");
        }
        require(before==tora::compat::ToraLpCodec::encode(*input.document()), "Desktop/print check changed the saved model");
        auto* screen=shell.screen();
        QJsonObject report{{QStringLiteral("platform"),app.platformName()},{QStringLiteral("qt_version"),QString::fromLatin1(qVersion())},
                           {QStringLiteral("screen"),screen->name()},{QStringLiteral("screen_count"),QApplication::screens().size()},
                           {QStringLiteral("screen_width"),screen->geometry().width()},{QStringLiteral("screen_height"),screen->geometry().height()},
                           {QStringLiteral("print_dialog"),panel},{QStringLiteral("model_preserved"),true},{QStringLiteral("captures"),captures},
                           {QStringLiteral("available_printer_count"),QPrinterInfo::availablePrinters().size()},
                           {QStringLiteral("default_printer_present"),!QPrinterInfo::defaultPrinter().isNull()},
                           {QStringLiteral("scope"),QStringLiteral("Actual backend and connected display; not mixed-monitor or physical printer acceptance")}};
        QFile file(output+QStringLiteral("/verification.json"));require(file.open(QIODevice::WriteOnly), "Cannot write report");
        file.write(QJsonDocument(report).toJson());
        return 0;
    } catch(const std::exception& error) {
        qCritical("Native acceptance failed: %s",error.what());return 1;
    }
}
