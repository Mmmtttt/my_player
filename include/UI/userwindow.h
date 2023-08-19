#ifndef USERWINDOW_H
#define USERWINDOW_H

#include <QWidget>
#include <QPushButton>
#include "player.h"

namespace Ui {
class UserWindow;
}

class UserWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UserWindow(QWidget *parent = nullptr);
    ~UserWindow();


private slots:
    void openSecondWindow();

private:
    Ui::UserWindow *ui;
    QPushButton *openSecondWindowButton;
};

#endif // USERWINDOW_H
