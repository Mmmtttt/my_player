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

class Filesystem : public QMainWindow
{
    Q_OBJECT

public:
    Filesystem(QWidget *parent = nullptr,PLAYER_TYPE TYPE=LOCAL);
    ~Filesystem();

    void refresh();

private slots:
    void on_fileListWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_fileListWidget_customContextMenuRequested(const QPoint &pos);

    void on_rehreshButton_clicked();

    void on_back_clicked();

    void on_addfolder_clicked();

private:
    Ui::Filesystem *ui;
    FileIconMapper iconMapper;
    QToolButton *refreshbutton;
    QDir currentDir;
    PLAYER_TYPE TYPE;

    void addFileToList(const File &file, FileType filetype);
    void openFile(const File &file);
    void sortItemsFromNonFolder();

};
#endif // FILESYSTEM_H
