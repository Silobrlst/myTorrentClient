#-------------------------------------------------
#
# Project created by QtCreator 2017-05-27T10:49:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myTorrentClient
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    addtorrentwindow.cpp \
    settingswindow.cpp

HEADERS  += mainwindow.h \
    addtorrentwindow.h \
    torrenttreedelegate.h \
    settingswindow.h

FORMS    += mainwindow.ui \
    addtorrentwindow.ui \
    settingswindow.ui

unix:!macx: LIBS += /home/q/Загрузки/boost_1_64_0/bin.v2/libs/system/build/gcc-4.8/release/link-static/threading-multi/libboost_system.a -lX11

INCLUDEPATH += $$PWD/../Загрузки/libtorrent-libtorrent-1_1_3/include
DEPENDPATH += $$PWD/../Загрузки/libtorrent-libtorrent-1_1_3/include

unix:!macx: LIBS += -ltorrent-rasterbar
