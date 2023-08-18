#include <QProcess>
#include "mainwindow.h"
#include "player.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    openSecondWindowButton = new QPushButton("Open Second Window", this);
    openSecondWindowButton->move(50, 50);  // 设置按钮位置
    connect(openSecondWindowButton, &QPushButton::clicked, this, &MainWindow::openSecondWindow);

    openButton = new QPushButton("Open Video Player", this);
    //connect(openButton, &QPushButton::clicked, this, &MainWindow::openVideoPlayer);
}

MainWindow::~MainWindow()
{
}

void MainWindow::openSecondWindow()
{
    //Form *secondWindow = new Form;
    //secondWindow->show();
    Player mainWin;

    mainWin.show();
    mainWin.video->play();
}

//#include "mainwindow.moc"