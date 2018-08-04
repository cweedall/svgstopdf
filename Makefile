all: svgstopdf
svgstopdf:
	gcc -o svgstopdf svgstopdf.c -pthread -I/mingw64/include/librsvg-2.0 -I/mingw64/include/gdk-pixbuf-2.0 -I/mingw64/include/libpng16 -I/mingw64/include/cairo -I/mingw64/include/glib-2.0 -I/mingw64/lib/glib-2.0/include -I/mingw64/include/freetype2 -I/mingw64/include/fontconfig -lrsvg-2 -lm -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lcairo

install:
	install -D svgstopdf ~/svgstopdf
