#pragma once

#include <QPersistentModelIndex>
#include <QPointer>
#include <QStyledItemDelegate>

class QAbstractItemView;

namespace tora::desktop {

enum class CellMove { Down, Up, Next, Previous, Left, Right };
QModelIndex spreadsheetNeighbor(const QAbstractItemView& view, const QModelIndex& from, CellMove move);

// Specialized input delegates keep their own validators and controller commits.
// This base supplies the same navigation without bypassing those commits.
class SpreadsheetDelegate : public QStyledItemDelegate {
public:
    explicit SpreadsheetDelegate(QAbstractItemView* view);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;
    void destroyEditor(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;
    bool eventFilter(QObject* object, QEvent* event) override;
    virtual bool writeCell(QAbstractItemModel* model, const QModelIndex& index,
                           const QVariant& value) const;

protected:
    virtual bool commitEditor(QWidget* editor, const QModelIndex& index) const;

private:
    bool finish(QWidget* editor, CellMove move);
    QAbstractItemView* view_;
    mutable QPointer<QWidget> editor_;
    mutable QPersistentModelIndex index_;
    bool finishing_ = false;
};

void installSpreadsheetNavigation(QWidget& content);
void installEnterActivation(QWidget& window);

} // namespace tora::desktop
