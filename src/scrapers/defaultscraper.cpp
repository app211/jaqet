#include "defaultscraper.h"

#include "themoviedbscraper.h"
#include "allocinescraper.h"
#include "thetvdbscraper.h"

DefaultScraper::DefaultScraper(QObject *parent)
    : Scraper(parent)
{
    m_bannerScraper = new TheTVDBScraper(this);
    m_bannerScraper2 = new AlloCineScraper(this);

    connect(m_bannerScraper, SIGNAL(found(FilmPrtList)), this,
            SLOT(_found(FilmPrtList)));

    connect(m_bannerScraper, SIGNAL(found(ShowPtrList)), this,
            SLOT(_found(ShowPtrList)));

    connect(m_bannerScraper, SIGNAL(found(const Scraper*, SearchMovieInfo)), this,
            SLOT(_found(const Scraper*,SearchMovieInfo)));

    connect(m_bannerScraper, SIGNAL(found(const Scraper*, SearchEpisodeInfo)), this,
            SLOT(_found(const Scraper*,SearchEpisodeInfo)));


    scrapers.append(QPair<Scraper*,SearchFor>(m_bannerScraper,SearchOption::AllMedia));
    scrapers.append(QPair<Scraper*,SearchFor>(m_bannerScraper2,SearchOption::Information));

    scrapers2 = QList<QPair<Scraper*,SearchFor>>(scrapers);
}


QString DefaultScraper::createURL(const QString& command, const QMap<QString, QString>& params) const {
    return "";
}

void DefaultScraper::internalFindMovieInfo(QNetworkAccessManager* , const QString&, const SearchFor&, const QString&) {
    emit scraperError("Unsupported Operation");
}

void DefaultScraper::doFindEpisodeInfo() {

    if (scrapers.isEmpty()){
        emit found(this,this->searchEpisodeInfo);
    } else {
        QPair<Scraper*,SearchFor> pair= scrapers.takeFirst();
        currentScraper   = pair.first;
        currentSearchFor = pair.second;
        currentScraper->findEpisodeInfo(manager, showCode,  season,  episode, currentSearchFor);
    }
}

void DefaultScraper::internalFindEpisodeInfo(QNetworkAccessManager *manager, const QString& showCode, const int season, const int episode, const SearchFor& searchFor, const QString& language)  {
    this->showCode=showCode;
    this->season=season;
    this->episode=episode;
    this->searchFor=searchFor;
    this->language=language;
    this->manager=manager;
    this->searchEpisodeInfo=SearchEpisodeInfo();

    doFindEpisodeInfo();
}

void DefaultScraper::internalSearchFilm(QNetworkAccessManager* manager, const QString& toSearch, const QString& language, int year) const {
    emit scraperError("Unsupported Operation");
}

void DefaultScraper::internalSearchTV(QNetworkAccessManager* manager, const QString& toSearch, const QString& language) const{
    m_bannerScraper->searchTV(manager,toSearch);
}

QString DefaultScraper::getBestImageUrl(const QString& url, const QSize& originalSize, const QSize& size,  Qt::AspectRatioMode mode, ImageType imageType) const {
    for ( QPair<Scraper*,SearchFor> pair : scrapers2){
        Scraper* scraper    = pair.first;
        SearchFor searchFor = pair.second;

        if (imageType==ImageType::BANNER && (searchFor & SearchOption::Banner)){
            return scraper->getBestImageUrl(url,originalSize,size,mode,imageType);
        } else if (imageType==ImageType::POSTER && (searchFor & SearchOption::Poster)){
            return scraper->getBestImageUrl(url,originalSize,size,mode,imageType);
        } else if (imageType==ImageType::BACKDROP && (searchFor & SearchOption::BackDrop)){
            return scraper->getBestImageUrl(url,originalSize,size,mode,imageType);
        }
    }

    return "";
}

bool DefaultScraper::haveCapability(const SearchCapabilities capability) const{
    return true;
}

void DefaultScraper::_searchScraper(){

}

void DefaultScraper::_found(FilmPrtList result){

}

void DefaultScraper::_found(ShowPtrList shows){
    emit found(shows);
}

void DefaultScraper::_found(const Scraper* scraper,SearchMovieInfo films) {


}

void DefaultScraper::_found(const Scraper* scraper,SearchEpisodeInfo shows) {
    if (this->currentSearchFor & SearchOption::Poster){
        this->searchEpisodeInfo.postersHref=shows.postersHref;
        this->searchEpisodeInfo.postersSize=shows.postersSize;
    }

    if (this->currentSearchFor & SearchOption::BackDrop){
        this->searchEpisodeInfo.backdropsHref=shows.backdropsHref;
        this->searchEpisodeInfo.backdropsSize=shows.backdropsSize;
    }

    if (this->currentSearchFor & SearchOption::Banner){
        this->searchEpisodeInfo.bannersHref=shows.bannersHref;
    }

    if (this->currentSearchFor & SearchOption::Information){
        this->searchEpisodeInfo.actors=shows.actors;
    }

    doFindEpisodeInfo();
}

const uchar DefaultScraper::icon_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff,
    0x61, 0x00, 0x00, 0x00, 0x04, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0x08, 0x7c, 0x08, 0x64,
    0x88, 0x00, 0x00, 0x01, 0xe4, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0x8d, 0x92, 0x3f, 0x68, 0x13,
    0x51, 0x1c, 0xc7, 0x3f, 0x8d, 0xd5, 0xb6, 0x90, 0xa4, 0x05, 0x9d, 0x72, 0x8d, 0xcd, 0xa0, 0x46,
    0xa3, 0x21, 0x14, 0x12, 0x48, 0x28, 0x92, 0xc3, 0xa5, 0xba, 0x84, 0x0e, 0x59, 0x25, 0x08, 0x2e,
    0x4e, 0xe2, 0xe0, 0xe4, 0x12, 0xb0, 0x83, 0x53, 0x5a, 0x5c, 0x0a, 0x52, 0x4b, 0x40, 0xa7, 0x0a,
    0x72, 0x2a, 0x0e, 0x59, 0xae, 0x29, 0x82, 0x2d, 0x5a, 0x24, 0x0e, 0xa9, 0x71, 0x10, 0xee, 0x34,
    0x77, 0x20, 0x14, 0x4c, 0x9a, 0x98, 0x8a, 0x42, 0x9f, 0x43, 0x7a, 0x4f, 0x53, 0x0e, 0xcf, 0x1f,
    0x3c, 0x78, 0xff, 0x7e, 0x9f, 0xdf, 0xf7, 0xfb, 0x7e, 0x6f, 0x88, 0x70, 0x5c, 0xf0, 0x9f, 0x71,
    0xef, 0xae, 0xc2, 0xe6, 0x9b, 0xef, 0x68, 0x2f, 0x5a, 0x72, 0x6f, 0xd8, 0x2b, 0x49, 0x09, 0x1d,
    0xe5, 0xda, 0xd5, 0x13, 0x00, 0xa8, 0x17, 0x03, 0x44, 0xcf, 0x8c, 0x72, 0x21, 0x36, 0x46, 0x6f,
    0x6f, 0x9f, 0xd2, 0xfd, 0xaf, 0xf8, 0xbc, 0x00, 0x96, 0xfd, 0x8b, 0xd7, 0x9b, 0x5d, 0xae, 0xcc,
    0x06, 0x19, 0x19, 0x19, 0x22, 0x72, 0xf2, 0x18, 0xd3, 0x89, 0x31, 0x9e, 0x6a, 0xdf, 0x00, 0xfa,
    0x80, 0xc8, 0x64, 0x88, 0xea, 0xea, 0x0a, 0xc5, 0x5b, 0x37, 0x5c, 0x21, 0xfa, 0x7a, 0x87, 0xf5,
    0x57, 0x5d, 0xb9, 0x5e, 0x2e, 0xef, 0x60, 0x7c, 0xfe, 0xf9, 0x17, 0x20, 0xac, 0x90, 0x4d, 0x27,
    0x51, 0x33, 0x29, 0x57, 0x80, 0xcf, 0x07, 0xe7, 0xce, 0x8e, 0x72, 0xfb, 0x8e, 0xc5, 0x83, 0x95,
    0x1d, 0x66, 0xd2, 0x7e, 0x79, 0x76, 0x24, 0x72, 0x7e, 0xba, 0xa8, 0x66, 0x52, 0x5c, 0x56, 0x67,
    0x68, 0xef, 0x76, 0x68, 0x7c, 0x32, 0x88, 0x84, 0x15, 0x8c, 0xa6, 0x2d, 0x81, 0xfb, 0xa2, 0xc7,
    0xf3, 0x97, 0x2d, 0xb6, 0x3f, 0xfc, 0x60, 0xeb, 0x5d, 0x8f, 0xbd, 0xde, 0x71, 0x4e, 0x4d, 0x9d,
    0xc6, 0x68, 0xda, 0xb0, 0xf8, 0xf0, 0xb1, 0x70, 0x8b, 0xb9, 0xeb, 0x37, 0x85, 0x10, 0x42, 0xd4,
    0xea, 0x0d, 0x41, 0x38, 0x3e, 0x30, 0x8c, 0xa6, 0x25, 0x84, 0x10, 0x82, 0x70, 0x5c, 0x0c, 0x6b,
    0x15, 0x9d, 0x89, 0x60, 0x80, 0x42, 0x3e, 0x87, 0x69, 0xd9, 0x94, 0x57, 0x9f, 0x01, 0xa0, 0x55,
    0x74, 0x4c, 0xcb, 0x26, 0x11, 0x8b, 0x12, 0x99, 0x0c, 0xf5, 0xab, 0x1d, 0xbc, 0xd7, 0x94, 0x12,
    0xe2, 0xfd, 0xf6, 0xc7, 0xbe, 0xbd, 0xea, 0xc6, 0x5b, 0xca, 0x4f, 0xfa, 0x49, 0xc6, 0x17, 0x9b,
    0xe2, 0xc2, 0x12, 0xc5, 0x85, 0xa5, 0x03, 0xc8, 0x1a, 0x00, 0x73, 0xb3, 0x97, 0xa4, 0x67, 0x67,
    0xee, 0xe4, 0xfc, 0xb3, 0x8d, 0xce, 0x25, 0x37, 0x80, 0x56, 0xd1, 0x01, 0x8f, 0x8f, 0x54, 0xab,
    0x37, 0x30, 0x2d, 0x9b, 0x6c, 0x3a, 0xc9, 0x44, 0x30, 0x00, 0x40, 0x36, 0x9d, 0xc4, 0xb4, 0x6c,
    0x69, 0xc9, 0xf3, 0x23, 0x55, 0x37, 0xb6, 0x64, 0xe5, 0x3f, 0xd5, 0xd7, 0xe4, 0xb9, 0x27, 0xc0,
    0x91, 0xaa, 0x66, 0x52, 0xb2, 0xad, 0x8e, 0x35, 0x69, 0xa1, 0x56, 0x6f, 0x00, 0x48, 0x99, 0x87,
    0x01, 0xed, 0x4e, 0x57, 0x56, 0x37, 0x2d, 0x5b, 0xde, 0x97, 0x0a, 0x5a, 0xbb, 0x1d, 0xda, 0x9d,
    0x2e, 0x89, 0x58, 0x94, 0x72, 0x69, 0x9e, 0x72, 0x69, 0x7e, 0x00, 0xa6, 0x55, 0x74, 0xc6, 0x03,
    0x7e, 0xc6, 0x03, 0xfe, 0x01, 0xf9, 0x03, 0x16, 0x16, 0x97, 0x1f, 0x01, 0x50, 0xc8, 0xe7, 0x28,
    0xe4, 0x73, 0xae, 0x36, 0x0e, 0xcb, 0x07, 0xf8, 0x0d, 0x41, 0xe5, 0xda, 0x32, 0x93, 0x20, 0x23,
    0x88, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82} ;

static QIcon m_icon;

QIcon DefaultScraper::getIcon() const {
    if (m_icon.isNull()){
        QPixmap pixmap;
        if (pixmap.loadFromData(DefaultScraper::icon_png, sizeof(DefaultScraper::icon_png)/sizeof(uchar))){
            m_icon=QIcon(pixmap);
        }
    }

    return m_icon;
}


QString DefaultScraper::getName() const {
    return "Default";
}