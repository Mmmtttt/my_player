#include "settingswindow.h"
#include "ui_settingswindow.h"
SettingsWindow::SettingsWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    filesystem=new Filesystem(parent,Server);
    layout()->addWidget(filesystem);
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}
