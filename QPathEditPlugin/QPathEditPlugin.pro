TEMPLATE    = lib

QT += core gui widgets uiplugin
CONFIG += plugin skip_target_version_ext c++11

TARGET      = $$qtLibraryTarget(qpatheditplugin)
VERSION = 1.2.1
win32 {
	QMAKE_TARGET_COMPANY = "Skycoder42"
	QMAKE_TARGET_PRODUCT = "QPathEdit QtDesigner Plugin"
	QMAKE_TARGET_DESCRIPTION = $$QMAKE_TARGET_PRODUCT
} else:unix:!mac: QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../../lib\''

DEFINES += DESIGNER_PLUGIN

include(../qpathedit.pri)

HEADERS     += qpatheditplugin.h
SOURCES     += qpatheditplugin.cpp
RESOURCES   += qpatheditplugin_res.qrc

DISTFILES += \
    qpathedit.json
