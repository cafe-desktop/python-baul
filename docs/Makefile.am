BUILDDIR = $(top_builddir)/docs

REF_VERSION = $(VERSION)
FULL_VERSION = $(VERSION)

FIXXREF = $(srcdir)/xsl/fixxref.py
HTML_STYLE = $(srcdir)/xsl/ref-html-style.xsl

XML_FILES = 							\
	reference/baul-python-ref.xml			\
	reference/baul-python-overview.xml			\
	reference/baul-python-overview-example.xml			\
	reference/baul-python-overview-methods.xml			\
	reference/baul-python-class-reference.xml			\
	reference/baul-python-provider-reference.xml			\
	reference/baul-python-column.xml   \
	reference/baul-python-column-provider.xml   \
	reference/baul-python-menu.xml  \
	reference/baul-python-menu-item.xml \
	reference/baul-python-menu-provider.xml \
	reference/baul-python-property-page.xml \
	reference/baul-python-property-page-provider.xml \
	reference/baul-python-location-widget-provider.xml \
	reference/baul-python-file-info.xml \
	reference/baul-python-info-provider.xml \
	reference/baul-python-enum-reference.xml \
	reference/baul-python-operation-result.xml \
	reference/baul-python-operation-handle.xml

HTMLdir = $(datadir)/gtk-doc/html/baul-python
HTML_FILES =						\
	html/index.html					\
	html/index.sgml					\
	html/baul-python-overview.html \
	html/baul-python-overview-example.html \
	html/baul-python-overview-methods.html \
	html/baul-python-class-reference.html \
	html/baul-python-provider-reference.html \
	html/class-baul-python-column.html \
	html/class-baul-python-column-provider.html \
	html/class-baul-python-menu.html  \
	html/class-baul-python-menu-item.html  \
	html/class-baul-python-menu-provider.html  \
	html/class-baul-python-property-page.html  \
	html/class-baul-python-property-page-provider.html  \
	html/class-baul-python-location-widget-provider.html  \
	html/class-baul-python-file-info.html  \
	html/class-baul-python-info-provider.html  \
	html/class-baul-python-operation-handle.html  \
	html/baul-python-enum-reference.html \
	html/enum-baul-python-operation-result.html  \
	html/baul-python.devhelp
HTML_DATA = $(HTML_FILES)

XSL_FILES =             \
	xsl/common.xsl      \
	xsl/devhelp.xsl     \
	xsl/html.xsl        \
	xsl/pdf-style.xsl   \
	xsl/pdf.xsl         \
	xsl/style.css

CSS_FILES = xsl/style.css
CSSdir = $(HTMLdir)
CSS_DATA = $(CSS_FILES)

CLEANFILES = 			\
	baul-python-ref.*	\
	reference/builddate.xml	\
	$(HTML_FILES)

EXTRA_DIST = 		\
	$(XML_FILES)     \
	$(XSL_FILES)    \
	$(FIXXREF)      \
	$(HTML_STYLE)

REFERENCE_DEPS = 	\
	reference 	\
	$(XML_FILES)    \
	$(XSL_FILES)    \
	$(FIXXREF)

if ENABLE_GTK_DOC

BUILT_SOURCES = 		\
	reference/builddate.xml	\
	$(HTML_FILES)

reference/builddate.xml: $(REFERENCE_DEPS)
	$(PYTHON) -c "import os, time, datetime; print (datetime.datetime.utcfromtimestamp(int(os.environ.get('SOURCE_DATE_EPOCH', time.time()))).strftime('%Y-%m-%d'))" > $@

$(HTML_FILES): $(REFERENCE_DEPS)
	xsltproc --nonet --xinclude -o $(BUILDDIR)/html/ \
		 --path $(BUILDDIR)/reference:$(srcdir)/reference \
		 --stringparam gtkdoc.bookname "baul-python" \
		 --stringparam gtkdoc.version ${REF_VERSION} \
		 $(HTML_STYLE) $(srcdir)/reference/baul-python-ref.xml
	$(FIXXREF) -i xsl $(BUILDDIR)/html
	touch $@

endif
