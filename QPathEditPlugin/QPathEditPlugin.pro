QT       += core gui widgets uiplugin
CONFIG      += plugin
TARGET      = $$qtLibraryTarget(qpatheditplugin)
TEMPLATE    = lib

HEADERS     = qpatheditplugin.h
SOURCES     = qpatheditplugin.cpp
RESOURCES   = \
    qpatheditplugin_res.qrc

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

DEFINES += DESIGNER_PLUGIN

include(../QPathEdit/qpathedit.pri)

DISTFILES += \
    qpathedit.json
