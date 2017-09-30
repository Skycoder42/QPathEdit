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

HEADERS     += qpatheditplugin.h
SOURCES     += qpatheditplugin.cpp
RESOURCES   += qpatheditplugin_res.qrc

OTHER_FILES += qpathedit.json

system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)):include($$OUT_PWD/qpmx_generated.pri)
else: error(qpmx initialization failed. Check the compilation log for details.)
