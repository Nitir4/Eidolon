#include <tora/build_info.hpp>

#include <QApplication>
#include <QCommandLineParser>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    QApplication::setApplicationName(QStringLiteral("ToraClone"));
    QApplication::setApplicationDisplayName(QStringLiteral("TORA Clone"));
    QApplication::setApplicationVersion(QString::fromLatin1(
        tora::version.data(), static_cast<qsizetype>(tora::version.size())));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption smoke_test(QStringLiteral("smoke-test"),
                                       QStringLiteral("Start Qt Widgets and exit."));
    parser.addOption(smoke_test);
    parser.process(application);

    // Bootstrap only; the main window and navigation are Foundation 5.
    QWidget window;
    window.setWindowTitle(QApplication::applicationDisplayName());
    auto* layout = new QVBoxLayout(&window);
    auto* title = new QLabel(QApplication::applicationDisplayName(), &window);
    title->setWordWrap(true);
    layout->addWidget(title);
    window.show();

    if (parser.isSet(smoke_test)) {
        QTimer::singleShot(0, &application, &QApplication::quit);
    }
    return application.exec();
}
