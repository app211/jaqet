#ifndef AV_H
#define AV_H

#include <QString>
#include <QList>

struct video_stream {
    unsigned int width;
    unsigned int height;
    QString codec;
};

struct audio_stream {
     QString codec;
     QString language;
};

struct subtitle_stream {
     QString language;
};

struct av_file {
    QList<video_stream> video_streams;
    QList<audio_stream> audio_streams;
    QList<subtitle_stream> subtitle_streams;
 };

#endif // AV_H
