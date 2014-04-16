#include "avprobe.h"

#include <QProcess>
#include <QRegExp>
#include <QStringList>
#include <QDir>
#include <QDebug>

avprobe::avprobe()
{
}


/*
 * bool QMediaInfo::MIInstalled()
{
#ifdef Q_OS_LINUX
  QProcess availableTest;
  availableTest.start("which", QStringList("mediainfo"));
  availableTest.waitForFinished();
  QString test = availableTest.readAll();
  if (test.contains("which: no mediainfo") || test.isEmpty())
    return false;
  else
    return true;
#endif
#ifdef Q_OS_WIN32
  QFile tempFile("MediaInfo.exe");
  if (tempFile.exists())
    return true;
  else
    return false;
#endif
}*/

avprobe_version avprobe::getVersion(const QString& unrarPath)
{
    avprobe_version result;
    result.isOk=false;

    QProcess process;
    process.start(QDir(unrarPath).filePath("avprobe"), QStringList() << "-version");
    if (!process.waitForFinished()){
        return result;

    }

    foreach(QString line,QString::fromLocal8Bit(process.readAllStandardOutput()).split("\n")){
        qDebug() << line;
        QRegExp regexp(".*k2pdfopt v(\\d*)\\.(\\d*)");
        regexp.setCaseSensitivity(Qt::CaseInsensitive);
        if (regexp.indexIn(line)>-1&& regexp.captureCount()==2){
            result.isOk=true;
            result.m_major=regexp.cap(1).trimmed();
            result.m_minor= regexp.cap(2).trimmed();
        }
    }

    return result;

}
bool analyse_video_stream( QMap<QString,QString>& values, video_stream& result){
    bool ok;
    if (!values.contains("width")){
        return false;
    } else {
        result.width = values["width"].toInt(&ok);
        if (!ok){
            return false;
        }
    }
    if (!values.contains("height")){
        return false;
    } else {
        result.height = values["height"].toInt(&ok);
        if (!ok){
            return false;
        }
    }
    if (!values.contains("codec_name")){
        return false;
    } else {
        result.codec = values["codec_name"];
    }
    return true;
}

bool analyse_audio_stream( QMap<QString,QString>& values, audio_stream& result){
    if (!values.contains("codec_name")){
        return false;
    } else {
        result.codec = values["codec_name"];
    }

    if (values.contains("TAG:language")){
       result.language = values["TAG:language"];
    }
    return true;
}

bool analyse_subtitle_stream( QMap<QString,QString>& values, subtitle_stream& result){
    if (!values.contains("TAG:language")){
        return false;
    } else {
        result.language = values["TAG:language"];
    }

    return true;
}

bool analyse_stream( QMap<QString,QString>& values, av_file& result){
    if (values.contains("codec_type")){
        if (values["codec_type"]=="video"){
            video_stream stream_video;
            if (analyse_video_stream(values, stream_video)){
                result.video_streams.append(stream_video);
            } else {
                return false;
            }
        } else if (values["codec_type"]=="audio"){
            audio_stream stream_audio;
            if (analyse_audio_stream(values, stream_audio)){
                result.audio_streams.append(stream_audio);
            } else {
                return false;
            }
        } else if (values["codec_type"]=="subtitle"){
            subtitle_stream stream_subtitle;
            if (analyse_subtitle_stream(values, stream_subtitle)){
                result.subtitle_streams.append(stream_subtitle);
            } else {
                return false;
            }
        }
    }

    return true;
}

void avprobe::getInfo(const QString& filePath){
    QProcess process;
    process.start("avprobe", QStringList() << "-show_streams" << filePath);
    bool bInStream = false;
    if (!process.waitForFinished()){
        return ;

    }

    qDebug() << process.exitStatus();
    qDebug() << process.error();

    QMap<QString,QString> streamValues;
    av_file result;
    bool ok=false;

    foreach(QString line,QString::fromLocal8Bit(process.readAllStandardOutput()).split("\n")){
        qDebug() << line;
        line=line.trimmed();
        if (!bInStream && line.startsWith("[STREAM]")){
            bInStream=true;
        } else if (bInStream && line.startsWith("[/STREAM]")){
            bInStream=false;
            ok = analyse_stream(streamValues, result);
            streamValues.clear();
        } else if (bInStream){
            if (line.indexOf('=')>0){
                QString lvalue=line.left(line.indexOf('=')).trimmed();
                QString rvalue=line.mid(line.indexOf('=')+1).trimmed();
                streamValues[lvalue]=rvalue;
            }
        }
    }

    if (ok){
        foreach ( const video_stream& vstream, result.video_streams){
            qDebug() << vstream.codec << vstream.width << vstream.height;
        }

        foreach ( const audio_stream& astream, result.audio_streams){
            qDebug() << astream.codec << astream.language;
        }
        foreach ( const subtitle_stream& sstream, result.subtitle_streams){
            qDebug() << sstream.language;
        }
    }
}
