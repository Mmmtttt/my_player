#ifndef FILEICONMAPPER_H
#define FILEICONMAPPER_H


#include <QString>
#include <QMap>
#include <qvector.h>
#include <QToolButton>
#include "file.h"

class FileIconMapper
{
public:
    FileIconMapper();

    QString getIconFormFileType(FileType) const;
    FileType getTypeFormFileExtension(const QString &fileType) ;

    void add_types(FileType type, QString extension);
    QVector<QString> show_types(FileType);




private:
    void load_types();


    QMap<FileType, QString> iconMap;
    QMap<QString, FileType> extensionMap;

    QVector<QString> text_types{"txt","log","cmake","c","cpp","h","hpp","java","py","bat","sh"};
    QVector<QString> picture_types{"jpg","gif","png"};
    QVector<QString> video_types{"mp4","mp3","avi","rmvb","flv","ts"};
};

#endif // FILEICONMAPPER_H
