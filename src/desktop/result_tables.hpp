#pragma once

#include <QHeaderView>
#include <QTableWidget>

namespace tora::desktop {
inline void configureResultTable(QTableView& table, const QString& name)
{
    table.setObjectName(name);
    table.setEditTriggers(QAbstractItemView::NoEditTriggers);
    table.setAlternatingRowColors(true);
    table.setWordWrap(false);
    table.setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    table.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
    table.setMinimumHeight(72);
    table.verticalHeader()->setDefaultSectionSize(table.fontMetrics().height() + 6);
    table.horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table.horizontalHeader()->setResizeContentsPrecision(64);
}

inline QTableWidget* resultTable(const QString& name)
{
    auto* table = new QTableWidget;
    configureResultTable(*table, name);
    return table;
}

inline void putResultCell(QTableWidget& table, int row, int column, const QString& text,
                          Qt::Alignment horizontal = Qt::AlignRight)
{
    auto* cell = new QTableWidgetItem(text);
    cell->setToolTip(text);
    cell->setTextAlignment(horizontal | Qt::AlignVCenter);
    table.setItem(row, column, cell);
}
}
