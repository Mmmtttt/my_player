#include "filesystem.h"
#include "./ui_Filesystem.h"
#include <QDebug>
#include <QInputDialog>

#include "fileiconmapper.h"
#include "win_net.h"
#include "my_portocol.h"



Filesystem::Filesystem(QWidget *parent,FILE_SYSTEM_TYPE _TYPE)
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



    ui->fileListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->fileListWidget, &QListWidget::customContextMenuRequested, this, &Filesystem::on_fileListWidget_customContextMenuRequested);
    connect(ui->refreshButton,&QToolButton::clicked,this,&Filesystem::on_rehreshButton_clicked);
//    connect(ui->back,&QToolButton::clicked,this,&Filesystem::on_back_clicked);
//    connect(ui->addfolder,&QToolButton::clicked,this,&Filesystem::on_addfolder_clicked);

    if(TYPE==Local){refresh();ui->connect->setVisible(false);ui->IP->setVisible(false); return;}

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR) {
        throw std::runtime_error("Initialization error\n");
    }
    else if(TYPE==Server){
        connect_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connect_socket == INVALID_SOCKET) {
            std::cout << "Error at socket: " << WSAGetLastError() << "\n";
            WSACleanup();
            return;
        }


        connect(ui->connect,&QToolButton::clicked,this,&Filesystem::on_openserver_clicked);
        ui->refreshButton->setVisible(false);
        ui->addfolder->setVisible(false);
        ui->IP->setVisible(false);


    }
    else{


        connect(ui->connect,&QToolButton::clicked,this,&Filesystem::on_connect_to_server_clicked);

    }
}

Filesystem::~Filesystem()
{
    delete ui;
}

void Filesystem::refresh(){
    ui->fileListWidget->clear();

    if(TYPE==Client){


        std::string message("REFRESH");
        Send_Message(connect_socket,message);
        Recv_Message(connect_socket,message);
        QString qMessage = QString::fromStdString(message);

        ui->path->clear();
        Recv_Message(connect_socket,message);
        ui->path->setText(QString(message.c_str()));
        currentDir.setPath(QString(message.c_str()));

        QJsonDocument jsonDoc = QJsonDocument::fromJson(qMessage.toUtf8());
        QJsonArray jsonArray = jsonDoc.array();

        // 遍历 JSON 数组
        for (const QJsonValue &value : jsonArray) {
            if (value.isObject()) {
                QJsonObject jsonObject = value.toObject();
                QString name = jsonObject["name"].toString();
                FileType fileType = (FileType)jsonObject["fileType"].toInt();
                File file(name, fileType);
                addFileToList(file,fileType);
            }
        }

        return;
    }

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

    if(TYPE==Server){



        std::string message=jsonDoc.toJson().toStdString();
        Send_Message(accept_socket,message);
        message=currentDir.absolutePath().toStdString();
        Send_Message(accept_socket,message);


    }
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
    if(TYPE==Local)openFile(file);
    else if(TYPE==Client){
        std::string message("OPEN");
        Send_Message(connect_socket,message);
        message=file.getName().toStdString();
        Send_Message(connect_socket,message);
        switch(file.getFileType()){
            case Video:message="VIDEO";break;
            case Folder:message="FOLDER";break;
        }


        Send_Message(connect_socket,message);
        openFile(file);
    }

}

void Filesystem::openFile(const File &file) {
    // 根据文件类型执行打开操作，例如展示详细信息窗口
    //if(TYPE==Client){std::string message("OPEN");Send_Message(connect_socket,message);}

    QString fileTypeName;
    switch (file.getFileType()) {
        case Folder: fileTypeName = "文件夹"; {currentDir.cd(file.getName()) ;if(TYPE!=Server)refresh();}break;
        case Video: fileTypeName = "视频";{//Player player(this,(currentDir.path()+'/'+file.getName()).toStdString());
                                            //player.show();player.play();
                                            QStringList arguments;
                                            arguments <<currentDir.path()+'/'+file.getName();
                                            if(TYPE==Server){


//                                                SOCKET accept_socket = accept(connect_socket, NULL, NULL);
//                                                if (accept_socket == INVALID_SOCKET) {
//                                                    std::cout << "accept() failed: " << WSAGetLastError() << '\n';
//                                                    closesocket(connect_socket);
//                                                    WSACleanup();
//                                                    return;
//                                                }
//                                                qDebug()<<file.getName();
//                                                qDebug()<<currentDir.path();
//                                                Session session(arguments[0].toStdString(),accept_socket,SERVER);
                                                    QProcess *process=new QProcess(this);
                                                    process->start("Process2.exe", arguments);

                                                break;
                                            }
                                            else if(TYPE==Client)  arguments <<"REMOTE";
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
            QString srcFilePath = currentDir.path()+'/' + fileName;
            QString destFilePath = currentDir.path()+'/'+newPath + '/' + fileName;
            //qDebug()<<destFilePath;
            QFile::copy(srcFilePath, destFilePath);
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

    if(TYPE==Client){std::string message("BACK");Send_Message(connect_socket,message);}
    else currentDir.cdUp();
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

void Filesystem::on_openserver_clicked(){
        SOCKADDR_IN serverService;
        serverService.sin_family = AF_INET;
        serverService.sin_addr.s_addr = INADDR_ANY;
        serverService.sin_port = htons(12345);

        if (bind(connect_socket, (SOCKADDR*)&serverService, sizeof(serverService)) == SOCKET_ERROR) {
               std::cout << "bind() failed.\n";
               closesocket(connect_socket);
               WSACleanup();
               return;
        }

        if (listen(connect_socket, 1) == SOCKET_ERROR) {
               std::cout << "Error listening on socket.\n";
               closesocket(connect_socket);
               WSACleanup();
               return;
        }

        accept_socket = accept(connect_socket, NULL, NULL);
        if (accept_socket == INVALID_SOCKET) {
               std::cout << "accept() failed: " << WSAGetLastError() << '\n';
               closesocket(connect_socket);
               WSACleanup();
               return;
        }
        std::cout << "Client connected.\n";



        std::string message;


        while(1){
               if(Recv_Message(accept_socket,message)<0)return;
               if(message=="OPEN"){
                      Recv_Message(accept_socket,message);
                      QString fileName=message.c_str();
                      Recv_Message(accept_socket,message);
                      FileType fileType;
                      if(message=="VIDEO")fileType=Video;
                      else if(message=="FOLDER")fileType=Folder;
                      else fileType=Other;

                      File file(fileName, fileType);
                      openFile(file);
               }
               else if(message=="REFRESH"){
                      refresh();
               }
               else if(message=="BACK"){
                      currentDir.cdUp();
               }

        }


        return;

}

void Filesystem::on_connect_to_server_clicked(){
        QString IP = ui->IP->text();

        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != NO_ERROR) {
               throw std::runtime_error("Initialization error\n");
        }

        connect_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connect_socket == INVALID_SOCKET) {
               std::cout << "Error at socket: " << WSAGetLastError() << "\n";
               WSACleanup();
               return;
        }

        SOCKADDR_IN clientService;
        clientService.sin_family = AF_INET;
        clientService.sin_addr.s_addr = inet_addr(IP.toStdString().c_str());
        clientService.sin_port = htons(12345);

        if (::connect(connect_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
               std::cout << "Failed to connect.\n";
               closesocket(connect_socket);
               WSACleanup();
               return;
        }

        refresh();
}

