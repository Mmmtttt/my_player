#ifndef PERMISSIONWINDOW_H
#define PERMISSIONWINDOW_H

#include <QWidget>

namespace Ui {
class PermissionWindow;
}

class PermissionWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PermissionWindow(QWidget *parent = nullptr);
    ~PermissionWindow();

private:
    Ui::PermissionWindow *ui;
};

#endif // PERMISSIONWINDOW_H
