#include "settingswindow.h"
#include "ui_settingswindow.h"

#include <qjsonobject.h>
#include <qjsonarray.h>
#include <QJsonDocument>
#include <qfile.h>
#include <qfiledialog.h>

SettingsWindow::SettingsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    loadJSON();

    connect(ui->addPathButton, SIGNAL(clicked()), this, SLOT(addPath()));
    connect(ui->viewDirectoriesButton, SIGNAL(clicked()), this, SLOT(choosePath()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveJSON()));
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::saveJSON(){
    QJsonObject json;
    QJsonArray paths;

    for(int i=0; i<ui->pathsList->count(); i++){
        paths.append(ui->pathsList->item(i)->text());
    }

    json["paths"] = paths;


    QFile saveFile("settings.json");

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open settings file.");
        return;
    }

    QJsonDocument doc(json);
    saveFile.write(doc.toJson());
}

void SettingsWindow::loadJSON(){
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
        ui->pathsList->addItem(paths[i].toString());
    }
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
