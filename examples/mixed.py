# This Python baul extension only consider files/folders with a mixed
# upper/lower case name. For those, the following is featured:
# - an emblem on the icon,
# - contextual menu entry.
# - a list view "Mixed" column,
# - a property page,
# - A top area widget.

import os

from gi.repository import Baul, GObject, Gtk


class Mixed(GObject.GObject,
                Baul.InfoProvider,
                Baul.ColumnProvider,
                Baul.MenuProvider,
                Baul.PropertyPageProvider,
                Baul.LocationWidgetProvider):

    emblem = 'favorite-symbolic.symbolic'       # Use one of the stock emblems.

    # Private methods.

    def _file_has_mixed_name(self, baulfile):
        name = baulfile.get_name()
        if name.upper() != name and name.lower() != name:
            return 'mixed'
        return ''

    # Baul.InfoProvider implementation.

    def update_file_info(self, baulfile):
        mixed = self._file_has_mixed_name(baulfile)
        baulfile.add_string_attribute('mixed', mixed)
        if mixed:
            baulfile.add_emblem(self.emblem)

    # Baul.ColumnProvider implementation.

    def get_columns(self):
        return [
            Baul.Column(
                name        = 'Mixed::mixed_column',
                attribute   = 'mixed',
                label       = 'Mixed',
                description = 'Column added by the mixed extension'
            )
        ]

    # Baul.MenuProvider implementation.

    def get_file_items(self, window, baulfiles):
        menuitems = []
        if len(baulfiles) == 1:
            for baulfile in baulfiles:
                mixed = baulfile.get_string_attribute('mixed')
                if mixed:
                    filename = baulfile.get_name()
                    menuitem = Baul.MenuItem(
                        name  = 'Mixed::FileMenu',
                        label = 'Mixed: %s has a mixed case name' % filename,
                        tip   = '',
                        icon  = ''
                    )
                    menuitems.append(menuitem)

        return menuitems

    def get_background_items(self, window, folder):
        mixed = self._file_has_mixed_name(folder)
        if not mixed:
            return []
        return [
            Baul.MenuItem(
                name  = 'Mixed::BackgroundMenu',
                label = 'Mixed: you are browsing a directory with a mixed case name',
                tip   = '',
                icon  = ''
            )
        ]

    # Baul.PropertyPageProvider implementation.

    def get_property_pages(self, baulfiles):
        pages = []
        if len(baulfiles) == 1:
            for baulfile in baulfiles:
                if self._file_has_mixed_name(baulfile):
                    page_label = Gtk.Label('Mixed')
                    page_label.show()
                    hbox = Gtk.HBox(homogeneous = False, spacing = 4)
                    hbox.show()
                    name_label = Gtk.Label(baulfile.get_name())
                    name_label.show()
                    comment_label = Gtk.Label('has a mixed-case name')
                    comment_label.show()
                    hbox.pack_start(name_label, False, False, 0)
                    hbox.pack_start(comment_label, False, False, 0)
                    pages.append(
                        Baul.PropertyPage(
                            name  = 'Mixed::PropertyPage',
                            label = page_label,
                            page  = hbox
                        )
                    )

        return pages

    # Baul.LocationWidgetProvider implementation.

    def get_widget(self, uri, window):
        baulfile = Baul.FileInfo.create_for_uri(uri)
        if not self._file_has_mixed_name(baulfile):
            return None
        label = Gtk.Label('In mixed-case directory ' + baulfile.get_name())
        label.show()
        return label
