#include "application_shell.hpp"
#include "lp_screens.hpp"
#include "transportation_screens.hpp"
#include "assignment_controller.hpp"
#include "equations_screens.hpp"
#include "games_screens.hpp"
#include "queuing_screens.hpp"
#include "networks_screens.hpp"
#include "planning_screens.hpp"
#include "integer_controller.hpp"
#include "ui_font.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QPrinter>
#include <QPushButton>
#include <QTest>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QComboBox>

using namespace tora::desktop;
class DesktopAllModules final : public QObject {
    Q_OBJECT
    void registerModules(ApplicationShell& shell) {
        for (auto module : {lpModule(), transportationModule(), assignmentModule(),
                equationsModule(), gamesModule(), queuingModule(), networksModule(),
                planningModule(), integerModule()})
            QVERIFY(shell.registerModule(std::move(module)));
    }
    void capture(ApplicationShell& shell, const QString& name) {
        const auto root = qEnvironmentVariable("TORA_UI_CAPTURES");
        if (root.isEmpty()) return;
        QVERIFY(QDir().mkpath(root));
        QApplication::processEvents();
        QVERIFY(shell.grab().save(root + "/" + name + "-" +
            qEnvironmentVariable("QT_SCALE_FACTOR", "1") + ".png"));
    }
private slots:
    void initTestCase() { QApplication::setFont(readableUiFont(QApplication::font())); }
    void everyMainEntryOpens() {
        ApplicationShell shell;
        registerModules(shell);
        shell.resize(1000, 700);
        shell.show();
        QVERIFY(shell.navigate("main"));
        capture(shell, "all-modules-main");
        const QList<QPair<QString, QString>> entries {
            {"Linear Equations", "equations/input"},
            {"Linear Programming", "lp/input-mode"},
            {"Transportation model", "transportation/input-mode"},
            {"Integer programming", "integer/input"},
            {"Network models", "networks/input"},
            {"Project Planning", "planning/input"},
            {"Queuing analysis", "queuing/input"},
            {"Zero-Sum Games", "games/input"},
            {"Assignment Problems", "assignment/input"}
        };
        for (const auto& entry : entries) {
            QVERIFY(shell.navigate("main"));
            auto* button = shell.currentContent()->findChild<QPushButton*>(
                entry.first == "Linear Programming" ? "main-lp" : entry.first);
            QVERIFY2(button, qPrintable(entry.first));
            QVERIFY2(button->isEnabled(), qPrintable(entry.first));
            QTest::mouseClick(button, Qt::LeftButton);
            QCOMPARE(shell.currentScreen(), entry.second);
        }
    }
    void extendedResultViews() {
        ApplicationShell shell;registerModules(shell);
        const auto dialogs=[](InputDialogs& d){d.askSave=[]{return SaveChoice::No;};d.showError=[](const QString&,const QString&){};};
        auto& equations=equationsController(shell);auto& queue=queuingController(shell);auto& network=networksController(shell);auto& integer=integerController(shell);
        dialogs(equations.dialogs());dialogs(queue.dialogs());dialogs(network.dialogs());dialogs(integer.dialogs());dialogs(planningController(shell).dialogs());
        double scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();if(scale<=0)scale=1;
        shell.resize(qRound(1366/scale),qRound(768/scale));shell.show();QString error;
        QVERIFY(equations.initialize("Inverse of [4,7;2,6]",2,error,true));
        const char* matrix[2][2]={{"4","7"},{"2","6"}};
        for(int i=0;i<2;++i)for(int j=0;j<2;++j)QVERIFY(equations.editCell(i,j,matrix[i][j],error));
        QVERIFY(equations.solve(error,true));QVERIFY(shell.navigate("equations/result"));
        auto* original = shell.currentContent()->findChild<QTableWidget*>("equations-original-grid");
        QVERIFY(original);
        QCOMPARE(original->columnCount(),2);
        capture(shell,"inverse-summary");
        auto* equationTabs = shell.currentContent()->findChild<QTabWidget*>("equations-result-tabs");
        QVERIFY(equationTabs);
        equationTabs->setCurrentIndex(equationTabs->count() - 1);
        capture(shell,"inverse-trace");
        QVERIFY(queue.initialize("Pure death, N=3, mu=1 at t=1",error));
        for(const auto& field:QList<QPair<int,QString>>{{0,"0"},{1,"1"},{3,"3"},{4,"3"}})QVERIFY(queue.editCell(0,field.first,field.second,error));
        QVERIFY(queue.solve(error));QVERIFY(shell.navigate("queuing/result"));QVERIFY(queue.outputText().contains("Mean population = 2.02"));capture(shell,"queue-pure-death");
        QVERIFY(shell.navigate("queuing/input"));QVERIFY(queue.editCell(0,0,"2",error));QVERIFY(queue.editCell(0,1,"0",error));QVERIFY(queue.solve(error));QVERIFY(shell.navigate("queuing/result"));capture(shell,"queue-pure-birth");
        QVERIFY(network.initialize(NetworkMode::ShortestRoute,"A-B-C distance5",3,error));
        QVERIFY(network.editCell(1,2,"2",error));QVERIFY(network.editCell(1,3,"8",error));QVERIFY(network.editCell(2,3,"3",error));
        QVERIFY(network.solve(0,2,error,NetworkRouteMethod::Dijkstra));QVERIFY(shell.navigate("networks/result"));capture(shell,"network-dijkstra");
        QVERIFY(network.solve(0,2,error,NetworkRouteMethod::Floyd));capture(shell,"network-floyd");
        QVERIFY(integer.loadExample());QVERIFY(integer.startGuided());QVERIFY(integer.guided()->awaitingBranch());capture(shell,"integer-guided-root");
        QVERIFY(integer.branchGuided(1));QVERIFY(integer.nextGuided());capture(shell,"integer-guided-child");
        QVERIFY(integer.finishGuided());QCOMPARE(*integer.result()->objective,6.);
        auto* tabs=shell.currentContent()->findChild<QTabWidget*>("integer-result-tabs");QVERIFY(tabs);tabs->setCurrentIndex(1);
        auto* tree=shell.currentContent()->findChild<QTreeWidget*>("integer-search-tree");QVERIFY(tree&&tree->topLevelItemCount()>0);capture(shell,"integer-guided-tree");
        auto& planning=planningController(shell);tora::planning::Problem project;
        project.activities={{1,2,"A",3},{2,4,"B",5},{1,3,"C",2},{3,4,"D",2}};
        QVERIFY(planning.setProblem("CPM schedule experiments",project,error));QVERIFY(planning.solve(error));QVERIFY(shell.navigate("planning/result"));
        auto* planningTabs=shell.currentContent()->findChild<QTabWidget*>("planning-output-tabs");QVERIFY(planningTabs);planningTabs->setCurrentIndex(2);QApplication::processEvents();auto* scheduleScroll=shell.currentContent()->parentWidget();QVERIFY(scheduleScroll);QVERIFY(shell.currentContent()->minimumSizeHint().width()<=qRound(1366/scale));capture(shell,"planning-bar-chart");
        QVERIFY(planning.shiftActivity(2,3,error));QCOMPARE(planning.schedule()->starts[2],3.);QCOMPARE(planning.schedule()->starts[3],5.);capture(shell,"planning-shifted-chart");
        auto* zoom=shell.currentContent()->findChild<QPushButton*>("planning-schedule-zoom-in");QVERIFY(zoom);QTest::mouseClick(zoom,Qt::LeftButton);capture(shell,"planning-chart-zoom");
        const auto root=qEnvironmentVariable("TORA_UI_CAPTURES");
        if(!root.isEmpty()){
            const auto pdf=[&](const QString& name,const auto& print){QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);printer.setOutputFileName(root+"/"+name+"-"+qEnvironmentVariable("QT_SCALE_FACTOR","1")+".pdf");QVERIFY(print(printer));};
            pdf("inverse-trace",[&](QPrinter& p){return printEquations(p,equations,equationsTraceText(equations,0));});
            pdf("queue-pure-birth",[&](QPrinter& p){return printQueuing(p,queue);});
            pdf("network-floyd",[&](QPrinter& p){return printNetworks(p,network);});
            pdf("integer-guided",[&](QPrinter& p){return printInteger(p,integer,shell.font());});
            pdf("planning-schedule",[&](QPrinter& p){return printPlanningSchedule(p,planning);});
        }
    }
    void nativeResultsAndExports() {
        ApplicationShell shell;
        registerModules(shell);
        const auto dialogs = [](InputDialogs& d) {
            d.askSave = [] { return SaveChoice::No; };
            d.showError = [](const QString&, const QString&) {};
        };
        dialogs(assignmentController(shell).dialogs());
        dialogs(equationsController(shell).dialogs());
        dialogs(gamesController(shell).dialogs());
        dialogs(queuingController(shell).dialogs());
        dialogs(networksController(shell).dialogs());
        dialogs(planningController(shell).dialogs());
        dialogs(integerController(shell).dialogs());
        QString error;
        auto& equations = equationsController(shell);
        QVERIFY(equations.initialize("Equations: 2x + y = 5, x - y = 1", 2, error));
        const char* cells[2][3] = {{"2", "1", "5"}, {"1", "-1", "1"}};
        for (int i=0;i<2;++i) for (int j=0;j<3;++j)
            QVERIFY(equations.editCell(i,j,cells[i][j],error));
        QVERIFY(equations.solve(error));
        auto& assignment = assignmentController(shell);
        QVERIFY(assignment.loadExample());
        QVERIFY(assignment.solve());
        auto& games = gamesController(shell);
        QVERIFY(games.initialize("Mixed strategies",2,3,error));
        const char* payoffs[2][3]={{"5","2","0"},{"1","3","4"}};
        for(int i=0;i<2;++i)for(int j=0;j<3;++j)
            QVERIFY(games.editCell(i,j,payoffs[i][j],error));
        QVERIFY(games.solve(error,true));
        auto& queue = queuingController(shell);
        QVERIFY(queue.initialize("M/M/1: lambda=2, mu=3",error));
        QVERIFY(queue.editCell(0,0,"2",error));
        QVERIFY(queue.editCell(0,1,"3",error));
        QVERIFY(queue.solve(error));
        auto& network = networksController(shell);
        QVERIFY(network.initialize(NetworkMode::MinimalTree,"Three-node minimum tree",3,error));
        QVERIFY(network.editCell(1,2,"1",error));
        QVERIFY(network.editCell(1,3,"3",error));
        QVERIFY(network.editCell(2,3,"2",error));
        QVERIFY(network.solve(0,2,error));
        auto& planning = planningController(shell);
        tora::planning::Problem project;
        project.activities={{1,2,"A",3},{2,3,"B",4},{1,3,"C",5}};
        QVERIFY(planning.setProblem("Project: critical path A, B",project,error));
        QVERIFY(planning.solve(error));
        auto& integer = integerController(shell);
        QVERIFY(integer.loadExample());
        QVERIFY(integer.solve());
        shell.show();
        double scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();
        if(scale<=0)scale=1;
        const QStringList routes {"equations/input","equations/result","assignment/input",
            "assignment/result","games/input","games/graphical","queuing/input","queuing/result",
            "networks/input","networks/result","planning/input","planning/result",
            "integer/input","integer/result"};
        for(const auto& route:routes){
            QVERIFY2(shell.navigate(route),qPrintable(route));
            for(const auto size:{QSize(1366,768),QSize(1920,1080)}){
                shell.resize(qRound(size.width()/scale),qRound(size.height()/scale));
                QApplication::processEvents();
                auto name=route;name.replace('/','-');
                capture(shell,name+"-"+QString::number(size.width()));
                if (route == "queuing/result" && size.width() == 1366) {
                    auto* queueTabs = shell.currentContent()->findChild<QTabWidget*>("queuing-result-tabs");
                    QVERIFY(queueTabs);
                    queueTabs->setCurrentIndex(1);
                    capture(shell,"queuing-probabilities-1366");
                    queueTabs->setCurrentIndex(0);
                }
            }
        }
        const auto root=qEnvironmentVariable("TORA_UI_CAPTURES");
        if(!root.isEmpty()){
            const auto pdf=[&](const QString& name,const auto& print){
                QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);
                const auto path=root+"/"+name+"-"+qEnvironmentVariable("QT_SCALE_FACTOR","1")+".pdf";
                printer.setOutputFileName(path);QVERIFY(print(printer));
                QFile file(path);QVERIFY(file.open(QIODevice::ReadOnly));
                QVERIFY(file.read(5)=="%PDF-");
            };
            pdf("equations",[&](QPrinter& p){return printEquations(p,equations);});
            pdf("assignment",[&](QPrinter& p){return printAssignment(p,assignment,shell.font());});
            pdf("games",[&](QPrinter& p){return printGames(p,games);});
            pdf("queuing",[&](QPrinter& p){return printQueuing(p,queue);});
            pdf("networks",[&](QPrinter& p){return printNetworks(p,network);});
            pdf("planning",[&](QPrinter& p){return printPlanning(p,planning);});
            pdf("integer",[&](QPrinter& p){return printInteger(p,integer,shell.font());});
        }
    }
};
QTEST_MAIN(DesktopAllModules)
#include "desktop_all_modules.moc"
