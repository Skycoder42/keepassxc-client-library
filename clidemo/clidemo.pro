TEMPLATE = app

QT = core

CONFIG += console
CONFIG -= app_bundle

TARGET = $${TARGET_BASE}-cli
QMAKE_TARGET_DESCRIPTION = "KeePassXC Demo Client"

SOURCES += \
	main.cpp

# Default rules for deployment.
target.path = $$INSTALL_BINS
install_cli: INSTALLS += target

# lib
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../src/release/ -lkpxcclient
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../src/debug/ -lkpxcclient
else:mac: LIBS += -F$$OUT_PWD/../src/ -framework kpxcclient
else:unix: LIBS += -L$$OUT_PWD/../src/ -lkpxcclient

INCLUDEPATH += $$PWD/../src
DEPENDPATH += $$PWD/../src
