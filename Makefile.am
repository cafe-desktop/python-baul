SUBDIRS = po src examples

if ENABLE_GTK_DOC
SUBDIRS += docs
endif

ACLOCAL_AMFLAGS = -I m4 -I . ${ACLOCAL_FLAGS}

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = baul-python.pc

extensiondir = $(datadir)/baul/extensions
extension_in_files = libbaul-python.baul-extension.desktop.in
extension_DATA = $(extension_in_files:.baul-extension.desktop.in=.baul-extension)
$(extension_DATA): $(extension_in_files)
	$(AM_V_GEN) $(MSGFMT) --desktop --keyword=Name --keyword=Description --template $< -d $(top_srcdir)/po -o $@

EXTRA_DIST = autogen.sh baul-python.pc.in libbaul-python.baul-extension.desktop.in
CLEANFILES = libbaul-python.baul-extension

DISTCHECK_CONFIGURE_FLAGS= \
	--enable-gtk-doc \
	--with-bauldir='$${libdir}/baul/extensions-2.0-distcheck' \
	CFLAGS='-Wno-deprecated-declarations'

# Build ChangeLog from GIT  history
ChangeLog:
	$(AM_V_GEN) if test -d $(top_srcdir)/.git; then \
		GIT_DIR="$(top_srcdir)/.git" git log --stat > $@; \
	fi

dist: ChangeLog

.PHONY: ChangeLog
