TEMPLATE = app

QT += widgets

TARGET = PathEditTest

include(../vendor/vendor.pri)
include(../qpathedit.pri)

HEADERS  += \
	form.h

SOURCES += main.cpp \
	form.cpp

FORMS += \
	form.ui
