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
    _mediaInfo.setBitrate(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("OverallBitRate"))).toInt() / 1000);
    _mediaInfo.setEncodedApplication(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Encoded_Application"))));
    _mediaInfo.setFormat(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0, __T("Format"))));

     _mediaInfo.setMetaData(MediaInfo::Format, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0, __T("Format"))));
    qDebug() << QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0, __T("Format/Family")));
    qDebug() << QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0, __T("Format/Extensions")));
    qDebug() << QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0, __T("Format")));



    //Metadata
    _mediaInfo.setMetaData(MediaInfo::Title, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Title"))).trimmed());
    _mediaInfo.setMetaData(MediaInfo::Artist, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Performer"))).trimmed());
    _mediaInfo.setMetaData(MediaInfo::Album, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Album"))).trimmed());
    _mediaInfo.setMetaData(MediaInfo::AlbumArtist, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Accompaniment"))).trimmed());
    _mediaInfo.setMetaData(MediaInfo::TrackNumber, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Track/Position"))).trimmed().toInt());
    _mediaInfo.setMetaData(MediaInfo::DiscNumber, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Part/Position"))).trimmed().toInt());
    _mediaInfo.setMetaData(MediaInfo::OriginalArtist, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Original/Performer"))).trimmed());
    _mediaInfo.setMetaData(MediaInfo::Composer, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Composer"))).trimmed());
    _mediaInfo.setMetaData(MediaInfo::Publisher, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Publisher"))).trimmed());
    _mediaInfo.setMetaData(MediaInfo::Genre, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Genre"))).trimmed());
    _mediaInfo.setMetaData(MediaInfo::Year, QDate::fromString(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Recorded_Date"))).trimmed()));
    _mediaInfo.setMetaData(MediaInfo::BPM, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("BPM"))).trimmed());
    _mediaInfo.setMetaData(MediaInfo::Copyright, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Copyright"))).trimmed());
    _mediaInfo.setMetaData(MediaInfo::Comment, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Comment"))).trimmed());
    _mediaInfo.setMetaData(MediaInfo::URL, QUrl(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("URL"))).trimmed()));
    _mediaInfo.setMetaData(MediaInfo::EncodedBy, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("Encoded_Library/String"))).trimmed());
    _mediaInfo.setMetaData(MediaInfo::MusicBrainzArtistId, /*QUuid(*/QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("MusicBrainz Artist Id"))).trimmed()/*)*/);
    _mediaInfo.setMetaData(MediaInfo::MusicBrainzReleaseId, /*QUuid(*/QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("MusicBrainz Album Id"))).trimmed()/*)*/);
    _mediaInfo.setMetaData(MediaInfo::MusicBrainzTrackId, /*QUuid(*/QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("MusicBrainz Track Id"))).trimmed()/*)*/);
    _mediaInfo.setMetaData(MediaInfo::AmazonASIN, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_General, 0,__T("ASIN"))).trimmed());

    //Audio
    int audioStreamCount = mediaInfo.Count_Get(MediaInfoLib::Stream_Audio);
    for (int audioStreamId = 0; audioStreamId < audioStreamCount; audioStreamId++) {
        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioBitrate, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("BitRate"))).trimmed().toInt() / 1000);
        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioBitrateMode, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("BitRate_Mode"))).trimmed());
        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioSampleRate, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("SamplingRate"))).trimmed().toFloat() / 1000.0);
        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioBitsPerSample, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Resolution"))).trimmed().toUInt());
        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioChannelCount, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Channel(s)"))).trimmed().toUInt());
        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioCodec, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Codec/String"))).trimmed());
        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioCodecProfile, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Codec_Profile"))).trimmed());
        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioLanguage, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Language/String2"))).trimmed());
        _mediaInfo.insertAudioStream(audioStreamId, MediaInfo::AudioEncodedLibrary, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Encoded_Library/String"))).trimmed());

        qDebug() << QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Audio, audioStreamId,__T("Codec/String")));

    }

    //Video
    int videoStreamCount = mediaInfo.Count_Get(MediaInfoLib::Stream_Video);
    if (videoStreamCount > 0){
       _mediaInfo.setFirstVideoCodec(QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, 0,__T("Codec/String"))));
    }

    for (int videoStreamId = 0; videoStreamId < videoStreamCount; videoStreamId++) {

        _mediaInfo.insertVideoStream(videoStreamId, MediaInfo::VideoBitrate, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("BitRate"))).trimmed().toUInt() / 1000);

         _mediaInfo.insertVideoStream(videoStreamId, MediaInfo::VideoAspectRatioString,QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("AspectRatio/String"))).trimmed());

         _mediaInfo.insertVideoStream(videoStreamId, MediaInfo::VideoScanType, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("ScanType"))).trimmed());

        int width = QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("Width"))).trimmed().toInt();
        int height = QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("Height"))).trimmed().toInt();
        _mediaInfo.insertVideoStream(videoStreamId, MediaInfo::VideoResolution, QSize(width, height));

        _mediaInfo.insertVideoStream(videoStreamId, MediaInfo::VideoFrameRate, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("FrameRate"))).trimmed().toUInt());
        _mediaInfo.insertVideoStream(videoStreamId, MediaInfo::VideoFormat, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("Format"))).trimmed());
        _mediaInfo.insertVideoStream(videoStreamId, MediaInfo::VideoCodec, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("Codec/String"))).trimmed());
        _mediaInfo.insertVideoStream(videoStreamId, MediaInfo::VideoEncodedLibrary, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("Encoded_Library/String"))).trimmed());
        _mediaInfo.insertVideoStream(videoStreamId, MediaInfo::VideoDisplayAspectRatioString, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("DisplayAspectRatio/String"))).trimmed());


        qDebug() << QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("Codec/String")));
        qDebug() << QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("FrameRate/String"))).trimmed();

        qDebug() << QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("Format")));
        qDebug() << QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("Format/String")));

        qDebug() << QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("DisplayAspectRatio/String")));
        qDebug() << QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Video, videoStreamId,__T("DisplayAspectRatio")));

    }




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
    int textStreamCount = mediaInfo.Count_Get(MediaInfoLib::Stream_Text);
    for (int textStreamId = 0; textStreamId < textStreamCount; textStreamId++) {
        _mediaInfo.insertTextStream(textStreamId, MediaInfo::TextFormat, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Text, textStreamId,__T("Format"))).trimmed());
        _mediaInfo.insertTextStream(textStreamId, MediaInfo::TextLanguage, QString::fromStdWString(mediaInfo.Get(MediaInfoLib::Stream_Text, textStreamId,__T("Language/String2"))).trimmed());
    }

    AnalysisResult result;
    result.mediaInfo=_mediaInfo;
    return result;
}
