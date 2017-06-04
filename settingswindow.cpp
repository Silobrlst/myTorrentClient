#include "settingswindow.h"
#include "ui_settingswindow.h"

SettingsWindow::SettingsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    loadConfig();

    connect(ui->addPathButton, SIGNAL(clicked()), this, SLOT(addPath()));
    connect(ui->viewDirectoriesButton, SIGNAL(clicked()), this, SLOT(choosePath()));
    connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(saveConfig()));

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(close()));
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::saveConfig(){
    QJsonObject json;
    QJsonArray paths;

    for(int i=0; i<ui->pathsList->count(); i++){
        paths.append(ui->pathsList->item(i)->text());
    }

    json["paths"] = paths;
    json["torrentFilesPath"] = ui->torrentFilesPath->text();
    saveJSON("settings.json", json);
}

void SettingsWindow::loadConfig(){
    QJsonObject json = loadJSON("settings.json");

    QJsonArray paths = json["paths"].toArray();
    for(int i = 0; i < paths.size(); i++){
        ui->pathsList->addItem(paths[i].toString());
    }

    QString torrentFilesPath = json["torrentFilesPath"].toString();

    if(torrentFilesPath.size() == 0){
        torrentFilesPath = ".";
    }

    ui->torrentFilesPath->setText(torrentFilesPath);
}

void SettingsWindow::addPath(){
    ui->pathsList->addItem(ui->pathLineEdit->text());
}

void SettingsWindow::choosePath(){
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::DirectoryOnly);

    if(dialog.exec() == QDialog::Accepted){
        ui->pathsList->addItems(dialog.selectedFiles());
    }
}
