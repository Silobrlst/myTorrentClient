#ifndef DELETETORRENTWINDOW_H
#define DELETETORRENTWINDOW_H

#include <QDialog>

namespace Ui {
class DeleteTorrentWindow;
}

class DeleteTorrentWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteTorrentWindow(QWidget *parent = 0);
    ~DeleteTorrentWindow();

    bool deleteFiles();

private:
    Ui::DeleteTorrentWindow *ui;
};

#endif // DELETETORRENTWINDOW_H
