#pragma once

#include <QMainWindow>
#include <QRect>
#include <QString>
#include <QVector>
#include <functional>

class QMenuBar;
class QScreen;
class QSettings;
class QStackedWidget;

namespace tora::desktop {

class ApplicationShell;
struct ScreenDescriptor {
    QString id;
    QString heading;
    std::function<QWidget*(ApplicationShell&)> create;
    std::function<void(ApplicationShell&, QMenuBar&)> menus;
};

// Compiled-in modules supply screens now; controllers/codecs can be attached
// to their factories later without teaching the shell LP-specific behavior.
struct ModuleDescriptor {
    QString id;
    QString displayName;
    QString entryScreen;
    QVector<ScreenDescriptor> screens;
};

// Work with individual screen rectangles: mixed-DPI desktops may have gaps.
QRect reachableGeometry(QRect requested, const QVector<QRect>& screens);

class ApplicationShell final : public QMainWindow {
    Q_OBJECT
public:
    explicit ApplicationShell(QSettings* settings = nullptr, QWidget* parent = nullptr);
    bool registerModule(ModuleDescriptor module);
    bool navigate(const QString& screenId);
    QString currentScreen() const { return currentScreen_; }
    QWidget* currentContent() const;
    QVector<ModuleDescriptor> modules() const { return modules_; }

signals:
    void screenChanged(const QString& screenId);
    void displayMetricsChanged();

protected:
    bool event(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void refreshMetrics();
    void watchScreen(QScreen* screen);
    void keepReachable();
    QSettings* settings_;
    QStackedWidget* stack_;
    QVector<ModuleDescriptor> modules_;
    QString currentScreen_;
    bool watchingWindow_ = false;
    bool refreshingMetrics_ = false;
    QVector<QMetaObject::Connection> screenConnections_;
};

ModuleDescriptor shellModule();
} // namespace tora::desktop
