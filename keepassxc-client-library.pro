TEMPLATE = subdirs

SUBDIRS += src \
	clidemo

clidemo.depends += src

DISTFILES += \
	.qmake.conf \
	README.md \
	LICENSE
