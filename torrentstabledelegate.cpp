#include "torrentstabledelegate.h"

void TorrentsTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    if(index.column() == TorrentsTableColumns::TorrentsTableCompleted){
        QRect rect = option.rect;
        float progress = index.data().toFloat()*100;
        int width = floor(index.data().toFloat() * option.rect.width());
        QString progressStr = QString::number(progress).left(4);

        if(progressStr[progressStr.size()-1] == '.'){
            progressStr.left(3);
        }

        rect.setWidth(width);

        painter->fillRect(option.rect, Qt::gray);
        painter->fillRect(rect, Qt::green);

        painter->setPen(Qt::blue);
        painter->drawText(option.rect, Qt::AlignCenter, progressStr + "%");
    }else{
        QStyledItemDelegate::paint(painter, option, index);
    }
}
