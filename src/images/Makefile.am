## Process this file with automake to produce Makefile.in

PACKAGE = @PACKAGE@
VERSION = @VERSION@
datadir = @datadir@

#Install Application icon
data16dir = $(datadir)/icons/hicolor/16x16/apps
data24dir = $(datadir)/icons/hicolor/24x24/apps
data32dir = $(datadir)/icons/hicolor/32x32/apps
data48dir = $(datadir)/icons/hicolor/48x48/apps
datascalabledir = $(datadir)/icons/hicolor/scalable/apps
data16_DATA = 16x16/$(PACKAGE).png
data24_DATA = 24x24/$(PACKAGE).png
data32_DATA = 32x32/$(PACKAGE).png
data48_DATA = 48x48/$(PACKAGE).png
datascalable_DATA = scalable/$(PACKAGE).svg

#Install Application Logo
pkgdata_DATA = logo.png goo.png google.png wikipedia.png book_green.png

#Install Toolbar/Menuitem Icons
datawordboundary16dir = $(datadir)/icons/hicolor/16x16/actions
datawordboundary22dir = $(datadir)/icons/hicolor/22x22/actions
datawordboundary24dir = $(datadir)/icons/hicolor/24x24/actions
datanonwordboundary16dir = $(datadir)/icons/hicolor/16x16/actions
datanonwordboundary22dir = $(datadir)/icons/hicolor/22x22/actions
datanonwordboundary24dir = $(datadir)/icons/hicolor/24x24/actions
dataunknowncharacter16dir = $(datadir)/icons/hicolor/16x16/actions
dataunknowncharacter22dir = $(datadir)/icons/hicolor/22x22/actions
dataunknowncharacter24dir = $(datadir)/icons/hicolor/24x24/actions
datawordboundary16_DATA = 16x16/word-boundary.png
datawordboundary22_DATA = 22x22/word-boundary.png
datawordboundary24_DATA = 24x24/word-boundary.png
datanonwordboundary16_DATA = 16x16/non-word-boundary.png
datanonwordboundary22_DATA = 22x22/non-word-boundary.png
datanonwordboundary24_DATA = 24x24/non-word-boundary.png
dataunknowncharacter16_DATA = 16x16/unknown-character.png
dataunknowncharacter22_DATA = 22x22/unknown-character.png
dataunknowncharacter24_DATA = 24x24/unknown-character.png


#
# GTK icon cache
#

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache


update-icon-cache:
	if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi



EXTRA_DIST = logo.png 16x16/$(PACKAGE).png 24x24/$(PACKAGE).png 32x32/$(PACKAGE).png 48x48/$(PACKAGE).png scalable/$(PACKAGE).svg 22x22/word-boundary.png 16x16/non-word-boundary.png 16x16/unknown-character.png 24x24/non-word-boundary.png 16x16/word-boundary.png 24x24/unknown-character.png 22x22/non-word-boundary.png 24x24/word-boundary.png 22x22/unknown-character.png goo.png google.png wikipedia.png book_green.png 

