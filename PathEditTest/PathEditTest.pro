TEMPLATE = app

QT += widgets

TARGET = PathEditTest

HEADERS  += \
	form.h

SOURCES += main.cpp \
	form.cpp

FORMS += \
	form.ui

system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_/..) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)):include($$OUT_PWD/qpmx_generated.pri)
else: error(qpmx initialization failed. Check the compilation log for details.)

include(../qpathedit.pri)
