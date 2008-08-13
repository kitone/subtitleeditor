## SUBDIRS = m4 share src plugins po
SUBDIRS = m4 share src docs po

EXTRA_DIST = autogen.sh prepare-ChangeLog.pl prepare-po.sh \
		intltool-extract.in intltool-merge.in intltool-update.in

ACLOCAL_AMFLAGS = -I m4

CLEANFILES = Makefile.am~ *.c~ *.cc~ *.h~ *.glade~ *.gladep *.gladep.bak

DISTCLEANFILES = intltool-extract intltool-merge intltool-update
