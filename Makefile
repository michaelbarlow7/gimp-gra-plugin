GIMPARGS = $(shell gimptool-2.0 --cflags --libs)
PCREARGS = $(shell pcre-config --cflags --libs)
SYSTEM_INSTALL_DIR = $(shell gimptool-2.0 --dry-run --install-admin-bin ./bin/bimp | sed 's/cp \S* \(\S*\)/\1/')
USER_INSTALL_DIR = $(shell gimptool-2.0 --dry-run --install-bin ./bin/bimp | sed 's/cp \S* \(\S*\)/\1/')

GEGL_CFLAGS = -pthread -I/usr/local/include/gegl-0.3 -I/usr/local/include/babl-0.1 -I/usr/include/json-glib-1.0 -I/usr/include/gio-unix-2.0/ -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
GEGL_LIBS = -Wl,--export-dynamic -pthread -L/usr/local/lib -lgegl-0.3 -lgegl-npd-0.3 -lgmodule-2.0 -ljson-glib-1.0 -lbabl-0.1 -lgio-2.0 -lgobject-2.0 
#GEGL_LIBS = -Wl,--export-dynamic -pthread -L/usr/local/lib -lgegl-0.3 -lgegl-npd-0.3 -lgmodule-2.0 -ljson-glib-1.0 -lbabl-0.1 -lgio-2.0 -lgobject-2.0 -lglib

make: 
	which gimptool-2.0 && which pcre-config && \
	gcc -o ./bin/file-gra -Wall -O2 -Wno-unused-variable -Wno-pointer-sign -Wno-parentheses *.c $(GIMPARGS) $(PCREARGS) $(GEGL_CFLAGS) $(GEGL_LIBS) -lm -DGIMP_DISABLE_DEPRECATED
	#gcc -o ./bin/file-gra -Wall -O2 -Wno-unused-variable -Wno-pointer-sign -Wno-parentheses src/*.c src/manipulation-gui/*.c $(GIMPARGS) $(PCREARGS) -lm -DGIMP_DISABLE_DEPRECATED
	
#install: 
	#gimptool-2.0 --install-bin ./bin/bimp
	#cp -Rf ./bimp-locale/ $(USER_INSTALL_DIR)
	
#uninstall: 
	#gimptool-2.0 --uninstall-bin bimp
	#rm -R $(USER_INSTALL_DIR)/bimp-locale

#install-admin:
	#gimptool-2.0 --install-admin-bin ./bin/bimp
	#cp -Rf ./bimp-locale/ $(SYSTEM_INSTALL_DIR)

#uninstall-admin:
	#gimptool-2.0 --uninstall-admin-bin bimp
	#rm -R $(SYSTEM_INSTALL_DIR)/bimp-locale

clean:
	rm ./bin/file-gra
	
all:
	make

