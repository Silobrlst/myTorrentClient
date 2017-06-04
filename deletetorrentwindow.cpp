#include "deletetorrentwindow.h"
#include "ui_deletetorrentwindow.h"

DeleteTorrentWindow::DeleteTorrentWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeleteTorrentWindow)
{
    ui->setupUi(this);
}

DeleteTorrentWindow::~DeleteTorrentWindow()
{
    delete ui;
}

bool DeleteTorrentWindow::deleteFiles(){
    return ui->deleteFilesCheckBox->isChecked();
}
