#ifndef __TOSZ_H__
#define __TOSZ_H__
long uncompress(unsigned char* compressed, long compressed_size, unsigned char ** decompressed);
//TODO: Return BYTE *
void CompressBuf(unsigned char *src,long size/*,CTask *mem_task=NULL*/);
#endif /*__TOSZ_H__*/