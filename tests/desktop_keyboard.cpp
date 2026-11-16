#include "application_shell.hpp"
#include "keyboard_navigation.hpp"
#include "lp_screens.hpp"
#include "lp_input_controller.hpp"
#include "lp_simplex_controller.hpp"
#include "lp_final_controller.hpp"
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDir>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QTableView>
#include <QTest>
#include <QToolButton>
#include <QVBoxLayout>
using namespace tora::desktop;
namespace {
class RejectModel final : public QStandardItemModel {
public:
    using QStandardItemModel::QStandardItemModel;
    bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole) override {
        if (role==Qt::EditRole && value.toString()=="bad") return false;
        return QStandardItemModel::setData(index,value,role);
    }
};
void capture(QWidget& widget,const QString& name) {
    const auto root=qEnvironmentVariable("TORA_UI_CAPTURES");
    if (!root.isEmpty()) { QDir().mkpath(root); QVERIFY(widget.grab().save(root+"/"+name+"-"+qEnvironmentVariable("QT_SCALE_FACTOR")+".png")); }
}
}
class DesktopKeyboard final : public QObject {
    Q_OBJECT
private slots:
    void controlsActivateOnEnter() {
        ApplicationShell shell; QVERIFY(shell.registerModule(lpModule())); shell.show();
        QCOMPARE(shell.currentScreen(),QString("main"));
        auto* lp=shell.currentContent()->findChild<QPushButton*>("main-lp"); QVERIFY(lp);
        lp->setFocus(); QTest::keyClick(lp,Qt::Key_Enter); QCOMPARE(shell.currentScreen(),QString("lp/input-mode"));
        QWidget window; QVBoxLayout layout(&window); auto* check=new QCheckBox("Test"); layout.addWidget(check);
        auto* combo=new QComboBox; combo->addItems({"One","Two"}); layout.addWidget(combo);
        installEnterActivation(window); window.show(); QVERIFY(QTest::qWaitForWindowActive(&window)); check->setFocus(); QTest::keyClick(check,Qt::Key_Return); QVERIFY(check->isChecked());
        combo->setFocus(); QTest::keyClick(combo,Qt::Key_Return); QVERIFY(combo->view()->isVisible());
        QTest::keyClick(combo->view(),Qt::Key_Down); QTest::keyClick(combo->view(),Qt::Key_Return); QCOMPARE(combo->currentIndex(),1);
    }
    void excelEditingAndHiddenCells() {
        QWidget window; QVBoxLayout layout(&window); QTableView grid; layout.addWidget(&grid);
        QStandardItemModel model(4,4); grid.setModel(&model);
        for(int r=0;r<4;++r)for(int c=0;c<4;++c)model.setItem(r,c,new QStandardItem("123"));
        model.item(1,0)->setEditable(false); grid.hideColumn(1); grid.hideRow(2);
        installSpreadsheetNavigation(window); window.show(); QVERIFY(QTest::qWaitForWindowActive(&window)); grid.setFocus(); grid.setCurrentIndex(model.index(0,0));
        QTest::keyClick(&grid,Qt::Key_Return); QCOMPARE(grid.currentIndex(),model.index(3,0));
        QVERIFY(!qobject_cast<QLineEdit*>(QApplication::focusWidget()));
        QTest::keyClick(&grid,Qt::Key_Return,Qt::ShiftModifier); QCOMPARE(grid.currentIndex(),model.index(0,0));
        QTest::keyClick(&grid,Qt::Key_7); QApplication::processEvents(); auto* editor=qobject_cast<QLineEdit*>(QApplication::focusWidget()); QVERIFY(editor); QTest::keyClicks(editor,".25"); QCOMPARE(editor->text(),QString("7.25"));
        QTest::keyClick(editor,Qt::Key_Tab); QCOMPARE(model.index(0,0).data().toString(),QString("7.25")); QCOMPARE(grid.currentIndex(),model.index(0,2));
        QTest::keyClick(&grid,Qt::Key_Backtab); QCOMPARE(grid.currentIndex(),model.index(0,0));
        QTest::keyClick(&grid,Qt::Key_F2); editor=qobject_cast<QLineEdit*>(QApplication::focusWidget()); QVERIFY(editor); QCOMPARE(editor->text(),QString("7.25"));
        QTest::keyClick(editor,Qt::Key_Left); QCOMPARE(grid.currentIndex(),model.index(0,0)); QCOMPARE(editor->cursorPosition(),3);
        QTest::keyClicks(editor,"9"); QTest::keyClick(editor,Qt::Key_Escape); QCOMPARE(model.index(0,0).data().toString(),QString("7.25"));
        grid.setCurrentIndex(model.index(3,3)); QTest::keyClick(&grid,Qt::Key_Return); QCOMPARE(grid.currentIndex(),model.index(3,3));
        QTest::keyClick(&grid,Qt::Key_Tab); QCOMPARE(grid.currentIndex(),model.index(3,3));
    }
    void clipboardBlocksAndRejectedWrites() {
        QWidget window; QVBoxLayout layout(&window); QTableView grid; layout.addWidget(&grid); RejectModel model(3,3); grid.setModel(&model);
        for(int r=0;r<3;++r)for(int c=0;c<3;++c)model.setItem(r,c,new QStandardItem("0"));
        installSpreadsheetNavigation(window); window.show(); QVERIFY(QTest::qWaitForWindowActive(&window)); grid.setFocus(); grid.setCurrentIndex(model.index(0,0));
        QApplication::clipboard()->setText("1\t2\r\n3\t4\r\n"); QTest::keyClick(&grid,Qt::Key_V,Qt::ControlModifier);
        QCOMPARE(model.index(0,0).data().toString(),QString("1")); QCOMPARE(model.index(1,1).data().toString(),QString("4"));
        grid.selectionModel()->select(QItemSelection(model.index(0,0),model.index(1,1)),QItemSelectionModel::ClearAndSelect);
        QTest::keyClick(&grid,Qt::Key_C,Qt::ControlModifier); QCOMPARE(QApplication::clipboard()->text(),QString("1\t2\n3\t4"));
        grid.setCurrentIndex(model.index(0,0)); QApplication::clipboard()->setText("8\tbad"); QTest::keyClick(&grid,Qt::Key_V,Qt::ControlModifier);
        QCOMPARE(model.index(0,0).data().toString(),QString("1")); QCOMPARE(model.index(0,1).data().toString(),QString("2")); QVERIFY(!grid.property("spreadsheetFeedback").toString().isEmpty());
        model.item(0,1)->setEditable(false); QApplication::clipboard()->setText("8\t9"); QTest::keyClick(&grid,Qt::Key_V,Qt::ControlModifier); QCOMPARE(model.index(0,0).data().toString(),QString("1"));
        grid.setCurrentIndex(model.index(2,2)); QApplication::clipboard()->setText("8\t9"); QTest::keyClick(&grid,Qt::Key_V,Qt::ControlModifier); QCOMPARE(model.index(2,2).data().toString(),QString("0"));
        grid.setCurrentIndex(model.index(0,0)); QApplication::clipboard()->setText("8\t9\n7"); QTest::keyClick(&grid,Qt::Key_V,Qt::ControlModifier); QCOMPARE(model.index(0,0).data().toString(),QString("1"));
    }
    void inputAndResultsUseWindowSpace() {
        ApplicationShell shell; QVERIFY(shell.registerModule(lpModule())); auto& input=inputController(shell);
        input.dialogs().askSave=[] {return SaveChoice::No;}; input.dialogs().showError=[](const QString&,const QString&){};
        QVERIFY(input.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+"/01_normal.txt"));
        QVERIFY(shell.navigate("lp/input")); shell.show(); shell.resize(1600,1000); QApplication::processEvents();
        auto* grid=shell.currentContent()->findChild<QTableView*>("input-grid"); QVERIFY(grid);
        const int contentHeight=grid->verticalHeader()->length()+grid->horizontalHeader()->height()+2*grid->frameWidth();
        QVERIFY2(grid->height()<=contentHeight+2,"Small input table reserves blank rows");
        capture(shell,"input-small");
        QVERIFY(input.initialize("Large",30,40)); QApplication::processEvents();
        QVERIFY(grid->height()>shell.currentContent()->height()/2); QVERIFY(grid->width()>shell.currentContent()->width()*0.85);
        QVERIFY(input.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+"/01_normal.txt"));
        auto& simplex=simplexController(shell); simplex.notice=[](const QString&,const QString&){};
        QVERIFY(simplex.requestAllSlack()); QVERIFY(simplex.start({})); simplex.all(); QTRY_VERIFY(!simplex.busy());
        auto* table=shell.currentContent()->findChild<QWidget*>("simplex-table-area"); QVERIFY(table);
        auto* view=shell.currentContent()->findChild<QTableView*>("simplex-grid"); QVERIFY(view);
        auto* help=shell.currentContent()->findChild<QToolButton*>("simplex-instructions-toggle"); QVERIFY(help);
        const auto history=simplex.session()->history().size();
        for(const auto size:{QSize(900,650),QSize(1600,1000),QSize(1100,720)}) {
            shell.resize(size); QApplication::processEvents();
            QVERIFY2(table->width()>shell.currentContent()->width()*0.95,"Solution table wastes horizontal window space");
            QVERIFY2(table->height()>shell.currentContent()->height()*0.60,"Solution table is confined to a small part of the screen");
            auto* scroll=shell.findChild<QWidget*>("lp/simplex")->findChild<QScrollArea*>("screen-scroll"); QVERIFY(scroll);
            QCOMPARE(scroll->horizontalScrollBar()->maximum(),0); QCOMPARE(scroll->verticalScrollBar()->maximum(),0);
            view->scrollToBottom(); QApplication::processEvents(); const auto last=view->model()->index(view->model()->rowCount()-1,1);
            QVERIFY(view->viewport()->rect().intersects(view->visualRect(last))); QCOMPARE(simplex.session()->history().size(),history);
            capture(shell,"simplex-"+QString::number(size.width()));
        }
        const int closed=table->height(); help->setFocus(); QTest::keyClick(help,Qt::Key_Return); QApplication::processEvents(); QVERIFY(help->isChecked()); QVERIFY(table->height()<closed);
        QTest::keyClick(help,Qt::Key_Return); QApplication::processEvents(); QVERIFY(!help->isChecked());
        auto& final=finalController(shell); final.notice=[](const QString&,const QString&){}; QVERIFY(final.request()); QVERIFY(final.start({}));
        auto* summary=shell.currentContent()->findChild<QTableView*>("summary-grid"); QVERIFY(summary);
        for(const auto size:{QSize(900,650),QSize(1600,1000)}) {shell.resize(size);QApplication::processEvents();QVERIFY(summary->width()>shell.currentContent()->width()*0.95);QVERIFY(summary->height()>shell.currentContent()->height()*0.60);capture(shell,"final-"+QString::number(size.width()));}
    }
};
int main(int argc,char** argv) {
#ifdef Q_OS_WIN
    qputenv("QT_QPA_FONTDIR",qgetenv("SystemRoot")+"/Fonts");
#endif
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc,argv); DesktopKeyboard test; return QTest::qExec(&test,argc,argv);
}
#include "desktop_keyboard.moc"
