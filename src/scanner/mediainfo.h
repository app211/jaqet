#ifndef MEDIAINFO_H
#define MEDIAINFO_H


#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>
#include <QSize>

/**
 * Infos and other metadata associated with a media/track.
 *
 * MediaInfo is a "stupid" data structure that stores infos and metadata
 * associated with a media (e.g a track).
 * MediaInfo is empty by default and should be used in association with
 * MediaInfoFetcher that will fetch all the datas and put them into MediaInfo.
 *
 * Each item inside the playlist is a MediaInfo object.
 * Thus MediaInfo should be a simple and lightweight class.
 *
 * @see MediaInfoFetcher
 * @see PlaylistParser
 * @author Tanguy Krotoff
 */
class MediaInfo {
public:

    MediaInfo();

    /**
     * Constructs a MediaInfo given a filename or a URL.
     *
     * @param fileName a media filename or a URL
     */
    explicit MediaInfo(const QString & fileName);

    ~MediaInfo();

    bool operator==(const MediaInfo & mediaInfo) const;

    bool operator!=(const MediaInfo & mediaInfo) const;

    /** Resets this class. */
    void clear();

    bool isEmpty() const{
        return *this==MediaInfo();
    }

    //FIXME isEmpty() API?

    /**
     * FIXME remove/rename this
     * Tells if the metadata were fetched or not.
     */
    bool fetched() const;
    void setFetched(bool fetched);

    /** MediaInfo filename, can also be a URL. */
    QString fileName() const;
    /** Internal. */
    void setFileName(const QString & fileName);

    /** Helper function: determine if a given filename is a URL or not. */
    static bool isUrl(const QString & fileName);

    /** Gets the file type. */
    //FileType fileType() const;
   // void setFileType(FileType fileType);

    /**
     * Gets the size of the file in bytes (B).
     *
     * Stored inside a qint64 (signed 64bits).
     *
     * @return file size in Bytes or -1 if failed
     */
    qint64 fileSize() const;
    void setFileSize(qint64 bytes);

    /**
     * Returns the duration/length of the file in a nice formatted way (i.e 03:45:02).
     *
     * @see TkTime::convertMilliseconds()
     */
    QString durationFormatted() const;

    /**
     * Returns the duration/length of the file in seconds, or -1.
     *
     * Qt uses abreviation Secs for seconds cf
     * http://qt.nokia.com/doc/4.6/qtime.html#addSecs
     */
    qint64 durationSecs() const;

    /**
     * Returns the duration/length of the file in milliseconds, or -1.
     *
     * Qt uses abreviation MSecs for milliseconds cf
     * http://qt.nokia.com/doc/4.6/qtime.html#addMSecs
     * http://qt.nokia.com/doc/4.6/qtime.html#msecsTo
     */
    qint64 durationMSecs() const;

    /** Sets the duration/length of the file in seconds. */
    void setDurationSecs(qint64 seconds);

    /** Sets the duration/length of the file in milliseconds. */
    void setDurationMSecs(qint64 milliseconds);


    QString format() const;
    void setFormat(const QString & format);

     QString firstVideoCodec() const;
    void setFirstVideoCodec(const QString& videoCodec);

    void setALanguages(const QStringList& alanguages);
    QStringList alanguages() const;

    void  setTLanguages(const QStringList& tlanguages);
    QStringList  tlanguages() const ;

    void setVCodec(const QString& vcodec) { _vcodec = vcodec; }
    QString vcodec() const { return _vcodec; }

    void setVResolution(const QSize& vresolution) { _vresolution = vresolution; }
    QSize vresolution() const { return _vresolution; }

    void setVAspect(const QString& vaspect) { _vaspect = vaspect; }
    QString vaspect() const { return _vaspect; }

    void setVScanType(const QString& vscantype) { _vscantype = vscantype; }
    QString vscantype() const { return _vscantype; }

    void setVDisplayAspect(const QString& vdisplayaspect) { _vdisplayaspect = vdisplayaspect; }
    QString vdisplayaspect() const { return _vdisplayaspect; }

private:


    bool _fetched;
    QString _fileName;
    //FileType _fileType;
    qint64 _fileSize;

    /** Duration/length of the file in milliseconds. */
    qint64 _duration;

    QString _format;
    QStringList _alanguages;
    QStringList _tlanguages;
    QString _vcodec;
    QSize _vresolution;
    QString _vaspect;
    QString _vscantype;
    QString _vdisplayaspect;
};

Q_DECLARE_METATYPE(MediaInfo);
Q_DECLARE_METATYPE(QList<MediaInfo>);

#include <QtCore/QList>

/**
 * A list of MediaInfo.
 */
typedef QList<MediaInfo> MediaInfoList;

#endif	//MEDIAINFO_H
