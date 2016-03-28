#-------------------------------------------------
#
# Project created by QtCreator 2015-09-05T13:09:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PathEditTest
TEMPLATE = app


SOURCES += main.cpp \
    form.cpp

HEADERS  += \
    form.h

include(../qpathedit.pri)

FORMS += \
    form.ui
