#include "file.h"

File::File(const QString& name, FileType fileType)
    : name(name), fileType(fileType) {}

const QString& File::getName() const {
    return name;
}

FileType File::getFileType() const {
    return fileType;
}

QJsonObject File::toJson() const {
    QJsonObject json;
    json["name"] = name;
    json["fileType"] = static_cast<int>(fileType);
    return json;
}
