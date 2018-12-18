TEMPLATE = lib

QT = core

CONFIG += lib_bundle
DEFINES += KPXCCLIENT_LIBRARY

TARGET = $$qtLibraryTarget($$TARGET_BASE)
QMAKE_TARGET_DESCRIPTION = "KeePassXC Client Library"

SOURCES += \
	kpxcclient.cpp \
	sodiumcryptor.cpp \
    securebytearray.cpp \
    kpxcconnector.cpp

PUBLIC_HEADERS += \
	kpxcclient_global.h \
	kpxcclient.h

HEADERS += $$PUBLIC_HEADERS \
	sodiumcryptor_p.h \
    securebytearray_p.h \
    kpxcconnector_p.h

unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += libsodium
}

include(../install.pri)
mac {
	FRAMEWORK_HEADERS.version = Versions
	FRAMEWORK_HEADERS.files = $$PUBLIC_HEADERS
	FRAMEWORK_HEADERS.path = Headers
	QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
} else {
	install_headers.files = $$PUBLIC_HEADERS
	install_headers.path = $$INSTALL_HEADERS/$$TARGET_BASE
	INSTALLS += install_headers
}
target.path = $$INSTALL_LIBS
dlltarget.path = $$INSTALL_BINS
INSTALLS += target dlltarget
