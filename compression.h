#ifndef __COMPRESSION_H__
#define __COMPRESSION_H__
long uncompress(unsigned char* compressed, long compressed_size, unsigned char ** decompressed);
long CompressBuf(unsigned char ** compressed, unsigned char *src, long size);
#endif /*__COMPRESSION_H__*/
