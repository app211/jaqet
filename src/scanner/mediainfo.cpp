#include "mediainfo.h"

#include <QtCore/QStringList>
#include <QtCore/QUrl>

MediaInfo::MediaInfo() {
    qRegisterMetaType<MediaInfo>("MediaInfo");

    clear();
}

MediaInfo::MediaInfo(const QString & fileName) {
    qRegisterMetaType<MediaInfo>("MediaInfo");

    clear();

    setFileName(fileName);
}

MediaInfo::~MediaInfo() {
}

bool MediaInfo::operator==(const MediaInfo & mediaInfo) const {
    bool equal = true;

    equal &= _fetched == mediaInfo._fetched;
    equal &= _fileName == mediaInfo._fileName;
    equal &= _fileSize == mediaInfo._fileSize;
    equal &= _duration == mediaInfo._duration;

    return equal;
}

bool MediaInfo::operator!=(const MediaInfo & mediaInfo) const {
    return !(*this == mediaInfo);
}

void MediaInfo::clear() {
    _fetched = false;

    //General
    _fileName.clear();
    _fileSize = -1;
    _duration = -1;
}

bool MediaInfo::fetched() const {
    return _fetched;
}

void MediaInfo::setFetched(bool fetched) {
    _fetched = fetched;
}

QString MediaInfo::fileName() const {
    return _fileName;
}

void MediaInfo::setFileName(const QString & fileName) {
    _fileName = fileName.trimmed();

    //This avoid a stupid bug: comparing a filename with \ separator and another with /
    //By replacing any \ by /, we don't have any comparison problem
    //This is needed by FileSearchModel.cpp
    //and PlaylistModel::updateMediaInfo()
    //_fileName.replace("\\", "/");
}

bool MediaInfo::isUrl(const QString & fileName) {
    //A filename that contains a host/server name is a remote/network media
    return !QUrl(fileName).host().isEmpty();
}

qint64 MediaInfo::fileSize() const {
    return _fileSize;
}

void MediaInfo::setFileSize(qint64 bytes) {
    _fileSize = bytes;
}

QString MediaInfo::durationFormatted() const {
    if (_duration > 0) {
        //return TkTime::convertMilliseconds(_duration);
    }   return QString();
}

qint64 MediaInfo::durationSecs() const {
    if (_duration == -1) {
        return _duration;
    } else {
        return _duration / 1000.0;
    }
}

qint64 MediaInfo::durationMSecs() const {
    return _duration;
}

void MediaInfo::setDurationSecs(qint64 seconds) {
    _duration = 1000 * seconds;
}

void MediaInfo::setDurationMSecs(qint64 milliseconds) {
    _duration = milliseconds;
}

void MediaInfo::setFormat(const QString & format) {
    _format = format.trimmed();
}

QString MediaInfo::format() const {
    return _format;
}

void MediaInfo::setALanguages(const QStringList& alanguages)
{ _alanguages = alanguages;
}
QStringList MediaInfo::alanguages() const {
    return _alanguages;
}

void  MediaInfo::setTLanguages(const QStringList& tlanguages) {
    _tlanguages = tlanguages;
}

QStringList  MediaInfo::tlanguages() const {
    return _tlanguages;
}


