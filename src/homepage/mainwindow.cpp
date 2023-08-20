#include <QProcess>
#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
     , ui(new Ui::MainWindow)
{
    

    ui->setupUi(this);

    ui->stackedWidget->addWidget(&userWnd);
    ui->stackedWidget->addWidget(&permissionWnd);
    ui->stackedWidget->addWidget(&settingsWnd);

//    btnGroup.setExclusive(false);
    btnGroup.addButton(ui->btnUser, 0);
    btnGroup.addButton(ui->btnPermission, 1);
    btnGroup.addButton(ui->btnSettings, 2);
    connect(&btnGroup, &QButtonGroup::buttonClicked,this, &MainWindow::handleButtonClicked);

    // 设置默认选中的页面
    btnGroup.button(0)->setChecked(true);
    ui->stackedWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}



//#include "mainwindow.moc"
