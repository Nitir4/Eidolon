#include "lp_input_controller.hpp"
#include "application_shell.hpp"
#include "lp_screens.hpp"
#include <QApplication>
#include <QFile>
#include <QLineEdit>
#include <QMenuBar>
#include <QPushButton>
#include <QRadioButton>
#include <QTemporaryDir>
#include <QTest>
#include <QTableView>
#include <QCryptographicHash>
#include <QAbstractItemModelTester>
#include <QDir>
#include <QMenu>
#include <QHeaderView>
#include <QTimer>
#include <QFileDialog>
#include <QCheckBox>
#include <cmath>

using namespace tora::desktop;
class InputTest final : public QObject {
    Q_OBJECT
    static QString golden(const QString& name) { return QStringLiteral(TORA_CODEC_GOLDENS)+QLatin1Char('/')+name+QStringLiteral(".txt"); }
    static void silent(LpInputController& c) { c.dialogs().showError=[](const QString&,const QString&){}; c.dialogs().askSave=[] {return SaveChoice::No;}; }
private slots:
    void capturedEdits_data() {
        QTest::addColumn<QString>("caseName");QTest::addColumn<QByteArray>("hash");
        QFile fixture(QStringLiteral(TORA_INPUT_EDITS));QVERIFY(fixture.open(QIODevice::ReadOnly));
        while(!fixture.atEnd()) {const auto line=fixture.readLine().trimmed();if(line.startsWith('#')||line.isEmpty())continue;
            const auto fields=line.split('\t');QCOMPARE(fields.size(),2);QTest::newRow(fields[0].constData())<<QString::fromUtf8(fields[0])<<fields[1];}
    }
    void capturedEdits() {
        QFETCH(QString,caseName);QFETCH(QByteArray,hash);
        LpInputController c;silent(c);
        if(caseName.startsWith(QStringLiteral("format-"))) {
            const int n=caseName==QStringLiteral("format-n99")?99:(caseName==QStringLiteral("format-n11")||caseName==QStringLiteral("format-scientific-n11")?11:5);
            const int d=caseName==QStringLiteral("format-d99")?99:(caseName.startsWith(QStringLiteral("format-d11"))?11:2);
            c.beginNew({caseName==QStringLiteral("format-scientific-n11")?tora::core::Notation::Scientific:tora::core::Notation::Decimal,n,d});
            QVERIFY(c.initialize(QStringLiteral("Format probe"),2,3));
        }else QVERIFY(c.openFile(golden(QStringLiteral("01_normal"))));
        QAbstractItemModelTester modelCheck(c.grid(),QAbstractItemModelTester::FailureReportingMode::QtTest);
        const auto edit=[&c](int r,int col,const QString& text){return c.grid()->setData(c.grid()->index(r,col),text);};
        if(caseName==QStringLiteral("numeric-replace-keyboard"))QVERIFY(edit(1,1,QStringLiteral("1.2345678901234567")));
        else if(caseName==QStringLiteral("numeric-invalid-replace"))QVERIFY(edit(1,1,QStringLiteral("bogus")));
        else if(caseName==QStringLiteral("numeric-prefix"))QVERIFY(edit(1,1,QStringLiteral("3junk")));
        else if(caseName==QStringLiteral("objective-toggle"))QVERIFY(c.toggleObjective());
        else if(caseName==QStringLiteral("alias-keyboard"))QVERIFY(edit(0,1,QStringLiteral("decision")));
        else if(caseName==QStringLiteral("quoted-alias-keyboard"))QVERIFY(edit(0,1,QStringLiteral("named, \"x\"")));
        else if(caseName==QStringLiteral("relation-replace-keyboard"))QVERIFY(edit(2,3,QStringLiteral(">")));
        else if(caseName==QStringLiteral("flag-replace-keyboard"))QVERIFY(edit(7,1,QStringLiteral("Y")));
        else if(caseName==QStringLiteral("flag-restore")){QVERIFY(edit(7,1,QStringLiteral("Y")));QVERIFY(edit(7,1,QStringLiteral("N")));}
        else if(caseName==QStringLiteral("column-copy-paste")){c.selectColumn(1);QVERIFY(c.copySelection());c.selectColumn(2);QVERIFY(c.pasteSelection());}
        else if(caseName==QStringLiteral("column-delete-second")){c.selectColumn(2);QVERIFY(!c.deleteSelection());}
        else if(caseName==QStringLiteral("column-delete-three")){c.selectColumn(2);QVERIFY(c.insertSelection());c.selectColumn(3);QVERIFY(c.deleteSelection());}
        else if(caseName==QStringLiteral("column-insert-menu")){c.selectColumn(2);QVERIFY(c.insertSelection());}
        else if(caseName==QStringLiteral("undo-menu-state")){c.selectRow(3);QVERIFY(c.insertSelection());}
        else if(caseName==QStringLiteral("count-return")||caseName==QStringLiteral("count-tab"))QVERIFY(c.initialize(QStringLiteral("Normal"),2,3));
        else if(caseName==QStringLiteral("format-d11-value"))QVERIFY(edit(1,1,QStringLiteral("1.2345678901234567")));
        else if(caseName.startsWith(QStringLiteral("format-"))) {}
        else QFAIL("Unhandled oracle case");
        const auto bytes=QByteArray::fromStdString(tora::compat::ToraLpCodec::encode(*c.document()));
        QCOMPARE(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex(),hash);
        const auto captureRoot=qEnvironmentVariable("TORA_UI_CAPTURES");
        if(!captureRoot.isEmpty()){QDir dir(captureRoot);QVERIFY(dir.mkpath(QStringLiteral("input-files")));QVERIFY(c.saveTo(dir.filePath(QStringLiteral("input-files/")+caseName+QStringLiteral(".txt"))));}
    }
    void defaultsDimensionsAndRows() {
        LpInputController c; silent(c); QVERIFY(!c.document());
        QVERIFY(!c.initialize(QStringLiteral("Title"),0,3)); QVERIFY(!c.document());
        QCOMPARE(c.lastErrorTitle(),QStringLiteral("Problem Size")); QCOMPARE(c.lastError(),QStringLiteral("Invalid problem size"));
        QVERIFY(!c.initialize({},101,3)); QCOMPARE(c.lastError(),QStringLiteral("Student version limits maximum size to 100"));
        QVERIFY(c.initialize(QStringLiteral("Title"),2,3)); QVERIFY(c.document()->dirty());
        auto* g=c.grid(); QCOMPARE(g->rowCount(),8); QCOMPARE(g->columnCount(),5);
        QCOMPARE(g->headerData(1,Qt::Horizontal).toString(),QStringLiteral("x1"));
        QCOMPARE(g->headerData(3,Qt::Horizontal).toString(),QStringLiteral("Enter <, >, or ="));
        QCOMPARE(g->data(g->index(0,0)).toString(),QStringLiteral("Var. Name"));
        QCOMPARE(g->data(g->index(1,0)).toString(),QStringLiteral("Maximize"));
        QCOMPARE(g->data(g->index(2,0)).toString(),QStringLiteral("Constr 1"));
        QCOMPARE(g->data(g->index(5,0)).toString(),QStringLiteral("Lower Bound"));
        QCOMPARE(g->data(g->index(6,1)).toString(),QStringLiteral("infinity"));
        QCOMPARE(g->data(g->index(7,1)).toString(),QStringLiteral("n"));
        QCOMPARE(g->data(g->index(1,1)).toString(),QStringLiteral("0.00"));
        QVERIFY(!(g->flags(g->index(1,3))&Qt::ItemIsEditable));
        QVERIFY(!(g->flags(g->index(0,0))&Qt::ItemIsEditable));
        QVERIFY(g->flags(g->index(1,1))&Qt::ItemIsEditable);
        c.beginNew({tora::core::Notation::Scientific,5,2});
        QVERIFY(c.initialize({},2,1)); QCOMPARE(c.grid()->data(c.grid()->index(1,1)).toString(),QStringLiteral("0.00000e+00"));
    }
    void allSavedInputBindings() {
        LpInputController c;silent(c);
        QDir dir(QStringLiteral(TORA_CODEC_GOLDENS));const auto files=dir.entryList({QStringLiteral("*.txt")},QDir::Files);
        QCOMPARE(files.size(),27);
        for(const auto& name:files) {
            QVERIFY(c.openFile(dir.filePath(name)));QFile source(dir.filePath(name));QVERIFY(source.open(QIODevice::ReadOnly));
            auto records=source.readAll().split('\n');for(auto& record:records)if(record.endsWith('\r'))record.chop(1);
            const int n=records[5].toInt(),m=records[6].toInt();QCOMPARE(c.grid()->rowCount(),m+5);QCOMPARE(c.grid()->columnCount(),n+3);
            QCOMPARE(c.grid()->data(c.grid()->index(1,0)).toString(),QString::fromLatin1(records[7].mid(1,records[7].size()-2)));
            for(int col=1;col<=n;++col)QCOMPARE(c.grid()->headerData(col,Qt::Horizontal).toString(),QStringLiteral("x%1").arg(col));
            for(int r=2;r<m+2;++r)QCOMPARE(c.grid()->data(c.grid()->index(r,0)).toString(),QStringLiteral("Constr %1").arg(r-1));
            for(int r=0;r<m+5;++r)for(int col=1;col<n+3;++col) {
                auto expected=records[8+r*(n+2)+col-1];QVERIFY(expected.startsWith('"')&&expected.endsWith('"'));
                expected=expected.mid(1,expected.size()-2);expected.replace("\"\"","\"");
                QString displayed=legacyText(expected.toStdString());if(col==n+1 && r>=2 && r<m+2)displayed=displayed.trimmed();
                QCOMPARE(c.grid()->data(c.grid()->index(r,col)).toString(),displayed);
            }
        }
    }
    void populatedLayouts_data() {
        QTest::addColumn<QSize>("physicalSize");
        QTest::newRow("1366x768")<<QSize(1366,768);QTest::newRow("1920x1080")<<QSize(1920,1080);QTest::newRow("4K")<<QSize(3840,2160);
    }
    void populatedLayouts() {
        QFETCH(QSize,physicalSize);ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();
        QTest::qWait(1);
        const double scale=qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();QVERIFY(scale>0);
        shell.resize(QSize(qRound(physicalSize.width()/scale),qRound(physicalSize.height()/scale)));
        QVERIFY(shell.navigate(QStringLiteral("lp/input")));auto& c=inputController(shell);silent(c);
        QVERIFY(c.openFile(golden(QStringLiteral("01_normal"))));QApplication::processEvents();
        auto* view=shell.currentContent()->findChild<QTableView*>(QStringLiteral("input-grid"));QVERIFY(view);
        auto* solve=shell.currentContent()->findChild<QPushButton*>(QStringLiteral("input-solve-menu"));QVERIFY(solve->isVisible());
        QVERIFY(solve->width()>=solve->minimumSizeHint().width());
        QCOMPARE(view->model()->data(view->model()->index(2,4)).toString(),QStringLiteral("4.00"));
        const auto captureRoot=qEnvironmentVariable("TORA_UI_CAPTURES");
        if(!captureRoot.isEmpty() && physicalSize==QSize(1920,1080)) {
            QDir dir(captureRoot);QVERIFY(dir.mkpath(QStringLiteral("input")));
            QVERIFY(shell.grab().save(dir.filePath(QStringLiteral("input/normal-%1.png").arg(scale))));
        }
        QVERIFY(c.initialize(QStringLiteral("Maximum input size"),100,100));QApplication::processEvents();
        QCOMPARE(view->model()->rowCount(),105);QCOMPARE(view->model()->columnCount(),103);
        const auto last=view->model()->index(104,100);view->scrollTo(last);QApplication::processEvents();
        QVERIFY(view->viewport()->rect().intersects(view->visualRect(last)));
        QVERIFY(c.grid()->setData(c.grid()->index(101,102),QStringLiteral("7.25")));
        QCOMPARE(c.document()->problem.constraints.back().rhs,7.25);
        QVERIFY(shell.navigate(QStringLiteral("main")));
    }
    void keyboardEditingAndSolveValidation() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();
        QVERIFY(shell.navigate(QStringLiteral("lp/input")));auto& c=inputController(shell);silent(c);
        QVERIFY(c.openFile(golden(QStringLiteral("01_normal"))));QApplication::processEvents();
        auto* view=shell.currentContent()->findChild<QTableView*>(QStringLiteral("input-grid"));QVERIFY(view);
        view->setCurrentIndex(c.grid()->index(0,1));view->edit(c.grid()->index(0,1));QApplication::processEvents();
        auto* editor=qobject_cast<QLineEdit*>(QApplication::focusWidget());QVERIFY(editor);editor->selectAll();QTest::keyClicks(editor,"decision");
        QVERIFY(c.editing());QVERIFY(!shell.menuBar()->actions()[0]->menu()->actions()[0]->isEnabled());
        QTest::keyClick(editor,Qt::Key_Return);QApplication::processEvents();
        QCOMPARE(c.document()->problem.variables[0].name,std::string("decision"));
        QCOMPARE(view->currentIndex(),c.grid()->index(0,2));
        editor=qobject_cast<QLineEdit*>(QApplication::focusWidget());QVERIFY(editor);
        QTest::keyClick(editor,Qt::Key_Tab);QApplication::processEvents();
        QCOMPARE(view->currentIndex(),c.grid()->index(1,1));
        editor=qobject_cast<QLineEdit*>(QApplication::focusWidget());QVERIFY(editor);editor->selectAll();QTest::keyClicks(editor,"1.2345678901234567");
        QTest::keyClick(editor,Qt::Key_Tab);QApplication::processEvents();QCOMPARE(c.document()->problem.objective[0],1.23);
        view->setFocus();QApplication::processEvents();
        c.selectColumn(1);QVERIFY(c.copySelection());c.selectColumn(2);QVERIFY(c.pasteSelection());
        QVERIFY(!c.validateForSolve());QCOMPARE(c.lastErrorTitle(),QStringLiteral("Input Data Error"));
        QCOMPARE(c.lastError(),QStringLiteral("All Left-hand side coefficients of Constraint 3 are\nzero.  Reenter or delete constraint"));
        QVERIFY(!shell.navigate(QStringLiteral("lp/solve-menu")));QCOMPARE(shell.currentScreen(),QStringLiteral("lp/input"));
        QVERIFY(shell.navigate(QStringLiteral("main")));
    }
    void nativeTextAndUnicodeFilename() {
        LpInputController c;silent(c);QVERIFY(c.initialize(QString::fromUtf8("café €"),2,1));
        QCOMPARE(legacyText(c.document()->problem.title),QString::fromUtf8("café €"));
        QVERIFY(c.grid()->setData(c.grid()->index(0,1),QString::fromUtf8("décision")));
        const auto before=tora::compat::ToraLpCodec::encode(*c.document());
        QVERIFY(!c.setTitle(QString::fromUtf8("δ")));QCOMPARE(tora::compat::ToraLpCodec::encode(*c.document()),before);
        QTemporaryDir dir;const auto path=dir.filePath(QString::fromUtf8("café-δ.txt"));QVERIFY(c.saveTo(path));
        LpInputController loaded;silent(loaded);QVERIFY(loaded.openFile(path));QCOMPARE(legacyText(loaded.document()->problem.title),QString::fromUtf8("café €"));
        QCOMPARE(loaded.grid()->data(loaded.grid()->index(0,1)).toString(),QString::fromUtf8("décision"));
    }
    void importedPrecisionAndEditedStorage() {
        LpInputController c;silent(c); QVERIFY(c.openFile(golden(QStringLiteral("probe-1var-1con"))));
        QCOMPARE(c.document()->problem.objective[0],1.234567);
        const auto before=tora::compat::ToraLpCodec::encode(*c.document());
        QTemporaryDir dir;QVERIFY(dir.isValid()); QVERIFY(c.saveTo(dir.filePath(QStringLiteral("unchanged.txt"))));
        QFile f(dir.filePath(QStringLiteral("unchanged.txt"))); QVERIFY(f.open(QIODevice::ReadOnly)); QCOMPARE(f.readAll(),QByteArray::fromStdString(before));
        QVERIFY(c.openFile(golden(QStringLiteral("01_normal"))));
        QVERIFY(c.grid()->setData(c.grid()->index(1,1),QStringLiteral("1.2345678901234567")));
        QCOMPARE(c.document()->problem.objective[0],1.23);
        QCOMPARE(c.grid()->data(c.grid()->index(1,1)).toString(),QStringLiteral("1.23"));
        const auto edited=tora::compat::ToraLpCodec::encode(*c.document());
        QVERIFY(edited.find("\"1.23\"\r\n")!=std::string::npos);
        QVERIFY(c.saveTo(dir.filePath(QStringLiteral("edited.txt")))); QVERIFY(!c.document()->dirty());
    }
    void capturedRowCommands() {
        LpInputController c;silent(c); QVERIFY(c.openFile(golden(QStringLiteral("01_normal"))));
        c.selectRow(2);QVERIFY(c.copySelection());c.selectRow(3);QVERIFY(c.pasteSelection());
        QCOMPARE(c.document()->problem.constraints[1].rhs,4.0);
        QCOMPARE(c.document()->problem.constraints[1].coefficients[1],1.0);
        QCOMPARE(c.document()->problem.constraints.size(),std::size_t(3));
        QVERIFY(c.deleteSelection()); QCOMPARE(c.document()->problem.constraints.size(),std::size_t(2));
        QCOMPARE(c.document()->problem.constraints[1].rhs,3.0);
        c.selectRow(2);QVERIFY(c.insertSelection());
        QCOMPARE(c.document()->problem.constraints.size(),std::size_t(3));
        QCOMPARE(c.document()->problem.constraints[0].rhs,4.0);
        QCOMPARE(c.document()->problem.constraints[1].rhs,0.0);
        QCOMPARE(c.document()->problem.constraints[1].coefficients[0],0.0);
        QCOMPARE(c.document()->problem.constraints[1].relation,tora::lp::Relation::LessEqual);
    }
    void filePickersReadOnlyAndCancelledLeave() {
        LpInputController c;silent(c); c.dialogs().chooseOpen=[] {return std::optional<OpenSelection>{{golden(QStringLiteral("01_normal")),true}};};
        QVERIFY(c.requestOpen()); QVERIFY(c.document()->readOnly());
        QVERIFY(c.grid()->setData(c.grid()->index(1,1),QStringLiteral("5")));
        const auto oldPath=c.document()->sourcePath();
        c.dialogs().chooseSave=[](const QString&){return std::optional<QString>{};};
        QVERIFY(!c.requestSave()); QCOMPARE(c.document()->sourcePath(),oldPath); QVERIFY(c.document()->dirty());
        c.dialogs().askSave=[] {return SaveChoice::Cancel;}; QVERIFY(!c.leaveInput());
        c.dialogs().askSave=[] {return SaveChoice::Yes;}; QVERIFY(!c.leaveInput());
        QTemporaryDir dir;const auto out=dir.filePath(QStringLiteral("copy.txt"));
        c.dialogs().chooseSave=[out](const QString&){return std::optional<QString>{out};};
        QVERIFY(c.leaveInput()); QVERIFY(!c.document()->readOnly()); QVERIFY(!c.document()->dirty());
        QVERIFY(c.document()->sourcePath()!=oldPath); QVERIFY(QFile::exists(out));
    }
    void openFailuresDoNotReplaceModel() {
        LpInputController c;silent(c); QVERIFY(c.openFile(golden(QStringLiteral("01_normal"))));
        QVERIFY(!c.openFile(QStringLiteral("missing-input-file.txt")));
        QCOMPARE(c.document()->problem.title,std::string("Normal"));
        QCOMPARE(c.document()->problem.objective[0],3.0);
    }
    void realPickerReadOnlyAndSaveCancel() {
        LpInputController c;silent(c);
        bool sawReadOnly=false;
        QTimer::singleShot(0,[&] {
            auto* picker=qobject_cast<QFileDialog*>(QApplication::activeModalWidget());QVERIFY(picker);
            for(auto* box:picker->findChildren<QCheckBox*>())if(box->text()==QStringLiteral("Open as read-only")){box->setChecked(true);sawReadOnly=true;}
            picker->selectFile(golden(QStringLiteral("01_normal")));QVERIFY(QMetaObject::invokeMethod(picker,"accept"));
        });
        QVERIFY(c.requestOpen());QVERIFY(sawReadOnly);QVERIFY(c.document()->readOnly());
        QVERIFY(c.grid()->setData(c.grid()->index(1,1),QStringLiteral("5")));
        const auto before=tora::compat::ToraLpCodec::encode(*c.document());const auto path=c.document()->sourcePath();
        QTimer::singleShot(0,[] {auto* picker=qobject_cast<QFileDialog*>(QApplication::activeModalWidget());QVERIFY(picker);picker->reject();});
        QVERIFY(!c.requestSave());QCOMPARE(c.document()->sourcePath(),path);QVERIFY(c.document()->dirty());
        QCOMPARE(tora::compat::ToraLpCodec::encode(*c.document()),before);
    }
    void unchangedCountReturnAndTabReinitialize() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();
        QVERIFY(shell.navigate(QStringLiteral("lp/input")));auto& c=inputController(shell);silent(c);
        auto* count=shell.currentContent()->findChild<QLineEdit*>(QStringLiteral("constraint-count"));QVERIFY(count);
        for(const auto key:{Qt::Key_Return,Qt::Key_Tab}) {
            QVERIFY(c.openFile(golden(QStringLiteral("01_normal"))));QApplication::processEvents();
            count->setFocus();QTest::keyClick(count,key);QApplication::processEvents();
            QCOMPARE(c.document()->problem.objective[0],0.0);QCOMPARE(c.document()->problem.constraints[0].rhs,0.0);
            const auto bytes=QByteArray::fromStdString(tora::compat::ToraLpCodec::encode(*c.document()));
            QCOMPARE(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex(),QByteArray("04ce77a49c9226d23e9a02a208d6ed618c1880dddc835416cdc8721e8a903e0f"));
            auto* view=shell.currentContent()->findChild<QTableView*>(QStringLiteral("input-grid"));QCOMPARE(view->currentIndex(),c.grid()->index(0,1));
        }
    }
    void pendingTitleSaveAndUnsupportedTextRollback() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();
        QVERIFY(shell.navigate(QStringLiteral("lp/input")));auto& c=inputController(shell);silent(c);
        QVERIFY(c.openFile(golden(QStringLiteral("01_normal"))));
        auto* title=shell.currentContent()->findChild<QLineEdit*>(QStringLiteral("problem-title"));QVERIFY(title);
        QTemporaryDir dir;const auto destination=dir.filePath(QStringLiteral("title.txt"));int pickers=0;
        c.dialogs().chooseSave=[&](const QString&){++pickers;return std::optional<QString>{destination};};
        title->setFocus();title->selectAll();QTest::keyClicks(title,"Pending title");
        QVERIFY(c.requestSave());QCOMPARE(c.document()->problem.title,std::string("Pending title"));
        QCOMPARE(pickers,1);const auto before=tora::compat::ToraLpCodec::encode(*c.document());
        title->setText(QString::fromUtf8("δ"));QVERIFY(!c.requestSave());QCOMPARE(pickers,1);
        QCOMPARE(tora::compat::ToraLpCodec::encode(*c.document()),before);
        QFile saved(destination);QVERIFY(saved.open(QIODevice::ReadOnly));QCOMPARE(saved.readAll(),QByteArray::fromStdString(before));
    }
    void nativeNavigationAndGrid() {
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();
        QVERIFY(shell.navigate(QStringLiteral("lp/input-mode")));
        auto* p=shell.currentContent(); QVERIFY(!p->findChild<QLineEdit*>(QStringLiteral("input-format-N"))->isReadOnly());
        p->findChild<QRadioButton*>(QStringLiteral("input-existing"))->click();
        QVERIFY(p->findChild<QPushButton*>(QStringLiteral("input-go"))->isEnabled());
        p->findChild<QRadioButton*>(QStringLiteral("input-new"))->click();
        p->findChild<QPushButton*>(QStringLiteral("input-go"))->click();
        QCOMPARE(shell.currentScreen(),QStringLiteral("lp/input"));
        p=shell.currentContent();QVERIFY(p->findChild<QLineEdit*>(QStringLiteral("variable-count"))->isEnabled());
        auto* c=shell.findChild<LpInputController*>();QVERIFY(c);silent(*c);
        QVERIFY(c->initialize(QStringLiteral("Normal"),2,3));QApplication::processEvents();
        for(int r=2;r<5;++r)QVERIFY(c->grid()->setData(c->grid()->index(r,1),QStringLiteral("1")));
        auto* view=p->findChild<QTableView*>(QStringLiteral("input-grid"));QVERIFY(view);QCOMPARE(view->model()->rowCount(),8);
        QVERIFY(p->findChild<QPushButton*>(QStringLiteral("input-solve-menu"))->isEnabled());
        c->dialogs().askSave=[] {return SaveChoice::Cancel;}; QVERIFY(!shell.navigate(QStringLiteral("main")));
        QCOMPARE(shell.currentScreen(),QStringLiteral("lp/input"));
        c->dialogs().askSave=[] {return SaveChoice::No;}; QVERIFY(shell.navigate(QStringLiteral("lp/solve-menu")));
        QVERIFY(shell.navigate(QStringLiteral("lp/input")));QCOMPARE(c->document()->problem.title,std::string("Normal"));
    }
};
QTEST_MAIN(InputTest)
#include "desktop_input.moc"
