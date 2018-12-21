# installing
isEmpty(PREFIX) {
	PREFIX = $$[QT_INSTALL_PREFIX]
	isEmpty(INSTALL_BINS): INSTALL_BINS = $$[QT_INSTALL_BINS]
	isEmpty(INSTALL_LIBS): INSTALL_LIBS = $$[QT_INSTALL_LIBS]
	isEmpty(INSTALL_HEADERS): INSTALL_HEADERS = $$[QT_INSTALL_HEADERS]
	isEmpty(INSTALL_TRANSLATIONS): INSTALL_TRANSLATIONS = $$[QT_INSTALL_TRANSLATIONS]
} else {
	isEmpty(INSTALL_BINS): INSTALL_BINS = $${PREFIX}/bin
	isEmpty(INSTALL_LIBS): INSTALL_LIBS = $${PREFIX}/lib
	isEmpty(INSTALL_HEADERS): INSTALL_HEADERS = $${PREFIX}/include
	isEmpty(INSTALL_TRANSLATIONS): INSTALL_TRANSLATIONS = $${PREFIX}/translations
}
isEmpty(INSTALL_SHARE): INSTALL_SHARE = $${PREFIX}/share
isEmpty(INSTALL_PKGCONFIG): INSTALL_PKGCONFIG = $${INSTALL_LIBS}/pkgconfig
