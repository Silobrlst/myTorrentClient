#include "addtorrentwindow.h"
#include "ui_addtorrentwindow.h"
#include "torrenttreedelegate.h"
#include "settingswindow.h"
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <QJsonDocument>
#include <qfile.h>
#include <qsettings.h>

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


using namespace libtorrent;
namespace lt = libtorrent;
using clk = std::chrono::steady_clock;


int load_file(std::string const& filename, std::vector<char>& v, libtorrent::error_code& ec, int limit = 8000000){
    ec.clear();
    FILE* f = fopen(filename.c_str(), "rb");
    if (f == NULL)
    {
        ec.assign(errno, boost::system::system_category());
        return -1;
    }

    int r = fseek(f, 0, SEEK_END);
    if (r != 0)
    {
        ec.assign(errno, boost::system::system_category());
        fclose(f);
        return -1;
    }
    long s = ftell(f);
    if (s < 0)
    {
        ec.assign(errno, boost::system::system_category());
        fclose(f);
        return -1;
    }

    if (s > limit)
    {
        fclose(f);
        return -2;
    }

    r = fseek(f, 0, SEEK_SET);
    if (r != 0)
    {
        ec.assign(errno, boost::system::system_category());
        fclose(f);
        return -1;
    }

    v.resize(s);
    if (s == 0)
    {
        fclose(f);
        return 0;
    }

    r = fread(&v[0], 1, v.size(), f);
    if (r < 0)
    {
        ec.assign(errno, boost::system::system_category());
        fclose(f);
        return -1;
    }

    fclose(f);

    if (r != s) return -3;

    return 0;
}

QString unitBytes(int64_t bytesNumIn){
    QString ret;
    int unitNum = 0;
    float bytes = bytesNumIn;

    while(bytes > 999){
        bytes /= 1024;
        unitNum++;
    }

    ret += QString::number(bytes).left(4);
    if(ret[3] == '.'){
        ret = ret.left(3);
    }

    switch (unitNum) {
    case 0:
        ret += " Б";
        break;
    case 1:
        ret += " кБ";
        break;
    case 2:
        ret += " МБ";
        break;
    case 3:
        ret += " ГБ";
        break;
    case 4:
        ret += " ТБ";
        break;
    default:
        break;
    }

    return ret;
}


void AddTorrentWindow::loadJSON(){
    QFile loadFile("settings.json");

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
        return;
    }

    QByteArray data = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(data));
    QJsonObject json = loadDoc.object();

    QJsonArray paths = json["paths"].toArray();
    for(int i = 0; i < paths.size(); i++){
        ui->saveDirectory->addItem(paths[i].toString());
    }
}

AddTorrentWindow::AddTorrentWindow(QString fileNameIn, QWidget *parent): QDialog(parent), ui(new Ui::AddTorrentWindow){
    ui->setupUi(this);

    model = new QStandardItemModel(0, 3);

    getTorrentFileInfo(fileNameIn);

    QStringList tableHead;
    tableHead<<"имя";
    tableHead<<"размер";
    tableHead<<"приоритет";
    tableHead<<"прогресс";
    model->setHorizontalHeaderLabels(tableHead);

    ui->torrentTree->setModel(model);

    TorrentTreeDelegate* delegate = new TorrentTreeDelegate(this);
    ui->torrentTree->setItemDelegate(delegate);
    ui->torrentTree->hideColumn(3);

    connect(ui->addDirectory, SIGNAL(clicked()), this, SLOT(openSettingsWindow()));
    connect(this, SIGNAL(finished(int)), this, SLOT(saveUIsettings(int)));

    loadJSON();

    QSettings settings("ui.plist", QSettings::NativeFormat);
    this->resize(settings.value("addTorrentWindowSize").toSize());
}

AddTorrentWindow::~AddTorrentWindow(){
    delete ui;
}

void AddTorrentWindow::getTorrentFileInfo(QString fileNameIn){
    int item_limit = 1000000;
    int depth_limit = 1000;

    std::vector<char> buf;
    error_code ec;
    int ret = load_file(fileNameIn.toStdString().c_str(), buf, ec, 40 * 1000000);
    if (ret == -1)
    {
        fprintf(stderr, "file too big, aborting\n");
        return;
    }

    if (ret != 0)
    {
        fprintf(stderr, "failed to load file: %s\n", ec.message().c_str());
        return;
    }
    bdecode_node e;
    int pos = -1;
    printf("decoding. recursion limit: %d total item count limit: %d\n", depth_limit, item_limit);
    ret = bdecode(&buf[0], &buf[0] + buf.size(), e, ec, &pos, depth_limit, item_limit);

    if (ret != 0)
    {
        fprintf(stderr, "failed to decode: '%s' at character: %d\n", ec.message().c_str(), pos);
        return;
    }

    torrent_info t(e, ec);
    if (ec)
    {
        fprintf(stderr, "%s\n", ec.message().c_str());
        return;
    }
    e.clear();
    std::vector<char>().swap(buf);

    typedef std::vector<std::pair<std::string, int> > node_vec;

    char ih[41];
    to_hex((char const*)&t.info_hash()[0], 20, ih);

    ui->torrentSize->setText(unitBytes(t.total_size()));
    ui->torrentComment->setText(t.comment().c_str());

    file_storage const& st = t.files();
    for (int i = 0; i < st.num_files(); ++i)
    {
        QString filePath = st.file_path(i).c_str();
        QList<QStandardItem*> row;
        row.append(new QStandardItem(filePath));
        row.append(new QStandardItem(unitBytes(st.file_size(i))));
        row.append(new QStandardItem(QString("1")));
        model->appendRow(row);
    }
}

void AddTorrentWindow::openSettingsWindow(){
    SettingsWindow *settingsWindow = new SettingsWindow(this);
    settingsWindow->show();
}

void AddTorrentWindow::saveUIsettings(int r) {
    QSettings settings("ui.plist", QSettings::NativeFormat);
    settings.setValue("addTorrentWindowSize", this->size());
}
