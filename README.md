# GIMP plugin to read/write GRA files

This is a simple plugin to allow GIMP to read and write .GRA image files as created by Terry Davis in his operating system [TempleOS](www.templeos.org).

## Installation
- Install libgimp2.0 to be able to build it. On ubuntu you can type `sudo apt-get install libgimp2.0-dev` to get this.
- Get the source from here (clone or whatever)
- Enter the directory in terminal, type `make` and then `make install`. This will install the plugin binary in `~/.gimp-2.8/plugins/` and a palette file in `~/.gimp-2.8/palettes/`.

## Usage
- To open .GRA files, just open them like you would any image file (File->Open). This only works with regular .GRA files (you will have to decompress any .GRA.Z files first).
- To export an image as a .GRA file, simply make sure the file has a .GRA extension. .GRA files are indexed images using a fix palette of 16-colors. If your image is not in this format you will be prompted before exporting the image. Clicking "Export" at this dialog will automatically convert the image.
