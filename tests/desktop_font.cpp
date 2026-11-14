#include "ui_font.hpp"

#include <QFontMetricsF>
#include <QApplication>
#include <QLabel>
#include <QWidget>

#include <iostream>

int main(int argc, char** argv)
{
#ifdef Q_OS_WIN
    // Windows offscreen uses FreeType rather than the native Windows font
    // database, so supply the runner's installed fonts for glyph checks.
    qputenv("QT_QPA_FONTDIR", qgetenv("SystemRoot") + "/Fonts");
#endif
    QApplication application(argc, argv);

    QFont preferred(QStringLiteral("sans-serif"), 16, QFont::Bold);
    if (tora::desktop::usableUiFont(preferred) != preferred) {
        std::cerr << "Valid desktop font preference was changed\n";
        return 1;
    }

    // A valid but tiny desktop font must not make screen controls unreadable.
    QFont tiny(QStringLiteral("sans-serif"), 7);
    QApplication::setFont(tiny);
    QWidget readableWindow;
    tora::desktop::polishUsableFonts(readableWindow);
    if (readableWindow.font().pointSizeF() < 12) {
        std::cerr << "Screen font is below the readable point-size minimum\n";
        return 1;
    }
    QApplication::setFont(preferred);

    // Attributes observed in the user's malformed qt6ct font configuration.
    QFont malformed = preferred;
    malformed.setPixelSize(167828479);
    malformed.setStyleHint(static_cast<QFont::StyleHint>(255));
    malformed.setStyleStrategy(static_cast<QFont::StyleStrategy>(254));
    malformed.setUnderline(true);
    const QFont recovered = tora::desktop::usableUiFont(malformed);
    const QFontMetricsF metrics(recovered);
    if (recovered.pointSizeF() <= 0 || recovered.underline()
        || metrics.height() <= 0 || metrics.height() > 1000) {
        std::cerr << "Malformed theme font was not recovered\n";
        return 1;
    }
    for (const QChar character : QStringLiteral("Eidolon")) {
        if (!metrics.inFont(character)) {
            std::cerr << "Fallback font cannot render label character U+"
                      << std::hex << static_cast<unsigned int>(character.unicode()) << '\n';
            return 1;
        }
    }
    QWidget window;
    window.setFont(recovered);
    QLabel label(QStringLiteral("Eidolon"), &window);
    QApplication::setFont(malformed);
    window.ensurePolished();
    application.processEvents();
    if (label.font().styleHint() != QFont::SansSerif
        || label.font().underline() || label.font().pixelSize() != -1) {
        std::cerr << "Theme reapplied its malformed font to the label\n";
        return 1;
    }
    std::cout << "Desktop font preference and malformed-setting recovery passed\n";
}
