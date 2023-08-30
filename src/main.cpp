#include "mainwindow.h"
#include <SDL.h>
#include <SDL_syswm.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}