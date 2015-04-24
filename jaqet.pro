TARGET = jaqet

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

DEFINES += QUAZIP_STATIC

include(src/third/quazip/quazip/quazip.pri)

INCLUDEPATH += src/third/quazip/
INCLUDEPATH += src

QT       += core gui network xml widgets

win32 {
INCLUDEPATH += C:\bin\MediaInfo_DLL_0.7.69_Windows_i386_WithoutInstaller\Developers\Source
LIBS += -LC:\bin\MediaInfo_DLL_0.7.69_Windows_i386_WithoutInstaller -lMediaInfo -lz
}

linux-g++: {
    LIBS += -lmediainfo -lz
    DEFINES += UNICODE
}

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/scrapers/allocinescraper.cpp \
    src/scrapers/scraper.cpp \
    src/scrapers/themoviedbscraper.cpp \
    src/scrapers/thetvdbscraper.cpp \
    src/scrapers/cncclassificationwrapper.cpp \
    src/promise.cpp \
    src/searchscraperdialog.cpp \
    src/scanner/filenamescanner.cpp \
    src/scanner/scanner.cpp \
    src/chooseitemdialog.cpp \
    src/inprogressdialog.cpp \
    src/scanner/mediainfoscanner.cpp \
    src/scanner/mediainfo.cpp \
    src/panelview.cpp \
    src/engine/engine.cpp \
    src/engine/template/templateyadis.cpp \
    src/engine/template/template.cpp \
    src/engine/tvixengine.cpp \
    src/engine/fileengine.cpp \
    src/engine/fileenginefilesystemwatcher.cpp \
    src/utils.cpp \
    src/scrapers/defaultscraper.cpp \
    src/mediachooserwidget.cpp \
    src/mybutton.cpp \
    src/mygraphicsobject.cpp


HEADERS  += src/mainwindow.h \
    src/scrapers/allocinescraper.h \
    src/scrapers/themoviedbscraper.h \
    src/scrapers/scraper.h \
    src/scrapers/allocinescraper.h \
    src/scrapers/scraper.h \
    src/scrapers/themoviedbscraper.h \
    src/template/templateyadis.h \
    src/scrapers/thetvdbscraper.h \
    src/scrapers/cncclassificationwrapper.h \
    src/promise.h \
    src/searchscraperdialog.h \
    src/scanner/filenamescanner.h \
    src/scanner/scanner.h \
    src/chooseitemdialog.h \
    src/inprogressdialog.h \
    src/scanner/mediainfoscanner.h \
    src/scanner/mediainfo.h \
    src/panelview.h \
    src/engine/engine.h \
    src/engine/template/template.h \
    src/engine/template/templateyadis.h \
    src/engine/tvixengine.h \
    src/engine/fileengine.h \
    src/engine/fileenginefilesystemwatcher.h \
    src/utils.h \
    src/scrapers/defaultscraper.h \
    src/mediachooserwidget.h \
    src/mybutton.h \
    src/mygraphicsobject.h


FORMS    += src/mainwindow.ui \
    src/searchscraperdialog.ui \
    src/chooseitemdialog.ui \
    src/inprogressdialog.ui \
    src/panelview.ui \
    src/mediachooserwidget.ui

RESOURCES += \
    resources/resources.qrc

OTHER_FILES += \
    attribution.txt
