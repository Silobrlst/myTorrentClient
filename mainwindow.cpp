#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingswindow.h"
#include "addtorrentwindow.h"
#include "torrenttreedelegate.h"
#include <qabstractitemmodel.h>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QSettings>
#include <qdebug.h>
#include <QFileDialog>



void MainWindow::saveUIsettings(){
    QSettings settings("ui.plist", QSettings::NativeFormat);
    settings.setValue("splitterSizes", ui->splitter->saveState());
    settings.setValue("splitter2Sizes", ui->splitter_2->saveState());
    settings.setValue("splitter3Sizes", ui->splitter_3->saveState());

    settings.setValue("mainWindowSize", this->size());
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveUIsettings();
}

void MainWindow::openSettingsWindow(){
    SettingsWindow *settingsWindow = new SettingsWindow(this);
    settingsWindow->show();
}

void MainWindow::openAddTorrentWindow(){
    QFileDialog dialog(this);
    dialog.setNameFilter(tr("торрент файлы (*.torrent);;любые (*.*)"));
    int result = dialog.exec();

    if(result == QDialog::Accepted){
        QStringList fileNames = dialog.selectedFiles();
        qInfo()<<fileNames[0];

        AddTorrentWindow *addTorrentWindow = new AddTorrentWindow(fileNames[0]);
        addTorrentWindow->show();
    }
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    settings_pack sett;
    sett.set_int(lt::settings_pack::alert_mask, lt::alert::error_notification | lt::alert::storage_notification | lt::alert::status_notification);
    sett.set_str(settings_pack::listen_interfaces, "0.0.0.0:6881");
    ses = new lt::session(sett);

    ui->setupUi(this);

    QObject::connect(ui->menuSettings, SIGNAL(triggered()), this, SLOT(openSettingsWindow()));
    QObject::connect(ui->menuOpen, SIGNAL(triggered()), this, SLOT(openAddTorrentWindow()));

    QStandardItemModel *model = new QStandardItemModel(4, 4);

    QStringList tableHead;
    tableHead<<"имя";
    tableHead<<"размер";
    tableHead<<"завершено";
    tableHead<<"статус";
    tableHead<<"сиды";
    tableHead<<"пиры";
    tableHead<<"прием";
    tableHead<<"отдача";
    tableHead<<"до завершения";
    tableHead<<"коэфф.";
    tableHead<<"добавлен";
    model->setHorizontalHeaderLabels(tableHead);

    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            QStandardItem *item = new QStandardItem(QString("row %0, column %1").arg(row).arg(column));
            model->setItem(row, column, item);
        }
    }

    ui->tableView->setModel(model);


    QStandardItem *treeItem;

    QStandardItemModel *torrentStatusGroupsModel = new QStandardItemModel();
    treeItem = new QStandardItem("торренты");
    torrentStatusGroupsModel->appendRow(treeItem);
    treeItem->appendRow(new QStandardItem(QString("скачивающиеся")));
    treeItem->appendRow(new QStandardItem(QString("завершенные")));
    treeItem->appendRow(new QStandardItem(QString("остановленные")));
    treeItem->appendRow(new QStandardItem(QString("активные")));
    treeItem->appendRow(new QStandardItem(QString("неактивные")));
    treeItem->appendRow(new QStandardItem(QString("все")));

    QStandardItemModel *torrentLabelsGroupsModel = new QStandardItemModel();
    treeItem = new QStandardItem("метки");
    torrentLabelsGroupsModel->appendRow(treeItem);
    treeItem->appendRow(new QStandardItem(QString("все")));
    treeItem->appendRow(new QStandardItem(QString("без метки")));

    ui->torrentStatusGroups->setModel(torrentStatusGroupsModel);
    ui->torrentLabelGroups->setModel(torrentLabelsGroupsModel);

    ui->torrentStatusGroups->expandAll();
    ui->torrentLabelGroups->expandAll();

    QSettings settings("ui.plist", QSettings::NativeFormat);
    ui->splitter->restoreState(settings.value("splitterSizes").toByteArray());
    ui->splitter_2->restoreState(settings.value("splitter2Sizes").toByteArray());
    ui->splitter_3->restoreState(settings.value("splitter3Sizes").toByteArray());


    QStringList tableHead2;
    tableHead2<<"имя";
    tableHead2<<"размер";
    tableHead2<<"приоритет";
    tableHead2<<"прогресс";
    model = new QStandardItemModel(0, 3);
    model->setHorizontalHeaderLabels(tableHead2);
    ui->torrentContentTree->setModel(model);
    TorrentTreeDelegate* delegate = new TorrentTreeDelegate(this);
    ui->torrentContentTree->setItemDelegate(delegate);

    this->resize(settings.value("mainWindowSize").toSize());
}

MainWindow::~MainWindow()
{
    delete ui;
}
