#ifndef ADDTORRENTWINDOW_H
#define ADDTORRENTWINDOW_H

#include <QDialog>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QApplication>
#include <qdebug.h>
#include <qregexp.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <QJsonDocument>
#include <qfile.h>
#include <qsettings.h>

#include "torrenttreemodel.h"
#include "torrenttreedelegate.h"
#include "settingswindow.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <map>

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


namespace Ui {
class AddTorrentWindow;
}

class AddTorrentWindow : public QDialog
{
    Q_OBJECT

public:
    TorrentTreeModel *model;
    QString choosedPath;

    QMenu *menu;

    explicit AddTorrentWindow(QString fileNameIn, QWidget *parent = 0);

    ~AddTorrentWindow();

    void getTorrentFileInfo(QString fileNameIn);

    void loadJSON();

    void setPriority();

public slots:
    void openSettingsWindow();
    void saveUIsettings(int r);

    void prioriyNotLoad();
    void prioriyNormal();
    void prioriyHigh();
    void prioriyMaximum();

    void contextMenu(QPoint pointIn);

private:
    Ui::AddTorrentWindow *ui;
};

#endif // ADDTORRENTWINDOW_H
