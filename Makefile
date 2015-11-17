GIMPCARGS = $(shell gimptool-2.0 --cflags)
GIMPLIBS = $(shell gimptool-2.0 --libs)
SYSTEM_INSTALL_DIR = $(shell gimptool-2.0 --dry-run --install-admin-bin file-gra | sed 's/cp \S* \(\S*\)/\1/')
USER_INSTALL_DIR = $(shell gimptool-2.0 --dry-run --install-bin file-gra | sed 's/cp \S* \(\S*\)/\1/')

make: 
	gcc -pthread -I$(GIMPCARGS) -DGTK_DISABLE_DEPRECATED -g -O2 -Wall -Wdeclaration-after-statement -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations -Winit-self -Wpointer-arith -Wold-style-definition -Wmissing-format-attribute -Wformat-security -Wlogical-op -Wtype-limits -fno-common -fdiagnostics-show-option -Wreturn-type *.c -o file-gra $(GIMPLIBS)
	
install: 
	gimptool-2.0 --install-bin file-gra
	# I think we should be getting these directories using gimptool-2.0 and sed with regex
	cp TempleOS.gpl $(HOME)/.gimp-2.8/palettes/
	
uninstall: 
	gimptool-2.0 --uninstall-bin file-gra
	# I think we should be getting these directories using gimptool-2.0 and sed with regex
	rm $(HOME)/.gimp-2.8/palettes/TempleOS.gpl

install-admin:
	gimptool-2.0 --install-admin-bin file-gra
	# I think we should be getting these directories using gimptool-2.0 and sed with regex
	cp TempleOS.gpl /usr/share/gimp/2.0/palettes/

uninstall-admin:
	gimptool-2.0 --uninstall-admin-bin file-gra
	# I think we should be getting these directories using gimptool-2.0 and sed with regex
	rm /usr/share/gimp/2.0/palettes/TempleOS.gpl

clean:
	rm file-gra
	
all:
	make

