#include "userwindow.h"
#include "ui_userwindow.h"

UserWindow::UserWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserWindow)
{
    ui->setupUi(this);

    openSecondWindowButton = new QPushButton("Open Second Window", this);
    openSecondWindowButton->move(50, 50);  // 设置按钮位置
    connect(openSecondWindowButton, &QPushButton::clicked, this, &UserWindow::openSecondWindow);
}

UserWindow::~UserWindow()
{
    delete ui;
}

void UserWindow::openSecondWindow()
{
    //Form *secondWindow = new Form;
    //secondWindow->show();
    Player mainWin;

    mainWin.show();
    mainWin.video->play();
}
