#include <stdio.h>
#include <glib/gstdio.h>
#include <libgimp/gimp.h>
#include "gra.h"

/*
0000000: f303 0000 f803 0000 9305 0000 0100 0000  ................
0000010: 2738 0000 0000 0000 681f 1600 0000 0000  '8......h.......
0000020: 0201 8081 8283 8485 8687 8100 8a8b 8c8d  ................
0000030: 8e80 8e8c 8893 9495 9697 8800 989b 9c9d  ................
0000040: 9e9f a0a1 a2a3 a4a5 99a6 a8a9 aaab acad  ................
0000050: aeaf b0af 91b3 b4b5 a4b5 b8b9 b386 babd  ................
0000060: b1bf 018d c0bc bd8f c1b4 c3c9 90ca cccd  ................
0000070: cecf ca9a d0d3 d4d5 d6d7 d890 c5db 92a2  ................
0000080: dcdf c684 e0bb d9a0 c2cf e38a c7e4 e5a5  ................
0000090: d2ed f0f1 f2ac eff3 f6f7 f8f9 c0e9 dbde  ................
 */

int main(int argc, char* argv[]){
    printf("Starting program\n");

    FILE            *fd;
    const gchar     *filename = argv[1];

    gint            width;
    gint            width_internal;
    gint            height;
    gint            flags;
    //CBGR49 palette[16]; // Not implemented yet
    long            lSize;
    long            body_size;
    guchar          *compressed_body;

    fd = g_fopen(filename, "rb");
    if (!fd){
        printf("Error: no fd available\n");
        return 1;
    }
    printf("Fd read and set\n");

    if (!ReadOK(fd, &width, 4)){
        printf("Error reading bytes\n");
        return 1;
    }
    printf("width is %d\n", width);

    if (!ReadOK(fd, &width_internal, 4)){
        printf("Error reading bytes\n");
        return 1;
    }
    printf("width_internal is %d\n", width_internal);

    if (!ReadOK(fd, &height, 4)){
        printf("Error reading bytes\n");
        return 1;
    }
    printf("height is %d\n", height);

    if (!ReadOK(fd, &flags, 4)){
        printf("Error reading bytes\n");
        return 1;
    }
    printf("flags is %d\n", flags);

    long original=ftell(fd);
    fseek(fd,0,SEEK_END);
    body_size = ftell(fd) - original;
    fseek(fd,original,SEEK_SET);

    printf("File size is %d, body_size is %d\n", lSize, body_size);

    // Allocate memory for the file
    compressed_body = (guchar*) malloc (sizeof(guchar)*body_size);
    if (!ReadOK(fd, compressed_body, body_size)){
        printf("Error reading bytes\n");
        return 1;
    }
    int i;
    for (i = 0; i < body_size; i++){
        printf("%x ", compressed_body[i]);
    }
    printf("\n");

    return 0;
}
