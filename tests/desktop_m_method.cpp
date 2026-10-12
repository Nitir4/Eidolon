#include "lp_simplex_controller.hpp"
#include "ui_colors.hpp"
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
class DesktopMMethod:public QObject {
    Q_OBJECT
private slots:
    void manualAndMValueWorkflow() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));
        auto& in=inputController(shell);in.dialogs().askSave=[] {return SaveChoice::No;};
        QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/06_equality.txt")));
        auto& c=simplexController(shell);QString notice;c.notice=[&](const QString&,const QString& text){notice=text;};
        QVERIFY(shell.navigate(QStringLiteral("lp/solve-menu")));
        auto* action=shell.findChild<QAction*>(QStringLiteral("solve-m-method"));QVERIFY(action && action->isEnabled());action->trigger();
        QCOMPARE(shell.currentScreen(),QStringLiteral("lp/m-value"));
        auto* value=shell.currentContent()->findChild<QLineEdit*>(QStringLiteral("m-value"));QVERIFY(value && !value->isReadOnly());QCOMPARE(value->text(),QStringLiteral("100"));
        auto* go=shell.currentContent()->findChild<QPushButton*>(QStringLiteral("m-go"));QVERIFY(go && go->isEnabled());go->click();
        QCOMPARE(shell.currentScreen(),QStringLiteral("lp/output-format"));QVERIFY(c.canStart());
        shell.currentContent()->findChild<QPushButton*>(QStringLiteral("output-go"))->click();
        QVERIFY(c.session());QCOMPARE(c.session()->method(),tora::lp::PrimalMethod::MMethod);
        auto* heading=shell.currentContent()->findChild<QLabel*>(QStringLiteral("simplex-method"));QVERIFY(heading);QCOMPARE(heading->text(),QStringLiteral("SIMPLEX TABLEAU - (M-Method)"));
        auto* grid=shell.currentContent()->findChild<QTableView*>(QStringLiteral("simplex-grid"));QVERIFY(grid);
        QCOMPARE(grid->model()->data(grid->model()->index(3,5),Qt::BackgroundRole).value<QColor>(),QColor(Qt::white));
        QCOMPARE(grid->model()->data(grid->model()->index(3,5),Qt::ForegroundRole).value<QColor>(),QColor(colors::text));
        QCOMPARE(grid->model()->data(grid->model()->index(2,5),Qt::BackgroundRole).value<QColor>(),QColor(colors::surface));
        c.selectCell(1,2);QVERIFY(!notice.isEmpty());QVERIFY(!c.session()->history().back().entering);
        c.selectCell(1,1);c.selectCell(4,0);QCOMPARE(c.session()->history().back().leaving,std::optional<std::size_t>(2));c.next();
        QCOMPARE(c.session()->history().size(),std::size_t(2));c.all();QTRY_VERIFY(!c.busy());QCOMPARE(notice,QStringLiteral("Iteration 4 is optimal"));
        QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/03_infeasible.txt")));QVERIFY(!c.session());
        c.redirect=[](const QString&){return false;};QVERIFY(!c.requestAllSlack());QCOMPARE(shell.currentScreen(),QStringLiteral("lp/m-value"));QVERIFY(c.setPenalty(QStringLiteral("100")));QVERIFY(c.start({}));c.all();QTRY_VERIFY(!c.busy());QCOMPARE(notice,QStringLiteral("Iteration 3 is optimal"));
    }
    void allSlackFallbackCancelAlsoContinues() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));auto& in=inputController(shell);in.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/01_normal.txt")));
        auto& c=simplexController(shell);QString warning;c.redirect=[&](const QString& text){warning=text;return false;};c.notice=[](const QString&,const QString&){};QVERIFY(c.requestMMethod());QCOMPARE(warning,QStringLiteral("Problem has all <= constraints. All-Slack option will be used"));QCOMPARE(shell.currentScreen(),QStringLiteral("lp/output-format"));QVERIFY(c.start({}));QCOMPARE(c.session()->method(),tora::lp::PrimalMethod::AllSlack);c.all();QTRY_VERIFY(!c.busy());QCOMPARE(c.session()->history().back().tableau.rhs(0),10.0);
    }
    void scientificResidualsUseNativeOutputControls() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));auto& in=inputController(shell);in.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/10_dual.txt")));
        auto& c=simplexController(shell);c.notice=[](const QString&,const QString&){};QVERIFY(c.requestMMethod());QVERIFY(c.setPenalty(QStringLiteral("100")));
        auto* page=shell.currentContent();page->findChild<QRadioButton*>(QStringLiteral("output-format-scientific"))->click();page->findChild<QLineEdit*>(QStringLiteral("output-format-N"))->setText(QStringLiteral("2"));page->findChild<QPushButton*>(QStringLiteral("output-go"))->click();c.next();c.next();
        auto* grid=shell.currentContent()->findChild<QTableView*>(QStringLiteral("simplex-grid"));QVERIFY(grid);QCOMPARE(grid->model()->data(grid->model()->index(23,1)).toString(),QStringLiteral("1.11e-16"));QCOMPARE(grid->model()->data(grid->model()->index(23,2)).toString(),QStringLiteral("5.55e-17"));QCOMPARE(grid->model()->data(grid->model()->index(25,2)).toString(),QStringLiteral("0.00e+00"));c.next();QCOMPARE(grid->model()->data(grid->model()->index(33,1)).toString(),QStringLiteral("0.00e+00"));
    }
    void allGoldenCellsInNativeGrid_data() {
        QTest::addColumn<QByteArray>("record");QFile f(QStringLiteral(TORA_M_GOLDENS));QVERIFY(f.open(QIODevice::ReadOnly));
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
        auto& c=simplexController(shell);QString message;c.notice=[&](const QString&,const QString& body){message=body;};c.redirect=[](const QString&){return true;};QVERIFY(c.requestMMethod());if(r[QStringLiteral("requires_artificial")].toBool()){QCOMPARE(shell.currentScreen(),QStringLiteral("lp/m-value"));QVERIFY(c.setPenalty(r.contains(QStringLiteral("m_input"))?r[QStringLiteral("m_input")].toString():r[QStringLiteral("m")].toString()));}QVERIFY(c.start({}));c.all();QTRY_VERIFY(!c.busy());
        if(r[QStringLiteral("operation")].toString()==QStringLiteral("alternative 1")){const int block=constraints.size()+7;c.selectCell((static_cast<int>(c.session()->history().size())-1)*block+1,2);c.next();}
        QCOMPARE(message,r[QStringLiteral("message")].toString());auto* grid=shell.currentContent()->findChild<QTableView*>(QStringLiteral("simplex-grid"));QVERIFY(grid);auto* model=grid->model();const auto tables=r[QStringLiteral("tables")].toArray();const int block=constraints.size()+7;QCOMPARE(model->rowCount(),tables.size()*block);
        for(int k=0;k<tables.size();++k){const auto t=tables[k].toObject();const auto columns=t[QStringLiteral("columns")].toArray(),rows=t[QStringLiteral("rows")].toArray();QCOMPARE(model->data(model->index(k*block,0)).toString(),t[QStringLiteral("heading")].toString());QCOMPARE(model->data(model->index(k*block+1,0)).toString(),QStringLiteral("Basic"));for(int j=0;j<columns.size();++j)QCOMPARE(model->data(model->index(k*block+1,j+1)).toString(),columns[j].toString());for(int i=0;i<rows.size();++i){const auto row=rows[i].toObject();QCOMPARE(model->data(model->index(k*block+i+2,0)).toString(),row[QStringLiteral("label")].toString());const auto values=row[QStringLiteral("values")].toArray();for(int j=0;j<values.size();++j)QCOMPARE(model->data(model->index(k*block+i+2,j+1)).toString(),values[j].toString());}}
    }
    void nativeLayoutsAndSessionReset() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();
        auto& in=inputController(shell);in.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(in.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/06_equality.txt")));
        auto& c=simplexController(shell);c.notice=[](const QString&,const QString&){};QVERIFY(c.requestMMethod());
        shell.currentContent()->findChild<QLineEdit*>(QStringLiteral("m-value"))->setText(QStringLiteral("-1"));QVERIFY(c.setPenalty(QStringLiteral("-1")));QVERIFY(c.start({}));c.all();QTRY_VERIFY(!c.busy());
        QVERIFY(c.requestMMethod());QCOMPARE(shell.currentContent()->findChild<QLineEdit*>(QStringLiteral("m-value"))->text(),QStringLiteral("100"));QVERIFY(c.setPenalty(QStringLiteral("100")));QVERIFY(c.start({}));c.all();QTRY_VERIFY(!c.busy());
        const auto scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();
        for(const auto size:{QSize(1366,768),QSize(1920,1080),QSize(3840,2160)}) {
            shell.resize(qRound(size.width()/scale),qRound(size.height()/scale));QApplication::processEvents();
            auto* page=shell.currentContent();auto* grid=page->findChild<QTableView*>(QStringLiteral("simplex-grid"));QVERIFY(grid && grid->width()>shell.width()/2 && grid->height()>0);
            if(grid->horizontalScrollBar()->maximum()==0)QVERIFY(grid->visualRect(grid->model()->index(1,grid->model()->columnCount()-1)).right()<grid->viewport()->width());
            for(auto* button:page->findChildren<QPushButton*>()){QVERIFY(button->width()>=button->minimumSizeHint().width());QVERIFY(page->rect().contains(button->geometry().center()));}
            if(size==QSize(1920,1080) && !qEnvironmentVariable("TORA_UI_CAPTURES").isEmpty()){
                const auto root=qEnvironmentVariable("TORA_UI_CAPTURES")+QStringLiteral("/m-method");QVERIFY(QDir().mkpath(root));QVERIFY(shell.grab().save(root+QStringLiteral("/equality-%1.png").arg(scale)));
            }
        }
        QPrinter printer(QPrinter::HighResolution);printer.setOutputFormat(QPrinter::PdfFormat);QTemporaryDir temp;printer.setOutputFileName(temp.filePath(QStringLiteral("m.pdf")));auto snapshot=c.session()->history().back().tableau.values().data();QVERIFY(printSimplexTrace(printer,c,shell.font()));QCOMPARE(c.session()->history().back().tableau.values().data(),snapshot);
        auto big=tora::lp::LpProblem::blank(100,100);for(std::size_t j=0;j<100;++j){big.objective[j]=1;big.constraints[j].coefficients[j]=1;big.constraints[j].relation=tora::lp::Relation::GreaterEqual;big.constraints[j].rhs=1;}
        auto doc=tora::compat::ToraLpCodec::create(big);const auto file=temp.filePath(QStringLiteral("large.txt"));tora::compat::LpFileStore::saveAs(doc,std::filesystem::path(file.toStdString()));QVERIFY(in.openFile(file));QVERIFY(!c.session());QVERIFY(c.requestMMethod());QVERIFY(c.setPenalty(QStringLiteral("100")));QVERIFY(c.start({}));
        auto* grid=shell.currentContent()->findChild<QTableView*>(QStringLiteral("simplex-grid"));QCOMPARE(grid->model()->columnCount(),302);
        auto* labels=grid->parentWidget()->findChild<QTableView*>(QStringLiteral("simplex-basic-column"));QVERIFY(labels && labels->isVisible());
        grid->horizontalScrollBar()->setValue(grid->horizontalScrollBar()->maximum());QApplication::processEvents();
        QVERIFY(labels->visualRect(labels->model()->index(1,0)).left()>=0);QCOMPARE(labels->model()->data(labels->model()->index(1,0)).toString(),QStringLiteral("Basic"));
        grid->verticalScrollBar()->setValue(grid->verticalScrollBar()->maximum());QCOMPARE(labels->verticalScrollBar()->value(),grid->verticalScrollBar()->value());
        c.next();const auto at=labels->model()->index(111,0);labels->scrollTo(at);QApplication::processEvents();QTest::mouseClick(labels->viewport(),Qt::LeftButton,Qt::NoModifier,labels->visualRect(at).center());QCOMPARE(c.session()->history().back().leaving,std::optional<std::size_t>(2));c.all();QVERIFY(c.busy());QVERIFY(shell.navigate(QStringLiteral("main")));QVERIFY(!c.busy());
    }
    void initTestCase(){
#ifdef Q_OS_WIN
        qputenv("QT_QPA_FONTDIR",(qEnvironmentVariable("SystemRoot")+QStringLiteral("/Fonts")).toUtf8());
#endif
    }
};
QTEST_MAIN(DesktopMMethod)
#include "desktop_m_method.moc"
