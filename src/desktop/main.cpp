#include <tora/build_info.hpp>
#include "ui_font.hpp"
#include "application_shell.hpp"
#include "lp_screens.hpp"
#include "transportation_screens.hpp"
#include "assignment_controller.hpp"
#include "games_screens.hpp"
#include "equations_screens.hpp"
#include "queuing_screens.hpp"
#include "networks_screens.hpp"
#include "integer_controller.hpp"
#include "planning_screens.hpp"

#include <QApplication>
#include <QCommandLineParser>
#include <QSettings>
#include <QStyleFactory>
#include <QTimer>

int main(int argc, char* argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication application(argc, argv);
    // Native Qt Widgets painted in a consistent classic style across OSes.
    // This also avoids third-party style plugins replacing recovered fonts.
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QApplication::setFont(tora::desktop::readableUiFont(application.font()));
    QApplication::setApplicationName(QStringLiteral("Eidolon"));
    QApplication::setOrganizationName(QStringLiteral("Eidolon"));
    QApplication::setApplicationDisplayName(QStringLiteral("Eidolon"));
    QGuiApplication::setDesktopFileName(QStringLiteral("eidolon"));
    QApplication::setApplicationVersion(QString::fromLatin1(
        tora::version.data(), static_cast<qsizetype>(tora::version.size())));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption smoke_test(QStringLiteral("smoke-test"),
                                       QStringLiteral("Start Qt Widgets and exit."));
    parser.addOption(smoke_test);
    const QCommandLineOption screen(QStringLiteral("screen"), QStringLiteral("Preview a registered screen (e.g. lp/output-format)."), QStringLiteral("id"));
    const QCommandLineOption no_settings(QStringLiteral("no-settings"), QStringLiteral("Do not read or save window preferences."));
    parser.addOption(screen);
    parser.addOption(no_settings);
    parser.process(application);

    // Keep the existing preference store across the display/executable rename.
    QSettings settings(QStringLiteral("ToraClone"), QStringLiteral("ToraClone"));
    tora::desktop::ApplicationShell window(parser.isSet(smoke_test) || parser.isSet(no_settings) ? nullptr : &settings);
    window.registerModule(tora::desktop::lpModule());
    window.registerModule(tora::desktop::transportationModule());
    window.registerModule(tora::desktop::assignmentModule());
    window.registerModule(tora::desktop::gamesModule());
    window.registerModule(tora::desktop::equationsModule());
    window.registerModule(tora::desktop::queuingModule());
    window.registerModule(tora::desktop::networksModule());
    window.registerModule(tora::desktop::integerModule());
    window.registerModule(tora::desktop::planningModule());
    if (parser.isSet(screen) && !window.navigate(parser.value(screen))) {
        qCritical("Unknown screen ID");
        return 2;
    }
    if (!parser.isSet(smoke_test) && (parser.isSet(no_settings) || !settings.contains(QStringLiteral("window/maximized"))))
        window.showMaximized();
    else
        window.show();

    if (parser.isSet(smoke_test)) {
        QTimer::singleShot(0, &application, &QApplication::quit);
    }
    return application.exec();
}
