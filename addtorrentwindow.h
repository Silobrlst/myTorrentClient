#ifndef ADDTORRENTWINDOW_H
#define ADDTORRENTWINDOW_H

#include <QDialog>

namespace Ui {
class AddTorrentWindow;
}

class AddTorrentWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AddTorrentWindow(QWidget *parent = 0);
    ~AddTorrentWindow();

private:
    Ui::AddTorrentWindow *ui;
};

#endif // ADDTORRENTWINDOW_H
