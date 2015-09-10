#include "mediainfoscanner.h"
#include <QDebug>

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif

// Cf .http://phonon-vlc-mplayer.googlecode.com/svn/trunk/quarkplayer-plugins/mainwindow/AboutWindow.cpp
#ifdef Q_OS_UNIX
    #include <MediaInfo/MediaInfo.h>
#else
    #include <MediaInfoDLL/MediaInfoDLL.h>
    #define MediaInfoLib MediaInfoDLL
#endif	//Q_OS_UNIX

#include "mediainfo.h"
#include <QDate>
#include <QUrl>
#include <QSize>

MediaInfoScanner::MediaInfoScanner()
{
 /*   MediaInfoLib::MediaInfo MI;
    QString mediaInfoLibVersion=QString::fromStdWString(MI.Option(__T("Info_Version"), __T("0.7.13;MediaInfoDLL_Example_MSVC;0.7.13")));
*/
    // Voir https://github.com/tkrotoff/QuarkPlayer/blob/3acf656b1127606ca2d3d46f4c72c0a604d89623/libs/MediaInfoFetcher/MediaInfoFetcher.cpp
}

Scanner::AnalysisResult MediaInfoScanner::analyze( const QFileInfo& fi ) const {

    MediaInfoLib::MediaInfo mediaInfo;

    mediaInfo.Open(fi.absoluteFilePath().toStdWString());

    MediaInfo _mediaInfo(fi.absoluteFilePath());

    _mediaInfo.setFileSize(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("FileSize"))).toInt());
    _mediaInfo.setDurationMSecs(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Duration"))).toULongLong());
    _mediaInfo.setFormat(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0, __T("Format"))));

    //Audio
    int audioStreamCount = mediaInfo.Count_Get(MediaInfoLib::Stream_Audio);
    if (audioStreamCount>0){
        _mediaInfo.setAChannelsCount(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, 0,__T("Channel(s)"))).trimmed().toUInt());
        _mediaInfo.setACodec(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, 0,__T("Codec/String"))));

    }
    for (int audioStreamId = 0; audioStreamId < audioStreamCount; audioStreamId++) {
//        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioBitrate, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("BitRate"))).trimmed().toInt() / 1000);
//        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioBitrateMode, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("BitRate_Mode"))).trimmed());
//        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioSampleRate, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("SamplingRate"))).trimmed().toFloat() / 1000.0);
//        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioBitsPerSample, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Resolution"))).trimmed().toUInt());
//        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioChannelCount, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Channel(s)"))).trimmed().toUInt());
//        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioCodec, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Codec/String"))).trimmed());
//        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioCodecProfile, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Codec_Profile"))).trimmed());
//        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioLanguage, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Language/String2"))).trimmed());
//        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioEncodedLibrary, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Encoded_Library/String"))).trimmed());

      //  qDebug() << QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Codec/String")));

    }


    //Video
    int videoStreamCount = mediaInfo.Count_Get(MediaInfoLib::Stream_Video);
    if (videoStreamCount > 0){
       _mediaInfo.setVCodec(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, 0,__T("Codec/String"))));

       int width = QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, 0,__T("Width"))).trimmed().toInt();
       int height = QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, 0,__T("Height"))).trimmed().toInt();
       _mediaInfo.setVResolution(QSize(width, height));

       _mediaInfo.setVAspect(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, 0,__T("AspectRatio/String"))).trimmed());

       _mediaInfo.setVScanType(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, 0,__T("ScanType"))).trimmed());

       _mediaInfo.setVDisplayAspect( QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, 0,__T("DisplayAspectRatio/String"))).trimmed());
    }

    QStringList audioLanguages;
    for (int audioStreamId = 0; audioStreamId < audioStreamCount; audioStreamId++) {
        QString audioLanguage=QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Language/String2"))).trimmed();
        if (!audioLanguage.isEmpty()){
            audioLanguages << audioLanguage;
        }
    }

    qDebug() << audioLanguages;
    _mediaInfo.setALanguages(audioLanguages);

    /*
     *
     * 1.0 2.0 2.1 5.0 5.1 6.1 7.1

    AAC AC3 AC3+ DTS DTS-HD 'DTS-HD HRA' 'DTS-HD MA' FLAC MP3 PCM TrueHD Vorbis WMA

    Codec : 3IVX AVC H.264 MPEG-1 MPEG-2 MPEG-4 Ogg QuickTime RealVideo VC-1 WMV XviD

    480i 480p 576i 576p 720p 1080i 1080p

    Format : AVI Blu-ray BDAV DVD Matroska MP4 MPEG-PS MPEG-TS Org QuickTime WindowMedia

    4:3 16:9 1.25:1 1.33:1 1.66:1 1.78:1 1.85:1 2.20:1 2.35:1 2.39:1 2.40:1 2.55:1 2.76:1

    23.976 24 25 29.97 30 50 59.94 60
    */

    //Text
   QStringList textLanguages;
   int textStreamCount = mediaInfo.Count_Get(MediaInfoLib::Stream_Text);
    for (int textStreamId = 0; textStreamId < textStreamCount; textStreamId++) {
           QString textLanguage= QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Text, textStreamId,__T("Language/String2"))).trimmed();
            if (!textLanguage.isEmpty()){
                textLanguages << textLanguage;
            }
     }

    _mediaInfo.setTLanguages(textLanguages);

    AnalysisResult result;
    result.mediaInfo=_mediaInfo;
    return result;
}
