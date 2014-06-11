TARGET = jaqet

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

QT       += core gui network xml widgets

win32 {
INCLUDEPATH += C:\bin\MediaInfo_DLL_0.7.69_Windows_i386_WithoutInstaller\Developers\Source
LIBS += -LC:\bin\MediaInfo_DLL_0.7.69_Windows_i386_WithoutInstaller -lMediaInfo
}

linux-g++: {
    LIBS += -lmediainfo
    DEFINES += UNICODE
}

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/filedownloader.cpp \
    src/template/templateyadis.cpp \
    src/scrapers/allocinescraper.cpp \
    src/scrapers/scraper.cpp \
    src/scrapers/themoviedbscraper.cpp \
     src/fileparser.cpp \
    src/scrapers/thetvdbscraper.cpp \
    src/scrapers/cncclassificationwrapper.cpp \
    src/promise.cpp \
    src/searchscraperdialog.cpp \
    src/engine.cpp \
    src/tvixengine.cpp \
    src/scanner/filenamescanner.cpp \
    src/scanner/scanner.cpp \
    src/chooseitemdialog.cpp \
    src/inprogressdialog.cpp \
    src/scanner/mediainfoscanner.cpp \
    src/scanner/mediainfo.cpp \
    src/panelview.cpp \
    src/template/template.cpp


HEADERS  += src/mainwindow.h \
    src/scrapers/allocinescraper.h \
    src/scrapers/themoviedbscraper.h \
    src/scrapers/scraper.h \
    src/template/templateyadis.h \
    src/fileparser.h \
    src/scrapers/allocinescraper.h \
    src/scrapers/scraper.h \
    src/scrapers/themoviedbscraper.h \
    src/template/templateyadis.h \
    src/filedownloader.h \
    src/fileparser.h \
    src/scrapers/thetvdbscraper.h \
    src/scrapers/cncclassificationwrapper.h \
    src/promise.h \
    src/searchscraperdialog.h \
    src/engine.h \
    src/tvixengine.h \
    src/scanner/filenamescanner.h \
    src/scanner/scanner.h \
    src/chooseitemdialog.h \
    src/inprogressdialog.h \
    src/scanner/mediainfoscanner.h \
    src/scanner/mediainfo.h \
    src/panelview.h \
    src/template/template.h


FORMS    += src/mainwindow.ui \
    src/searchscraperdialog.ui \
    src/chooseitemdialog.ui \
    src/inprogressdialog.ui \
    src/panelview.ui

RESOURCES += \
    resources/resources.qrc
