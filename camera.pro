#-------------------------------------------------
#
# Project created by QtCreator 2014-09-26T10:16:34
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = camera
TEMPLATE = app

INCLUDEPATH += /usr/local/include

#LIBS += -LC:/usr/local/lib -lavcodec -lavutil -lavformat -lavdevice -lavfilter -lswscale

SOURCES += main.cpp\
        mainwindow.cpp \
    video_device.cpp \
    qvideooutput.cpp

HEADERS  += mainwindow.h \
    v4l2grab.h \
    video_device.h \
    qvideooutput.h

FORMS    += mainwindow.ui

RESOURCES += \
    picture.qrc

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libavcodec

unix: PKGCONFIG += libavutil

unix: PKGCONFIG += libavformat

unix: PKGCONFIG += libavdevice

unix: PKGCONFIG += libavfilter

unix: PKGCONFIG += libswscale
