#include "permissionwindow.h"
#include "ui_permissionwindow.h"

PermissionWindow::PermissionWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PermissionWindow)
{
    ui->setupUi(this);

    filesystem=new Filesystem(parent);
    filesystem->show();
}

PermissionWindow::~PermissionWindow()
{
    delete ui;
}
