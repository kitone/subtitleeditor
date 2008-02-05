SUBDIRS = icons glade

subtitleeditor_pixmapsdir = $(prefix)/${NO_PREFIX_PACKAGE_SHARE_DIR}

desktopdir = $(datadir)/applications
desktop_DATA = subtitleeditor.desktop

subtitleeditor_pixmaps_DATA = \
							menubar.xml \
							subtitleeditor.svg

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi


EXTRA_DIST = $(subtitleeditor_pixmaps_DATA) $(desktop_DATA)

CLEANFILES = Makefile.am~ *.c~ *.h~ *.glade~ *.gladep *.gladep.bak


