#ifndef TORRENTTREEMODEL_H
#define TORRENTTREEMODEL_H

#include <QStandardItemModel>
#include <QTreeView>
#include <qdebug.h>

#include "commons.h"

class TorrentTreeModel: public QStandardItemModel{
public:
    QTreeView *treeView;

    TorrentTreeModel(QTreeView *treeViewIn): QStandardItemModel(0, 3){
        treeView = treeViewIn;

        QStringList tableHead2;
        tableHead2<<"имя";
        tableHead2<<"размер";
        tableHead2<<"приоритет";
        tableHead2<<"прогресс";
        this->setHorizontalHeaderLabels(tableHead2);
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole){
        if(index.column() == TorrentContenTreeColumns::TorrentContenTreePriority){
            prioritizeSelected(value.toInt());
            return true;
        }else{
            emit dataChanged(index, index);
            return QStandardItemModel::setData(index, value, role);
        }
    }

    void prioritizeSelected(int priorityIn){
        QModelIndexList rows = treeView->selectionModel()->selectedRows();

        for(int i=0; i<rows.size(); i++){
            QStandardItemModel::setData(this->index(rows[i].row(), TorrentContenTreeColumns::TorrentContenTreePriority), priorityIn);
        }
    }
};

#endif // TORRENTTREEMODEL_H

