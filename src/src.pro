TEMPLATE = lib

QT = core

CONFIG += lib_bundle
DEFINES += KPXCCLIENT_LIBRARY
enable_msg_debug: DEFINES += KPXCCLIENT_MSG_DEBUG

TARGET = $$qtLibraryTarget($$TARGET_BASE)
QMAKE_TARGET_DESCRIPTION = "KeePassXC Client Library"

PUBLIC_HEADERS += \
	kpxcclient_global.h \
	securebytearray.h \
	entry.h \
	client.h \
	idatabaseregistry.h \
	defaultdatabaseregistry.h

PRIVATE_HEADERS += \
	sodiumcryptor_p.h \
	securebytearray_p.h \
	client_p.h \
	connector_p.h \
	defaultdatabaseregistry_p.h \
	entry_p.h

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

SOURCES += \
	sodiumcryptor.cpp \
	securebytearray.cpp \
	connector.cpp \
	client.cpp \
	defaultdatabaseregistry.cpp \
	entry.cpp

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
	install_private_headers {
		FRAMEWORK_PRIVATE_HEADERS.version = Versions
		FRAMEWORK_PRIVATE_HEADERS.files = $$PRIVATE_HEADERS
		FRAMEWORK_PRIVATE_HEADERS.path = Headers/private
		QMAKE_BUNDLE_DATA += FRAMEWORK_PRIVATE_HEADERS
	}
} else {
	install_headers.files = $$PUBLIC_HEADERS
	install_headers.path = $$INSTALL_HEADERS/$$TARGET_BASE
	INSTALLS += install_headers
	install_private_headers {
		install_pheaders.files = $$PRIVATE_HEADERS
		install_pheaders.path = $$INSTALL_HEADERS/$$TARGET_BASE/private
		INSTALLS += install_pheaders
	}
}
target.path = $$INSTALL_LIBS
dlltarget.path = $$INSTALL_BINS
INSTALLS += target dlltarget

unix {
	pc_target.target = libkpxcclient.pc
	pc_target.depends = $$PWD/libkpxcclient.pc.in
	pc_target.commands = echo $$shell_quote(prefix=$$PREFIX) > libkpxcclient.pc \
		$$escape_expand(\n\t)echo $$shell_quote(libdir=$$INSTALL_LIBS) >> libkpxcclient.pc \
		$$escape_expand(\n\t)echo $$shell_quote(includedir=$$INSTALL_HEADERS/$$TARGET_BASE) >> libkpxcclient.pc \
		$$escape_expand(\n\t)cat $$shell_path($$PWD/libkpxcclient.pc.in) >> libkpxcclient.pc
	QMAKE_EXTRA_TARGETS += pc_target
	PRE_TARGETDEPS += libkpxcclient.pc
	pc_install.files = $$OUT_PWD/libkpxcclient.pc
	pc_install.CONFIG += no_check_exist
	pc_install.path += $$INSTALL_PKGCONFIG
	INSTALLS += pc_install
}

DISTFILES += \
	libkpxcclient.pc.in
