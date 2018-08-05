# SVGs to PDF

A simple command allowing to convert multiple SVG files into a single multi-paged PDF.

It is mainly based on `librsvg` and therefore, also on `cairo`.

In order to compile it on Windows-based systems, you will need `MSYS2` and/or `Mingw` (either `Mingw32` or `Mingw64`).  Within either 32-bit or 64-bit `Mingw`, you will need the `librsvg2-dev` package as well as its dependencies. You can then simply execute `make` without parameters to compile as binary and then optionally `make install` to make the command available system-wide (copy in `/mingw32/bin/` or `/mingw64/bin/`).

This tool is more efficient than converting the SVGs into separate PDFs before merging them together as it does not duplicate font subsets.

**WARNING:: You must keep `fc-cache.exe` and `fc-list.exe` in the same directory as `svgstopdf.exe`.  Otherwise, Pango may not find all your fonts and automatically substitute the font for a different one.**
*****
## Installation
See [**Releases section**](https://github.com/flueterflam/svgstopdf/releases) for Windows binaries.

Win32: [v1.0-win32/svgstopdf.exe](https://github.com/flueterflam/svgstopdf/releases/download/v1.0-win32/svgstopdf.exe)

Win64: [v1.0-win64/svgstopdf.exe](https://github.com/flueterflam/svgstopdf/releases/download/v1.0-win64/svgstopdf.exe)

Auxiliary: [v1.0-auxiliary/fc-cache.exe](https://github.com/flueterflam/svgstopdf/releases/download/v1.0-auxiliary/fc-cache.exe) and [v1.0-auxiliary/fc-list.exe](https://github.com/flueterflam/svgstopdf/releases/download/v1.0-auxiliary/fc-list.exe)

### MSYS2
To compile, first install MSYS2 (https://www.msys2.org/).  Download 32-bit or 64-bit version, depending on your system.  Follow MSYS2 instructions to install and update MSYS2 and Mingw.

### Installing necessary libraries
For 32-bit, open `MSYS2` or `Mingw32` console and run:
```
pacman -S mingw-w64-i686-gcc-objc mingw-w64-i686-gcc mingw-w64-i686-libpng \ 
           mingw-w64-i686-fontconfig mingw-w64-i686-cairo mingw-w64-i686-cairomm \
           mingw-w64-i686-librsvg mingw-w64-i686-gdk-pixbuf2 mingw-w64-i686-glib2 \
           mingw-w64-i686-json-glib mingw-w64-i686-python2-gobject \
           mingw-w64-i686-python3-gobject glib2 mingw-w64-i686-freetype
```
For 64-bit, open `MSYS2` or `Mingw64` console and run:
```
pacman -S mingw-w64-x86_64-gcc-objc mingw-w64-x86_64-gcc mingw-w64-x86_64-libpng \
          mingw-w64-x86_64-fontconfig mingw-w64-x86_64-cairo mingw-w64-x86_64-cairomm \
          mingw-w64-x86_64-librsvg mingw-w64-x86_64-gdk-pixbuf2 mingw-w64-x86_64-glib2 \
          mingw-w64-x86_64-json-glib mingw-w64-x86_64-python2-gobject \
          mingw-w64-x86_64-python3-gobject glib2 mingw-w64-x86_64-freetype
```

Type `Y` to install, including all dependencies.  This process will take several minutes, depending on download speed and your system.

### Download and create directory for `svgstoppdf`
Download `.zip` or clone to your MSYS2 `home` directory.  Default installation path is `C:\msys64\home\USERNAME\` or `~/` from `Mingw32` or `Mingw64` console.

Keep files in `svgstopdf` directory (i.e. `C:\msys64\home\USERNAME\svgstopdf` or `~/svgstopdf`.

### Compiling
For 32-bit, delete or rename `Makefile` (which is 64-bit).  Rename `Makefile(32-bit)` to `Makefile`.  Open `Mingw32` console and run:
```
>cd svgstopdf
>make
```
For 64-bit, open `Mingw64` console and run:
```
>cd svgstopdf
>make
```

### Executable file
You may now copy the `svgstopdf.exe`, `fc-cache.exe`, and `fc-list.exe` executables to whichever directory you choose.
*****
## Usage
Here is an example of use of this script:

```bash
svgstopdf 1.svg 2.svg out.pdf
```
or
```bash
svgstopdf *.svg out.pdf
```

The main purpose of this script is to avoid loading duplicate font subsets while including several vector images in a LaTeX document. After generating the PDF from various SVGs, you can load them easily using the LaTeX `graphicx` package:

```latex
\includegraphics[page=1]{out.pdf}
\includegraphics[page=2]{out.pdf}
```

Because this syntax using pages makes it hard to figure out which image is being included, an additional `--latex-package` option may be passed to the script in order to generate a LaTeX package that allows for easier integration:

```bash
svgstopdf 1.svg 2.svg out.pdf --latex-package
```
or
```bash
svgstopdf *.svg out.pdf --latex-package
```

This command will generate an additional `svgimages.sty` that you must keep in the same directory as your generated PDF file as well as your LaTeX source files. Your LaTeX code to load the images will now become:

```latex
\usepackage{svgimages}

\includesvg{1.svg}
\includesvg{2.svg}
```

You may specify a `path` option to `svgimages`.  Use this option similarly to the `path` option for `graphicx`.  Multiple directories can be specified (e.g. `path={{/location1}{./location2}{C:/path/to/location3}}`.  However, you should generally use only one path location -- the one where you will story the generated PDF file (containing all the SVGs).

```latex
\usepackage[path={{./subdirectory/}}]{svgimages}

\includesvg{1.svg}
\includesvg{2.svg}
```

You may even pass some `includegraphics`-supported parameters:

```latex
\includesvg[scale=0.8]{1.svg}
```
or 
```latex
\includesvg[width=0.8\textwidth]{2.svg}
```
