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
class DesktopBounded:public QObject {
    Q_OBJECT
private slots:
    void actualMenuRoutingWithoutWarnings() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();
        QVERIFY(shell.navigate(QStringLiteral("lp/solve-menu")));
        auto* action=shell.findChild<QAction*>(QStringLiteral("solve-bounded"));QVERIFY(action && !action->isEnabled());
        auto& in=inputController(shell);in.dialogs().askSave=[] {return SaveChoice::No;};
        auto& c=simplexController(shell);int warnings=0;QString title,message;
        c.redirect=[&](const QString&){++warnings;return false;};c.notice=[&](const QString& t,const QString& body){title=t;message=body;};
        for(const auto& file:{QStringLiteral("01_normal.txt"),QStringLiteral("06_equality.txt"),QStringLiteral("03_infeasible.txt")}) {
            QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QLatin1Char('/')+file));
            QVERIFY(shell.navigate(QStringLiteral("lp/solve-menu")));action=shell.findChild<QAction*>(QStringLiteral("solve-bounded"));QVERIFY(action && action->isEnabled());action->trigger();QCOMPARE(shell.currentScreen(),QStringLiteral("lp/output-format"));
            shell.currentContent()->findChild<QPushButton*>(QStringLiteral("output-go"))->click();QVERIFY(c.session());
            const bool artificial=file!=QStringLiteral("01_normal.txt");QCOMPARE(c.session()->method(),artificial?tora::lp::PrimalMethod::TwoPhase:tora::lp::PrimalMethod::AllSlack);
            QCOMPARE(shell.currentContent()->findChild<QLabel*>(QStringLiteral("simplex-method"))->text(),artificial?QStringLiteral("SIMPLEX TABLEAU - (Two-Phase Method)"):QStringLiteral("SIMPLEX TABLEAU - (Starting All-Slack Method)"));
            c.all();QTRY_VERIFY(!c.busy());QCOMPARE(warnings,0);
        }
        QCOMPARE(title,QStringLiteral("Phase 1"));QCOMPARE(message,QStringLiteral("Phase 2 nixed. Phase1 shows the problem has no feasible solution"));
    }
    void manualComplementsLayoutPrintingAndLifecycle() {
        auto p=tora::lp::LpProblem::blank(2,3);p.objective={4,3};p.variables[0].upperBound=1;
        p.constraints[0].coefficients={2,1};p.constraints[0].rhs=4;p.constraints[1].coefficients={1,0};p.constraints[1].rhs=5;p.constraints[2].coefficients={0,1};p.constraints[2].rhs=6;
        QTemporaryDir temp;auto doc=tora::compat::ToraLpCodec::create(p);const auto file=temp.filePath(QStringLiteral("return.txt"));tora::compat::LpFileStore::saveAs(doc,std::filesystem::path(file.toStdString()));
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();auto& in=inputController(shell);in.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(in.openFile(file));auto& c=simplexController(shell);c.notice=[](const QString&,const QString&){};
        QVERIFY(c.requestBounded());QVERIFY(c.start({}));const auto initial=c.session()->history().front().tableau.values().data();
        c.next();QCOMPARE(c.session()->history().back().tableau.columns()[0],std::string("Ux1"));
        c.next();c.next();QCOMPARE(c.session()->history().size(),std::size_t(4));QCOMPARE(c.session()->history().back().tableau.columns()[0],std::string("x1"));QCOMPARE(c.session()->history().back().tableau.rhs(0),12.0);QCOMPARE(c.session()->history().front().tableau.values().data(),initial);
        auto* grid=shell.currentContent()->findChild<QTableView*>(QStringLiteral("simplex-grid"));QVERIFY(grid);
        const auto scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();for(const auto size:{QSize(1366,768),QSize(1920,1080),QSize(3840,2160)}){
            shell.resize(qRound(size.width()/scale),qRound(size.height()/scale));QApplication::processEvents();QVERIFY(grid->height()>0 && grid->width()>shell.width()/2);
            if(grid->horizontalScrollBar()->maximum()==0)QVERIFY(grid->visualRect(grid->model()->index(1,grid->model()->columnCount()-1)).right()<grid->viewport()->width());
            for(auto* button:shell.currentContent()->findChildren<QPushButton*>())QVERIFY(button->width()>=button->minimumSizeHint().width());
            if(size==QSize(1920,1080) && !qEnvironmentVariable("TORA_UI_CAPTURES").isEmpty()){const auto root=qEnvironmentVariable("TORA_UI_CAPTURES")+QStringLiteral("/bounded");QVERIFY(QDir().mkpath(root));QVERIFY(shell.grab().save(root+QStringLiteral("/complement-return-%1.png").arg(scale)));}
        }
        QPrinter printer(QPrinter::HighResolution);printer.setOutputFormat(QPrinter::PdfFormat);printer.setOutputFileName(temp.filePath(QStringLiteral("bounded.pdf")));const auto snapshot=c.session()->history().back().tableau.values().data();QVERIFY(printSimplexTrace(printer,c,shell.font()));QCOMPARE(c.session()->history().back().tableau.values().data(),snapshot);
        QVERIFY(c.requestBounded());QVERIFY(c.start({}));c.all();QVERIFY(c.busy());QVERIFY(shell.navigate(QStringLiteral("main")));QVERIFY(!c.busy());
        QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/01_normal.txt")));QVERIFY(!c.session());QVERIFY(!c.canStart());
    }
    void allGoldenCellsInNativeGrid_data() {
        QTest::addColumn<QByteArray>("record");QFile f(QStringLiteral(TORA_BOUNDED_GOLDENS));QVERIFY(f.open(QIODevice::ReadOnly));
        const auto records=QJsonDocument::fromJson(f.readAll()).array();
        for(const auto& value:records){auto r=value.toObject();QTest::newRow(r[QStringLiteral("case")].toString().toUtf8().constData())<<QJsonDocument(r).toJson(QJsonDocument::Compact);}
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
        auto& c=simplexController(shell);QString message;c.notice=[&](const QString&,const QString& body){message=body;};c.redirect=[](const QString&){return true;};QVERIFY(c.requestBounded());tora::core::NumberFormat format{};if(r.contains(QStringLiteral("format"))){const auto f=r[QStringLiteral("format")].toArray();format={f[0].toInt()?tora::core::Notation::Scientific:tora::core::Notation::Decimal,f[1].toInt(),f[2].toInt()};}QVERIFY(c.start(format));c.all();QTRY_VERIFY(!c.busy());
        if(r[QStringLiteral("operation")].toString()==QStringLiteral("alternative 1")){const int block=constraints.size()+7;c.selectCell((static_cast<int>(c.session()->history().size())-1)*block+1,2);c.next();}
        QCOMPARE(message,r[QStringLiteral("message")].toString());auto* grid=shell.currentContent()->findChild<QTableView*>(QStringLiteral("simplex-grid"));QVERIFY(grid);auto* model=grid->model();const auto tables=r[QStringLiteral("tables")].toArray();const int block=constraints.size()+7;QCOMPARE(model->rowCount(),tables.size()*block);
        for(int k=0;k<tables.size();++k){const auto t=tables[k].toObject();const auto columns=t[QStringLiteral("columns")].toArray(),rows=t[QStringLiteral("rows")].toArray();QCOMPARE(model->data(model->index(k*block,0)).toString(),t[QStringLiteral("heading")].toString());QCOMPARE(model->data(model->index(k*block+1,0)).toString(),QStringLiteral("Basic"));for(int j=0;j<columns.size();++j)QCOMPARE(model->data(model->index(k*block+1,j+1)).toString(),columns[j].toString());for(int i=0;i<rows.size();++i){const auto row=rows[i].toObject();QCOMPARE(model->data(model->index(k*block+i+2,0)).toString(),row[QStringLiteral("label")].toString());const auto values=row[QStringLiteral("values")].toArray();for(int j=0;j<values.size();++j)QCOMPARE(model->data(model->index(k*block+i+2,j+1)).toString(),values[j].toString());}}
    }
    void initTestCase(){
#ifdef Q_OS_WIN
        qputenv("QT_QPA_FONTDIR",(qEnvironmentVariable("SystemRoot")+QStringLiteral("/Fonts")).toUtf8());
#endif
    }
};
QTEST_MAIN(DesktopBounded)
#include "desktop_bounded.moc"
