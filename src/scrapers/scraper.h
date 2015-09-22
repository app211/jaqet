#ifndef SCRAPER_H
#define SCRAPER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QList>
#include <QStringList>
#include <QSize>
#include <QSharedPointer>
#include <QIcon>
#include <QDateTime>
#include <QFileInfo>
#include "scanner/mediainfo.h"

class QNetworkAccessManager;

enum class ImageType {
    Banner = 1,
    Poster = 2,
    Backdrop = 4,
    Thumbnail = 8,
    All=Banner | Poster | Backdrop | Thumbnail
};

class Film {
public:
    QString code;
    QString originalTitle;
    QString title;
    QString productionYear;
    QString posterHref;

};

class Show {
public:
    QString code;
    QString originalTitle;
    QString title;
    QString productionYear;
    QString posterHref;
};

typedef QSharedPointer<Film> FilmPtr;
typedef QList<FilmPtr> FilmPrtList;
typedef QSharedPointer<Show> ShowPtr;
typedef QList<ShowPtr> ShowPtrList;


class Scraper;

struct FoundResult {

    FoundResult() {
    }

    FoundResult(Scraper *scraper, const FilmPtr& filmPtr, const QString& language)
        :
          scraper(scraper),
          filmPtr(filmPtr), season(-1)
        ,episode(-1),m_language(language)
    {
    }

    FoundResult(Scraper *scraper, const ShowPtr& showPtr, const int season, const int episode, const QString& language)
        : scraper(scraper),
          showPtr( showPtr)
        , season(season)
        ,episode(episode),m_language(language)
    {
    }

    bool isNull() const {
        return showPtr.isNull() && filmPtr.isNull();
    }

    bool isTV() const {
        return !showPtr.isNull();
    }

    QString getCode() const {
        return isNull()?"":(isTV()?showPtr->code:filmPtr->code);
    }

    QString getOriginalTitle() const {
        return isNull()?"":(isTV()?showPtr->originalTitle:filmPtr->originalTitle);

    }

    QString getTitle() const {
        return isNull()?"":(isTV()?showPtr->title:filmPtr->title);

    }
    Scraper *getScraper() const {
        return scraper;
    }

    int getSeason() const{
        return isTV()?season:-1;
    }

    int getEpisode() const{
        return  isTV()?episode:-1;
    }

    QString getProductionYear() const{
        return isNull()?"":(isTV()?showPtr->productionYear:filmPtr->productionYear);

    }

    QString getPosterHref() const {
        return isNull()?QString::null:(isTV()?showPtr->posterHref:filmPtr->posterHref);
    }

    QString language() const{
        return m_language;
    }


    Scraper *scraper;
    int season;
    int episode;
    ShowPtr showPtr;
    FilmPtr filmPtr;
    QString m_language;
};


enum class NETRESOURCE {
    UNKNOWN, NONE, ERROR, OK, CANCELED
};


class Scraper;

class ScraperResource {
    QString _resources;
    QSize _originalSize;
    const Scraper* _scraper;

public:
    ScraperResource(const QString& resources, const QSize& originalSize, const Scraper* scraper)
        : _resources(resources),
          _originalSize(originalSize),
          _scraper(scraper){

    }

    ScraperResource():_scraper(nullptr){
    }

    const QString resources() const {
        return _resources;

    }
    const QSize originalSize() const {
        return _originalSize;

    }

    const Scraper* scraper() const{
        return _scraper;
    }


    bool isNull() const{
        return _resources.isEmpty() || _scraper==nullptr;
    }
};


Q_DECLARE_METATYPE(ScraperResource)

class Engine;

struct media {
    QString href;
    QSize size;
    QString language;
    float rating;
};

template< typename P>
class MediaSearch
{
public:

    void setTitle(const QString& title) { d->title = title; }
    QString title() const { return d->title; }

    void setSynopsis(const QString& synopsis) { d->synopsis = synopsis; }
    QString synopsis() const { return d->synopsis; }

    void setGenre(const QStringList& genre) { d->genre = genre; }
    QStringList genre() const { return d->genre; }

    void setActors(const QStringList& actors) { d->actors = actors; }
    QStringList actors() const { return d->actors; }


    void setDirectors(const QStringList& directors) { d->directors = directors; }
    QStringList directors() const { return d->directors; }

    void setLinkName(const QString& linkName) { d->linkName = linkName; }
    QString linkName() const { return d->linkName; }

    void setLinkHref(const QString& linkHref) { d->linkHref = linkHref; }
    QString linkHref() const { return d->linkHref; }

    QStringList backdropsHref() const { return d->backdropsHref; }
    QList<QSize> backdropsSize() const { return d->backdropsSize; }
    QStringList thumbailHref() const { return d->thumbailHref; }
    QList<QSize> thumbailSize() const { return d->thumbailSize; }
    QList<media> posters() const { return d->posters; }

    void addPoster(const QString& posterPath, const QSize& posterSize=QSize(), const QString language=QString::null, float rating=10.f){
        if (!posterPath.isEmpty()){
            if (rating<0. || rating>10.){
                rating=10.;
            }

            media m;
            m.href=posterPath;
            m.language=language;
            m.size=posterSize;
            m.rating=rating;

            d->posters.append(m);
        }
    }

    void addBackdrop(const QString& backdropPath, const QSize& backdropSize=QSize(), const QString language=QString::null, float rating=10.f){
        Q_UNUSED(language)
        Q_UNUSED(rating)

        if (!backdropPath.isEmpty()){
            d->backdropsHref.append(backdropPath);
            d->backdropsSize.append(backdropSize);
        }
    }


    void addThumbail(const QString& thumbailPath, const QSize& thumbailSize=QSize(), const QString language=QString::null, float rating=10.f){
        Q_UNUSED(language)
        Q_UNUSED(rating)

        if (!thumbailPath.isEmpty()){
            d->thumbailHref.append(thumbailPath);
            d->thumbailSize.append(thumbailSize);
        }
    }

    FoundResult foundResult() const{
        return d->fd;
    }

    Engine* engine() const{
        return d->engine;
    }

    MediaInfo mediaInfo() const{
        return d->mediaInfo;
    }

    QFileInfo fileInfo() const{
        return d->fileInfo;
    }

public:
    QSharedDataPointer< P > d;
};

class MediaMovieSearchPrivate  : public QSharedData {
public:
    MediaMovieSearchPrivate()  { }

    MediaMovieSearchPrivate(const MediaMovieSearchPrivate &other)
        : QSharedData(other),
          title(other.title),
          originalTitle(other.originalTitle),
          synopsis(other.synopsis),
          backdropsHref(other.backdropsHref),
          backdropsSize(other.backdropsSize),
          posters(other.posters),
          thumbailHref(other.thumbailHref),
          thumbailSize(other.thumbailSize),
          linkName(other.linkName),
          linkHref(other.linkHref),
          productionYear(other.productionYear),
          actors(other.actors),
          runtimeInSec(other.runtimeInSec),
          rating(other.rating),
          directors(other.directors),
          genre(other.genre),
          engine(other.engine),
          fileInfo(other.fileInfo),
          mediaInfo(other.mediaInfo),
          fd(other.fd),
          countries(other.countries),
          tagLine(other.tagLine),
          certificate(other.certificate)
    { }

    ~MediaMovieSearchPrivate() { }


    QString title;
    QString originalTitle;
    QString synopsis;
    QStringList backdropsHref;
    QList<QSize> backdropsSize;
    QList<media> posters;
    QStringList thumbailHref;
    QList<QSize> thumbailSize;
    QString linkName;
    QString linkHref;
    int productionYear=0;
    QStringList actors;
    int runtimeInSec=0;
    double rating=-1.;
    QStringList directors;
    QStringList countries;
    QStringList genre;
    QFileInfo fileInfo;
    MediaInfo mediaInfo;
    FoundResult fd;
    Engine* engine=nullptr;
    QString tagLine;
    QString certificate;
};

class MediaMovieSearch : public MediaSearch< MediaMovieSearchPrivate > {

public:

    void setOriginalTitle(const QString& originalTitle) { d->originalTitle = originalTitle; }
    QString originalTitle() const { return d->originalTitle; }

    void setProductionYear( int productionYear) { d->productionYear=productionYear; }
    int productionYear() const { return d->productionYear;}

    void setRuntimeInSec( int runtimeInSec) { d->runtimeInSec=runtimeInSec; }
    int runtimeInSec() const { return d->runtimeInSec;}

    void setRating( double rating) { d->rating=rating; }
    double rating() const { return d->rating;}

    void setCountries(const QStringList& countries) { d->countries = countries; }
    QStringList countries() const { return d->countries; }

    void setTagLine(const QString& tagLine) { d->tagLine = tagLine; }
    QString tagLine() const { return d->tagLine; }

    void setCertificate(const QString& certificate) { d->certificate = certificate; }
    QString certificate() const { return d->certificate; }

public :
    MediaMovieSearch( const QFileInfo& fileInfo, const MediaInfo& mediaInfo, const FoundResult& fd, Engine* engine)
    {
        d = new MediaMovieSearchPrivate;
        d->engine=engine;
        d->fileInfo=fileInfo;
        d->mediaInfo=mediaInfo;
        d->fd=fd;
    }

    MediaMovieSearch(const MediaMovieSearch &other)
    {
        d=other.d;
    }


};

typedef QSharedPointer<MediaMovieSearch> MediaMovieSearchPtr;

class MediaTVSearchPrivate  : public QSharedData {
public:
    MediaTVSearchPrivate():
        productionYear(0),
        season(-1),
        episode(-1),
        runtimeInSec(0),
        showRating(-1),
        seasonRating(-1),
        episodeRating(-1),
        engine(nullptr)
    { }

    MediaTVSearchPrivate( const QFileInfo& fileInfo, const MediaInfo& mediaInfo, const FoundResult& fd, Engine* engine)
        : MediaTVSearchPrivate(){
        this->engine=engine;
        this->fileInfo=fileInfo;
        this->mediaInfo=mediaInfo;
        this->fd=fd;
    }

    MediaTVSearchPrivate(const MediaTVSearchPrivate &other)
        : QSharedData(other),
          code(other.code),
          title(other.title),
          originalTitle(other.originalTitle),
          episodeTitle(other.episodeTitle),
          originalEpisodeTitle(other.originalEpisodeTitle),
          synopsis(other.synopsis),
          backdropsHref(other.backdropsHref),
          backdropsSize(other.backdropsSize),
          posters(other.posters),
          bannersHref(other.bannersHref),
          bannersSize(other.bannersSize),
          thumbailHref(other.thumbailHref),
          thumbailSize(other.thumbailSize),
          linkName(other.linkName),
          linkHref(other.linkHref),
          season(other.season),
          episode(other.episode),
          showTitle(other.showTitle),
          originalShowTitle(other.originalShowTitle),
          productionYear(other.productionYear),
          actors(other.actors),
          runtimeInSec(other.runtimeInSec),
          showRating(other.showRating),
          seasonRating(other.seasonRating),
          episodeRating(other.episodeRating),
          directors(other.directors),
          network(other.network),
          aired(other.aired),
          genre(other.genre),
          engine(other.engine),
          fileInfo(other.fileInfo),
          mediaInfo(other.mediaInfo),
          fd(other.fd)
    { }

    ~MediaTVSearchPrivate() { }

    QString code;
    QString title;
    QString originalTitle;
    QString episodeTitle;
    QString originalEpisodeTitle;
    QString synopsis;
    QStringList backdropsHref;
    QList<QSize> backdropsSize;
    QList<media> posters;
    QStringList bannersHref;
    QList<QSize> bannersSize;
    QStringList thumbailHref;
    QList<QSize> thumbailSize;
    QString linkName;
    QString linkHref;
    QString showTitle;
    QString originalShowTitle;
    QStringList actors;
    int productionYear=0;
    int season=-1;
    int episode=-1;
    int runtimeInSec=0;
    double showRating=-1.;
    double seasonRating=-1.;
    double episodeRating=-1.;
    QStringList directors;
    QStringList network;
    QDateTime aired;
    QStringList genre;
    QFileInfo fileInfo;
    MediaInfo mediaInfo;
    FoundResult fd;
    Engine* engine;
};

class MediaTVSearch : public MediaSearch< MediaTVSearchPrivate > {

public:
    void setShowRating( double showRating) { d->showRating=showRating; }
    double showRating() const { return d->showRating;}

    void setSeasonRating( double seasonRating) { d->seasonRating=seasonRating; }
    double seasonRating() const { return d->seasonRating;}

    void setEpisodeRating( double episodeRating) { d->episodeRating=episodeRating; }
    double episodeRating() const { return d->episodeRating;}

    void setCode(const QString& code) { d->code = code; }
    QString code() const { return d->code; }

    void setOriginalTitle(const QString& originalTitle) { d->originalTitle = originalTitle; }
    QString originalTitle() const { return d->originalTitle; }

    void setEpisodeTitle(const QString& episodeTitle) { d->episodeTitle = episodeTitle; }
    QString episodeTitle() const { return d->episodeTitle; }

    void setOriginalEpisodeTitle(const QString& originalEpisodeTitle) { d->originalEpisodeTitle = originalEpisodeTitle; }
    QString originalEpisodeTitle() const { return d->originalEpisodeTitle; }

    void setSeason( int season) { d->season=season; }
    int season() const { return d->season;}

    void setEpisode( int episode) { d->episode=episode; }
    int episode() const { return d->episode;}

    void setAired(const QDateTime& aired) { d->aired = aired; }
    QDateTime aired() const { return d->aired; }

    QStringList networks() const { return d->network; }
    void addNetwork(const QString& network)
    { if (!network.isEmpty() && !d->network.contains(network)) d->network.append(network) ;}

    void setProductionYear( int productionYear) { d->productionYear=productionYear; }
    int productionYear() const { return d->productionYear;}

    void setRuntimeInSec( int runtimeInSec) { d->runtimeInSec=runtimeInSec; }
    int runtimeInSec() const { return d->runtimeInSec;}

    QStringList bannersHref() const { return d->bannersHref; }
    QList<QSize> bannersSize() const { return d->bannersSize; }

    void addBanner(const QString& bannerPath, const QSize& bannerSize=QSize()){
        if (!bannerPath.isEmpty()){
            d->bannersHref.append(bannerPath);
            d->bannersSize.append(bannerSize);
        }
    }


public :

    MediaTVSearch( const QFileInfo& fileInfo, const MediaInfo& mediaInfo, const FoundResult& fd, Engine* engine)
    {
        d =new MediaTVSearchPrivate(fileInfo, mediaInfo, fd, engine);
    }

    MediaTVSearch(const MediaTVSearch &other)

    {
        d=other.d;
    }


};

typedef QSharedPointer<MediaTVSearch> MediaTVSearchPtr;

class CurrentItemData {
private:
    class CurrentItemDataPrivate  : public QSharedData {
    public:
        CurrentItemDataPrivate()  { }

        CurrentItemDataPrivate(Engine* engine, bool tv, const QFileInfo& fileInfo, const MediaInfo& mediaInfo){
            this->engine=engine;
            this->tv=tv;
            this->fileInfo=fileInfo;
            this->mediaInfo = mediaInfo;
        }

        CurrentItemDataPrivate(const CurrentItemDataPrivate &other)
            : QSharedData(other),
              code(other.code),
              title(other.title),
              originalTitle(other.originalTitle),
              episodeTitle(other.episodeTitle),
              originalEpisodeTitle(other.originalEpisodeTitle),
              synopsis(other.synopsis),
              linkName(other.linkName),
              linkHref(other.linkHref),
              season(other.season),
              episode(other.episode),
              year(other.year),
              actors(other.actors),
              runtimeInSec(other.runtimeInSec),
              rating(other.rating),
              showRating(other.showRating),
              seasonRating(other.seasonRating),
              episodeRating(other.episodeRating),
              directors(other.directors),
              network(other.network),
              aired(other.aired),
              genre(other.genre),
              vcodec(other.vcodec),
              vresolution(other.vresolution),
              vaspect(other.vaspect),
              vdisplayaspect(other.vdisplayaspect),
              vdurationSecs(other.vdurationSecs),
              vscantype(other.vscantype),
              achannelsCount(other.achannelsCount) ,
              acodec(other.acodec),
              format(other.format),
              alanguages(other.alanguages),
              tv(other.tv),
              tlanguages(other.tlanguages),
              engine(other.engine),
              fileInfo(other.fileInfo),
              countries(other.countries),
              tagLine(other.tagLine),
              certificate(other.certificate),
              mediaInfo(other.mediaInfo)

        { }

        ~CurrentItemDataPrivate() { }

        QString code;
        QString title;
        QString originalTitle;
        QString episodeTitle;
        QString originalEpisodeTitle;
        QString synopsis;
        QString linkName;
        QString linkHref;
        int season=-1;
        int episode=-1;
        int year=0;
        QStringList actors;
        int runtimeInSec=0;
        double rating=-1.;
        QStringList directors;
        QStringList network;
        QDateTime aired;
        QStringList genre;
        QString vcodec;
        QString acodec;
        QString format;
        QSize vresolution;
        QString vaspect;
        QString vdisplayaspect;
        QString vscantype;
        int vdurationSecs=0;
        int achannelsCount=-1;
        QStringList alanguages;
        bool tv=false;
        QStringList tlanguages;
        double showRating=-1.;
        double seasonRating=-1.;
        double episodeRating=-1.;
        QFileInfo fileInfo;
        Engine* engine=nullptr;
        QStringList countries;
        QString tagLine;
        QString certificate;
        MediaInfo mediaInfo;
    };

    QSharedDataPointer<CurrentItemDataPrivate> d;

public:

    CurrentItemData(){

    }

    CurrentItemData(Engine* engine, bool tv, const MediaInfo& mediaInfo, const QFileInfo& fileInfo)
        : d(new CurrentItemDataPrivate(engine,tv,fileInfo,mediaInfo))
    {
        if (!mediaInfo.isEmpty()){
            setVDurationSecs(mediaInfo.durationSecs());
            setVCodec(mediaInfo.vcodec());
            setVResolution(mediaInfo.vresolution());
            setVAspect(mediaInfo.vaspect());
            setVScanType(mediaInfo.vscantype());
            setFormat(mediaInfo.format());
            setALanguages(mediaInfo.alanguages());
            setTLanguages(mediaInfo.tlanguages());
            setVDisplayAspect(mediaInfo.vdisplayaspect());
            setACodec(mediaInfo.acodec());
            setAChannelsCount(mediaInfo.aChannelsCount());
        }
    }

    CurrentItemData(const CurrentItemData &other)
        : d (other.d)
    {
    }

    QFileInfo fileInfo() const{
        return d->fileInfo;
    }

    MediaInfo mediaInfo() const{
        return d->mediaInfo;
    }

    ScraperResource _poster;
    NETRESOURCE posterState=NETRESOURCE::UNKNOWN;
    QPixmap currentPoster;

    ScraperResource _backdrop;
    NETRESOURCE backdropState=NETRESOURCE::UNKNOWN;
    QPixmap currentBackdrop;

    ScraperResource _banner;
    NETRESOURCE bannerState=NETRESOURCE::UNKNOWN;
    QPixmap currentBanner;

    ScraperResource _thumbnail;
    NETRESOURCE thumbnailState=NETRESOURCE::UNKNOWN;
    QPixmap currentThumbail;


private:
    QPixmap getMedia(NETRESOURCE mediaState, const QPixmap& media) const {
        if (mediaState == NETRESOURCE::OK){
            return media;
        }

        return QPixmap();
    }

public:
    QPixmap getBackdrop() const {
        return getMedia(backdropState,currentBackdrop);
    }

    QPixmap getThumbail() const {
        return getMedia(thumbnailState,currentThumbail);
    }

    QPixmap getBanner() const {
        return getMedia(bannerState,currentBanner);
    }

    QPixmap getPoster() const {
        return getMedia(posterState,currentPoster);
    }

    void setSynopsis(const QString& synopsis) { d->synopsis = synopsis; }
    QString synopsis() const { return d->synopsis; }

    void setActors(const QStringList& actors) { d->actors = actors; }
    QStringList actors() const { return d->actors; }

    void setVCodec(const QString& vcodec) { d->vcodec = vcodec; }
    QString vcodec() const { return d->vcodec; }

    void setVResolution(const QSize& vresolution) { d->vresolution = vresolution; }
    QSize vresolution() const { return d->vresolution; }

    void setVAspect(const QString& vaspect) { d->vaspect = vaspect; }
    QString vaspect() const { return d->vaspect; }

    void setVDisplayAspect(const QString& vdisplayaspect) { d->vdisplayaspect = vdisplayaspect; }
    QString vdisplayaspect() const { return d->vdisplayaspect; }

    void setVScanType(const QString& vscantype) { d->vscantype = vscantype; }
    QString vscantype() const { return d->vscantype; }

    void setRuntimeInSec( int runtimeInSec) { d->runtimeInSec=runtimeInSec; }
    int runtimeInSec() const { return d->runtimeInSec;}

    void setVDurationSecs( int vdurationSecs) { d->vdurationSecs=vdurationSecs; }
    int vdurationSecs() const { return d->vdurationSecs;}

    void setYear( int year) { d->year=year; }
    int year() const { return d->year;}

    void setDirectors(const QStringList& directors) { d->directors = directors; }
    QStringList directors() const { return d->directors; }

    void setTitle(const QString& title) { d->title = title; }
    QString title() const { return d->title; }

    void setOriginalTitle(const QString& originalTitle) { d->originalTitle = originalTitle; }
    QString originalTitle() const { return d->originalTitle; }

    void setTagLine(const QString& tagLine) { d->tagLine = tagLine; }
    QString tagLine() const { return d->tagLine; }

    void setEpisodeTitle(const QString& episodeTitle) { d->episodeTitle = episodeTitle; }
    QString episodeTitle() const { return d->episodeTitle; }

    void setOriginalEpisodeTitle(const QString& originalEpisodeTitle) { d->originalEpisodeTitle = originalEpisodeTitle; }
    QString originalEpisodeTitle() const { return d->originalEpisodeTitle; }

    void setRating( double rating) { d->rating=rating; }
    double rating() const { return d->rating;}

    void setShowRating( double showRating) { d->showRating=showRating; }
    double showRating() const { return d->showRating;}

    void setSeasonRating( double seasonRating) { d->seasonRating=seasonRating; }
    double seasonRating() const { return d->seasonRating;}

    void setEpisodeRating( double episodeRating) { d->episodeRating=episodeRating; }
    double epidodeRating() const { return d->episodeRating;}

    void setAired(const QDateTime& aired) { d->aired = aired; }
    QDateTime aired() const { return d->aired; }

    void setGenre(const QStringList& genre) { d->genre = genre; }
    QStringList genre() const { return d->genre; }

    QStringList networks() const { return d->network; }
    void setNetworks(const QStringList& network) { d->network = network; }

    void setSeason( int season) { d->season=season; }
    int season() const { return d->season;}

    void setEpisode( int episode) { d->episode=episode; }
    int episode() const { return d->episode;}

    void setAChannelsCount( int achannelsCount) { d->achannelsCount=achannelsCount; }
    int achannelsCount() const { return d->achannelsCount;}

    void setACodec(const QString& acodec) { d->acodec = acodec; }
    QString acodec() const { return d->acodec; }

    void setFormat(const QString& format) { d->format = format; }
    QString format() const { return d->format; }

    void setALanguages(const QStringList& alanguages) { d->alanguages = alanguages; }
    QStringList alanguages() const { return d->alanguages; }

    void setTLanguages(const QStringList& tlanguages) { d->tlanguages = tlanguages; }
    QStringList tlanguages() const { return d->tlanguages; }

    bool isTV() const { return d->tv; }
    Engine* engine() const { return d->engine; }

    void setCountries(const QStringList& countries) { d->countries = countries; }
    QStringList countries() const { return d->countries; }

    void setCertificate(const QString& certificate) { d->certificate = certificate; }
    QString certificate() const { return d->certificate; }

};

class Scraper : public QObject
{
    Q_OBJECT

public:

    enum SearchCapabilitie {
        Movie = 0x0001,
        TV = 0x0002
    };

    enum SearchOption {
        Information = 1,
        Poster = 2,
        BackDrop = 4,
        Banner = 8,
        Thumbnail = 16,
        AllMedia = Poster | BackDrop | Banner | Thumbnail,
        All = Information | AllMedia

    };

    Q_DECLARE_FLAGS(SearchFor, SearchOption)
    Q_DECLARE_FLAGS(SearchCapabilities, SearchCapabilitie)

    Scraper(QObject *parent=0);

    virtual bool haveCapability(const SearchCapabilities ) const=0;
    virtual bool supportLanguage(const QString& languageCodeISO639) const=0;

    virtual QIcon icon() const=0;
    virtual QString name() const = 0;

    virtual QString createURL(const QString& , const QMap<QString, QString>& params) const=0;

    void searchFilm(QNetworkAccessManager* manager, const QString& toSearch, int year, const QString &language) const;
    void searchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString &language) ;
    void findMovieInfo(QNetworkAccessManager *manager, MediaMovieSearchPtr mediaMovieSearchPtr, const SearchFor& searchFor, const QString &language) ;
    void findEpisodeInfo(QNetworkAccessManager *manager, MediaTVSearchPtr mediaTVSearchPtr, const SearchFor& searchFor, const QString &language) ;

    virtual QString getBestImageUrl(const QString& filePath, const QSize& originalSize, const QSize& size, Qt::AspectRatioMode mode=Qt::KeepAspectRatio,QFlags<ImageType> imageType=ImageType::All) const=0;



protected:
    virtual void internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year) const=0;
    virtual void internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const=0;
    virtual void internalFindMovieInfo(QNetworkAccessManager *manager, MediaMovieSearchPtr mediaMovieSearchPtr, const SearchFor& searchFor, const QString& language) =0;
    virtual void internalFindEpisodeInfo(QNetworkAccessManager *manager, MediaTVSearchPtr mediaTVSearchPtr, const SearchFor& searchFor, const QString& language) =0;

public slots:
    void closeDialog();
    void showErrorDialog(const QString& error=QString::null);


signals:

    void canceled() const;
    void scraperError() const;
    void scraperError(const QString& error) const;
    void found(FilmPrtList films) const;
    void found(ShowPtrList shows) const;
    void found(const Scraper* scraper,MediaMovieSearchPtr film) const;
    void found(const Scraper* scraper,MediaTVSearchPtr show) const;
    void progress(const QString& progressInfo) const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Scraper::SearchFor)


#endif // SCRAPER_H
