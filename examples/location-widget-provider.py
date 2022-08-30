from gi.repository import Baul, Ctk, GObject

class LocationProviderExample(GObject.GObject, Baul.LocationWidgetProvider):
    def __init__(self):
        pass
    
    def get_widget(self, uri, window):
        entry = Ctk.Entry()
        entry.set_text(uri)
        entry.show()
        return entry
