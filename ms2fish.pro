#-------------------------------------------------
#
# Project created by QtCreator 2018-05-14T11:50:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17

TARGET = ms2fish
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += /home/aru/Repos/LookingGlass/common
INCLUDEPATH += /home/aru/Repos/ocv_libs/qt/
INCLUDEPATH += /home/aru/Repos/random/include/effolkronium

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    KVMFRHelper.cpp \
    autofishcv.cpp \
    ../../Repos/ocv_libs/qt/mat_and_qimage.cpp \
    autofishkb.cpp

HEADERS += \
        mainwindow.h \
    KVMFRHelper.h \
    KVMFRException.h \
    autofishcv.h \
    ../../Repos/ocv_libs/qt/mat_and_qimage.hpp \
    autofishkb.h

FORMS += \
        mainwindow.ui

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv
    PKGCONFIG += libvirt
}

RESOURCES += \
    images.qrc
