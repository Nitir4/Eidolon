#pragma once

#include <QFont>
#include <QString>
#include <QWidget>

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

inline void polishUsableFonts(QWidget& root)
{
    // Style-sheet polishing can select the application/theme font even when
    // the top-level window has an explicit font. Resolve every widget after
    // polishing so later theme merges cannot restore corrupted attributes.
    auto widgets = root.findChildren<QWidget*>();
    widgets.prepend(&root);
    for (auto* widget : widgets) {
        widget->ensurePolished();
        const QFont font = usableUiFont(widget->font());
        if (font != widget->font()) {
            widget->setProperty("toraRecoveredFont", true);
            widget->setFont(font);
        }
    }
}

} // namespace tora::desktop
