#pragma once

#include <QFont>
#include <QString>

namespace tora::desktop {

inline QFont usableUiFont(const QFont& preferred)
{
    // Some desktop themes supply malformed serialized QFont values. Validate
    // them before font metrics or layout can consume the corrupted attributes.
    const int hint = static_cast<int>(preferred.styleHint());
    if (hint >= QFont::SansSerif && hint <= QFont::Fantasy
        && (preferred.pointSizeF() > 0 || preferred.pixelSize() > 0)) {
        return preferred;
    }

    // Points scale with monitor DPI. Qt matches this generic family to an
    // installed sans-serif font on each platform.
    QFont fallback(QStringLiteral("sans-serif"), 12, QFont::Normal);
    fallback.setStyleHint(QFont::SansSerif);
    // Keep malformed theme attributes from being merged back when the theme
    // reapplies its application font during widget polishing.
    fallback.setResolveMask(QFont::AllPropertiesResolved);
    return fallback;
}

} // namespace tora::desktop
