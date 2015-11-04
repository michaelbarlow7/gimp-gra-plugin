GIMPCARGS = $(shell gimptool-2.0 --cflags)
GIMPLIBS = $(shell gimptool-2.0 --libs)
SYSTEM_INSTALL_DIR = $(shell gimptool-2.0 --dry-run --install-admin-bin ./bin/bimp | sed 's/cp \S* \(\S*\)/\1/')
USER_INSTALL_DIR = $(shell gimptool-2.0 --dry-run --install-bin ./bin/bimp | sed 's/cp \S* \(\S*\)/\1/')

make: 
	gcc -pthread -I$(GIMPCARGS) -DGTK_DISABLE_DEPRECATED -g -O2 -Wall -Wdeclaration-after-statement -Wmissing-prototypes -Werror=missing-prototypes -Wstrict-prototypes -Wmissing-declarations -Winit-self -Wpointer-arith -Wold-style-definition -Wmissing-format-attribute -Wformat-security -Wlogical-op -Wtype-limits -fno-common -fdiagnostics-show-option -Wreturn-type -MD -MP -MF .deps/gra.Tpo *.c -o file-gra $(GIMPLIBS)
	
install: 
	gimptool-2.0 --install-bin file-gra
	
uninstall: 
	gimptool-2.0 --uninstall-bin file-gra

install-admin:
	gimptool-2.0 --install-admin-bin file-gra

uninstall-admin:
	gimptool-2.0 --uninstall-admin-bin file-gra

clean:
	rm file-gra
	
all:
	make

