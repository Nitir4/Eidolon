#include "application_shell.hpp"
#include "lp_screens.hpp"
#include "ui_font.hpp"
#include "ui_colors.hpp"

#include <QApplication>
#include <QAbstractItemView>
#include <QComboBox>
#include <QEvent>
#include <QDir>
#include <QFontMetrics>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QToolButton>
#include <QWindow>
#include <cmath>

using namespace tora::desktop;
namespace {
QStringList labels(const QMenu& menu)
{
    QStringList result;
    for (const auto* action : menu.actions()) if (!action->isSeparator()) result.append(action->text());
    return result;
}
QStringList routes()
{
    return {QStringLiteral("main"), QStringLiteral("lp/input-mode"),
            QStringLiteral("lp/input"), QStringLiteral("lp/solve-menu"),
            QStringLiteral("lp/output-format"), QStringLiteral("lp/m-value")};
}
}

class ShellTest final : public QObject {
    Q_OBJECT
private slots:
    void navigationAndDefaults()
    {
        ApplicationShell shell;
        QVERIFY(shell.registerModule(lpModule()));
        shell.show();
        QTest::qWait(1);
        QCOMPARE(shell.currentScreen(), QStringLiteral("main"));
        QVERIFY(!shell.navigate(QStringLiteral("welcome")));
        QVERIFY(!shell.findChild<QPushButton*>(QStringLiteral("welcome-enter")));
        QSignalSpy changed(&shell, &ApplicationShell::screenChanged);
        auto* main = shell.menuBar()->actions().front()->menu();
        QCOMPARE(labels(*main), (QStringList{QStringLiteral("Linear Equations"), QStringLiteral("Linear Programming"),
            QStringLiteral("Transportation model"), QStringLiteral("Integer programming"), QStringLiteral("Network models"),
            QStringLiteral("Project Planning"), QStringLiteral("Queuing analysis"), QStringLiteral("Zero-Sum Games"), QStringLiteral("EXIT EIDOLON")}));
        for (int i = 0; i < 8; ++i) QCOMPARE(main->actions()[i]->isEnabled(), i == 1);
        auto* lpButton = shell.findChild<QPushButton*>(QStringLiteral("main-lp"));
        QVERIFY(lpButton);
        QVERIFY(lpButton->isEnabled());
        QTest::mouseClick(lpButton, Qt::LeftButton);
        QCOMPARE(shell.currentScreen(), QStringLiteral("lp/input-mode"));
        auto* page = shell.currentContent();
        QVERIFY(page->findChild<QRadioButton*>(QStringLiteral("input-new"))->isChecked());
        QVERIFY(page->findChild<QRadioButton*>(QStringLiteral("input-format-decimal"))->isChecked());
        QCOMPARE(page->findChild<QLineEdit*>(QStringLiteral("input-format-N"))->text(), QStringLiteral("5"));
        QCOMPARE(page->findChild<QLineEdit*>(QStringLiteral("input-format-D"))->text(), QStringLiteral("2"));
        auto* scientific = page->findChild<QRadioButton*>(QStringLiteral("input-format-scientific"));
        scientific->click();
        QVERIFY(scientific->isChecked());
        QVERIFY(!page->findChild<QRadioButton*>(QStringLiteral("input-format-decimal"))->isChecked());
        auto* existing = page->findChild<QRadioButton*>(QStringLiteral("input-existing"));
        existing->click();
        QVERIFY(page->findChild<QPushButton*>(QStringLiteral("input-go"))->isEnabled());
        page->findChild<QRadioButton*>(QStringLiteral("input-new"))->click();
        page->findChild<QPushButton*>(QStringLiteral("input-go"))->click();
        QCOMPARE(shell.currentScreen(), QStringLiteral("lp/input"));
        QVERIFY(shell.currentContent()->findChild<QLineEdit*>(QStringLiteral("variable-count"))->isEnabled());
        auto* file = shell.menuBar()->actions()[0]->menu();
        const auto nativeMenuCount = shell.menuBar()->findChildren<QMenu*>(QString(), Qt::FindDirectChildrenOnly).size();
        QCOMPARE(labels(*file), (QStringList{QStringLiteral("Save"), QStringLiteral("Save As"), QStringLiteral("Main Menu"), QStringLiteral("Exit Eidolon")}));
        QVERIFY(!file->actions()[0]->isEnabled());
        auto* edit = shell.menuBar()->actions()[1]->menu();
        QCOMPARE(labels(*edit), (QStringList{QStringLiteral("Undo Last Insert, Paste, Or Delete"), QStringLiteral("Copy Column or Row"),
            QStringLiteral("Paste on Column or Row"), QStringLiteral("Delete Column or Row"), QStringLiteral("Insert Column or Row")}));
        QCOMPARE(edit->actions()[0]->shortcut(), QKeySequence(QStringLiteral("Ctrl+U")));
        QVERIFY(!shell.currentContent()->findChild<QPushButton*>(QStringLiteral("input-solve-menu"))->isEnabled());
        QVERIFY(shell.navigate(QStringLiteral("lp/solve-menu")));
        QCOMPARE(shell.currentScreen(), QStringLiteral("lp/solve-menu"));
        shell.currentContent()->findChild<QPushButton*>(QStringLiteral("solve-view-input"))->click();
        QCOMPARE(shell.currentScreen(), QStringLiteral("lp/input"));
        shell.currentContent()->findChild<QPushButton*>(QStringLiteral("return-main"))->click();
        QCOMPARE(shell.currentScreen(), QStringLiteral("main"));
        QVERIFY(changed.count() >= 5);
        QVERIFY(!shell.navigate(QStringLiteral("unknown-screen")));
        QCOMPARE(shell.currentScreen(), QStringLiteral("main"));
        // Repeated navigation must not retain old menus or reset page choices.
        for (int i = 0; i < 20; ++i) {
            QVERIFY(shell.navigate(QStringLiteral("lp/input")));
            QCOMPARE(shell.menuBar()->findChildren<QMenu*>(QString(), Qt::FindDirectChildrenOnly).size(), nativeMenuCount);
            QVERIFY(shell.navigate(QStringLiteral("lp/input-mode")));
        }
        QVERIFY(shell.currentContent()->findChild<QRadioButton*>(QStringLiteral("input-format-scientific"))->isChecked());
    }

    void methodHierarchy()
    {
        ApplicationShell shell;
        shell.registerModule(lpModule());
        QVERIFY(!shell.currentContent()->findChild<QLabel*>(QStringLiteral("preview-notice")));
        QVERIFY(shell.navigate(QStringLiteral("lp/solve-menu")));
        QCOMPARE(shell.menuBar()->actions().front()->text(), QStringLiteral("SolveModify"));
        const auto* menu = shell.menuBar()->actions().front()->menu();
        QCOMPARE(labels(*menu), (QStringList{QStringLiteral("Solve Problem"), QStringLiteral("Class extensions"), QStringLiteral("View/Modify Input Data"), QStringLiteral("MAIN Menu"), QStringLiteral("Exit Eidolon")}));
        const auto* solve = menu->actions()[0]->menu();
        QCOMPARE(labels(*solve), (QStringList{QStringLiteral("Graphical"), QStringLiteral("Algebraic")}));
        const auto* algebraic = solve->actions()[1]->menu();
        QCOMPARE(labels(*algebraic), (QStringList{QStringLiteral("Final solution"), QStringLiteral("Iterations")}));
        const auto* iterations = algebraic->actions()[1]->menu();
        const QStringList expected{QStringLiteral("All-slack starting solution"), QStringLiteral("M-method"),
            QStringLiteral("Two-phase method"), QStringLiteral("Bounded simplex"), QStringLiteral("Dual simplex")};
        QCOMPARE(labels(*iterations), expected);
        QVERIFY(!solve->actions()[0]->isEnabled());
        QVERIFY(!algebraic->actions()[0]->isEnabled());
        for (const auto* action : iterations->actions()) QVERIFY(!action->isEnabled());
        const auto* panelMenu = shell.currentContent()->findChild<QToolButton*>(QStringLiteral("solve-problem"))->menu();
        QCOMPARE(labels(*panelMenu->actions()[1]->menu()->actions()[1]->menu()), expected);
        QVERIFY(shell.navigate(QStringLiteral("lp/output-format")));
        QVERIFY(shell.currentContent()->findChild<QRadioButton*>(QStringLiteral("output-format-decimal"))->isChecked());
        QVERIFY(!shell.currentContent()->findChild<QPushButton*>(QStringLiteral("output-go"))->isEnabled());
        QVERIFY(shell.navigate(QStringLiteral("lp/m-value")));
        QCOMPARE(shell.currentContent()->findChild<QLineEdit*>(QStringLiteral("m-value"))->text(), QStringLiteral("100"));
        QVERIFY(!shell.currentContent()->findChild<QPushButton*>(QStringLiteral("m-go"))->isEnabled());
        shell.currentContent()->findChild<QPushButton*>(QStringLiteral("m-go"))->click();
        QCOMPARE(shell.currentScreen(), QStringLiteral("lp/m-value"));
    }

    void moduleRegistration()
    {
        ApplicationShell shell;
        shell.registerModule(lpModule());
        QVERIFY(!shell.registerModule(lpModule()));
        QVERIFY(!shell.registerModule({QStringLiteral("broken"), {}, QStringLiteral("absent"), {}}));
        ModuleDescriptor extra{QStringLiteral("test-extension"), QStringLiteral("Test extension"), QStringLiteral("test/view"),
            {{QStringLiteral("test/view"), QStringLiteral("TEST"), [](ApplicationShell&) { return new QLabel(QStringLiteral("Extension fixture")); }, {}}}};
        shell.navigate(QStringLiteral("main"));
        QVERIFY(shell.registerModule(extra));
        auto* menu = shell.menuBar()->actions().front()->menu();
        QVERIFY(labels(*menu).contains(QStringLiteral("Test extension")));
        for (auto* action : menu->actions()) {
            if (action->text() == QStringLiteral("Test extension")) { action->trigger(); break; }
        }
        QCOMPARE(shell.currentScreen(), QStringLiteral("test/view"));
        QCOMPARE(qobject_cast<QLabel*>(shell.currentContent())->text(), QStringLiteral("Extension fixture"));
        extra.id = QStringLiteral("collision");
        QVERIFY(!shell.registerModule(extra));
        QCOMPARE(shell.modules().size(), 3);
    }

    void malformedThemeOnEveryScreen()
    {
        const QFont original = QApplication::font();
        QFont malformed(QStringLiteral("invalid theme font"));
        malformed.setPixelSize(167828479);
        malformed.setStyleHint(static_cast<QFont::StyleHint>(255));
        malformed.setStyleStrategy(static_cast<QFont::StyleStrategy>(254));
        malformed.setUnderline(true);
        QApplication::setFont(malformed);
        ApplicationShell shell;
        shell.registerModule(lpModule());
        shell.show();
        for (const auto& route : routes()) {
            QVERIFY(shell.navigate(route));
            QApplication::processEvents();
            auto widgets = shell.currentContent()->findChildren<QWidget*>();
            widgets.append(shell.currentContent());
            widgets.append(shell.menuBar());
            widgets.append(shell.menuBar()->findChildren<QWidget*>());
            for (auto* widget : widgets) {
                const QFontMetrics metrics(widget->font());
                QVERIFY2(!widget->font().underline(), qPrintable(route + QLatin1Char('/') + widget->objectName()
                    + QLatin1Char('/') + QString::fromLatin1(widget->metaObject()->className()) + QLatin1Char('/') + widget->font().toString()));
                QVERIFY(metrics.height() > 0 && metrics.height() < 200);
                for (const QChar character : QStringLiteral("MAIN Linear Programming"))
                    QVERIFY2(metrics.inFont(character), qPrintable(route + QLatin1Char('/') + widget->objectName()));
            }
        }
        QApplication::setFont(original);
    }

    void latePopupsRemainReadable()
    {
        struct RestorePalette {
            QPalette palette = QApplication::palette();
            ~RestorePalette() { QApplication::setPalette(palette); }
        } restore;
        QPalette dark(QColor(32, 32, 32));
        dark.setColor(QPalette::WindowText, Qt::white);
        dark.setColor(QPalette::Text, Qt::white);
        QApplication::setPalette(dark);
        ApplicationShell shell;
        QVERIFY(shell.registerModule(lpModule()));
        shell.show();
        QVERIFY(shell.navigate(QStringLiteral("lp/solve-menu")));

        auto* solve = shell.currentContent()->findChild<QToolButton*>(QStringLiteral("solve-problem"));
        QVERIFY(solve);
        auto* menu = solve->menu();
        QVERIFY(menu);
        auto* nested = menu->actions()[1]->menu();
        QVERIFY(nested);
        QMenu lateMenu(&shell);
        lateMenu.addAction(QStringLiteral("Late popup 123"));
        QLabel latePopup(QStringLiteral("Popup text 123"), &shell, Qt::Popup);
        QMessageBox notice(QMessageBox::Information, QStringLiteral("Result"),
                           QStringLiteral("Optimal solution 123"), QMessageBox::Ok, &shell);
        QFont broken(QStringLiteral("invalid theme font"), 12);
        broken.setStyleHint(static_cast<QFont::StyleHint>(255));
        broken.setUnderline(true);
        int popupIndex = 0;
        for (QWidget* popup : {static_cast<QWidget*>(menu), static_cast<QWidget*>(nested),
                              static_cast<QWidget*>(&lateMenu), static_cast<QWidget*>(&latePopup),
                              static_cast<QWidget*>(&notice)}) {
            popup->setFont(broken);
            popup->show();
            QApplication::processEvents();
            QVERIFY(popup->isVisible());
            auto widgets = popup->findChildren<QWidget*>();
            widgets.prepend(popup);
            for (auto* widget : widgets) {
                const QFontMetrics metrics(widget->font());
                QVERIFY2(!widget->font().underline(), qPrintable(QStringLiteral("Popup %1: %2 %3")
                    .arg(popupIndex).arg(QString::fromLatin1(widget->metaObject()->className()), widget->font().toString())));
                QVERIFY(metrics.height() > 0 && metrics.height() < 200);
                for (const QChar character : QStringLiteral("Optimal solution 123"))
                    QVERIFY(metrics.inFont(character));
            }
            for (const auto group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
                const auto palette = popup->palette();
                QVERIFY(palette.color(group, QPalette::WindowText) != palette.color(group, QPalette::Window));
                QVERIFY(palette.color(group, QPalette::Text) != palette.color(group, QPalette::Base));
                QVERIFY(palette.color(group, QPalette::HighlightedText) != palette.color(group, QPalette::Highlight));
                QVERIFY(palette.color(group, QPalette::Window).lightness() > 200);
            }
            if (auto* popupMenu = qobject_cast<QMenu*>(popup)) {
                QCOMPARE(popupMenu->palette().color(QPalette::Window), QColor(Qt::white));
                QCOMPARE(popupMenu->palette().color(QPalette::WindowText), QColor(Qt::black));
                for (auto* action : popupMenu->actions()) {
                    if (action->isEnabled() && !action->isSeparator()) {
                        popupMenu->setActiveAction(action);
                        break;
                    }
                }
            }
            const auto captureRoot = qEnvironmentVariable("TORA_UI_CAPTURES");
            if (!captureRoot.isEmpty()) {
                const auto directory = captureRoot + QStringLiteral("/popups");
                QVERIFY(QDir().mkpath(directory));
                QVERIFY(popup->grab().save(directory + QStringLiteral("/popup-%1-scale-%2.png")
                    .arg(popupIndex).arg(shell.devicePixelRatioF())));
            }
            ++popupIndex;
            popup->hide();
        }
    }

    void comboPopupRemainsReadable()
    {
        ApplicationShell shell;
        shell.show();
        QComboBox combo(&shell);
        combo.addItems({QStringLiteral("Text Files (*.txt)"), QStringLiteral("All Files (*)")});
        QPalette dark(QColor(32, 32, 32));
        dark.setColor(QPalette::Text, Qt::white);
        combo.setPalette(dark);
        QFont tiny = combo.font();
        tiny.setPointSize(7);
        combo.setFont(tiny);
        combo.show();
        combo.showPopup();
        QApplication::processEvents();
        auto* view = combo.view();
        QVERIFY(view->isVisible());
        QVERIFY(view->font().pointSizeF() >= 12);
        QVERIFY(QFontMetrics(view->font()).inFont(QLatin1Char('A')));
        const auto palette = view->palette();
        QVERIFY(palette.color(QPalette::Text) != palette.color(QPalette::Base));
        QVERIFY(palette.color(QPalette::Base).lightness() > 200);
        QTest::keyClick(view, Qt::Key_Down);
        QTest::keyClick(view, Qt::Key_Return);
        QCOMPARE(combo.currentIndex(), 1);
        QVERIFY(!view->isVisible());
    }

    void smallWindowsWithLargeFonts_data()
    {
        QTest::addColumn<int>("points");
        QTest::newRow("normal-font") << 12;
        QTest::newRow("large-font") << 24;
    }

    void smallWindowsWithLargeFonts()
    {
        QFETCH(int, points);
        struct RestoreFont {
            QFont font = QApplication::font();
            ~RestoreFont() { QApplication::setFont(font); }
        } restore;
        auto font = restore.font;
        font.setPointSize(points);
        QApplication::setFont(font);
        ApplicationShell shell;
        QVERIFY(shell.registerModule(lpModule()));
        shell.resize(640, 360);
        shell.show();
        for (const auto& route : routes()) {
            QVERIFY(shell.navigate(route));
            QApplication::processEvents();
            auto* page = shell.currentContent();
            auto* area = qobject_cast<QScrollArea*>(page->parentWidget()->parentWidget());
            QVERIFY(area);
            const auto captureRoot = qEnvironmentVariable("TORA_UI_CAPTURES");
            if (!captureRoot.isEmpty()) {
                const auto directory = captureRoot + QStringLiteral("/small-windows");
                QVERIFY(QDir().mkpath(directory));
                auto name = route;
                name.replace(QLatin1Char('/'), QLatin1Char('-'));
                QVERIFY(shell.grab().save(directory + QStringLiteral("/%1-%2pt-scale-%3.png")
                    .arg(name).arg(points).arg(shell.devicePixelRatioF())));
            }
            for (auto* control : page->findChildren<QPushButton*>()) {
                QVERIFY(control->size().expandedTo(control->minimumSizeHint()) == control->size());
                area->ensureWidgetVisible(control);
                QApplication::processEvents();
                const QRect bounds(control->mapTo(area->viewport(), QPoint()), control->size());
                QVERIFY2(area->viewport()->rect().contains(bounds), qPrintable(route + QLatin1Char('/') + control->objectName()));
            }
        }
    }

    void geometryAndPreferences()
    {
        const QVector<QRect> screens{QRect(0, 0, 1366, 740), QRect(-1920, 100, 1920, 1000)};
        for (const QRect request : {QRect(200, 80, 900, 500), QRect(-1700, 200, 1200, 600),
                                   QRect(3000, 400, 900, 500), QRect(0, 0, 4000, 3000), QRect(-50, -50, 0, 0)}) {
            const QRect safe = reachableGeometry(request, screens);
            QVERIFY(safe.isValid());
            QVERIFY(screens[0].contains(safe) || screens[1].contains(safe));
        }
        QCOMPARE(reachableGeometry(QRect(-1700, 200, 1200, 600), screens), QRect(-1700, 200, 1200, 600));
        const QVector<QRect> islands{QRect(0, 0, 960, 540), QRect(1920, 0, 960, 540)};
        const auto gap = reachableGeometry(QRect(1100, 200, 500, 300), islands);
        QVERIFY(islands[0].contains(gap) || islands[1].contains(gap));
        const auto unplugged = reachableGeometry(QRect(2000, 100, 800, 400), {screens[0]});
        QVERIFY(screens[0].contains(unplugged));
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QSettings settings(directory.filePath(QStringLiteral("window.ini")), QSettings::IniFormat);
        settings.setValue(QStringLiteral("window/normalGeometry"), QRect(50000, 50000, 9000, 5000));
        ApplicationShell shell(&settings);
        shell.show();
        QTest::qWait(1);
        QVERIFY(shell.screen()->availableGeometry().contains(shell.frameGeometry()));
        shell.close();
        QVERIFY(settings.value(QStringLiteral("window/normalGeometry")).toRect().width() < 9000);
        ApplicationShell restored(&settings);
        QCOMPARE(restored.geometry(), settings.value(QStringLiteral("window/normalGeometry")).toRect());
    }

    void scaledLayouts_data()
    {
        QTest::addColumn<QSize>("physicalSize");
        QTest::newRow("1366x768") << QSize(1366, 768);
        QTest::newRow("1920x1080") << QSize(1920, 1080);
        QTest::newRow("3840x2160") << QSize(3840, 2160);
    }

    void scaledLayouts()
    {
        QFETCH(QSize, physicalSize);
        ApplicationShell shell;
        shell.registerModule(lpModule());
        shell.show();
        QTest::qWait(1);
        const qreal factor = qEnvironmentVariable("QT_SCALE_FACTOR").toDouble();
        QVERIFY(std::abs(shell.devicePixelRatioF() - factor) < 0.01);
        shell.resize(QSize(qRound(physicalSize.width() / factor), qRound(physicalSize.height() / factor)));
        for (const auto& route : routes()) {
            QVERIFY(shell.navigate(route));
            QApplication::processEvents();
            auto* page = shell.currentContent();
            auto* scroll = page->parentWidget()->parentWidget();
            auto* area = qobject_cast<QScrollArea*>(scroll);
            QVERIFY(area);
            for (auto* control : page->findChildren<QPushButton*>()) {
                QVERIFY(control->width() >= control->minimumSizeHint().width());
                QVERIFY(control->height() >= control->minimumSizeHint().height());
                area->ensureWidgetVisible(control);
                QApplication::processEvents();
                const QRect mapped(control->mapTo(area->viewport(), QPoint()), control->size());
                QVERIFY2(area->viewport()->rect().contains(mapped), qPrintable(route + QStringLiteral("/") + control->objectName()));
                if (control->isEnabled()) {
                    control->setFocus();
                    QVERIFY(control->hasFocus());
                }
            }
            const auto image = page->grab().toImage();
            QVERIFY(!image.isNull());
            QCOMPARE(image.pixelColor(qRound(factor), qRound(factor)), QColor(colors::canvas));
            if (auto* notice = page->findChild<QLabel*>(QStringLiteral("preview-notice")))
                QCOMPARE(notice->palette().color(QPalette::WindowText), QColor(Qt::white));
            QVERIFY(std::abs(image.devicePixelRatio() - factor) < 0.01);
            QVERIFY(image.width() >= page->width() * factor - 1);
            // Optional original clone screenshots for gate review, never
            // reference images or product/release resources.
            const QString captureRoot = qEnvironmentVariable("TORA_UI_CAPTURES");
            if (!captureRoot.isEmpty() && physicalSize == QSize(1920, 1080)) {
                const QString directory = captureRoot + QStringLiteral("/scale-%1").arg(factor);
                QVERIFY(QDir().mkpath(directory));
                QString name = route;
                name.replace(QLatin1Char('/'), QLatin1Char('-'));
                QVERIFY(image.save(directory + QLatin1Char('/') + name + QStringLiteral(".png")));
            }
            const QFontMetrics metrics(page->font());
            QVERIFY(metrics.height() > 0 && metrics.height() < 100);
        }
        QSignalSpy metricsChanged(&shell, &ApplicationShell::displayMetricsChanged);
        QFont enlarged = shell.font();
        enlarged.setPointSizeF(enlarged.pointSizeF() * 1.5);
        shell.setFont(enlarged);
        QEvent dpiChanged(QEvent::DevicePixelRatioChange);
        QApplication::sendEvent(&shell, &dpiChanged);
        QVERIFY(metricsChanged.count() >= 2);
        QApplication::processEvents();
        for (auto* control : shell.currentContent()->findChildren<QPushButton*>())
            QVERIFY(control->width() >= control->minimumSizeHint().width());
    }
};

int main(int argc, char** argv)
{
#ifdef Q_OS_WIN
    qputenv("QT_QPA_FONTDIR", qgetenv("SystemRoot") + "/Fonts");
#endif
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication application(argc, argv);
    ShellTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "desktop_shell.moc"
