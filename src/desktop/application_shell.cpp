#include "application_shell.hpp"
#include "screen_widgets.hpp"
#include "ui_font.hpp"

#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QFontMetrics>
#include <QFrame>
#include <QGuiApplication>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QSettings>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>
#include <algorithm>
#include <utility>

namespace tora::desktop {
namespace {
QVector<QRect> availableScreens()
{
    QVector<QRect> result;
    for (const auto* screen : QGuiApplication::screens()) result.append(screen->availableGeometry());
    return result;
}
QStringList mainLabels(const ApplicationShell& shell)
{
    QStringList labels{QStringLiteral("Linear Equations"), QStringLiteral("Linear Programming"),
        QStringLiteral("Transportation model"), QStringLiteral("Integer programming"), QStringLiteral("Network models"),
        QStringLiteral("Project Planning"), QStringLiteral("Queuing analysis"), QStringLiteral("Zero-Sum Games")};
    for (const auto& module : shell.modules())
        if (!module.displayName.isEmpty() && !labels.contains(module.displayName)) labels.append(module.displayName);
    return labels;
}
QString entryFor(const ApplicationShell& shell, const QString& label)
{
    for (const auto& module : shell.modules()) if (module.displayName == label) return module.entryScreen;
    return {};
}
}

QRect reachableGeometry(QRect requested, const QVector<QRect>& screens)
{
    if (screens.isEmpty()) return requested;
    QRect target = screens.front();
    qint64 bestArea = -1;
    for (const auto& screen : screens) {
        if (!screen.isValid()) continue;
        if (screen.contains(requested)) return requested;
        const QRect intersection = screen.intersected(requested);
        const qint64 area = static_cast<qint64>(intersection.width()) * intersection.height();
        if (area > bestArea) { bestArea = area; target = screen; }
    }
    QSize size = requested.size().boundedTo(target.size());
    if (!size.isValid() || size.isEmpty()) size = target.size() * 0.8;
    requested.setSize(size);
    if (bestArea <= 0) requested.moveCenter(target.center());
    requested.moveLeft(std::clamp(requested.left(), target.left(), target.right() - size.width() + 1));
    requested.moveTop(std::clamp(requested.top(), target.top(), target.bottom() - size.height() + 1));
    return requested;
}

void blueCanvas(QWidget& widget)
{
    QPalette palette = widget.palette();
    palette.setColor(QPalette::Window, QColor(Qt::blue));
    palette.setColor(QPalette::WindowText, QColor(Qt::white));
    palette.setColor(QPalette::Text, QColor(Qt::black));
    palette.setColor(QPalette::Base, QColor(Qt::cyan));
    palette.setColor(QPalette::Button, QColor(239, 239, 239));
    palette.setColor(QPalette::ButtonText, QColor(Qt::black));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(100, 100, 100));
    widget.setPalette(palette);
    widget.setAutoFillBackground(true);
}

QPushButton* button(const QString& text, const QString& id, QVBoxLayout& layout,
                    const std::function<void()>& clicked)
{
    auto* control = new QPushButton(text);
    control->setObjectName(id);
    control->setAccessibleName(text);
    layout.addWidget(control);
    QObject::connect(control, &QPushButton::clicked, control, clicked);
    return control;
}

QLabel* previewNotice(QVBoxLayout& layout)
{
    auto* notice = new QLabel(QStringLiteral("Navigation preview — problem editing, files and solving will be available in a later update."));
    notice->setObjectName(QStringLiteral("preview-notice"));
    notice->setWordWrap(true);
    notice->setAlignment(Qt::AlignCenter);
    layout.addWidget(notice);
    return notice;
}

void centerPanel(QVBoxLayout& layout, QWidget* panel)
{
    panel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    auto* row = new QHBoxLayout;
    row->addStretch();
    row->addWidget(panel);
    row->addStretch();
    layout.addStretch();
    layout.addLayout(row);
    layout.addStretch();
}

ApplicationShell::ApplicationShell(QSettings* settings, QWidget* parent)
    : QMainWindow(parent), settings_(settings), stack_(new QStackedWidget(this))
{
    // Internal Qt menus (including the menubar's overflow menu) can be created
    // after page polishing and inherit the application font directly.
    QApplication::setFont(usableUiFont(QApplication::font()));
    setObjectName(QStringLiteral("application-shell"));
    setFont(usableUiFont(QApplication::font()));
    setWindowTitle(QStringLiteral("TORA Clone — Navigation Preview"));
    // Decorations are logical-pixel strokes; control sizes remain native font
    // metrics/layout hints. Scope palette overrides to this window so desktop
    // themes cannot turn TORA's blue/cyan roles into unreadable dark panels.
    setStyleSheet(QStringLiteral(
        "QLabel { color: white; }"
        "QGroupBox { color: white; border: 1px solid white; margin-top: 1em; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; }"
        "QRadioButton { color: white; }"
        "QPushButton, QToolButton { background: #efefef; color: black; border: 1px solid #777777; padding: 0.25em 0.7em; }"
        "QPushButton:focus, QToolButton:focus { border: 1px solid #000080; }"
        "QPushButton:hover, QToolButton:hover { background: white; }"
        "QPushButton:disabled, QToolButton:disabled { color: #666666; }"
        "QLineEdit { background: cyan; color: blue; border: 1px solid white; }"
        "QFrame#welcome-panel { border: 1px solid white; }"));
    setCentralWidget(stack_);
    // Keep menus inside the window on all OSes, matching the observed layout.
    menuBar()->setNativeMenuBar(false);
    menuBar()->setStyleSheet(QStringLiteral("QMenuBar, QMenu { background: white; color: black; }"
        "QMenuBar::item:selected, QMenu::item:selected { background: #000080; color: white; }"
        "QMenu::item:disabled { color: #666666; }"));
    registerModule(shellModule());
    navigate(QStringLiteral("welcome"));
    const QFontMetrics metrics(font());
    QRect initial(QPoint(), QSize(metrics.horizontalAdvance(QStringLiteral("M")) * 72, metrics.height() * 30));
    if (settings_) initial = settings_->value(QStringLiteral("window/normalGeometry"), initial).toRect();
    setGeometry(reachableGeometry(initial, availableScreens()));
    if (settings_ && settings_->value(QStringLiteral("window/maximized"), false).toBool())
        setWindowState(windowState() | Qt::WindowMaximized);
    connect(qApp, &QGuiApplication::screenRemoved, this, [this](QScreen*) {
        QTimer::singleShot(0, this, [this] { keepReachable(); refreshMetrics(); });
    });
}

bool ApplicationShell::registerModule(ModuleDescriptor module)
{
    if (module.id.isEmpty() || module.screens.isEmpty()) return false;
    QStringList incoming;
    for (const auto& descriptor : module.screens) {
        if (descriptor.id.isEmpty() || !descriptor.create || incoming.contains(descriptor.id)) return false;
        incoming.append(descriptor.id);
    }
    if (!incoming.contains(module.entryScreen)) return false;
    for (const auto& existing : modules_) {
        if (existing.id == module.id) return false;
        for (const auto& screen : existing.screens) if (incoming.contains(screen.id)) return false;
    }
    modules_.append(std::move(module));
    // The MAIN panel reflects registrations, including future modules.
    const bool wasMain = currentScreen_ == QStringLiteral("main");
    for (int i = 0; i < stack_->count(); ++i) {
        if (stack_->widget(i)->objectName() == QStringLiteral("main")) {
            auto* old = stack_->widget(i);
            stack_->removeWidget(old);
            old->deleteLater();
            break;
        }
    }
    if (wasMain) navigate(QStringLiteral("main"));
    return true;
}

bool ApplicationShell::navigate(const QString& screenId)
{
    const ScreenDescriptor* descriptor = nullptr;
    for (const auto& module : modules_)
        for (const auto& screen : module.screens)
            if (screen.id == screenId) descriptor = &screen;
    if (!descriptor) return false;
    QWidget* page = nullptr;
    for (int i = 0; i < stack_->count(); ++i)
        if (stack_->widget(i)->objectName() == screenId) page = stack_->widget(i);
    if (!page) {
        page = new QWidget;
        page->setObjectName(screenId);
        blueCanvas(*page);
        auto* outer = new QVBoxLayout(page);
        outer->setContentsMargins(0, 0, 0, 0);
        if (!descriptor->heading.isEmpty()) {
            auto* heading = new QLabel(descriptor->heading);
            heading->setObjectName(QStringLiteral("screen-heading"));
            heading->setAlignment(Qt::AlignCenter);
            heading->setStyleSheet(QStringLiteral("background: white; color: blue;"));
            outer->addWidget(heading);
        }
        auto* scroll = new QScrollArea;
        scroll->setObjectName(QStringLiteral("screen-scroll"));
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setWidgetResizable(true);
        blueCanvas(*scroll);
        auto* content = descriptor->create(*this);
        content->setObjectName(QStringLiteral("screen-content"));
        blueCanvas(*content);
        if (content->layout()) content->layout()->setSizeConstraint(QLayout::SetMinimumSize);
        scroll->setWidget(content);
        outer->addWidget(scroll);
        polishUsableFonts(*page);
        stack_->addWidget(page);
    }
    // QMenuBar::clear removes actions but does not delete their QMenus.
    const auto previousMenus = menuBar()->actions();
    for (auto* action : previousMenus) {
        if (auto* menu = action->menu()) {
            menu->hide();
            menu->setParent(nullptr);
            // Navigation can originate in QAction::triggered. Defer deletion
            // until Qt has finished dispatching that action/menu event.
            menu->deleteLater();
        }
    }
    menuBar()->clear();
    if (descriptor->menus) descriptor->menus(*this, *menuBar());
    polishUsableFonts(*menuBar());
    menuBar()->setVisible(!menuBar()->actions().isEmpty());
    stack_->setCurrentWidget(page);
    currentScreen_ = screenId;
    emit screenChanged(screenId);
    const auto controls = page->findChildren<QPushButton*>();
    for (auto* control : controls) if (control->isEnabled()) { control->setFocus(); break; }
    return true;
}

QWidget* ApplicationShell::currentContent() const
{
    return stack_->currentWidget()->findChild<QWidget*>(QStringLiteral("screen-content"));
}

bool ApplicationShell::event(QEvent* event)
{
    const bool result = QMainWindow::event(event);
    if (stack_ && (event->type() == QEvent::FontChange || event->type() == QEvent::DevicePixelRatioChange))
        refreshMetrics();
    return result;
}

void ApplicationShell::refreshMetrics()
{
    if (refreshingMetrics_) return;
    refreshingMetrics_ = true;
    for (int i = 0; i < stack_->count(); ++i) {
        auto* page = stack_->widget(i);
        for (auto* widget : page->findChildren<QWidget*>()) {
            if (widget->property("toraRecoveredFont").toBool()) widget->setFont(usableUiFont(font()));
            if (widget->objectName() == QStringLiteral("welcome-title")) {
                QFont titleFont = usableUiFont(font());
                if (titleFont.pointSizeF() > 0) titleFont.setPointSizeF(titleFont.pointSizeF() * 4);
                else titleFont.setPixelSize(titleFont.pixelSize() * 4);
                titleFont.setBold(true);
                widget->setFont(titleFont);
            }
            if (widget->objectName() == QStringLiteral("welcome-panel"))
                widget->setMinimumHeight(fontMetrics().height() * 12);
            if (widget->layout()) widget->layout()->invalidate();
            widget->updateGeometry();
            widget->update();
        }
    }
    refreshingMetrics_ = false;
    emit displayMetricsChanged();
}

void ApplicationShell::watchScreen(QScreen* screen)
{
    for (const auto& connection : screenConnections_) disconnect(connection);
    screenConnections_.clear();
    if (screen) {
        screenConnections_.append(connect(screen, &QScreen::logicalDotsPerInchChanged, this,
            [this](qreal) { refreshMetrics(); }));
        screenConnections_.append(connect(screen, &QScreen::availableGeometryChanged, this,
            [this](const QRect&) { keepReachable(); refreshMetrics(); }));
    }
    refreshMetrics();
}

void ApplicationShell::keepReachable()
{
    if (!isMaximized() && !isFullScreen()) {
        const QRect repaired = reachableGeometry(frameGeometry(), availableScreens());
        if (repaired != frameGeometry()) {
            // Frame size includes native decoration. Resize by the difference,
            // then move the frame origin (QWidget::move includes decorations).
            resize(size() + repaired.size() - frameGeometry().size());
            move(repaired.topLeft());
        }
    }
}

void ApplicationShell::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    if (!watchingWindow_ && windowHandle()) {
        watchingWindow_ = true;
        connect(windowHandle(), &QWindow::screenChanged, this, &ApplicationShell::watchScreen);
        watchScreen(windowHandle()->screen());
        QTimer::singleShot(0, this, [this] { keepReachable(); });
    }
}

void ApplicationShell::closeEvent(QCloseEvent* event)
{
    if (settings_) {
        settings_->setValue(QStringLiteral("window/normalGeometry"), isMaximized() ? normalGeometry() : geometry());
        settings_->setValue(QStringLiteral("window/maximized"), isMaximized());
    }
    QMainWindow::closeEvent(event);
}

ModuleDescriptor shellModule()
{
    ModuleDescriptor module{QStringLiteral("shell"), {}, QStringLiteral("welcome"), {}};
    module.screens.append({QStringLiteral("welcome"), {}, [](ApplicationShell& shell) {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        auto* panel = new QFrame;
        panel->setObjectName(QStringLiteral("welcome-panel"));
        panel->setFrameShape(QFrame::Box);
        panel->setMinimumHeight(shell.fontMetrics().height() * 12);
        panel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        auto* inner = new QVBoxLayout(panel);
        auto* title = new QLabel(QStringLiteral("TORA Clone"));
        title->setObjectName(QStringLiteral("welcome-title"));
        QFont font = shell.font();
        font.setPointSizeF(font.pointSizeF() > 0 ? font.pointSizeF() * 4 : 48);
        font.setBold(true);
        title->setFont(font);
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet(QStringLiteral("color: cyan;"));
        inner->addWidget(title);
        auto* description = new QLabel(QStringLiteral("Operations Research\nLinear Programming"));
        description->setAlignment(Qt::AlignCenter);
        description->setStyleSheet(QStringLiteral("background: cyan; color: black;"));
        inner->addWidget(description);
        button(QStringLiteral("Click Here"), QStringLiteral("welcome-enter"), *inner,
            [&shell] { shell.navigate(QStringLiteral("main")); });
        centerPanel(*layout, panel);
        previewNotice(*layout);
        return page;
    }, {}});
    const auto mainMenus = [](ApplicationShell& shell, QMenuBar& bar) {
        auto* menu = bar.addMenu(QStringLiteral("MainMenu"));
        const QStringList labels = mainLabels(shell);
        for (const auto& label : labels) {
            const QString entry = entryFor(shell, label);
            auto* action = menu->addAction(label);
            action->setEnabled(!entry.isEmpty());
            if (action->isEnabled()) QObject::connect(action, &QAction::triggered, &shell,
                [&shell, entry] { shell.navigate(entry); });
        }
        menu->addSeparator();
        menu->addAction(QStringLiteral("EXIT TORA"), &shell, &QWidget::close);
    };
    module.screens.append({QStringLiteral("main"), {}, [](ApplicationShell& shell) {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        auto* panel = new QFrame;
        panel->setObjectName(QStringLiteral("main-panel"));
        panel->setStyleSheet(QStringLiteral("QFrame { background: white; color: black; }"
            "QPushButton { background: white; color: black; border: none; text-align: left; padding: 0.15em 0.25em; }"
            "QPushButton:disabled { color: #666666; }"
            "QPushButton:focus, QPushButton:hover { background: #000080; color: white; }"));
        auto* inner = new QVBoxLayout(panel);
        auto* heading = new QLabel(QStringLiteral("MAIN"));
        heading->setAlignment(Qt::AlignCenter);
        heading->setStyleSheet(QStringLiteral("background: #efefef; color: black;"));
        inner->addWidget(heading);
        const QStringList labels = mainLabels(shell);
        for (const auto& label : labels) {
            const QString entry = entryFor(shell, label);
            const bool lp = label == QStringLiteral("Linear Programming");
            auto* action = button(label, lp ? QStringLiteral("main-lp") : label, *inner,
                [&shell, entry] { shell.navigate(entry); });
            action->setEnabled(!entry.isEmpty());
            if (entry.isEmpty()) action->setToolTip(QStringLiteral("This module is not available in the LP edition."));
        }
        auto* separator = new QFrame;
        separator->setFrameShape(QFrame::HLine);
        inner->addWidget(separator);
        button(QStringLiteral("EXIT TORA"), QStringLiteral("main-exit"), *inner, [&shell] { shell.close(); });
        centerPanel(*layout, panel);
        return page;
    }, mainMenus});
    return module;
}
} // namespace tora::desktop
