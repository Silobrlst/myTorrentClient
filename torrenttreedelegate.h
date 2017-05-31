#ifndef TORRENTTREEDELEGATE_H
#define TORRENTTREEDELEGATE_H

#include <QStyledItemDelegate>
#include <qlineedit.h>
#include <qpainter.h>
#include <QStylePainter>
#include <QComboBox>
#include <QLabel>
#include <qdebug.h>

class TorrentTreeDelegate: public QStyledItemDelegate{
    Q_OBJECT

public:
    TorrentTreeDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override{
        if(index.column() == 2){
            if (option.state & QStyle::State_Selected)
            {
                painter->fillRect(option.rect, option.palette.highlight());
                // Whitee pen while selection
                painter->setPen(Qt::white);
                painter->setBrush(option.palette.highlightedText());
            }
            else
            {
                painter->setPen(QPen(option.palette.foreground(), 0));
                painter->setBrush(qvariant_cast<QBrush>(index.data(Qt::ForegroundRole)));
            }

            painter->drawText(option.rect, comboBoxList()[index.data().toInt()]);
        }else{
            QStyledItemDelegate::paint(painter, option, index);
        }
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override{
        if (index.column() == 2) {
            QComboBox editor;
            editor.addItems(comboBoxList());
            editor.setCurrentIndex(index.data().toInt());
            return editor.sizeHint();
        } else {
            return QStyledItemDelegate::sizeHint(option, index);
        }
    }

    //запустить виджет для редактирования
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override{
        if (index.column() == 2) {
            QComboBox *editor = new QComboBox(parent);
            editor->addItems(comboBoxList());
            editor->setCurrentIndex(index.data().toInt());
            connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(commitAndCloseEditor(int)));
            return editor;
        } else {
            return 0;
        }
    }

    //занести в виджет редактирования данные, чтобы отредактировать их
    void setEditorData(QWidget *editorIn, const QModelIndex &index) const override{
        if (index.column() == 2) {
            QString text = qvariant_cast<QString>(index.data());
            QComboBox *editor = qobject_cast<QComboBox *>(editorIn);
            editor->setCurrentIndex(index.data().toInt());
        }
    }

    //сохранить отредактированные данные в модель
    void setModelData(QWidget *editorIn, QAbstractItemModel *model, const QModelIndex &index) const override{
        if (index.column() == 2) {
            QComboBox *editor = qobject_cast<QComboBox *>(editorIn);
            model->setData(index, QVariant::fromValue(editor->currentIndex()));
        }
    }

    QStringList comboBoxList() const {
        QStringList list;
        list<<"не загружать";
        list<<"обычный";
        list<<"высокий";
        list<<"максимальный";
        return list;
    }

private slots:
    void commitAndCloseEditor(int){
        QLineEdit *editor = qobject_cast<QLineEdit *>(sender());
        emit commitData(editor);
        emit closeEditor(editor);
    }
};

#endif // TORRENTTREEDELEGATE_H

