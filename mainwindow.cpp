#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "torrentstabledelegate.h"
#include <QDateTime>
#include "deletetorrentwindow.h"


using namespace libtorrent;
namespace lt = libtorrent;
using clk = std::chrono::steady_clock;


QJsonObject MainWindow::torrentToJSON(QString fileNameIn, torrent_handle torrentIn){
    QJsonObject json;
    QJsonArray priotities;

    std::vector<int> priotitiesVec = torrentIn.file_priorities();
    for(int i=0; i<priotitiesVec.size(); i++){
        priotities.append(priotitiesVec[i]);
    }

    QDateTime now = QDateTime::currentDateTime();

    QRegExp re("/|\\\\");
    QString fileName = fileNameIn.split(re).last();

    json["path"] = torrentIn.save_path().c_str();
    json["torrentFile"] = fileName;
    json["priotities"] = priotities;
    json["date"] = now.toString(Qt::SystemLocaleShortDate);
    json["isPaused"] = torrentIn.is_paused();

    return json;
}

void MainWindow::saveConfig(){
    QJsonObject json;
    QJsonObject myTorrentClientConfig;

    myTorrentClientConfig["torrentsConfig"] = torrentsConfig;
    json["myTorrentClientConfig"] = myTorrentClientConfig;

    saveJSON("torrents.json", json);
}

void MainWindow::loadConfig(){
    QJsonObject json = loadJSON("settings.json");
    torrentFilesPath = json["torrentFilesPath"].toString();

    json = loadJSON("torrents.json");
    QJsonObject myTorrentClientConfig = json["myTorrentClientConfig"].toObject();
    torrentsConfig = myTorrentClientConfig["torrentsConfig"].toArray();

    for(int i=0; i<torrentsConfig.size(); i++){
        QJsonObject t = torrentsConfig[i].toObject();
        QString filePath = t["path"].toString()+ "/" + t["torrentFile"].toString();

        torrentFromJSON(filePath.toStdString(), t);
    }
}

void MainWindow::torrentFromJSON(std::string torrentFileNameIn, QJsonObject jsonIn){
    add_torrent_params p;
    error_code ec;
    p.ti = boost::make_shared<torrent_info>(torrentFileNameIn, boost::ref(ec), 0);
    if (ec)
    {
        fprintf(stderr, "%s\n", ec.message().c_str());
        return;
    }

    p.save_path = jsonIn["path"].toString().toStdString();

    QJsonArray priorities = jsonIn["priorities"].toArray();

    for(int i=0; i<priorities.size(); i++){
        p.file_priorities.push_back(priorities[i].toInt());
    }

    p.paused = jsonIn["isPaused"].toBool();

    loadTorrent(torrentFileNameIn, p);
}

void MainWindow::torrentFromAddTorrentWindow(std::string torrentFileNameIn, AddTorrentWindow *addTorrentWindowIn){
    add_torrent_params p;
    error_code ec;
    p.ti = boost::make_shared<torrent_info>(torrentFileNameIn, boost::ref(ec), 0);
    if (ec)
    {
        fprintf(stderr, "%s\n", ec.message().c_str());
        return;
    }

    p.save_path = addTorrentWindowIn->choosedPath.toStdString();
    for(int i=0; i<addTorrentWindowIn->model->rowCount(); i++){
        QString fileStr = addTorrentWindowIn->model->data(addTorrentWindowIn->model->index(i, TorrentContenTreeColumns::TorrentContenTreeName)).toString();

        if(fileStr[fileStr.size()-1] != '/'){
            p.file_priorities.push_back(addTorrentWindowIn->model->data(addTorrentWindowIn->model->index(i, TorrentContenTreeColumns::TorrentContenTreePriority)).toInt());
        }
    }

    torrentsConfig.append((torrentToJSON(torrentFileNameIn.c_str(), loadTorrent(torrentFileNameIn, p))));
    saveConfig();
}

torrent_handle MainWindow::loadTorrent(std::string torrentFileNameIn, add_torrent_params add_torrent_paramsIn){
    error_code ec;
    torrent_handle th = ses->add_torrent(add_torrent_paramsIn, ec);
    if (ec)
    {
        fprintf(stderr, "%s\n", ec.message().c_str());
    }else{
        QString hexHashStr = hexHash(th.info_hash());
        int rowId = torrentsMapRows.size();
        torrentsMapRows[hexHashStr] = rowId;
        torrentsMapRows[hexHashStr] = rowId;
        torrentsMapHandles[hexHashStr] = th;

        QList<QStandardItem*> row;
        row.append(new QStandardItem(th.name().c_str()));
        row.append(new QStandardItem(unitBytes(th.get_torrent_info().total_size())));
        torrentsTableModel->insertRow(rowId, row);

        QRegExp re("/|\\\\");
        QString fileName = QString(torrentFileNameIn.c_str()).split(re).last();
        QFile::copy(torrentFileNameIn.c_str(), torrentFilesPath + "/" + fileName);

        groupStatusAll->setText(QString("все (") + QString::number(torrentsTableModel->rowCount()) + ")");
    }

    return th;
}

void MainWindow::updateTorrentsTable(){
    clk::time_point last_save_resume = clk::now();

    lt::torrent_handle h;
    std::vector<lt::alert*> alerts;
    ses->pop_alerts(&alerts);

    for (lt::alert const* a : alerts) {
        if (auto at = lt::alert_cast<lt::add_torrent_alert>(a)) {
            h = at->handle;
        }
        // if we receive the finished alert or an error, we're done
        if (lt::alert_cast<lt::torrent_finished_alert>(a)) {
            h.save_resume_data();
        }
        if (lt::alert_cast<lt::torrent_error_alert>(a)) {
            std::cout << a->message() << std::endl;
        }

        // when resume data is ready, save it
        if (auto rd = lt::alert_cast<lt::save_resume_data_alert>(a)) {
            std::ofstream of(".resume_file", std::ios_base::binary);
            of.unsetf(std::ios_base::skipws);
            lt::bencode(std::ostream_iterator<char>(of), *rd->resume_data);
        }
    }

    // ask the session to post a state_update_alert, to update our
    // state output for the torrent
    ses->post_torrent_updates();

    // save resume data once every 30 seconds
    if (clk::now() - last_save_resume > std::chrono::seconds(30)) {
        h.save_resume_data();
        last_save_resume = clk::now();
    }

    QList<torrent_handle> ths = torrentsMapHandles.values();

    int download = 0;
    int completed = 0;
    int stoped = 0;
    int seeding = 0;

    for(int i=0; i<ths.size(); i++){
        if(ths[i].status().paused){
            stoped++;
        }else{
            switch(ths[i].status().state){
            case torrent_status::state_t::downloading:
                download++;
                break;
            case torrent_status::state_t::finished:
                completed++;
                break;
            case torrent_status::state_t::seeding:
                seeding++;
                break;
            case torrent_status::state_t::queued_for_checking:
                break;
            case torrent_status::state_t::checking_files:
                break;
            case torrent_status::state_t::downloading_metadata:
                break;
            case torrent_status::state_t::allocating:
                break;
            case torrent_status::state_t::checking_resume_data:
                break;
            }
        }

        updateTorrent(ths[i]);
    }

    this->groupStatusStoped->setText(QString("остановленные (" + QString::number(stoped ) + ")"));
    this->groupStatusCompleted->setText(QString("завершенные (" + QString::number(completed ) + ")"));
    this->groupStatusDownloading->setText(QString("скачивающиеся (" + QString::number(download ) + ")"));
    this->groupStatusSeeding->setText(QString("раздающиеся (" + QString::number(seeding ) + ")"));
}

void MainWindow::updateTorrent(torrent_status torrent_statusIn){
    int row = rowIndex(torrent_statusIn.info_hash);
    if(row != -1){
        if(torrent_statusIn.paused){
            torrentsTableModel->setData(torrentsTableModel->index(row, TorrentsTableColumns::TorrentsTableStatus), "остановлен");
        }else{
            torrentsTableModel->setData(torrentsTableModel->index(row, TorrentsTableColumns::TorrentsTableStatus), torrentStateName(torrent_statusIn.state));
        }

        torrentsTableModel->setData(torrentsTableModel->index(row, TorrentsTableColumns::TorrentsTablePeers), torrent_statusIn.num_peers);
        torrentsTableModel->setData(torrentsTableModel->index(row, TorrentsTableColumns::TorrentsTableCompleted), torrent_statusIn.progress);
        torrentsTableModel->setData(torrentsTableModel->index(row, TorrentsTableColumns::TorrentsTableSeeds), torrent_statusIn.num_seeds);
        torrentsTableModel->setData(torrentsTableModel->index(row, TorrentsTableColumns::TorrentsTableInputSpeed), unitBytes(torrent_statusIn.download_payload_rate) + "/с");
        torrentsTableModel->setData(torrentsTableModel->index(row, TorrentsTableColumns::TorrentsTableOutputSpeed), unitBytes(torrent_statusIn.upload_payload_rate) + "/с");
    }
}

void MainWindow::updateTorrent(torrent_handle torrent_handleIn){
    updateTorrent(torrent_handleIn.status());
}

void MainWindow::saveUIsettings(){
    QSettings settings("ui.plist", QSettings::NativeFormat);
    settings.setValue("splitterSizes", ui->splitter->saveState());
    settings.setValue("splitter2Sizes", ui->splitter_2->saveState());
    settings.setValue("splitter3Sizes", ui->splitter_3->saveState());
    settings.setValue("mainWindowSize", this->size());

    for(int i=0; i<ui->torrentsTable->model()->columnCount(); i++){
        settings.setValue("MainWindowTreeColumn"+QString::number(i), ui->torrentsTable->columnWidth(i));
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveUIsettings();
}

void MainWindow::openSettingsWindow(){
    SettingsWindow *settingsWindow = new SettingsWindow(this);
    settingsWindow->show();
}

void MainWindow::openAddTorrentWindow(){
    QSettings settings("ui.plist", QSettings::NativeFormat);

    QFileDialog dialog(this);
    dialog.setDirectory(settings.value("openTorrentDirectory").toString());
    dialog.setNameFilter(tr("торрент файлы (*.torrent);;любые (*.*)"));
    int result = dialog.exec();

    if(result == QDialog::Accepted){
        QStringList fileNames = dialog.selectedFiles();
        settings.setValue("openTorrentDirectory", dialog.directory().absolutePath());

        int existRow = checkTorrentExist(fileNames[0]);

        if(existRow != -1){
            ui->torrentsTable->selectRow(existRow);

            QMessageBox msgBox;
            msgBox.setText("торрент уже добавлен");
            msgBox.exec();
        }else{
            AddTorrentWindow *addTorrentWindow = new AddTorrentWindow(fileNames[0]);
            if(addTorrentWindow->exec() == QDialog::Accepted){
                QString fileName = fileNames[0];
                torrentFromAddTorrentWindow(fileName.toStdString(), addTorrentWindow);
            }
        }
    }
}

QString MainWindow::hexHash(sha1_hash hashIn){
    char ih[41];
    to_hex((char const*)&hashIn[0], 20, ih);
    return ih;
}

//взвращает номер строки соответствующий хешу торрента если он есть в таблице, иначе -1
int MainWindow::rowIndex(sha1_hash hashIn){
    QString hexHashStr = hexHash(hashIn);
    if(torrentsMapRows.contains(hexHashStr)){
        return torrentsMapRows[hexHashStr];
    }

    return -1;
}

//возвращает номер строки в таблице торрентов если найден, иначе -1
int MainWindow::checkTorrentExist(QString fileName){
    int item_limit = 1000000;
    int depth_limit = 1000;

    std::vector<char> buf;
    error_code ec;
    int ret = load_file(fileName.toStdString(), buf, ec, 40 * 1000000);
    if (ret == -1)
    {
        fprintf(stderr, "file too big, aborting\n");
        return 1;
    }

    if (ret != 0)
    {
        fprintf(stderr, "failed to load file: %s\n", ec.message().c_str());
        return 1;
    }
    bdecode_node e;
    int pos = -1;
    ret = bdecode(&buf[0], &buf[0] + buf.size(), e, ec, &pos, depth_limit, item_limit);

    if (ret != 0)
    {
        fprintf(stderr, "failed to decode: '%s' at character: %d\n", ec.message().c_str(), pos);
        return 1;
    }

    torrent_info t(e, ec);
    if (ec)
    {
        fprintf(stderr, "%s\n", ec.message().c_str());
        return 1;
    }
    e.clear();
    std::vector<char>().swap(buf);

    return rowIndex(t.info_hash());
}

void MainWindow::selectedRow(QModelIndex indexIn){
    if(torrentsMapHandles.contains(torrentsMapRows.key(indexIn.row()))){
        torrent_handle th = torrentsMapHandles[torrentsMapRows.key(indexIn.row())];

        ui->torrentPath->setText(th.save_path().c_str());
        ui->torrentCreated->setText(torrentsConfig[indexIn.row()].toObject()["date"].toString());
        ui->torrentHash->setText(hexHash(th.info_hash()));
        ui->torrentPieceSize->setText(unitBytes(th.get_torrent_info().piece_length()));
        ui->torrentComment->setText(th.get_torrent_info().comment().c_str());
    }
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    settings_pack sett;
    sett.set_int(lt::settings_pack::alert_mask, lt::alert::error_notification | lt::alert::storage_notification | lt::alert::status_notification);
    sett.set_str(settings_pack::listen_interfaces, "0.0.0.0:6881");
    ses = new lt::session(sett);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTorrentsTable()));
    timer->start(1000);

    ui->setupUi(this);

    QObject::connect(ui->menuSettings, SIGNAL(triggered()), this, SLOT(openSettingsWindow()));
    QObject::connect(ui->menuOpen, SIGNAL(triggered()), this, SLOT(openAddTorrentWindow()));
    QObject::connect(ui->torrentsTable, SIGNAL(clicked(QModelIndex)), this, SLOT(selectedRow(QModelIndex)));

    torrentsTableModel = new QStandardItemModel(this);

    QStringList tableHead;
    tableHead<<"имя";
    tableHead<<"размер";
    tableHead<<"прогресс";
    tableHead<<"статус";
    tableHead<<"сиды";
    tableHead<<"пиры";
    tableHead<<"прием";
    tableHead<<"отдача";
    tableHead<<"до завершения";
    tableHead<<"коэфф.";
    tableHead<<"добавлен";
    torrentsTableModel->setHorizontalHeaderLabels(tableHead);

    ui->torrentsTable->setModel(torrentsTableModel);
    ui->torrentsTable->setItemDelegate(new TorrentsTableDelegate());


    QStandardItem *treeItem;

    QStandardItemModel *torrentStatusGroupsModel = new QStandardItemModel();
    treeItem = new QStandardItem("торренты");
    torrentStatusGroupsModel->appendRow(treeItem);

    groupStatusDownloading = new QStandardItem(QString("скачивающиеся"));
    groupStatusCompleted = new QStandardItem(QString("завершенные"));
    groupStatusStoped = new QStandardItem(QString("остановленные"));
    groupStatusSeeding = new QStandardItem(QString("раздающиеся"));
    groupStatusAll = new QStandardItem(QString("все"));

    treeItem->appendRow(groupStatusDownloading);
    treeItem->appendRow(groupStatusCompleted);
    treeItem->appendRow(groupStatusStoped);
    treeItem->appendRow(groupStatusSeeding);
    treeItem->appendRow(groupStatusAll);

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

    for(int i=0; i<ui->torrentsTable->model()->columnCount(); i++){
        int width = settings.value("MainWindowTreeColumn"+QString::number(i), 100).toInt();
        ui->torrentsTable->setColumnWidth(i, width);
    }

    torrentTreeModel = new TorrentTreeModel(ui->torrentContentTree);
    ui->torrentContentTree->setModel(torrentTreeModel);
    TorrentTreeDelegate* delegate = new TorrentTreeDelegate(this);
    ui->torrentContentTree->setItemDelegate(delegate);

    this->resize(settings.value("mainWindowSize").toSize());
    trayIcon = new QSystemTrayIcon(QIcon("trayIcon.png"));
    trayIcon->show();

    loadConfig();

    torrentsTableMenu = new QMenu(this);
    connect(ui->torrentsTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(torrentsTableContextMenu(QPoint)));
    connect(torrentsTableMenu->addAction("возобновить"), SIGNAL(triggered()), this, SLOT(resumeTorrents()));
    connect(torrentsTableMenu->addAction("остановить"), SIGNAL(triggered()), this, SLOT(stopTorrents()));
    connect(torrentsTableMenu->addAction("удалить"), SIGNAL(triggered()), this, SLOT(deleteTorrents()));
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        this->show();
        break;
    case QSystemTrayIcon::Context:
        break;
    default:
        ;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::torrentsTableContextMenu(QPoint){
    torrentsTableMenu->exec(QCursor::pos());
}

void MainWindow::deleteTorrent(int rowIn, int option){
    QString hexHashStr = torrentsMapRows.key(rowIn);

    torrentsTableModel->removeRow(torrentsMapRows[hexHashStr]);
    ses->remove_torrent(torrentsMapHandles[hexHashStr], option);

    torrentsMapRows.remove(hexHashStr);
    torrentsMapHandles.remove(hexHashStr);

    torrentsConfig.removeAt(rowIn);
}

void MainWindow::stopTorrent(int rowIn){
    torrent_handle th = torrentsMapHandles[torrentsMapRows.key(rowIn)];
    th.pause();
    torrentsConfig[rowIn].toObject()["isPaused"] = true;
    updateTorrent(th);
}

void MainWindow::resumeTorrent(int rowIn){
    torrent_handle th = torrentsMapHandles[torrentsMapRows.key(rowIn)];
    th.resume();
    torrentsConfig[rowIn].toObject()["isPaused"] = false;
    updateTorrent(th);
}

void MainWindow::deleteTorrents(){
    QModelIndexList rows = ui->torrentsTable->selectionModel()->selectedRows();

    DeleteTorrentWindow *deleteTorrentWindow = new DeleteTorrentWindow(this);

    if(deleteTorrentWindow->exec() == QDialog::Accepted){
        for(int i=0; i<rows.size(); i++){
            if(deleteTorrentWindow->deleteFiles()){
                deleteTorrent(rows[i].row(), session::delete_files);
            }else{
                deleteTorrent(rows[i].row(), 0);
            }
        }
    }

    saveConfig();
}

void MainWindow::openTorrentContentFolder(){

}

void MainWindow::openTorrentContent(){

}

void MainWindow::stopTorrents(){
    QModelIndexList rows = ui->torrentsTable->selectionModel()->selectedRows();

    for(int i=0; i<rows.size(); i++){
        stopTorrent(rows[i].row());
    }

    saveConfig();
}

void MainWindow::resumeTorrents(){
    QModelIndexList rows = ui->torrentsTable->selectionModel()->selectedRows();

    for(int i=0; i<rows.size(); i++){
        resumeTorrent(rows[i].row());
    }

    saveConfig();
}
