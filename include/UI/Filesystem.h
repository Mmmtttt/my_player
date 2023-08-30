#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QLabel>



#include "fileiconmapper.h"


QT_BEGIN_NAMESPACE
namespace Ui { class Filesystem; }
QT_END_NAMESPACE

enum FILE_SYSTEM_TYPE{
    Server,
    Client,
    Local
};

class Filesystem : public QMainWindow
{
    Q_OBJECT

public:
    Filesystem(QWidget *parent = nullptr,FILE_SYSTEM_TYPE TYPE=Local);
    ~Filesystem();

    void refresh();

private slots:
    void on_fileListWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_fileListWidget_customContextMenuRequested(const QPoint &pos);

    void on_rehreshButton_clicked();

    void on_back_clicked();

    void on_addfolder_clicked();

    void on_openserver_clicked();

    void on_connect_to_server_clicked();





    //void handleConnection(SOCKET accept_socket);
    //void readData();

private:
    Ui::Filesystem *ui;
    FileIconMapper iconMapper;
    QToolButton *refreshbutton;
    QDir currentDir;
    FILE_SYSTEM_TYPE TYPE;

    SOCKET connect_socket;
    SOCKET accept_socket;

    QString IP;

    void addFileToList(const File &file, FileType filetype);
    void openFile(const File &file);
    void sortItemsFromNonFolder();

};
#endif // FILESYSTEM_H
