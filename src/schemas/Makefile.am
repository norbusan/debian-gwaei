## Process this file with automake to produce Makefile.in

top_srcdir = @top_srcdir@
SCHEMAS_DIRECTORY = $(top_srcdir)/src/schemas
PACKAGE = @PACKAGE@
VERSION = @VERSION@
PREFIX = $(prefix)

KILLALL=@KILLALL@
SCHEMA_PATH = $(SCHEMAS_DIRECTORY)/@PACKAGE@.schemas

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = $(top_srcdir)/src/schemas/@PACKAGE@.schemas

install-data-local: install-schemas
uninstall-local: uninstall-schemas

if GCONF_SCHEMAS_INSTALL
uninstall-schemas:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	$(GCONFTOOL) --makefile-uninstall-rule $(SCHEMAS_DIRECTORY)/@PACKAGE@.schemas 
	`$(KILLALL) -HUP gconfd-2 || true`
else
uninstall-schemas:
endif

if GCONF_SCHEMAS_INSTALL
install-schemas:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(SCHEMA_PATH)
	`$(KILLALL) -HUP gconfd-2 || true`
else
install-schemas:
endif

EXTRA_DIST = @PACKAGE@.schemas
