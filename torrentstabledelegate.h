#ifndef TORRENTSTABLEDELEGATE_H
#define TORRENTSTABLEDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <qdebug.h>

#include "commons.h"

class TorrentsTableDelegate : public QStyledItemDelegate
{
public:
    TorrentsTableDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // TORRENTSTABLEDELEGATE_H
