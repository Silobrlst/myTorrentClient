#ifndef ADDTORRENTWINDOW_H
#define ADDTORRENTWINDOW_H

#include <QDialog>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QApplication>
#include <qdebug.h>
#include <qregexp.h>


namespace Ui {
class AddTorrentWindow;
}

class AddTorrentWindow : public QDialog
{
    Q_OBJECT

    QStandardItemModel *model;

public:
    explicit AddTorrentWindow(QString fileNameIn, QWidget *parent = 0);

    ~AddTorrentWindow();

    void getTorrentFileInfo(QString fileNameIn);

    void loadJSON();

public slots:
    void openSettingsWindow();
    void saveUIsettings(int r);

private:
    Ui::AddTorrentWindow *ui;
};

#endif // ADDTORRENTWINDOW_H
