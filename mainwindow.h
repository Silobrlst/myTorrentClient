#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qsplitter.h>
#include <qdebug.h>
#include <QFileInfo>
#include <qmessagebox.h>
#include <qdir.h>

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

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void closeEvent(QCloseEvent *event);

    void saveUIsettings();

public slots:
    void openSettingsWindow();
    void openAddTorrentWindow();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
