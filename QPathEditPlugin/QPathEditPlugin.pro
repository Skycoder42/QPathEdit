PLUGIN_TYPE = designer
PLUGIN_CLASS_NAME = QPathEditPlugin

TARGET = qpathedit
VERSION = 2.0.0

load(qt_plugin)

QT += widgets uiplugin

win32 {
	QMAKE_TARGET_COMPANY = "Skycoder42"
	QMAKE_TARGET_PRODUCT = "QPathEdit QtDesigner Plugin"
	QMAKE_TARGET_DESCRIPTION = $$QMAKE_TARGET_PRODUCT
}

DEFINES += DESIGNER_PLUGIN

include(../qpathedit.pri)

HEADERS     += qpatheditplugin.h
SOURCES     += qpatheditplugin.cpp
RESOURCES   += qpatheditplugin_res.qrc

OTHER_FILES += qpathedit.json
