#include "fileiconmapper.h"

FileIconMapper::FileIconMapper()
{
    iconMap[Folder] = ":/icons/folder.png";
    iconMap[Text] = ":/icons/text.png";
    iconMap[Video] = ":/icons/video.png";
    iconMap[Image] = ":/icons/image.png";
    iconMap[Other] = ":/icons/other.png";

    load_types();
    extensionMap["folder"]=Folder;
}

void FileIconMapper::load_types(){
    for (const QString &type : text_types) {
        extensionMap[type]=Text;
    }
    for (const QString &type : picture_types) {
        extensionMap[type]=Image;
    }
    for (const QString &type : video_types) {
        extensionMap[type]=Video;
    }
}


QString FileIconMapper::getIconFormFileType(FileType fileType) const
{
    if (iconMap.contains(fileType)) {
        return iconMap[fileType];
    } else {
        return iconMap[Other];
    }
}

FileType FileIconMapper::getTypeFormFileExtension(const QString &extension) {
    if(extensionMap.contains(extension)){
        return extensionMap[extension];
    }else {
        return Other;
    }
}

void FileIconMapper::add_types(FileType type, QString extension){
    if(type==Text){
        text_types.append(extension);
        extensionMap[extension]=Text;
    }else if(type==Image){
        picture_types.append(extension);
        extensionMap[extension]=Image;
    }else if(type==Video){
        video_types.append(extension);
        extensionMap[extension]=Video;
    }
}

QVector<QString> FileIconMapper::show_types(FileType type){
    if(type==Text){
        return text_types;
    }else if(type==Image){
        return picture_types;
    }else if(type==Video){
        return video_types;
    }
    else return text_types+picture_types+video_types;
}

