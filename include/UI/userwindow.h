#ifndef USERWINDOW_H
#define USERWINDOW_H

#include <QWidget>
#include <QPushButton>
#include "filesystem.h"

namespace Ui {
class UserWindow;
}

class UserWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UserWindow(QWidget *parent = nullptr);
    ~UserWindow();



private:
    Ui::UserWindow *ui;
    Filesystem *filesystem;
};

#endif // USERWINDOW_H
