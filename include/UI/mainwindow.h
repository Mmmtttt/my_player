#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QPushButton>
#include <SDL.h>
#include <SDL_syswm.h>
#include <QWidget>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QMouseEvent>
#include <QTimer>

#include "settingswindow.h"
#include "userwindow.h"
#include "permissionwindow.h"
#include "ui_mainwindow.h"

#include "player.h"

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
    void handleButtonClicked(QAbstractButton *button)
    {
        int id = btnGroup.id(button);
        ui->stackedWidget->setCurrentIndex(id);
    }

private:
    Ui::MainWindow *ui;

    QButtonGroup btnGroup;
    UserWindow userWnd;
    PermissionWindow permissionWnd;
    SettingsWindow settingsWnd;

    
};
#endif // MAINWINDOW_H
