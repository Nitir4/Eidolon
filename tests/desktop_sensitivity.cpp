#include "lp_final_controller.hpp"
#include "lp_input_controller.hpp"
#include "lp_screens.hpp"
#include <QTest>
#include <QApplication>
#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTemporaryDir>
#include <QDir>
#include <QPrinter>
using namespace tora::desktop;
class DesktopSensitivity:public QObject {
 Q_OBJECT
private slots:
 void initTestCase(){for(auto ch:QStringLiteral("Eidolon 0123456789"))QVERIFY(QFontMetrics(QApplication::font()).inFont(ch));}
 void lifecycleLayoutAndPrint(){
  ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();QVERIFY(shell.navigate(QStringLiteral("lp/solve-menu")));
  auto* action=shell.findChild<QAction*>(QStringLiteral("solve-final"));QVERIFY(action && !action->isEnabled());
  auto& input=inputController(shell);input.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(input.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/01_normal.txt")));
  shell.navigate(QStringLiteral("lp/solve-menu"));action=shell.findChild<QAction*>(QStringLiteral("solve-final"));QVERIFY(action && action->isEnabled());
  auto& c=finalController(shell);c.notice=[](const QString&,const QString&){};action->trigger();QCOMPARE(shell.currentScreen(),QStringLiteral("lp/output-format"));
  QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("output-go")),Qt::LeftButton);QCOMPARE(shell.currentScreen(),QStringLiteral("lp/final"));
  QVERIFY(c.session());auto* view=shell.currentContent()->findChild<QTableView*>(QStringLiteral("summary-grid"));QVERIFY(view);
  QVERIFY(!shell.currentContent()->findChild<QPushButton*>(QStringLiteral("summary-next"))->isEnabled());QVERIFY(!shell.currentContent()->findChild<QPushButton*>(QStringLiteral("summary-all"))->isEnabled());
  const double scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();
  for(auto size:{QSize(1366,768),QSize(1920,1080),QSize(3840,2160)}){
   const QSize logical(qRound(size.width()/scale),qRound(size.height()/scale));shell.resize(logical);QApplication::processEvents();QCOMPARE(shell.size(),logical);QVERIFY(view->viewport()->width()>100 && view->viewport()->height()>100);
   for(auto* b:shell.currentContent()->findChildren<QPushButton*>())QVERIFY(b->width()>=b->minimumSizeHint().width());
   if(!qEnvironmentVariable("TORA_UI_CAPTURES").isEmpty()){const auto root=qEnvironmentVariable("TORA_UI_CAPTURES")+QStringLiteral("/sensitivity");QVERIFY(QDir().mkpath(root));QVERIFY(shell.grab().save(root+QStringLiteral("/normal-%1x%2-%3.png").arg(size.width()).arg(size.height()).arg(scale)));}
  }
  QTemporaryDir tmp;QPrinter printer(QPrinter::HighResolution);printer.setOutputFormat(QPrinter::PdfFormat);printer.setOutputFileName(tmp.filePath(QStringLiteral("summary.pdf")));const auto before=c.session()->solver().history().size();QVERIFY(printSummary(printer,c,shell.font()));QFile pdf(printer.outputFileName());QVERIFY(pdf.open(QIODevice::ReadOnly));QVERIFY(pdf.size()>1000);QCOMPARE(pdf.read(5),QByteArray("%PDF-"));QCOMPARE(c.session()->solver().history().size(),before);
  QTest::mouseClick(shell.currentContent()->findChild<QPushButton*>(QStringLiteral("summary-view-input")),Qt::LeftButton);QCOMPARE(shell.currentScreen(),QStringLiteral("lp/input"));
  QVERIFY(input.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/03_infeasible.txt")));QVERIFY(!c.session() && !c.canStart());
 }
 void goldens_data(){QTest::addColumn<QJsonObject>("record");QFile f(QStringLiteral(TORA_FINAL_GOLDENS));QVERIFY(f.open(QIODevice::ReadOnly));for(auto v:QJsonDocument::fromJson(f.readAll()).array()){const auto r=v.toObject();QTest::newRow(qPrintable(r["case"].toString()))<<r;}}
 void goldens(){
  QFETCH(QJsonObject,record);ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();auto& input=inputController(shell);input.dialogs().askSave=[] {return SaveChoice::No;};
  const auto source=record["problem"].toObject();const auto vars=source["variables"].toArray(),cons=source["constraints"].toArray();
  auto problem=tora::lp::LpProblem::blank(static_cast<std::size_t>(vars.size()),static_cast<std::size_t>(cons.size()));problem.title=source["title"].toString().toStdString();problem.sense=source["sense"]==QStringLiteral("min")?tora::lp::ObjectiveSense::Minimize:tora::lp::ObjectiveSense::Maximize;
  for(int j=0;j<vars.size();++j){const auto v=vars[j].toObject();auto& x=problem.variables[static_cast<std::size_t>(j)];x.name=v["name"].toString().toStdString();x.lowerBound=std::stod(v["lower"].toString().toStdString());x.upperBound=std::stod(v["upper"].toString().toStdString());x.unrestricted=v["unrestricted"].toBool();problem.objective[static_cast<std::size_t>(j)]=std::stod(source["objective"].toArray()[j].toString().toStdString());}
  for(int i=0;i<cons.size();++i){const auto v=cons[i].toObject();auto& r=problem.constraints[static_cast<std::size_t>(i)];r.rhs=std::stod(v["rhs"].toString().toStdString());const auto relation=v["relation"].toString();r.relation=relation==QStringLiteral("<=")?tora::lp::Relation::LessEqual:relation==QStringLiteral(">=")?tora::lp::Relation::GreaterEqual:tora::lp::Relation::Equal;for(int j=0;j<vars.size();++j)r.coefficients[static_cast<std::size_t>(j)]=std::stod(v["coefficients"].toArray()[j].toString().toStdString());}
  QTemporaryDir temp;auto document=tora::compat::ToraLpCodec::create(problem);const auto file=temp.filePath(QStringLiteral("problem.txt"));tora::compat::LpFileStore::saveAs(document,std::filesystem::path(file.toStdString()));QVERIFY(input.openFile(file));auto& c=finalController(shell);c.notice=[](const QString&,const QString&){};QVERIFY(c.request());const auto f=record["format"].toArray();QVERIFY(c.start({f[0].toInt()?tora::core::Notation::Scientific:tora::core::Notation::Decimal,f[1].toInt(),f[2].toInt()}));
  QCOMPARE(shell.currentScreen(),QStringLiteral("lp/final"));QCOMPARE(shell.currentContent()->findChild<QLabel*>(QStringLiteral("summary-iteration"))->text(),QStringLiteral("Final Iteration No.: ")+QString::number(record["iteration"].toInt()));
  auto* view=shell.currentContent()->findChild<QTableView*>(QStringLiteral("summary-grid"));QVERIFY(view);int row=0;
  for(auto section:record["sections"].toArray()){
   const auto s=section.toObject();if(s.contains("heading")){QCOMPARE(view->model()->index(row++,0).data().toString(),s["heading"].toString());}int col=0;for(auto label:s["columns"].toArray())QCOMPARE(view->model()->index(row,col++).data().toString(),label.toString());++row;
   for(auto item:s["rows"].toArray()){const auto r=item.toObject();QCOMPARE(view->model()->index(row,0).data().toString(),r["label"].toString());col=1;for(auto cell:r["values"].toArray())QCOMPARE(view->model()->index(row,col++).data().toString(),cell.toString());++row;}++row;
  }
  QCOMPARE(shell.currentContent()->findChild<QLabel*>(QStringLiteral("summary-message"))->text(),record["message"].toString());
  if(!qEnvironmentVariable("TORA_UI_CAPTURES").isEmpty()){shell.resize(1920,1080);QApplication::processEvents();const auto root=qEnvironmentVariable("TORA_UI_CAPTURES")+QStringLiteral("/sensitivity/cases");QVERIFY(QDir().mkpath(root));QVERIFY(shell.grab().save(root+QStringLiteral("/%1-%2.png").arg(record["case"].toString(),qEnvironmentVariable("QT_SCALE_FACTOR"))));}
 }
};
int main(int argc,char** argv){
#ifdef Q_OS_WIN
 qputenv("QT_QPA_FONTDIR",qgetenv("SystemRoot")+"/Fonts");
#endif
 QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);QApplication app(argc,argv);DesktopSensitivity test;return QTest::qExec(&test,argc,argv);
}
#include "desktop_sensitivity.moc"
