baul_extensiondir=$(BAUL_EXTENSION_DIR)

baul_extension_LTLIBRARIES = libbaul-python.la

libbaul_python_la_SOURCES =     	\
	baul-python.c		\
	baul-python.h		\
	baul-python-object.c	\
	baul-python-object.h

libbaul_python_la_CPPFLAGS = \
	-I$(top_srcdir)	\
	-I$(top_builddir) \
	-DG_LOG_DOMAIN=\"Baul-Python\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\" \
	-DPYTHON_VERSION=\"$(PYTHON_VERSION)\" \
	-DPYTHON_ABIFLAGS=\"$(PYTHON_ABIFLAGS)\" \
	-DPY_LIB_LOC="\"$(PYTHON_LIB_LOC)\"" \
	$(BAUL_PYTHON_CFLAGS) \
	$(AM_CPPFLAGS)

libbaul_python_la_CFLAGS = \
	$(PYTHON_CFLAGS)
	$(BAUL_PYTHON_CFLAGS) \
	$(AM_CFLAGS)

libbaul_python_la_LDFLAGS = -module -avoid-version -Wl,-z,defs
libbaul_python_la_LIBADD  = $(PYTHON_LIBS) $(BAUL_PYTHON_LIBS)
