#include "addtorrentwindow.h"
#include "ui_addtorrentwindow.h"
#include <QFileIconProvider>
#include <QContextMenuEvent>
#include <QMenu>


using namespace libtorrent;
namespace lt = libtorrent;
using clk = std::chrono::steady_clock;


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

    menu = new QMenu(this);
    QMenu *priority = menu->addMenu("приоритет");
    connect(priority->addAction("не загружать"), SIGNAL(triggered()), this, SLOT(prioriyNotLoad()));
    connect(priority->addAction("обычный"), SIGNAL(triggered()), this, SLOT(prioriyNormal()));
    connect(priority->addAction("высокий"), SIGNAL(triggered()), this, SLOT(prioriyHigh()));
    connect(priority->addAction("максимальный"), SIGNAL(triggered()), this, SLOT(prioriyMaximum()));
    connect(menu->addAction("выделиь всё"), SIGNAL(triggered()), ui->torrentTree, SLOT(selectAll()));

    model = new TorrentTreeModel(ui->torrentTree);
    ui->torrentTree->setModel(model);

    TorrentTreeDelegate* delegate = new TorrentTreeDelegate(this);
    ui->torrentTree->setItemDelegate(delegate);
    ui->torrentTree->hideColumn(TorrentContenTreeColumns::TorrentContenTreeProgress);

    connect(ui->addDirectory, SIGNAL(clicked()), this, SLOT(openSettingsWindow()));
    connect(this, SIGNAL(finished(int)), this, SLOT(saveUIsettings(int)));
    connect(ui->torrentTree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));

    loadJSON();

    QSettings settings("ui.plist", QSettings::NativeFormat);
    this->resize(settings.value("addTorrentWindowSize").toSize());
    for(int i=0; i<ui->torrentTree->model()->columnCount(); i++){
        ui->torrentTree->setColumnWidth(i, settings.value("addTorrentWindowTreeColumn"+QString::number(i), 100).toInt());
    }

    getTorrentFileInfo(fileNameIn);
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

    QFileIconProvider icons;

    file_storage const& st = t.files();
    for (int i = 0; i < st.num_files(); ++i)
    {
        QString filePath = st.file_path(i).c_str();
        QList<QStandardItem*> row;
        QStandardItem *fileItem = new QStandardItem(icons.icon(QFileIconProvider::File), filePath);
        row.append(fileItem);
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

    for(int i=0; i<ui->torrentTree->model()->columnCount(); i++){
        settings.setValue("addTorrentWindowTreeColumn"+QString::number(i), ui->torrentTree->columnWidth(i));
    }

    choosedPath = ui->saveDirectory->currentData().toString();
}

void AddTorrentWindow::contextMenu(QPoint){
    menu->exec(QCursor::pos());
}

void AddTorrentWindow::prioriyNotLoad(){
    model->prioritizeSelected(TorrentContentPriority::TorrentContentNoLoad);
}

void AddTorrentWindow::prioriyNormal(){
    model->prioritizeSelected(TorrentContentPriority::TorrentContentNormal);
}

void AddTorrentWindow::prioriyHigh(){
    model->prioritizeSelected(TorrentContentPriority::TorrentContentHigh);
}

void AddTorrentWindow::prioriyMaximum(){
    model->prioritizeSelected(TorrentContentPriority::TorrentContentMaximum);
}
