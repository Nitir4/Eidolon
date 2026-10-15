#include "keyboard_navigation.hpp"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMetaProperty>
#include <QPushButton>
#include <QScopedValueRollback>
#include <QTableView>
#include <QToolButton>
#include <QToolTip>
#include <algorithm>

namespace tora::desktop {
namespace {
bool editable(const QAbstractItemView& view, const QModelIndex& index)
{
    const auto* table = qobject_cast<const QTableView*>(&view);
    return index.isValid() && (!table || (!table->isRowHidden(index.row()) && !table->isColumnHidden(index.column()))) && view.isEnabled()
        && view.editTriggers() != QAbstractItemView::NoEditTriggers
        && index.flags().testFlag(Qt::ItemIsEnabled)
        && index.flags().testFlag(Qt::ItemIsEditable);
}

void feedback(QAbstractItemView& view, const QString& message)
{
    view.setProperty("spreadsheetFeedback", message);
    if (!message.isEmpty())
        QToolTip::showText(view.mapToGlobal(QPoint(0, 0)), message, &view);
}

class SpreadsheetNavigation final : public QObject {
public:
    explicit SpreadsheetNavigation(QAbstractItemView* view) : QObject(view), view_(view)
    {
        view->installEventFilter(this);
    }

    bool eventFilter(QObject* object, QEvent* event) override
    {
        if (object != view_ || !view_->model()) return false;
        if (event->type() == QEvent::ShortcutOverride) {
            auto* key = static_cast<QKeyEvent*>(event);
            if (key->matches(QKeySequence::Copy) || key->matches(QKeySequence::Paste)) { event->accept(); return true; }
        }
        if (event->type() != QEvent::KeyPress) return false;
        auto* key = static_cast<QKeyEvent*>(event);
        if (key->matches(QKeySequence::Copy)) { copy(); return true; }
        if (key->matches(QKeySequence::Paste)) { paste(); return true; }
        if (key->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) return false;
        const auto current = view_->currentIndex();
        if (key->key() == Qt::Key_F2 && editable(*view_, current)) {
            view_->setProperty("spreadsheetEditInPlace", true);
            view_->edit(current);
            view_->setProperty("spreadsheetEditInPlace", false);
            if (auto* line = qobject_cast<QLineEdit*>(QApplication::focusWidget())) {
                line->deselect(); line->setCursorPosition(line->text().size());
            }
            return true;
        }
        CellMove move;
        switch (key->key()) {
        case Qt::Key_Return: case Qt::Key_Enter:
            move = key->modifiers().testFlag(Qt::ShiftModifier) ? CellMove::Up : CellMove::Down; break;
        case Qt::Key_Tab: move = key->modifiers().testFlag(Qt::ShiftModifier) ? CellMove::Previous : CellMove::Next; break;
        case Qt::Key_Backtab: move = CellMove::Previous; break;
        case Qt::Key_Up: move = CellMove::Up; break;
        case Qt::Key_Down: move = CellMove::Down; break;
        case Qt::Key_Left: move = CellMove::Left; break;
        case Qt::Key_Right: move = CellMove::Right; break;
        default: return false;
        }
        // Output tables retain their native selection/activation behavior.
        if (!editable(*view_, current)) return false;
        if ((key->modifiers() & Qt::ShiftModifier) && key->key() >= Qt::Key_Left && key->key() <= Qt::Key_Down) return false;
        const auto next = spreadsheetNeighbor(*view_, current, move);
        view_->setCurrentIndex(next);
        view_->scrollTo(next);
        return true;
    }

private:
    void copy()
    {
        auto selected = view_->selectionModel()->selectedIndexes();
        if (selected.isEmpty() && view_->currentIndex().isValid()) selected.append(view_->currentIndex());
        if (selected.isEmpty()) return;
        int top = selected.front().row(), bottom = top, left = selected.front().column(), right = left;
        for (const auto& index : selected) {
            top = std::min(top, index.row()); bottom = std::max(bottom, index.row());
            left = std::min(left, index.column()); right = std::max(right, index.column());
        }
        QStringList rows;
        for (int row = top; row <= bottom; ++row) {
            QStringList cells;
            for (int column = left; column <= right; ++column)
                {
                const auto index = view_->model()->index(row, column);
                const auto value = index.data(Qt::EditRole);
                cells.append((value.isValid() ? value : index.data()).toString());
            }
            rows.append(cells.join('\t'));
        }
        QApplication::clipboard()->setText(rows.join('\n'));
    }

    void paste()
    {
        auto* delegate = dynamic_cast<SpreadsheetDelegate*>(view_->itemDelegate());
        const auto start = view_->currentIndex();
        QString text = QApplication::clipboard()->text();
        if (!delegate || !editable(*view_, start) || text.isEmpty()) return;
        if (text.size() > 4 * 1024 * 1024) { feedback(*view_, "Clipboard data exceeds 4 MiB."); return; }
        text.replace("\r\n", "\n"); text.replace('\r', '\n');
        auto rows = text.split('\n');
        if (rows.size() > 1 && rows.back().isEmpty()) rows.removeLast();
        QList<QStringList> cells;
        int columns = -1;
        for (const auto& row : rows) {
            cells.append(row.split('\t'));
            if (columns < 0) columns = static_cast<int>(cells.back().size());
            if (cells.back().size() != columns) { feedback(*view_, "Paste a rectangular block of cells."); return; }
        }
        auto* model = view_->model();
        if (start.row() + cells.size() > model->rowCount() || start.column() + columns > model->columnCount()) {
            feedback(*view_, "The pasted block does not fit in this table."); return;
        }
        struct Change { QPersistentModelIndex index; QVariant before; QString value; };
        QList<Change> changes;
        for (int row = 0; row < cells.size(); ++row) for (int column = 0; column < columns; ++column) {
            const auto index = model->index(start.row() + row, start.column() + column);
            if (!editable(*view_, index)) { feedback(*view_, "The pasted block includes a label or read-only cell."); return; }
            changes.append({index, index.data(Qt::EditRole), cells[row][column]});
        }
        int applied = 0;
        for (const auto& change : changes) {
            if (!change.index.isValid() || !delegate->writeCell(model, change.index, change.value)) {
                for (int undo = applied - 1; undo >= 0; --undo)
                    if (changes[undo].index.isValid()) delegate->writeCell(model, changes[undo].index, changes[undo].before);
                feedback(*view_, "The pasted data was rejected. Correct the value and try again."); return;
            }
            ++applied;
        }
        feedback(*view_, {});
        view_->setCurrentIndex(model->index(start.row() + static_cast<int>(cells.size()) - 1,
                                           start.column() + columns - 1));
        view_->scrollTo(view_->currentIndex());
    }
    QAbstractItemView* view_;
};

class EnterActivation final : public QObject {
public:
    explicit EnterActivation(QWidget* window) : QObject(window), window_(window)
    { qApp->installEventFilter(this); }
    bool eventFilter(QObject* object, QEvent* event) override
    {
        if (event->type() != QEvent::KeyPress) return false;
        auto* widget = qobject_cast<QWidget*>(object);
        if (!widget || widget->window() != window_ || !widget->isEnabled()) return false;
        auto* key = static_cast<QKeyEvent*>(event);
        if ((key->key() != Qt::Key_Return && key->key() != Qt::Key_Enter)
            || (key->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) return false;
        if (auto* button = qobject_cast<QAbstractButton*>(widget)) {
            if (!key->isAutoRepeat()) {
                if (auto* tool = qobject_cast<QToolButton*>(button); tool && tool->menu()) tool->showMenu();
                else if (auto* push = qobject_cast<QPushButton*>(button); push && push->menu()) push->showMenu();
                else button->click();
            }
            return true;
        }
        if (auto* combo = qobject_cast<QComboBox*>(widget); combo && !combo->isEditable()) {
            if (!key->isAutoRepeat()) combo->showPopup();
            return true;
        }
        return false;
    }
private:
    QWidget* window_;
};
}

QModelIndex spreadsheetNeighbor(const QAbstractItemView& view, const QModelIndex& from, CellMove move)
{
    const auto* model = view.model();
    if (!model || !from.isValid()) return from;
    int row = from.row(), column = from.column();
    if (move == CellMove::Next || move == CellMove::Previous) {
        const int step = move == CellMove::Next ? 1 : -1;
        for (int at = row * model->columnCount() + column + step;
             at >= 0 && at < model->rowCount() * model->columnCount(); at += step) {
            const auto next = model->index(at / model->columnCount(), at % model->columnCount());
            if (editable(view, next)) return next;
        }
    } else {
        const int dr = move == CellMove::Down ? 1 : move == CellMove::Up ? -1 : 0;
        const int dc = move == CellMove::Right ? 1 : move == CellMove::Left ? -1 : 0;
        for (row += dr, column += dc; row >= 0 && row < model->rowCount()
             && column >= 0 && column < model->columnCount(); row += dr, column += dc) {
            const auto next = model->index(row, column);
            if (editable(view, next)) return next;
        }
    }
    return from;
}

SpreadsheetDelegate::SpreadsheetDelegate(QAbstractItemView* view) : QStyledItemDelegate(view), view_(view) {}
QWidget* SpreadsheetDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const
{
    auto* editor = QStyledItemDelegate::createEditor(parent, option, index);
    editor_ = editor; index_ = index;
    if (editor) editor->setProperty("spreadsheetEditInPlace", view_->property("spreadsheetEditInPlace"));
    return editor;
}
void SpreadsheetDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
    if (editor_ == editor) editor_.clear();
    QStyledItemDelegate::destroyEditor(editor, index);
}
bool SpreadsheetDelegate::writeCell(QAbstractItemModel* model, const QModelIndex& index, const QVariant& value) const
{ return model->setData(index, value, Qt::EditRole); }
bool SpreadsheetDelegate::commitEditor(QWidget* editor, const QModelIndex& index) const
{
    if (!index.isValid()) return false;
    if (const auto* line = qobject_cast<QLineEdit*>(editor); line && !line->hasAcceptableInput()) return false;
    const auto property = editor->metaObject()->userProperty();
    return property.isValid() && writeCell(view_->model(), index, property.read(editor));
}
void SpreadsheetDelegate::setModelData(QWidget* editor, QAbstractItemModel*, const QModelIndex& index) const
{ commitEditor(editor, index); }
bool SpreadsheetDelegate::finish(QWidget* editor, CellMove move)
{
    if (finishing_) return true;
    const QScopedValueRollback<bool> finishing(finishing_, true);
    const int row = index_.row(), column = index_.column();
    QPointer<QWidget> alive(editor);
    if (!commitEditor(editor, index_)) return true;
    if (alive) emit closeEditor(editor, QAbstractItemDelegate::NoHint);
    const auto origin = view_->model()->index(row, column);
    const auto next = spreadsheetNeighbor(*view_, origin, move);
    view_->setCurrentIndex(next); view_->scrollTo(next); view_->setFocus();
    return true;
}
bool SpreadsheetDelegate::eventFilter(QObject* object, QEvent* event)
{
    if (finishing_) return true;
    if (object == editor_ && event->type() == QEvent::KeyPress) {
        auto* key = static_cast<QKeyEvent*>(event);
        auto* editor = qobject_cast<QWidget*>(object);
        if (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter)
            return finish(editor, key->modifiers().testFlag(Qt::ShiftModifier) ? CellMove::Up : CellMove::Down);
        if (key->key() == Qt::Key_Tab || key->key() == Qt::Key_Backtab)
            return finish(editor, key->key() == Qt::Key_Backtab || key->modifiers().testFlag(Qt::ShiftModifier)
                          ? CellMove::Previous : CellMove::Next);
        if (key->modifiers() == Qt::NoModifier && !editor->property("spreadsheetEditInPlace").toBool()) {
            auto* line = qobject_cast<QLineEdit*>(editor);
            if (key->key() == Qt::Key_Up) return finish(editor, CellMove::Up);
            if (key->key() == Qt::Key_Down) return finish(editor, CellMove::Down);
            if (line && key->key() == Qt::Key_Left && (line->hasSelectedText() || line->cursorPosition() == 0))
                return finish(editor, CellMove::Left);
            if (line && key->key() == Qt::Key_Right && (line->hasSelectedText() || line->cursorPosition() == line->text().size()))
                return finish(editor, CellMove::Right);
        }
    }
    return QStyledItemDelegate::eventFilter(object, event);
}

void installSpreadsheetNavigation(QWidget& content)
{
    for (auto* view : content.findChildren<QAbstractItemView*>()) {
        // Combo-box popups and trees keep their native item behavior.
        if (!qobject_cast<QTableView*>(view) || view->property("spreadsheetInstalled").toBool()) continue;
        view->setProperty("spreadsheetInstalled", true);
        if (view->editTriggers() != QAbstractItemView::NoEditTriggers)
            view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
        if (!dynamic_cast<SpreadsheetDelegate*>(view->itemDelegate())) {
            auto* old = view->itemDelegate();
            view->setItemDelegate(new SpreadsheetDelegate(view));
            if (old->parent() == view) old->deleteLater();
        }
        new SpreadsheetNavigation(view);
    }
}
void installEnterActivation(QWidget& window) { new EnterActivation(&window); }
} // namespace tora::desktop
