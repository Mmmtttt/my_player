#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <SDL.h>
#include <SDL_syswm.h>
#include <QWidget>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QMouseEvent>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openSecondWindow();
    //void openVideoPlayer();

private:
    QPushButton *openSecondWindowButton;
    QPushButton *openButton;
};

#endif