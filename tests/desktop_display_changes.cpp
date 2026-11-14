#include "application_shell.hpp"
#include "lp_input_controller.hpp"
#include "lp_simplex_controller.hpp"
#include "lp_graphical_controller.hpp"
#include "lp_final_controller.hpp"
#include "lp_screens.hpp"
#include <QApplication>
#include <QDir>
#include <QFontMetrics>
#include <QJsonArray>
#include <QJsonObject>
#include <QPushButton>
#include <QScreen>
#include <QSignalSpy>
#include <QTest>
#include <QWindow>
#include <qpa/qplatformnativeinterface.h>

using namespace tora::desktop;
class DisplayChanges final : public QObject {
    Q_OBJECT
    // Test-only hooks in Qt's offscreen platform. Product code uses public Qt APIs.
    using SetConfiguration = void (*)(const QJsonObject&, QPlatformNativeInterface*);
    using Configuration = QJsonObject (*)(QPlatformNativeInterface*);
    QPlatformNativeInterface* native_ = nullptr;
    SetConfiguration set_ = nullptr;
    QJsonObject original_;
    static QJsonObject monitor(const QString& name, int x, int width, int height, int dpi) {
        return {{"name",name},{"x",x},{"y",0},{"width",width},{"height",height},
                {"logicalDpi",dpi},{"logicalBaseDpi",96},{"dpr",1}};
    }
    void configure(const QJsonArray& screens) {
        set_({{"screens",screens},{"windowFrameMargins",true}},native_);
        QApplication::processEvents();
    }
    static QScreen* screen(const QString& name) {
        for(auto* s:QGuiApplication::screens())if(s->name()==name)return s;
        return nullptr;
    }
    void checkView(ApplicationShell& shell, const QString& stage, int dpi) {
        for(auto* button:shell.currentContent()->findChildren<QPushButton*>())
            QVERIFY(button->size().expandedTo(button->minimumSizeHint())==button->size());
        for(auto ch:QStringLiteral("Eidolon 0123456789"))QVERIFY(QFontMetrics(shell.font()).inFont(ch));
        const auto bitmap=shell.grab();
        QCOMPARE(bitmap.devicePixelRatio(),shell.devicePixelRatioF());
        QCOMPARE(bitmap.width(),qRound(shell.width()*shell.devicePixelRatioF()));
        QCOMPARE(bitmap.height(),qRound(shell.height()*shell.devicePixelRatioF()));
        if(!qEnvironmentVariable("TORA_UI_CAPTURES").isEmpty()) {
            const auto folder=qEnvironmentVariable("TORA_UI_CAPTURES")+QStringLiteral("/display-changes");
            QVERIFY(QDir().mkpath(folder));
            auto route=shell.currentScreen();route.replace('/', '-');
            QVERIFY(bitmap.save(folder+QStringLiteral("/%1-%2-%3.png").arg(route).arg(dpi).arg(stage)));
        }
    }
private slots:
    void initTestCase() {
        QCOMPARE(QGuiApplication::platformName(),QStringLiteral("offscreen"));
        native_=QGuiApplication::platformNativeInterface();QVERIFY(native_);
        set_=reinterpret_cast<SetConfiguration>(native_->nativeResourceForIntegration("setConfiguration"));
        const auto get=reinterpret_cast<Configuration>(native_->nativeResourceForIntegration("configuration"));
        QVERIFY(set_ && get);original_=get(native_);
    }
    void cleanup() {if(set_) {set_(original_,native_);QApplication::processEvents();}}
    void transitions_data() {
        QTest::addColumn<QString>("route");QTest::addColumn<int>("dpi");
        for(const auto& route:{QStringLiteral("lp/input"),QStringLiteral("lp/simplex"),QStringLiteral("lp/graphical"),QStringLiteral("lp/final")})
            for(int dpi:{120,144,192})QTest::newRow(qPrintable(route+QString::number(dpi)))<<route<<dpi;
    }
    void transitions() {
        QFETCH(QString,route);QFETCH(int,dpi);
        auto primary=monitor(QStringLiteral("Primary"),0,1920,1080,96);
        auto secondary=monitor(QStringLiteral("Secondary"),2500,3840,2160,dpi);
        configure({primary,secondary});QCOMPARE(QGuiApplication::screens().size(),2);
        ApplicationShell shell;QVERIFY(shell.registerModule(lpModule()));shell.show();
        auto& input=inputController(shell);input.dialogs().askSave=[] {return SaveChoice::No;};
        QVERIFY(input.openFile(QStringLiteral(TORA_CODEC_GOLDENS)+QStringLiteral("/01_normal.txt")));
        const auto bytes=tora::compat::ToraLpCodec::encode(*input.document());
        if(route==QStringLiteral("lp/simplex")) {
            auto& c=simplexController(shell);c.notice=[](const QString&,const QString&){};
            QVERIFY(c.requestAllSlack());QVERIFY(c.start({}));
        } else if(route==QStringLiteral("lp/graphical")) {
            auto& c=graphicalController(shell);c.notice=[](const QString&,const QString&){};
            QVERIFY(c.request());QVERIFY(c.start({}));c.selectRow(7);
        } else if(route==QStringLiteral("lp/final")) {
            auto& c=finalController(shell);c.notice=[](const QString&,const QString&){};
            QVERIFY(c.request());QVERIFY(c.start({}));
        } else QVERIFY(shell.navigate(route));
        const auto history=route==QStringLiteral("lp/simplex")?simplexController(shell).session()->history().size():std::size_t(0);
        shell.resize(1000,650);shell.windowHandle()->setScreen(screen(QStringLiteral("Primary")));
        shell.move(20,40);QApplication::processEvents();QCOMPARE(shell.devicePixelRatioF(),qreal(1));
        checkView(shell,QStringLiteral("primary"),dpi);
        QSignalSpy changed(&shell,&ApplicationShell::displayMetricsChanged);
        auto* second=screen(QStringLiteral("Secondary"));QVERIFY(second);
        shell.windowHandle()->setScreen(second);shell.move(second->availableGeometry().topLeft()+QPoint(30,30));
        QTRY_COMPARE(shell.windowHandle()->screen(),second);
        QTRY_COMPARE(shell.devicePixelRatioF(),qreal(dpi)/96);QVERIFY(changed.count()>0);
        checkView(shell,QStringLiteral("secondary"),dpi);
        changed.clear();secondary["logicalDpi"]=96;configure({primary,secondary});
        QTRY_COMPARE(shell.devicePixelRatioF(),qreal(1));QTRY_VERIFY(changed.count()>0);
        checkView(shell,QStringLiteral("live-scale-change"),dpi);
        primary["width"]=1366;primary["height"]=768;configure({primary});
        QTRY_COMPARE(QGuiApplication::screens().size(),1);
        QTRY_COMPARE(shell.windowHandle()->screen(),screen(QStringLiteral("Primary")));
        QTRY_VERIFY(screen(QStringLiteral("Primary"))->availableGeometry().contains(shell.frameGeometry()));
        checkView(shell,QStringLiteral("secondary-removed"),dpi);
        QCOMPARE(shell.currentScreen(),route);
        QCOMPARE(tora::compat::ToraLpCodec::encode(*input.document()),bytes);
        if(route==QStringLiteral("lp/simplex"))QCOMPARE(simplexController(shell).session()->history().size(),history);
    }
};
int main(int argc,char** argv) {
#ifdef Q_OS_WIN
    qputenv("QT_QPA_FONTDIR",qgetenv("SystemRoot")+"/Fonts");
#endif
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc,argv);DisplayChanges tests;return QTest::qExec(&tests,argc,argv);
}
#include "desktop_display_changes.moc"
