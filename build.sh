#!/bin/bash

rm *.o
rm file-gra

# gra.c
gcc -pthread -I$(gimptool-2.0 --cflags) -DGTK_DISABLE_DEPRECATED -g -O2 -Wall -Wdeclaration-after-statement -Wmissing-prototypes -Werror=missing-prototypes -Wstrict-prototypes -Wmissing-declarations -Winit-self -Wpointer-arith -Wold-style-definition -Wmissing-format-attribute -Wformat-security -Wlogical-op -Wtype-limits -fno-common -fdiagnostics-show-option -Wreturn-type -MD -MP -MF .deps/gra.Tpo -c gra.c

# gra-read.c
gcc -pthread -I$(gimptool-2.0 --cflags) -DGTK_DISABLE_DEPRECATED -g -O2 -Wall -Wdeclaration-after-statement -Wmissing-prototypes -Werror=missing-prototypes -Wstrict-prototypes -Wmissing-declarations -Winit-self -Wpointer-arith -Wold-style-definition -Wmissing-format-attribute -Wformat-security -Wlogical-op -Wtype-limits -fno-common -fdiagnostics-show-option -Wreturn-type -MD -MP -MF .deps/gra.Tpo -c gra-read.c

# gra-write.c
gcc -pthread -I$(gimptool-2.0 --cflags) -DGTK_DISABLE_DEPRECATED -g -O2 -Wall -Wdeclaration-after-statement -Wmissing-prototypes -Werror=missing-prototypes -Wstrict-prototypes -Wmissing-declarations -Winit-self -Wpointer-arith -Wold-style-definition -Wmissing-format-attribute -Wformat-security -Wlogical-op -Wtype-limits -fno-common -fdiagnostics-show-option -Wreturn-type -MD -MP -MF .deps/gra.Tpo -c gra-write.c

# link
libtool --silent --tag=CC   --mode=link gcc  -g -O2 -Wall -Wdeclaration-after-statement -Wmissing-prototypes -Werror=missing-prototypes -Wstrict-prototypes -Wmissing-declarations -Winit-self -Wpointer-arith -Wold-style-definition -Wmissing-format-attribute -Wformat-security -Wlogical-op -Wtype-limits -fno-common -fdiagnostics-show-option -Wreturn-type -o file-gra gra.o gra-read.o gra-write.o $(gimptool-2.0 --libs)

