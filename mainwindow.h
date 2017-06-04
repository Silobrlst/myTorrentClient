#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qsplitter.h>
#include <qdebug.h>
#include <QFileInfo>
#include <qmessagebox.h>
#include <qdir.h>
#include <qabstractitemmodel.h>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QSettings>
#include <qdebug.h>
#include <QFileDialog>
#include <QTimer>
#include <QSystemTrayIcon>

#include "settingswindow.h"
#include "addtorrentwindow.h"
#include "torrenttreedelegate.h"

#include <stdlib.h>
#include </home/q/Загрузки/boost_1_64_0/boost/make_shared.hpp>
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/settings.hpp"
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_status.hpp>
#include "libtorrent/announce_entry.hpp"
#include "libtorrent/bdecode.hpp"
#include "libtorrent/magnet_uri.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>

using namespace libtorrent;
namespace lt = libtorrent;
using clk = std::chrono::steady_clock;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    lt::session *ses;

    QStandardItemModel *torrentsTableModel;
    TorrentTreeModel *torrentTreeModel;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QJsonArray torrentsConfig;

    QString torrentFilesPath;

    QMap<QString, int> torrentsMapRows; //<infoHash, tableRow>
    QMap<QString, torrent_handle> torrentsMapHandles; //<infoHash, torrent_handle>

    QMenu *torrentsTableMenu;

    QStandardItem *groupStatusDownloading;
    QStandardItem *groupStatusCompleted;
    QStandardItem *groupStatusStoped;
    QStandardItem *groupStatusSeeding;
    QStandardItem *groupStatusAll;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void closeEvent(QCloseEvent *event);

    void saveUIsettings();

    torrent_handle loadTorrent(std::string torrentFileNameIn, add_torrent_params add_torrent_paramsIn);

    void torrentFromJSON(std::string torrentFileNameIn, QJsonObject jsonIn);

    void torrentFromAddTorrentWindow(std::string torrentFileNameIn, AddTorrentWindow *addTorrentWindowIn);

    int checkTorrentExist(QString fileName);

    QJsonObject torrentToJSON(QString fileNameIn, torrent_handle torrentIn);

    void saveConfig();

    void loadConfig();

    int rowIndex(sha1_hash hashIn);

    QString hexHash(sha1_hash hashIn);

    void stopTorrent(int rowIn);

    void resumeTorrent(int rowIn);

    void deleteTorrent(int rowIn, int option=0);

    void updateTorrent(torrent_handle torrent_handleIn);

    void updateTorrent(torrent_status torrent_statusIn);

public slots:
    void openSettingsWindow();
    void openAddTorrentWindow();
    void updateTorrentsTable();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void selectedRow(QModelIndex indexIn);

    void deleteTorrents();
    void openTorrentContentFolder();
    void openTorrentContent();
    void stopTorrents();
    void resumeTorrents();

    void torrentsTableContextMenu(QPoint);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
