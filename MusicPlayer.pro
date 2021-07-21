#-------------------------------------------------
#
# Project created by QtCreator 2021-07-13T20:26:27
#
#-------------------------------------------------

QT       += core gui network multimedia sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MusicPlayer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    main.cpp \
    musicplayer.cpp \
    mylistwidget.cpp \
    login.cpp \
    registerdialog.cpp \
    songspider.cpp \
    myslider.cpp

HEADERS += \
    musicplayer.h \
    mylistwidget.h \
    login.h \
    registerdialog.h \
    songspider.h \
    myslider.h

FORMS += \
        musicplayer.ui \
    musicplayer.ui \
    login.ui \
    registerdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES += \
    resources.qrc \
    resources.qrc


SUBDIRS += \
    MusicPlayer.pro

RC_ICONS = logo.ico
