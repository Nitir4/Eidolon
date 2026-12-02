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
class DesktopDual:public QObject {
    Q_OBJECT
private slots:
    void menuWarningsAndEquality() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();
        QVERIFY(shell.navigate(QStringLiteral("lp/solve-menu")));
        auto* action=shell.findChild<QAction*>(QStringLiteral("solve-dual"));QVERIFY(action && !action->isEnabled());
        auto& in=inputController(shell);in.dialogs().askSave=[] {return SaveChoice::No;};
        auto& c=simplexController(shell);QString title,body,warning;bool accept=true;int one=0,two=0;
        c.notice=[&](const QString& t,const QString& b){title=t;body=b;};
        c.singleWarning=[&](const QString& b){warning=b;++one;};c.redirect=[&](const QString& b){warning=b;++two;return accept;};
        // TORA rejects equations only after the bound and objective-sign tests: 08_two_phase is a
        // nonnegative-cost minimization with an equation (original capture equation-requirement-warning.png).
        QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/08_two_phase.txt")));
        QVERIFY(!c.requestDual());QCOMPARE(title,QStringLiteral("Dual SImplex Error"));
        QCOMPARE(body,QStringLiteral("Equations must be converted to inequalities before applying Dual Simplex"));QVERIFY(!c.canStart() && !c.session());QCOMPARE(one+two,0);
        QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/10_dual.txt")));
        QVERIFY(shell.navigate(QStringLiteral("lp/solve-menu")));action=shell.findChild<QAction*>(QStringLiteral("solve-dual"));QVERIFY(action && action->isEnabled());action->trigger();
        QCOMPARE(shell.currentScreen(),QStringLiteral("lp/output-format"));QVERIFY(c.start({}));QCOMPARE(c.session()->method(),tora::lp::PrimalMethod::Dual);
        QCOMPARE(shell.currentContent()->findChild<QLabel*>(QStringLiteral("simplex-method"))->text(),QStringLiteral("SIMPLEX TABLEAU - (Dual Simplex Method)"));QCOMPARE(one+two,0);
        QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/01_normal.txt")));QVERIFY(c.requestDual());QCOMPARE(one,1);
        QCOMPARE(warning,QStringLiteral("Dual Simplex not applicable directly. All-slack will be used"));QVERIFY(c.start({}));QCOMPARE(c.session()->method(),tora::lp::PrimalMethod::AllSlack);
        QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/03_infeasible.txt")));QVERIFY(c.requestDual());QCOMPARE(two,1);
        QCOMPARE(warning,QStringLiteral("Dual simplex not applicable directly. Two-phase method will be used; else click CANCEL to use M-method"));QVERIFY(c.start({}));QCOMPARE(c.session()->method(),tora::lp::PrimalMethod::TwoPhase);
        accept=false;QVERIFY(c.requestDual());QCOMPARE(shell.currentScreen(),QStringLiteral("lp/m-value"));QVERIFY(c.canEnterM());QVERIFY(!c.canStart());QVERIFY(c.setPenalty(QStringLiteral("100")));QVERIFY(c.start({}));QCOMPARE(c.session()->method(),tora::lp::PrimalMethod::MMethod);
        // An equation model whose objective is not dual feasible is converted, not rejected
        // (original TORA on 06_equality, LP06-Q02, TP07, AP02 and AP07, 23 November 2026).
        accept=true;one=two=0;title.clear();
        QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/06_equality.txt")));QVERIFY(c.requestDual());QCOMPARE(two,1);QVERIFY(title.isEmpty());
        QCOMPARE(warning,QStringLiteral("Dual simplex not applicable directly. Two-phase method will be used; else click CANCEL to use M-method"));QVERIFY(c.start({}));QCOMPARE(c.session()->method(),tora::lp::PrimalMethod::TwoPhase);
        accept=false;QVERIFY(c.requestDual());QCOMPARE(shell.currentScreen(),QStringLiteral("lp/m-value"));QVERIFY(c.setPenalty(QStringLiteral("100")));QVERIFY(c.start({}));QCOMPARE(c.session()->method(),tora::lp::PrimalMethod::MMethod);
    }
    void dualSetupOrderMatchesTora() {
        using namespace tora::lp;
        // Order observed in original TORA: (1) bound or unrestricted variable, (2) objective sign, (3) equations.
        auto model=[](ObjectiveSense sense,std::vector<double> cost,std::vector<Relation> relations,bool freeFirst){
            LpProblem p;p.title="order";p.sense=sense;p.objective=cost;
            for(std::size_t j=0;j<cost.size();++j){Variable v;v.name="x"+std::to_string(j+1);v.unrestricted=freeFirst&&j==0;p.variables.push_back(v);}
            for(auto r:relations){Constraint c;c.coefficients=std::vector<double>(cost.size(),1.0);c.relation=r;c.rhs=1;p.constraints.push_back(c);}
            return p;};
        // Free variable plus an equation (LP09-Q02): bounds test fires first.
        QCOMPARE(PrimalSimplexSession::dualSetup(model(ObjectiveSense::Minimize,{1,2},{Relation::Equal,Relation::LessEqual},true)),PrimalMethod::TwoPhase);
        // Not dual feasible plus an equation: converted to Two-phase.
        QCOMPARE(PrimalSimplexSession::dualSetup(model(ObjectiveSense::Maximize,{2,1},{Relation::Equal,Relation::GreaterEqual},false)),PrimalMethod::TwoPhase);
        // Not dual feasible, all <=: All-slack.
        QCOMPARE(PrimalSimplexSession::dualSetup(model(ObjectiveSense::Maximize,{3,2},{Relation::LessEqual},false)),PrimalMethod::AllSlack);
        // Dual feasible plus an equation: rejected.
        bool threw=false;try{(void)PrimalSimplexSession::dualSetup(model(ObjectiveSense::Minimize,{2,1},{Relation::Equal,Relation::GreaterEqual},false));}catch(const std::invalid_argument& e){threw=true;QCOMPARE(QString::fromUtf8(e.what()),QStringLiteral("Equations must be converted to inequalities before applying Dual Simplex"));}
        QVERIFY(threw);
        // Dual feasible, inequalities only: applied directly.
        QCOMPARE(PrimalSimplexSession::dualSetup(model(ObjectiveSense::Minimize,{1,2},{Relation::GreaterEqual,Relation::GreaterEqual},false)),PrimalMethod::Dual);
    }
    void manualHistoryLayoutPrintingAndLifecycle() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();
        auto& in=inputController(shell);in.dialogs().askSave=[] {return SaveChoice::No;};
        QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/10_dual.txt")));
        auto& c=simplexController(shell);QString title,body;c.notice=[&](const QString& t,const QString& b){title=t;body=b;};
        QVERIFY(c.requestDual());QVERIFY(c.start({}));const auto initial=c.session()->history().front().tableau.values().data();
        c.selectCell(1,1);QCOMPARE(title,QStringLiteral("Entering Variable"));QVERIFY(!c.session()->history().back().entering);
        c.selectCell(4,0);QCOMPARE(c.session()->history().back().leaving,std::optional<std::size_t>(2));QVERIFY(!c.session()->history().back().entering);
        c.selectCell(1,2);QCOMPARE(c.session()->history().back().leaving,std::optional<std::size_t>(2));
        c.next();QCOMPARE(c.session()->history().size(),std::size_t(2));c.all();QTRY_VERIFY(!c.busy());QCOMPARE(body,QStringLiteral("Iteration 4 is optimal"));
        QCOMPARE(c.session()->history().front().tableau.values().data(),initial);
        auto* grid=shell.currentContent()->findChild<QTableView*>(QStringLiteral("simplex-grid"));auto* labels=shell.currentContent()->findChild<QTableView*>(QStringLiteral("simplex-basic-column"));QVERIFY(grid && labels);
        const auto scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();
        for(const auto size:{QSize(1366,768),QSize(1920,1080),QSize(3840,2160)}){
            shell.resize(qRound(size.width()/scale),qRound(size.height()/scale));QApplication::processEvents();QVERIFY(grid->width()>shell.width()/2 && grid->height()>0);
            grid->verticalScrollBar()->setValue(grid->verticalScrollBar()->maximum());QCOMPARE(labels->verticalScrollBar()->value(),grid->verticalScrollBar()->value());
            for(auto* button:shell.currentContent()->findChildren<QPushButton*>())QVERIFY(button->width()>=button->minimumSizeHint().width());
            if(size==QSize(1920,1080) && !qEnvironmentVariable("TORA_UI_CAPTURES").isEmpty()){
                const auto root=qEnvironmentVariable("TORA_UI_CAPTURES")+QStringLiteral("/dual");QVERIFY(QDir().mkpath(root));QVERIFY(shell.grab().save(root+QStringLiteral("/seed-%1.png").arg(scale)));
            }
        }
        QTemporaryDir temp;QPrinter printer(QPrinter::HighResolution);printer.setOutputFormat(QPrinter::PdfFormat);printer.setOutputFileName(temp.filePath(QStringLiteral("dual.pdf")));
        const auto last=c.session()->history().back().tableau.values().data();QVERIFY(printSimplexTrace(printer,c,shell.font()));QCOMPARE(c.session()->history().back().tableau.values().data(),last);
        QVERIFY(c.requestDual());QVERIFY(c.start({}));c.all();QVERIFY(c.busy());QVERIFY(shell.navigate(QStringLiteral("main")));QVERIFY(!c.busy());
        QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/01_normal.txt")));QVERIFY(!c.session() && !c.canStart());
    }
    void allGoldenCellsInNativeGrid_data() {
        QTest::addColumn<QByteArray>("record");QFile f(QStringLiteral(TORA_DUAL_GOLDENS));QVERIFY(f.open(QIODevice::ReadOnly));
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
        auto& c=simplexController(shell);QString message;c.notice=[&](const QString&,const QString& body){message=body;};c.redirect=[&](const QString&){return r[QStringLiteral("route")].toString()!=QStringLiteral("m");};c.singleWarning=[](const QString&){};QVERIFY(c.requestDual());if(r[QStringLiteral("route")].toString()==QStringLiteral("m"))QVERIFY(c.setPenalty(r[QStringLiteral("m")].toString()));tora::core::NumberFormat format{};if(r.contains(QStringLiteral("format"))){const auto f=r[QStringLiteral("format")].toArray();format={f[0].toInt()?tora::core::Notation::Scientific:tora::core::Notation::Decimal,f[1].toInt(),f[2].toInt()};}QVERIFY(c.start(format));
        const auto actual=r[QStringLiteral("actual_method")].toString();
        QCOMPARE(c.session()->method(),actual==QStringLiteral("dual")?tora::lp::PrimalMethod::Dual:actual==QStringLiteral("two_phase")?tora::lp::PrimalMethod::TwoPhase:actual==QStringLiteral("m_method")?tora::lp::PrimalMethod::MMethod:tora::lp::PrimalMethod::AllSlack);
        c.all();QTRY_VERIFY(!c.busy());
        if(r[QStringLiteral("operation")].toString()==QStringLiteral("alternative 1")){const int block=constraints.size()+7;const auto priorNotice=message;c.selectCell((static_cast<int>(c.session()->history().size())-1)*block+1,2);c.next();QCOMPARE(message,priorNotice);QVERIFY(!c.canNext());}
        QCOMPARE(message,r[QStringLiteral("message")].toString());auto* grid=shell.currentContent()->findChild<QTableView*>(QStringLiteral("simplex-grid"));QVERIFY(grid);auto* model=grid->model();const auto tables=r[QStringLiteral("tables")].toArray();const int block=constraints.size()+7;QCOMPARE(model->rowCount(),tables.size()*block);
        for(int k=0;k<tables.size();++k){const auto t=tables[k].toObject();const auto columns=t[QStringLiteral("columns")].toArray(),rows=t[QStringLiteral("rows")].toArray();QCOMPARE(model->data(model->index(k*block,0)).toString(),t[QStringLiteral("heading")].toString());QCOMPARE(model->data(model->index(k*block+1,0)).toString(),QStringLiteral("Basic"));for(int j=0;j<columns.size();++j)QCOMPARE(model->data(model->index(k*block+1,j+1)).toString(),columns[j].toString());for(int i=0;i<rows.size();++i){const auto row=rows[i].toObject();QCOMPARE(model->data(model->index(k*block+i+2,0)).toString(),row[QStringLiteral("label")].toString());const auto values=row[QStringLiteral("values")].toArray();for(int j=0;j<values.size();++j)QCOMPARE(model->data(model->index(k*block+i+2,j+1)).toString(),values[j].toString());}}
    }
    void initTestCase(){
#ifdef Q_OS_WIN
        qputenv("QT_QPA_FONTDIR",(qEnvironmentVariable("SystemRoot")+QStringLiteral("/Fonts")).toUtf8());
#endif
    }
};
QTEST_MAIN(DesktopDual)
#include "desktop_dual.moc"
