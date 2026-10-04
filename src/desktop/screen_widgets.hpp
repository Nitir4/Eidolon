#pragma once

#include <QWidget>
#include <functional>

class QVBoxLayout;
class QPushButton;
class QLabel;

namespace tora::desktop {
void blueCanvas(QWidget& widget);
QPushButton* button(const QString& text, const QString& id, QVBoxLayout& layout,
                    const std::function<void()>& clicked);
QLabel* previewNotice(QVBoxLayout& layout);
void centerPanel(QVBoxLayout& layout, QWidget* panel);
} // namespace tora::desktop
