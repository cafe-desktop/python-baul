import os

from gi.repository import GObject, Baul

class ColumnExtension(GObject.GObject, Baul.ColumnProvider, Baul.InfoProvider):
    def __init__(self):
        pass
    
    def get_columns(self):
        return Baul.Column(name="BaulPython::block_size_column",
                               attribute="block_size",
                               label="Block size",
                               description="Get the block size"),

    def update_file_info(self, file):
        if file.get_uri_scheme() != 'file':
            return
        
        filename = file.get_location().get_path()
        
        file.add_string_attribute('block_size', str(os.stat(filename).st_blksize))
