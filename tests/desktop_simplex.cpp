#include "lp_simplex_controller.hpp"
#include "lp_input_controller.hpp"
#include "lp_screens.hpp"
#include "application_shell.hpp"
#include <QApplication>
#include <QTest>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QRadioButton>
#include <QDir>
#include <QColor>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTemporaryDir>
#include <QAction>
#include <QPrinter>
#include <QScrollBar>
#include <QLabel>

using namespace tora::desktop;
class SimplexTest final:public QObject {
    Q_OBJECT
    static QString golden(const QString& name){return QStringLiteral(TORA_CODEC_GOLDENS)+QLatin1Char('/')+name+QStringLiteral(".txt");}
    static void input(ApplicationShell& shell,const QString& name){auto& c=inputController(shell);c.dialogs().showError=[](const QString&,const QString&){};c.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(c.openFile(golden(name)));}
private slots:
    void normalManualIterationsAndOutputFormat() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();input(shell,QStringLiteral("01_normal"));
        auto& c=simplexController(shell);QStringList notices;c.notice=[&](const QString& title,const QString& body){notices<<title<<body;};
        QVERIFY(c.requestAllSlack());QCOMPARE(shell.currentScreen(),QStringLiteral("lp/output-format"));
        auto* page=shell.currentContent();QVERIFY(!page->findChild<QLineEdit*>(QStringLiteral("output-format-D"))->isReadOnly());
        page->findChild<QPushButton*>(QStringLiteral("output-go"))->click();QCOMPARE(shell.currentScreen(),QStringLiteral("lp/simplex"));
        auto* view=shell.currentContent()->findChild<QTableView*>(QStringLiteral("simplex-grid"));QVERIFY(view);
        QCOMPARE(view->model()->rowCount(),10);QCOMPARE(view->model()->data(view->model()->index(1,1)).toString(),QStringLiteral("x1"));
        c.selectCell(1,2);QCOMPARE(notices.takeFirst(),QStringLiteral("Entering Variable"));notices.clear();
        QCOMPARE(view->model()->data(view->model()->index(3,3),Qt::BackgroundRole).value<QColor>(),QColor(Qt::white));
        c.selectCell(1,1);QCOMPARE(view->model()->data(view->model()->index(1,1),Qt::BackgroundRole).value<QColor>(),QColor(Qt::green));
        c.selectCell(5,0);QCOMPARE(notices.takeFirst(),QStringLiteral("Leaving Variable"));notices.clear();
        c.selectCell(4,0);QCOMPARE(view->model()->data(view->model()->index(4,0),Qt::BackgroundRole).value<QColor>(),QColor(Qt::red));
        c.next();QCOMPARE(c.session()->history().size(),std::size_t(2));QVERIFY(notices.empty());
        c.next();QCOMPARE(c.session()->history().size(),std::size_t(3));QCOMPARE(notices,QStringList({QStringLiteral("Optimal Tableau"),QStringLiteral("Iteration 3 is optimal")}));
        QCOMPARE(view->model()->data(view->model()->index(23,3),Qt::BackgroundRole).value<QColor>(),QColor(Qt::white));
        QCOMPARE(view->model()->data(view->model()->index(25,4),Qt::ForegroundRole).value<QColor>(),QColor(Qt::blue));
        QCOMPARE(view->model()->rowCount(),30);QVERIFY(!shell.currentContent()->findChild<QPushButton*>(QStringLiteral("simplex-all"))->isEnabled());
        QVERIFY(shell.navigate(QStringLiteral("lp/input")));QCOMPARE(inputController(shell).document()->problem.objective[0],3.0);
    }
    void alternativesAndAsyncAll() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();input(shell,QStringLiteral("05_alternative"));
        auto& c=simplexController(shell);c.notice=[](const QString&,const QString&){};QVERIFY(c.requestAllSlack());QVERIFY(c.start({}));
        c.all();QTRY_VERIFY(!c.busy());QCOMPARE(c.session()->history().size(),std::size_t(2));
        QCOMPARE(c.session()->status(),tora::lp::SimplexStatus::Alternative);
        c.selectCell(11,2);QVERIFY(c.canNext());c.next();QCOMPARE(c.session()->history().size(),std::size_t(3));
        QCOMPARE(c.session()->history().back().tableau.basicLabel(1),std::string("x2"));
        input(shell,QStringLiteral("01_normal"));QVERIFY(!c.session());
    }
    void redirectsStayDistinctFromComputedAllSlack() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));input(shell,QStringLiteral("03_infeasible"));
        auto& c=simplexController(shell);c.notice=[](const QString&,const QString&){};QString warning;
        c.redirect=[&](const QString& text){warning=text;return true;};
        QVERIFY(!c.requestAllSlack());QVERIFY(warning.contains(QStringLiteral("Two-phase method will be used")));
        QCOMPARE(shell.currentScreen(),QStringLiteral("lp/output-format"));QVERIFY(!c.canStart());QVERIFY(!c.session());
        c.redirect=[](const QString&){return false;};QVERIFY(!c.requestAllSlack());QCOMPARE(shell.currentScreen(),QStringLiteral("lp/m-value"));
    }
    void initialOptimalNoticeAndScientificOutput() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));input(shell,QStringLiteral("01_normal"));
        auto& in=inputController(shell);QVERIFY(in.grid()->setData(in.grid()->index(1,2),QStringLiteral("0")));QVERIFY(in.grid()->setData(in.grid()->index(1,1),QStringLiteral("0")));
        auto& c=simplexController(shell);QStringList notices;c.notice=[&](const QString& title,const QString& body){notices<<title<<body;};
        QVERIFY(c.requestAllSlack());QVERIFY(c.start({}));QCOMPARE(notices.size(),2);QCOMPARE(notices.front(),QStringLiteral("Optimal Tableau"));QVERIFY(notices.back().startsWith(QStringLiteral("Iteration 1 is optimal")));
        input(shell,QStringLiteral("01_normal"));QVERIFY(shell.navigate(QStringLiteral("lp/solve-menu")));
        auto* action=shell.findChild<QAction*>(QStringLiteral("solve-all-slack"));QVERIFY(action && action->isEnabled());action->trigger();
        auto* page=shell.currentContent();page->findChild<QRadioButton*>(QStringLiteral("output-format-scientific"))->click();
        page->findChild<QLineEdit*>(QStringLiteral("output-format-N"))->setText(QStringLiteral("2"));page->findChild<QLineEdit*>(QStringLiteral("output-format-D"))->setText(QStringLiteral("3"));
        page->findChild<QPushButton*>(QStringLiteral("output-go"))->click();
        auto* grid=shell.currentContent()->findChild<QTableView*>(QStringLiteral("simplex-grid"));QVERIFY(grid);QCOMPARE(grid->model()->data(grid->model()->index(2,1)).toString(),QStringLiteral("-3.00e+000"));
        c.all();QTRY_VERIFY(!c.busy());QCOMPARE(c.session()->history().back().tableau.rhs(0),10.0);
    }
    void allGoldenCellsInNativeGrid_data() {
        QTest::addColumn<QByteArray>("record");QFile f(QStringLiteral(TORA_SIMPLEX_GOLDENS));QVERIFY(f.open(QIODevice::ReadOnly));
        const auto records=QJsonDocument::fromJson(f.readAll()).array();
        for(const auto& value:records){auto r=value.toObject();if(r[QStringLiteral("requires_artificial")].toBool())continue;QTest::newRow(r[QStringLiteral("case")].toString().toUtf8().constData())<<QJsonDocument(r).toJson(QJsonDocument::Compact);}
    }
    void allGoldenCellsInNativeGrid() {
        QFETCH(QByteArray,record);const auto r=QJsonDocument::fromJson(record).object(),p=r[QStringLiteral("problem")].toObject();
        const auto vars=p[QStringLiteral("variables")].toArray(),constraints=p[QStringLiteral("constraints")].toArray();
        auto problem=tora::lp::LpProblem::blank(static_cast<std::size_t>(vars.size()),static_cast<std::size_t>(constraints.size()));problem.title=p[QStringLiteral("title")].toString().toStdString();
        problem.sense=p[QStringLiteral("sense")].toString()==QStringLiteral("max")?tora::lp::ObjectiveSense::Maximize:tora::lp::ObjectiveSense::Minimize;
        for(int j=0;j<vars.size();++j){const auto v=vars[j].toObject();problem.variables[static_cast<std::size_t>(j)].lowerBound=std::stod(v[QStringLiteral("lower")].toString().toStdString());problem.variables[static_cast<std::size_t>(j)].upperBound=std::stod(v[QStringLiteral("upper")].toString().toStdString());problem.variables[static_cast<std::size_t>(j)].unrestricted=v[QStringLiteral("unrestricted")].toBool();problem.objective[static_cast<std::size_t>(j)]=std::stod(p[QStringLiteral("objective")].toArray()[j].toString().toStdString());}
        for(int i=0;i<constraints.size();++i){const auto row=constraints[i].toObject();auto& target=problem.constraints[static_cast<std::size_t>(i)];target.rhs=std::stod(row[QStringLiteral("rhs")].toString().toStdString());const auto relation=row[QStringLiteral("relation")].toString();target.relation=relation==QStringLiteral("<=")?tora::lp::Relation::LessEqual:relation==QStringLiteral(">=")?tora::lp::Relation::GreaterEqual:tora::lp::Relation::Equal;for(int j=0;j<vars.size();++j)target.coefficients[static_cast<std::size_t>(j)]=std::stod(row[QStringLiteral("coefficients")].toArray()[j].toString().toStdString());}
        auto doc=tora::compat::ToraLpCodec::create(problem);QTemporaryDir temp;QVERIFY(temp.isValid());const auto file=temp.filePath(QStringLiteral("problem.txt"));tora::compat::LpFileStore::saveAs(doc,std::filesystem::path(file.toStdString()));
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));auto& in=inputController(shell);in.dialogs().showError=[](const QString&,const QString&){};in.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(in.openFile(file));
        auto& c=simplexController(shell);QString message;c.notice=[&](const QString&,const QString& body){message=body;};QVERIFY(c.requestAllSlack());QVERIFY(c.start({}));c.all();QTRY_VERIFY(!c.busy());
        if(r[QStringLiteral("operation")].toString()==QStringLiteral("alternative 1")){const int block=constraints.size()+7;c.selectCell((static_cast<int>(c.session()->history().size())-1)*block+1,2);c.next();}
        QCOMPARE(message,r[QStringLiteral("message")].toString());auto* grid=shell.currentContent()->findChild<QTableView*>(QStringLiteral("simplex-grid"));QVERIFY(grid);auto* model=grid->model();const auto tables=r[QStringLiteral("tables")].toArray();const int block=constraints.size()+7;QCOMPARE(model->rowCount(),tables.size()*block);
        for(int k=0;k<tables.size();++k){const auto t=tables[k].toObject();const auto columns=t[QStringLiteral("columns")].toArray(),rows=t[QStringLiteral("rows")].toArray();QCOMPARE(model->data(model->index(k*block,0)).toString(),t[QStringLiteral("heading")].toString());QCOMPARE(model->data(model->index(k*block+1,0)).toString(),QStringLiteral("Basic"));for(int j=0;j<columns.size();++j)QCOMPARE(model->data(model->index(k*block+1,j+1)).toString(),columns[j].toString());for(int i=0;i<rows.size();++i){const auto row=rows[i].toObject();QCOMPARE(model->data(model->index(k*block+i+2,0)).toString(),row[QStringLiteral("label")].toString());const auto values=row[QStringLiteral("values")].toArray();for(int j=0;j<values.size();++j)QCOMPARE(model->data(model->index(k*block+i+2,j+1)).toString(),values[j].toString());}}
    }
    void nativePrinterProducesPdfWithoutChangingTrace() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));input(shell,QStringLiteral("01_normal"));auto& c=simplexController(shell);c.notice=[](const QString&,const QString&){};QVERIFY(c.requestAllSlack());QVERIFY(c.start({}));c.all();QTRY_VERIFY(!c.busy());
        const auto before=c.session()->history().back().tableau.values().data();QTemporaryDir temp;QVERIFY(temp.isValid());
        QPrinter printer(QPrinter::HighResolution);printer.setOutputFormat(QPrinter::PdfFormat);printer.setOutputFileName(temp.filePath(QStringLiteral("trace.pdf")));
        QVERIFY(printSimplexTrace(printer,c,shell.font()));QFile pdf(printer.outputFileName());QVERIFY(pdf.open(QIODevice::ReadOnly));const auto data=pdf.readAll();QVERIFY(data.startsWith("%PDF-"));QVERIFY(data.size()>1000);QCOMPARE(c.session()->history().back().tableau.values().data(),before);
    }
    void largeTableCanScrollAndPauseAutomaticIterations() {
        auto problem=tora::lp::LpProblem::blank(100,100);
        for(std::size_t j=0;j<100;++j){problem.objective[j]=1;problem.constraints[j].coefficients[j]=1;problem.constraints[j].rhs=1;}
        auto doc=tora::compat::ToraLpCodec::create(problem);QTemporaryDir temp;const auto file=temp.filePath(QStringLiteral("large.txt"));tora::compat::LpFileStore::saveAs(doc,std::filesystem::path(file.toStdString()));
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();QTest::qWait(1);const auto scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();shell.resize(qRound(1366/scale),qRound(768/scale));
        auto& in=inputController(shell);in.dialogs().showError=[](const QString&,const QString&){};in.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(in.openFile(file));auto& c=simplexController(shell);c.notice=[](const QString&,const QString&){};QVERIFY(c.requestAllSlack());QVERIFY(c.start({}));
        auto* grid=shell.currentContent()->findChild<QTableView*>(QStringLiteral("simplex-grid"));QVERIFY(grid);QCOMPARE(grid->model()->columnCount(),202);QCOMPARE(grid->model()->rowCount(),107);QApplication::processEvents();QVERIFY(grid->horizontalScrollBar()->maximum()>0);QVERIFY(grid->verticalScrollBar()->maximum()>0);
        c.next();QCOMPARE(c.session()->history().size(),std::size_t(2));c.all();QVERIFY(c.busy());QVERIFY(shell.navigate(QStringLiteral("main")));QVERIFY(!c.busy());const auto count=c.session()->history().size();QTest::qWait(1);QCOMPARE(c.session()->history().size(),count);
        input(shell,QStringLiteral("01_normal"));QVERIFY(!c.session());
    }
    void populatedNativeLayouts() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();QTest::qWait(1);
        input(shell,QStringLiteral("01_normal"));auto& c=simplexController(shell);c.notice=[](const QString&,const QString&){};
        QVERIFY(c.requestAllSlack());QVERIFY(c.start({}));c.all();QTRY_VERIFY(!c.busy());
        const double scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();
        for(const auto size:{QSize(1366,768),QSize(1920,1080),QSize(3840,2160)}) {
            shell.resize(qRound(size.width()/scale),qRound(size.height()/scale));QApplication::processEvents();
            auto* page=shell.currentContent();auto* grid=page->findChild<QTableView*>(QStringLiteral("simplex-grid"));QVERIFY(grid);
            QVERIFY(grid->width()>shell.width()/2 && grid->height()>0);
            auto* heading=shell.findChild<QWidget*>(QStringLiteral("lp/simplex"))->findChild<QLabel*>(QStringLiteral("screen-heading"));QVERIFY(heading && heading->isVisible());QVERIFY(heading->height()>=heading->fontMetrics().height());
            for(auto* button:page->findChildren<QPushButton*>()){QVERIFY(button->width()>=button->minimumSizeHint().width());QVERIFY(page->rect().contains(button->geometry().center()));}
            if(size==QSize(1920,1080) && !qEnvironmentVariable("TORA_UI_CAPTURES").isEmpty()) {
                const auto root=qEnvironmentVariable("TORA_UI_CAPTURES")+QStringLiteral("/simplex");QVERIFY(QDir().mkpath(root));
                QVERIFY(shell.grab().save(root+QStringLiteral("/normal-%1.png").arg(scale)));
            }
        }
    }
};
int main(int argc,char** argv) {
#ifdef Q_OS_WIN
    qputenv("QT_QPA_FONTDIR",qgetenv("SystemRoot")+"/Fonts");
#endif
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc,argv);SimplexTest test;return QTest::qExec(&test,argc,argv);
}
#include "desktop_simplex.moc"
