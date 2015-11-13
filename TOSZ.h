#ifndef __TOSZ_H__
#define __TOSZ_H__
long uncompress(unsigned char* compressed, long compressed_size, unsigned char ** decompressed);
long CompressBuf(unsigned char ** compressed, unsigned char *src, long size);
#endif /*__TOSZ_H__*/
