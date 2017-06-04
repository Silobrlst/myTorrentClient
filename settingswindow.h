#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

#include "commons.h"

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = 0);
    ~SettingsWindow();

    void loadConfig();

public slots:
    void saveConfig();
    void addPath();
    void choosePath();

private:
    Ui::SettingsWindow *ui;
};

#endif // SETTINGS_H
