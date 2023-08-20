#ifndef FILE_H
#define FILE_H

#include <QString>
#include <QJsonObject>

enum FileType {
    Folder,
    Video,
    Image,
    Text,
    Other
};

class File {
public:
    File(const QString& name, FileType fileType);
    File(std::initializer_list<File> il, QObject *parent = nullptr)
    {
        if (il.size() > 0)
        {
            this->name = il.begin()->name;
            this->fileType = il.begin()->fileType;
        }
    }

    const QString& getName() const;
    FileType getFileType() const;
    QJsonObject toJson() const;

private:
    QString name;
    FileType fileType;
};

#endif // FILE_H
