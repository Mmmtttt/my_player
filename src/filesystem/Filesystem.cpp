#include "filesystem.h"
#include "./ui_Filesystem.h"
#include <QDebug>
#include <QInputDialog>

#include "fileiconmapper.h"



Filesystem::Filesystem(QWidget *parent,PLAYER_TYPE _TYPE)
    : QMainWindow(parent),TYPE(_TYPE)
    , ui(new Ui::Filesystem)
{
    ui->setupUi(this);
    currentDir.setPath(".");
    ui->path->setText(currentDir.absolutePath());

    // 获取默认字体
    QFont font = QApplication::font();

    // 设置图标的大小
    ui->fileListWidget->setIconSize(QSize(64, 64)); // 设置图标大小

    // 设置项的默认字体大小
    font.setPointSize(10); // 设置字体大小
    ui->fileListWidget->setFont(font);

    refreshbutton = ui->refreshButton;

    refresh();

    ui->fileListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->fileListWidget, &QListWidget::customContextMenuRequested, this, &Filesystem::on_fileListWidget_customContextMenuRequested);
    connect(ui->refreshButton,&QToolButton::clicked,this,&Filesystem::on_rehreshButton_clicked);
    connect(ui->back,&QToolButton::clicked,this,&Filesystem::on_back_clicked);
    connect(ui->addfolder,&QToolButton::clicked,this,&Filesystem::on_addfolder_clicked);
}

Filesystem::~Filesystem()
{
    delete ui;
}

void Filesystem::refresh(){
    ui->fileListWidget->clear();

    QFileInfoList fileList = currentDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    QJsonArray jsonArray;



    foreach (const QFileInfo &fileInfo, fileList) {
        QJsonObject jsonObject;
        QString fileName = fileInfo.fileName();
        QString extension = fileInfo.isDir() ? "folder" : fileInfo.suffix(); // Modify this to determine file types

        FileType fileType = iconMapper.getTypeFormFileExtension(extension);
        //QString iconFileName = iconMapper.getIconFormFileType(fileType);
        jsonObject["name"] = fileName;
        jsonObject["fileType"] = fileType;

        jsonArray.append(jsonObject);


        File file(fileName, fileType);
        addFileToList(file,fileType);
    }
    sortItemsFromNonFolder();

    ui->path->clear();
    ui->path->setText(currentDir.absolutePath());

    QJsonDocument jsonDoc(jsonArray);
    qDebug() << jsonDoc.toJson();
}

void Filesystem::addFileToList(const File &file, FileType filetype) {

    QPixmap icon(iconMapper.getIconFormFileType(filetype));

    QListWidgetItem* item = new QListWidgetItem(icon,file.getName());

    item->setData(Qt::UserRole, QVariant::fromValue(file)); // 存储文件信息
    if (file.getFileType() == Folder) {
        ui->fileListWidget->insertItem(0, item);
    } else {
        ui->fileListWidget->addItem(item);
    }
}


void Filesystem::sortItemsFromNonFolder() {
    // 找到第一个不是文件夹类型的元素
    int startIndex = -1;
    for (int i = 0; i < ui->fileListWidget->count(); ++i) {
        QListWidgetItem* item = ui->fileListWidget->item(i);
        File file = item->data(Qt::UserRole).value<File>();
        if (file.getFileType() != Folder) {
            startIndex = i;
            break;
        }
    }

    // 如果没有找到非文件夹元素，无需排序
    if (startIndex == -1) {
        return;
    }

    // 提取从非文件夹元素开始到最后的元素
    QList<QListWidgetItem*> itemsToSort;
    for (int i = startIndex; i < ui->fileListWidget->count(); ++i) {
        QListWidgetItem* item = ui->fileListWidget->takeItem(startIndex);
        itemsToSort.append(item);
    }

    // 按名称排序
    std::sort(itemsToSort.begin(), itemsToSort.end(), [](QListWidgetItem* a, QListWidgetItem* b) {
        return a->text() < b->text();
    });

    // 插入排序后的元素回列表
    for (QListWidgetItem* item : itemsToSort) {
        ui->fileListWidget->addItem(item);
    }
}





void Filesystem::on_fileListWidget_itemDoubleClicked(QListWidgetItem *item) {
    File file = item->data(Qt::UserRole).value<File>();
    openFile(file);
}

void Filesystem::openFile(const File &file) {
    // 根据文件类型执行打开操作，例如展示详细信息窗口
    QString fileTypeName;
    switch (file.getFileType()) {
        case Folder: fileTypeName = "文件夹"; currentDir.cd(file.getName()) ;refresh();break;
        case Video: fileTypeName = "视频";{//Player player(this,(currentDir.path()+'/'+file.getName()).toStdString());
                                            //player.show();player.play();
                                            QStringList arguments;
                                            arguments <<currentDir.path()+'/'+file.getName();
                                            if(TYPE==REMOTE) arguments <<"REMOTE";
                                            else arguments <<"LOCAL";
                                            QProcess *process=new QProcess(this);
                                            process->start("Process.exe", arguments);
                                            connect(process, &QProcess::finished, process, &QProcess::deleteLater);
                                           }  break;
        case Image: fileTypeName = "图片"; break;
        case Text: fileTypeName = "文本"; break;
    }

    QString message = QString("打开 %1：%2").arg(fileTypeName).arg(file.getName());
    qDebug()<<message;
}

void Filesystem::on_fileListWidget_customContextMenuRequested(const QPoint &pos) {
    QListWidgetItem *selectedItem = ui->fileListWidget->itemAt(pos);
    if (selectedItem) {
        QMenu contextMenu(this);
        QAction openAction("open", this);
        QAction openAction2("delete", this);
        QAction openAction3("rename", this);
        QAction openAction4("copy", this);
        qDebug()<<"右键";
        connect(&openAction, &QAction::triggered, [this, selectedItem]() {
            File file = selectedItem->data(Qt::UserRole).value<File>();
            openFile(file);
        });
        connect(&openAction2, &QAction::triggered, [this, selectedItem]() {
            File file = selectedItem->data(Qt::UserRole).value<File>();
            if(file.getFileType()==Folder){currentDir.rmdir(file.getName());}
            else currentDir.remove(file.getName());
            refresh();
        });
        connect(&openAction3, &QAction::triggered, [this, selectedItem]() {
            File file = selectedItem->data(Qt::UserRole).value<File>();
            bool ok;
            QString newName = QInputDialog::getText(this, tr("Rename"),
                                                          tr("Enter new name:"), QLineEdit::Normal,
                                                          currentDir.dirName(), &ok);
            currentDir.rename(file.getName(),newName);
            refresh();
        });
        connect(&openAction4, &QAction::triggered, [this, selectedItem]() {
            File file = selectedItem->data(Qt::UserRole).value<File>();
            bool ok;
            QString newPath = QInputDialog::getText(this, tr("copy"),
                                                    tr("Enter new path:"), QLineEdit::Normal,
                                                    currentDir.dirName(), &ok);

            QString fileName=file.getName();
            QString destFilePath = newPath + "\"" + fileName;
            QFile::copy(fileName, destFilePath);
            qDebug()<<destFilePath;
            refresh();
        });
        contextMenu.addAction(&openAction);
        contextMenu.addAction(&openAction2);
        contextMenu.addAction(&openAction3);
        contextMenu.addAction(&openAction4);
        contextMenu.exec(ui->fileListWidget->mapToGlobal(pos));
    }
}


void Filesystem::on_rehreshButton_clicked()
{
    refresh();
}


void Filesystem::on_back_clicked()
{
    currentDir.cdUp();
    refresh();
}


void Filesystem::on_addfolder_clicked()
{
    currentDir.mkdir("newfolder");
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Rename"),
                                            tr("Enter new name:"), QLineEdit::Normal,
                                            currentDir.dirName(), &ok);
    currentDir.rename("newfolder",newName);
    refresh();
}

