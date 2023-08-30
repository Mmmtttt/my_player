#include "userwindow.h"
#include "ui_userwindow.h"

UserWindow::UserWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserWindow)
{
    ui->setupUi(this);


    filesystem=new Filesystem(parent,Local);
    layout()->addWidget(filesystem);
}

UserWindow::~UserWindow()
{
    delete ui;
}


