#include "games_screens.hpp"
#include "ui_font.hpp"
#include <QApplication>
#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QPrinter>
#include <QTableWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>
#include <QTabWidget>
using namespace tora::desktop;
class DesktopGames:public QObject {
 Q_OBJECT
private slots:
 void initTestCase(){QApplication::setFont(readableUiFont(QApplication::font()));}
 void gridFormatsAndCancel(){ApplicationShell shell;auto& c=gamesController(shell);QString e;QVERIFY(c.initialize("Edit",2,2,e));QVERIFY(c.editCell(0,0,"3",e));QVERIFY(c.editCell(0,1,"4",e));QVERIFY(c.selectLine(true,0));QVERIFY(c.copySelection());QVERIFY(c.selectLine(true,1));QVERIFY(c.editSelection(1,e));QCOMPARE(c.document()->payoff()(1,0),3.);QVERIFY(c.editSelection(0,e));QCOMPARE(c.document()->payoff()(1,0),0.);QVERIFY(c.selectLine(false,0,true));QVERIFY(c.editSelection(3,e));QCOMPARE(c.document()->payoff().columns(),std::size_t(3));QCOMPARE(c.document()->payoff()(0,1),3.);QVERIFY(c.editSelection(0,e));QVERIFY(c.selectLine(true,1));QVERIFY(c.editSelection(2,e));QCOMPARE(c.document()->payoff().rows(),std::size_t(1));QVERIFY(c.selectLine(true,0));QVERIFY(!c.editSelection(2,e));tora::core::NumberFormat f;f.notation=tora::core::Notation::Scientific;f.n=5;f.d=3;QVERIFY(c.setFormat(f,e));QCOMPARE(c.document()->format().d,3);c.dialogs().askSave=[] {return SaveChoice::Cancel;};QVERIFY(!c.leaveInput());QVERIFY(c.dirty());c.dialogs().askSave=[] {return SaveChoice::No;};QVERIFY(c.leaveInput());}
 void originalRecords(){const std::string bytes="9000\r\n5\r\n2\r\n1\r\n\"Class game asymmetric\"\r\n2\r\n2\r\n\"3\"\r\n\"0\"\r\n\"1\"\r\n\"2\"\r\n";auto d=GamesDocument::decode(bytes);QCOMPARE(d.encode(),bytes);d.setCell(0,0,"4.25");auto next=GamesDocument::decode(d.encode());QCOMPARE(next.payoff()(0,0),4.25);next.setTitle("A \"quoted\" title\\path");QCOMPARE(GamesDocument::decode(next.encode()).title(),next.title());next.setCell(0,0," +4.25 ");QCOMPARE(next.payoff()(0,0),4.25);QVERIFY_EXCEPTION_THROWN(next.setCell(0,0,"nan"),std::invalid_argument);QCOMPARE(next.payoff()(0,0),4.25);QVERIFY_EXCEPTION_THROWN(GamesDocument::decode(bytes+"junk"),std::invalid_argument);}
 void solveFilesAndInvalidation(){ApplicationShell shell;auto& c=gamesController(shell);QString e;QVERIFY(c.initialize("Asymmetric",2,2,e));for(int r=0;r<2;++r)for(int col=0;col<2;++col)QVERIFY(c.editCell(r,col,QString::number(r==0?(col==0?3:0):(col==0?1:2)),e));QVERIFY(c.solve(e));QCOMPARE(c.result()->value,1.5);QCOMPARE(c.result()->rowStrategy[0],.25);QVERIFY(c.outputText().contains("A1 = 0.25"));QVERIFY(c.solve(e,true));QCOMPARE(c.graph()->probability,.25);QTemporaryDir dir;auto path=dir.filePath("game.txt");QVERIFY(c.saveTo(path,e));QVERIFY(c.openFile(path,e,true));QVERIFY(!c.editCell(0,0,"5",e));QVERIFY(!c.saveTo(path,e));QVERIFY(c.saveTo(dir.filePath("copy.txt"),e));QVERIFY(!c.readOnly());QVERIFY(c.solve(e));QVERIFY(!c.openFile(dir.filePath("absent"),e));QVERIFY(c.result());QVERIFY(!c.editCell(0,0,"inf",e));QVERIFY(c.result());QVERIFY(c.editCell(0,0,"5",e));QVERIFY(!c.result());QVERIFY(!c.graph());}
 void nativeAlgebraicSolution() {
  ApplicationShell shell;
  QVERIFY(shell.registerModule(gamesModule()));
  auto& controller = gamesController(shell);
  QString error;
  QVERIFY(controller.initialize("Asymmetric", 2, 3, error));
  QVERIFY(controller.setFormat({tora::core::Notation::Decimal, 5, 4}, error));
  const int payoff[2][3] = {{5, 2, 0}, {1, 3, 4}};
  for (int row = 0; row < 2; ++row)
   for (int col = 0; col < 3; ++col)
    QVERIFY(controller.editCell(row, col, QString::number(payoff[row][col]), error));
  QVERIFY(controller.solve(error));
  QVERIFY(shell.navigate("games/result"));
  auto* page = shell.currentContent();
  auto* tabs = page->findChild<QTabWidget*>();
  auto* value = page->findChild<QLabel*>("games-value");
  auto* playerA = page->findChild<QTableWidget*>("games-strategies-a");
  auto* playerB = page->findChild<QTableWidget*>("games-strategies-b");
  auto* lp = page->findChild<QTableWidget*>("games-lp-formulation");
  QVERIFY(tabs && value && playerA && playerB && lp);
  QCOMPARE(tabs->tabText(0), "Solution");
  QCOMPARE(tabs->tabText(1), "Steps / Iterations");
  QVERIFY(value->text().endsWith("2.5000"));
  QCOMPARE(playerA->rowCount(), 2);
  QCOMPARE(playerB->rowCount(), 3);
  QCOMPARE(playerA->item(0, 0)->text(), "A1");
  QCOMPARE(playerA->item(0, 1)->text(), "0.3750");
  QCOMPARE(playerB->item(2, 0)->text(), "B3");
  QCOMPARE(lp->rowCount(), 5);
  QCOMPARE(lp->columnCount(), 6);
  QCOMPARE(lp->horizontalHeaderItem(1)->text(), "v");
  QCOMPARE(lp->item(1, 2)->text(), "-5.0000");
  QCOMPARE(lp->item(1, 4)->text(), "≤");
  QCOMPARE(lp->item(4, 4)->text(), "=");
  QCOMPARE(lp->item(4, 5)->text(), "1.0000");
  QCOMPARE(lp->editTriggers(), QAbstractItemView::NoEditTriggers);

  QVERIFY(controller.initialize("Single strategy", 1, 1, error));
  QVERIFY(controller.solve(error));
  QCOMPARE(lp->columnCount(), 5);
  QVERIFY(!lp->item(0, 3));
  QVERIFY(!lp->item(0, 4));
 }
 void nativeGraphAndPdf(){ApplicationShell shell;QVERIFY(shell.registerModule(gamesModule()));auto& c=gamesController(shell);QString e;QVERIFY(c.initialize("Graph",2,3,e));const double a[2][3]={{5,2,0},{1,3,4}};for(int r=0;r<2;++r)for(int col=0;col<3;++col)QVERIFY(c.editCell(r,col,QString::number(a[r][col]),e));QVERIFY(c.solve(e,true));QCOMPARE(c.result()->value,2.5);QCOMPARE(c.result()->rowStrategy[0],.375);QVERIFY(shell.navigate("games/graphical"));shell.resize(640,480);shell.show();QTest::qWait(20);QVERIFY(shell.currentContent()->findChild<QWidget*>("games-graph"));const auto artifact=QDir::current().filePath("desktop-games-native-scale-"+qEnvironmentVariable("QT_SCALE_FACTOR","1"));QVERIFY(shell.grab().save(artifact+".png"));auto* in=shell.currentContent()->findChild<QPushButton*>("games-zoom-in");QVERIFY(in);QTest::mouseClick(in,Qt::LeftButton);QTemporaryDir dir;QPrinter printer;printer.setOutputFormat(QPrinter::PdfFormat);auto path=dir.filePath("game.pdf");printer.setOutputFileName(path);QVERIFY(printGames(printer,c));QFile pdf(path);QVERIFY(pdf.open(QIODevice::ReadOnly));QVERIFY(pdf.readAll().startsWith("%PDF"));pdf.close();QFile::remove(artifact+".pdf");QVERIFY(QFile::copy(path,artifact+".pdf"));QVERIFY(shell.navigate("games/input"));auto* grid=shell.currentContent()->findChild<QTableWidget*>("games-input-grid");QCOMPARE(grid->columnCount(),3);c.dialogs().showError=[](const QString&,const QString&){};grid->item(0,0)->setText("invalid");QVERIFY(!c.solve(e));QVERIFY(c.result());grid->item(0,0)->setText("6");QVERIFY(!c.result());QVERIFY(c.solve(e));}
};
QTEST_MAIN(DesktopGames)
#include "desktop_games.moc"
