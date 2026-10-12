#include "application_shell.hpp"
#include "transportation_screens.hpp"
#include "ui_font.hpp"
#include <QDir>
#include <QFile>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>

using namespace tora::desktop;
class TransportationInputTest : public QObject {
    Q_OBJECT
private slots:
    void documentAndSaveLifecycle() {
        ApplicationShell shell; shell.registerModule(transportationModule());
        auto& c = transportationController(shell); QString error; QTemporaryDir temp; QVERIFY(temp.isValid());
        c.dialogs().showError = [](const QString&, const QString&) {};
        QVERIFY(c.initialize(QStringLiteral("New model"), 2, 3, error)); QVERIFY(c.dirty());
        QCOMPARE(c.document()->cell(1, 1), std::string("0.00"));
        QVERIFY(c.editCell(1, 1, QStringLiteral("1.234567"), error)); QCOMPARE(c.problem().costs[0][0], 1.23);
        QVERIFY(c.editCell(1, 4, QStringLiteral("20"), error));
        const auto bytes = c.document()->encode();
        QVERIFY(!c.editCell(1, 4, QStringLiteral("-2"), error)); QCOMPARE(c.document()->encode(), bytes);
        QVERIFY(!c.initialize(QStringLiteral("bad"), 0, 3, error)); QCOMPARE(c.document()->encode(), bytes);
        const auto path = temp.filePath(QStringLiteral("spaced \u03bb model.txt"));
        QVERIFY(c.saveTo(path, error)); QVERIFY(!c.dirty()); QCOMPARE(c.sourcePath(), path);
        QFile saved(path); QVERIFY(saved.open(QIODevice::ReadOnly)); QCOMPARE(saved.readAll().toStdString(), bytes); saved.close();
        QVERIFY(c.openFile(path, error, true)); QVERIFY(c.readOnly());
        QVERIFY(!c.editCell(1, 1, QStringLiteral("5"), error));
        QVERIFY(!c.saveTo(path, error));
        QVERIFY(c.saveTo(temp.filePath(QStringLiteral("copy.txt")), error)); QVERIFY(!c.readOnly());
        QVERIFY(c.setTitle(QStringLiteral("Edited"), error)); QVERIFY(c.dirty());
        const auto edited = c.document()->encode();
        QVERIFY(!c.saveTo(temp.filePath(QStringLiteral("absent/model.txt")), error));
        QVERIFY(c.dirty()); QCOMPARE(c.document()->encode(), edited);
        QVERIFY(!c.openFile(temp.filePath(QStringLiteral("missing.txt")), error));
        QCOMPARE(c.document()->encode(), edited);
    }
    void cancellationAndNavigation() {
        ApplicationShell shell; shell.registerModule(transportationModule());
        auto& c = transportationController(shell); QString error;
        c.dialogs().showError = [](const QString&, const QString&) {};
        c.loadExample(); QVERIFY(shell.navigate(QStringLiteral("transportation/input")));
        QVERIFY(c.setTitle(QStringLiteral("Unsaved"), error));
        int prompts = 0;
        c.dialogs().askSave = [&] { ++prompts; return SaveChoice::Cancel; };
        QAction* least = nullptr;
        for (auto* top : shell.menuBar()->actions()) if (top->menu())
            for (auto* action : top->menu()->actions()) if (action->text().contains(QStringLiteral("Least-cost"))) least = action;
        QVERIFY(least);
        int notices = 0; QTimer dismiss;
        QObject::connect(&dismiss, &QTimer::timeout, &shell, [&] {
            for (auto* widget : QApplication::topLevelWidgets()) if (auto* box = qobject_cast<QMessageBox*>(widget)) { ++notices; box->accept(); }
        });
        dismiss.start(0); least->trigger(); dismiss.stop();
        QCOMPARE(shell.currentScreen(), QStringLiteral("transportation/input"));
        QVERIFY(c.trace().empty()); QCOMPARE(notices, 0); QCOMPARE(prompts, 1);
        QVERIFY(!shell.navigate(QStringLiteral("main"))); QCOMPARE(prompts, 2);
        QCOMPARE(shell.currentScreen(), QStringLiteral("transportation/input")); QVERIFY(c.dirty());
        c.dialogs().askSave = [] { return SaveChoice::Yes; };
        c.dialogs().chooseSave = [](const QString&) -> std::optional<QString> { return {}; };
        QVERIFY(!shell.navigate(QStringLiteral("main"))); QVERIFY(c.dirty());
        c.dialogs().askSave = [] { return SaveChoice::No; };
        QVERIFY(shell.navigate(QStringLiteral("main")));
    }
    void editsInvalidateTrace() {
        ApplicationShell shell; shell.registerModule(transportationModule());
        auto& c = transportationController(shell); QString error; c.loadExample(); QVERIFY(c.solve(error));
        QVERIFY(!c.trace().empty()); const auto traceSize = c.trace().size();
        QVERIFY(!c.editCell(1, 1, QStringLiteral("nan"), error)); QCOMPARE(c.trace().size(), traceSize);
        QVERIFY(c.editCell(1, 1, QStringLiteral("2"), error)); QCOMPARE(c.trace().size(), traceSize); // unchanged lexeme
        QVERIFY(c.editCell(1, 1, QStringLiteral("3"), error)); QVERIFY(c.trace().empty());
        QVERIFY(c.solve(error)); QVERIFY(c.setTitle(QStringLiteral("Changed"), error)); QVERIFY(c.trace().empty());
    }
    void nativeEditingAndActiveSave() {
        ApplicationShell shell; shell.registerModule(transportationModule());
        auto& c = transportationController(shell); c.loadExample(); QString error; QTemporaryDir temp;
        c.dialogs().showError = [](const QString&, const QString&) {};
        const auto path = temp.filePath(QStringLiteral("active.txt"));
        c.dialogs().chooseSave = [&](const QString&) -> std::optional<QString> { return path; };
        QVERIFY(shell.navigate(QStringLiteral("transportation/input"))); shell.show(); QTest::qWait(30);
        auto* grid = shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("transport-input-grid")); QVERIFY(grid);
        QVERIFY(grid->item(2, 2)->flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(!grid->item(0, 2)->flags().testFlag(Qt::ItemIsEditable));
        grid->setCurrentCell(2, 2); grid->editItem(grid->item(2, 2)); QTest::qWait(10);
        auto* editor = grid->findChild<QLineEdit*>(); QVERIFY(editor); editor->setText(QStringLiteral("9.876"));
        QVERIFY(c.requestSave()); QCOMPARE(c.problem().costs[0][0], 9.88); QVERIFY(!c.dirty());
        QFile saved(path); QVERIFY(saved.open(QIODevice::ReadOnly)); QVERIFY(saved.readAll().contains("\"9.88\"\r\n"));
        QTest::qWait(10);
        grid->setCurrentCell(2, 2); grid->editItem(grid->item(2, 2)); QTest::qWait(10);
        editor = grid->findChild<QLineEdit*>(); QVERIFY(editor); editor->setText(QStringLiteral("nan"));
        QVERIFY(!c.requestSave()); QCOMPARE(c.problem().costs[0][0], 9.88);
    }
    void keyboardCommitKeepsNextEditor() {
        ApplicationShell shell; shell.registerModule(transportationModule());
        auto& c = transportationController(shell); c.loadExample();
        c.dialogs().showError = [](const QString&, const QString&) {};
        QVERIFY(shell.navigate(QStringLiteral("transportation/input"))); shell.show(); QTest::qWait(20);
        auto* grid = shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("transport-input-grid")); QVERIFY(grid);
        grid->setCurrentCell(2,2); grid->editItem(grid->item(2,2)); QTest::qWait(10);
        auto* editor = qobject_cast<QLineEdit*>(QApplication::focusWidget()); QVERIFY(editor);
        editor->setText(QStringLiteral("7.125")); QTest::keyClick(editor, Qt::Key_Return); QTest::qWait(20);
        QCOMPARE(c.problem().costs[0][0], 7.13); QCOMPARE(grid->currentColumn(), 3);
        editor = qobject_cast<QLineEdit*>(QApplication::focusWidget()); QVERIFY(editor);
        editor->setText(QStringLiteral("8.25")); QTest::keyClick(editor, Qt::Key_Tab); QTest::qWait(20);
        QCOMPARE(c.problem().costs[0][1], 8.25); QCOMPARE(grid->currentColumn(), 4);
        editor = qobject_cast<QLineEdit*>(QApplication::focusWidget()); QVERIFY(editor);
        editor->setText(QStringLiteral("nan")); QTest::keyClick(editor, Qt::Key_Return); QTest::qWait(20);
        QCOMPARE(c.problem().costs[0][2], 1.0); QCOMPARE(grid->currentColumn(), 4);
        QVERIFY(qobject_cast<QLineEdit*>(QApplication::focusWidget()));
    }
    void newProblemRouteAndPreview() {
        ApplicationShell shell; shell.registerModule(transportationModule());
        QVERIFY(shell.navigate(QStringLiteral("transportation/input-mode"))); shell.show();
        auto* create = shell.currentContent()->findChild<QPushButton*>(QStringLiteral("transport-new")); QVERIFY(create); create->click();
        QCOMPARE(shell.currentScreen(), QStringLiteral("transportation/input"));
        auto* sources = shell.currentContent()->findChild<QLineEdit*>(QStringLiteral("transport-sources"));
        auto* destinations = shell.currentContent()->findChild<QLineEdit*>(QStringLiteral("transport-destinations"));
        auto* title = shell.currentContent()->findChild<QLineEdit*>(QStringLiteral("transport-title"));
        QVERIFY(sources && destinations && title);
        title->setText(QStringLiteral("My Transportation model")); sources->setText(QStringLiteral("2")); destinations->setText(QStringLiteral("3"));
        QTest::keyClick(destinations, Qt::Key_Return); QTest::qWait(10);
        auto& c = transportationController(shell); QVERIFY(c.loaded()); QCOMPARE(c.problem().title, std::string("My Transportation model"));
        auto* grid = shell.currentContent()->findChild<QTableWidget*>(QStringLiteral("transport-input-grid")); QVERIFY(grid);
        QCOMPARE(grid->item(2, 2)->text(), QStringLiteral("0.00")); QCOMPARE(grid->item(2, 5)->text(), QString());
        for (const auto physical : {QSize(1366,768), QSize(1920,1080), QSize(3840,2160)}) {
            const auto scale = shell.devicePixelRatioF(); shell.resize(qRound(physical.width()/scale), qRound(physical.height()/scale)); QTest::qWait(20);
            for (auto* button : shell.currentContent()->findChildren<QPushButton*>()) QVERIFY(button->width() >= button->minimumSizeHint().width());
            const auto base = qEnvironmentVariable("TORA_UI_CAPTURES");
            if (!base.isEmpty()) {
                const auto folder = base + QStringLiteral("/transportation-input"); QVERIFY(QDir().mkpath(folder));
                QVERIFY(shell.grab().save(folder + QStringLiteral("/input-%1x%2-%3.png").arg(physical.width()).arg(physical.height()).arg(scale)));
            }
        }
    }
};
int main(int argc, char** argv) {
#ifdef Q_OS_WIN
    qputenv("QT_QPA_FONTDIR", qgetenv("SystemRoot") + "/Fonts");
#endif
    QApplication app(argc, argv); app.setFont(readableUiFont(app.font()));
    TransportationInputTest tests; return QTest::qExec(&tests, argc, argv);
}
#include "desktop_transportation_input.moc"
