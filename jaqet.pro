
QT       += core gui network xml

win32:INCLUDEPATH += C:\Developpement\curl-7.33.0-devel-mingw32\include
INCLUDEPATH+=src/third/networktest-1-3-0_src

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tizzbird
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/scrapers/allocinescraper.cpp \
    src/filedownloader.cpp \
    src/scrapers/themoviedbscraper.cpp \
    src/scrapers/scraper.cpp \
    src/third/networktest-1-3-0_src/webfile.cpp \
    src/third/networktest-1-3-0_src/weberror.cpp \
    src/third/networktest-1-3-0_src/httpfileinfo.cpp \
    src/third/networktest-1-3-0_src/httpdir.cpp \
    src/third/networktest-1-3-0_src/entities.cpp \
    src/myproxymodel.cpp \
    src/av/avprobe.cpp \
    src/template/templateyadis.cpp \
    src/fileparser.cpp \
    src/av/avprobe.cpp \
    src/scrapers/allocinescraper.cpp \
    src/scrapers/scraper.cpp \
    src/scrapers/themoviedbscraper.cpp \
    src/template/templateyadis.cpp \
    src/third/networktest-1-3-0_src/entities.cpp \
    src/third/networktest-1-3-0_src/httpdir.cpp \
    src/third/networktest-1-3-0_src/httpfileinfo.cpp \
    src/third/networktest-1-3-0_src/weberror.cpp \
    src/third/networktest-1-3-0_src/webfile.cpp \
    src/filedownloader.cpp \
    src/fileparser.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/myproxymodel.cpp


HEADERS  += src/mainwindow.h \
    src/scrapers/allocinescraper.h \
    src/filedownloader.h \
    src/scrapers/themoviedbscraper.h \
    src/scrapers/scraper.h \
    src/third/networktest-1-3-0_src/webfile.h \
    src/third/networktest-1-3-0_src/weberror.h \
    src/third/networktest-1-3-0_src/httpfileinfo.h \
    src/third/networktest-1-3-0_src/httpdir.h \
    src/third/networktest-1-3-0_src/entities.h \
    third/networktest-1-3-0_src/debugout.h \
    src/myproxymodel.h \
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
    src/third/networktest-1-3-0_src/debugout.h \
    src/third/networktest-1-3-0_src/entities.h \
    src/third/networktest-1-3-0_src/httpdir.h \
    src/third/networktest-1-3-0_src/httpfileinfo.h \
    src/third/networktest-1-3-0_src/weberror.h \
    src/third/networktest-1-3-0_src/webfile.h \
    src/filedownloader.h \
    src/fileparser.h \
    src/mainwindow.h \
    src/myproxymodel.h


FORMS    += src/mainwindow.ui


win32:LIBS += -LC:\Developpement\curl-7.33.0-devel-mingw32\lib -lcurldll
linux-g++:LIBS += -lcurl

RESOURCES += \
    resources/resources.qrc
