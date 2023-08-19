#include "permissionwindow.h"
#include "ui_permissionwindow.h"

PermissionWindow::PermissionWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PermissionWindow)
{
    ui->setupUi(this);
}

PermissionWindow::~PermissionWindow()
{
    delete ui;
}
