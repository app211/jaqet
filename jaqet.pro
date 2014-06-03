QT       += core gui network xml widgets

win32:INCLUDEPATH += C:\Developpement\curl-7.33.0-devel-mingw32\include
INCLUDEPATH+=src/third/networktest-1-3-0_src

win32:LIBS += -LC:\Developpement\curl-7.33.0-devel-mingw32\lib -lcurldll
win32:LIBS += -LC:\bin\MediaInfo_DLL_0.7.69_Windows_i386_WithoutInstaller -lMediaInfo

linux-g++:LIBS += -lcurl

TARGET = tizzbird
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/filedownloader.cpp \
    src/av/avprobe.cpp \
    src/template/templateyadis.cpp \
    src/scrapers/allocinescraper.cpp \
    src/scrapers/scraper.cpp \
    src/scrapers/themoviedbscraper.cpp \
     src/fileparser.cpp \
    src/scrapers/thetvdbscraper.cpp \
    src/scrapers/httpaccess.cpp \
    src/scrapers/curlaccess.cpp \
    src/scrapers/cncclassificationwrapper.cpp \
    src/promise.cpp \
    src/searchscraperdialog.cpp \
    src/engine.cpp \
    src/tvixengine.cpp \
    src/scanner/filenamescanner.cpp \
    src/scanner/scanner.cpp \
    src/chooseitemdialog.cpp \
    src/inprogressdialog.cpp


HEADERS  += src/mainwindow.h \
    src/scrapers/allocinescraper.h \
    src/scrapers/themoviedbscraper.h \
    src/scrapers/scraper.h \
    src/av/avprobe.h \
    src/av/av.h \
    src/template/templateyadis.h \
    src/fileparser.h \
    src/av/av.h \
    src/av/avprobe.h \
    src/scrapers/allocinescraper.h \
    src/scrapers/scraper.h \
    src/scrapers/themoviedbscraper.h \
    src/template/templateyadis.h \
    src/filedownloader.h \
    src/fileparser.h \
    src/scrapers/thetvdbscraper.h \
    src/scrapers/httpaccess.h \
    src/scrapers/curlaccess.h \
    src/scrapers/cncclassificationwrapper.h \
    src/promise.h \
    src/searchscraperdialog.h \
    src/engine.h \
    src/tvixengine.h \
    src/scanner/filenamescanner.h \
    src/scanner/scanner.h \
    src/chooseitemdialog.h \
    src/inprogressdialog.h


FORMS    += src/mainwindow.ui \
    src/searchscraperdialog.ui \
    src/chooseitemdialog.ui \
    src/inprogressdialog.ui


win32:LIBS += -LC:\Developpement\curl-7.33.0-devel-mingw32\lib -lcurldll
linux-g++:LIBS += -lcurl

RESOURCES += \
    resources/resources.qrc
