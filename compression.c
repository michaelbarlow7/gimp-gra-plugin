#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "compression.h"

#pragma pack(1)

#define TRUE	1
#define FALSE	0

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD; typedef unsigned char BOOL;

#define ARC_MAX_BITS 12

#define CT_NONE 	1
#define CT_7_BIT	2
#define CT_8_BIT	3

#define MAX_INT     0xFFFFFFFFl

typedef struct _CArcEntry
{ 
    struct _CArcEntry *next;
    WORD basecode;
    BYTE ch,pad;
} CArcEntry;

typedef struct _CArcCtrl //control structure
{ 
    DWORD src_pos,src_size,
          dst_pos,dst_size;
    BYTE *src_buf,*dst_buf;
    DWORD min_bits,min_table_entry;
    CArcEntry *cur_entry,*next_entry;
    DWORD cur_bits_in_use,next_bits_in_use;
    BYTE *stk_ptr,*stk_base;
    DWORD free_index,free_limit,
          saved_basecode,
          entry_used,
          last_ch;
    CArcEntry compress[1<<ARC_MAX_BITS],
              *hash[1<<ARC_MAX_BITS];
} CArcCtrl;

typedef struct _CArcCompress
{ 
    DWORD compressed_size,compressed_size_hi,
          expanded_size,expanded_size_hi;
    BYTE	compression_type;
    BYTE body[1]; 
} CArcCompress;

// Returns the bit within bit_field at bit_num (assuming it's stored as little-endian). Whole bunch of finicky stuff because of bytes
int Bt(int bit_num, BYTE *bit_field)
{
    bit_field+=bit_num>>3; // bit_field now points to the appropriate byte in src byte-array
    bit_num&=7; // Get the last 3 bits of bit_num ( 7 is 111 in binary). Basically bit_num % 8. Signifies which bit in the byte now specified by bit_num we are looking at.
    return (*bit_field & (1<<bit_num)) ? 1:0;
}

int Bts(int bit_num, BYTE *bit_field)
{
    int result;
    bit_field+=bit_num>>3; //Get the relevant byte (now in the dest bitfield)
    bit_num&=7; // Get the right bit in that byte

    result=*bit_field & (1<<bit_num); // Only used for return value

    *bit_field|=(1<<bit_num); //Make a byte with the relevant bit switched on and OR it on the relevant byte
    return (result) ? 1:0;
}

DWORD BFieldExtU32(BYTE *src,DWORD pos,DWORD bits)
{
    DWORD i,result=0;
    for (i=0;i<bits;i++)
        if (Bt(pos+i,src))
            Bts(i,(BYTE *)&result);
    return result;
}

void ArcEntryGet(CArcCtrl *c)
{
    DWORD i;
    CArcEntry *temp,*temp1;

    if (c->entry_used) {
        i=c->free_index;

        c->entry_used=FALSE;
        c->cur_entry=c->next_entry;
        c->cur_bits_in_use=c->next_bits_in_use;
        if (c->next_bits_in_use<ARC_MAX_BITS) {
            c->next_entry = &c->compress[i++];
            if (i==c->free_limit) {
                c->next_bits_in_use++;
                c->free_limit=1<<c->next_bits_in_use;
            }
        } else {
            do if (++i==c->free_limit) i=c->min_table_entry;
            while (c->hash[i]);
            temp=&c->compress[i];
            c->next_entry=temp;
            temp1=(CArcEntry *)&c->hash[temp->basecode];
            while (temp1 && temp1->next!=temp)
                temp1=temp1->next;
            if (temp1)
                temp1->next=temp->next;
        }
        c->free_index=i;
    }
}

void ArcExpandBuf(CArcCtrl *c)
{
    BYTE *dst_ptr,*dst_limit;
    DWORD basecode,lastcode,code;
    CArcEntry *temp,*temp1;

    dst_ptr=c->dst_buf+c->dst_pos;
    dst_limit=c->dst_buf+c->dst_size;

    while (dst_ptr<dst_limit && c->stk_ptr!=c->stk_base)
        *dst_ptr++ = * -- c->stk_ptr;

    if (c->stk_ptr==c->stk_base && dst_ptr<dst_limit) {
        if (c->saved_basecode==0xFFFFFFFFl) {
            lastcode=BFieldExtU32(c->src_buf,c->src_pos,
                    c->next_bits_in_use);
            c->src_pos=c->src_pos+c->next_bits_in_use;
            *dst_ptr++=lastcode;
            ArcEntryGet(c);
            c->last_ch=lastcode;
        } else
            lastcode=c->saved_basecode;
        while (dst_ptr<dst_limit && c->src_pos+c->next_bits_in_use<=c->src_size) {
            basecode=BFieldExtU32(c->src_buf,c->src_pos,
                    c->next_bits_in_use);
            c->src_pos=c->src_pos+c->next_bits_in_use;
            if (c->cur_entry==&c->compress[basecode]) {
                *c->stk_ptr++=c->last_ch;
                code=lastcode;
            } else
                code=basecode;
            while (code>=c->min_table_entry) {
                *c->stk_ptr++=c->compress[code].ch;
                code=c->compress[code].basecode;
            }
            *c->stk_ptr++=code;
            c->last_ch=code;

            c->entry_used=TRUE;
            temp=c->cur_entry;
            temp->basecode=lastcode;
            temp->ch=c->last_ch;
            temp1=(CArcEntry *)&c->hash[lastcode];
            temp->next=temp1->next;
            temp1->next=temp;

            ArcEntryGet(c);
            while (dst_ptr<dst_limit && c->stk_ptr!=c->stk_base)
                *dst_ptr++ = * -- c->stk_ptr;
            lastcode=basecode;
        }
        c->saved_basecode=lastcode;
    }
    c->dst_pos=dst_ptr-c->dst_buf;
}

CArcCtrl *ArcCtrlNew(DWORD expand,DWORD compression_type)
{
    CArcCtrl *c;
    c=(CArcCtrl *)malloc(sizeof(CArcCtrl));
    memset(c,0,sizeof(CArcCtrl)); // Couldn't you just do calloc here?
    if (expand) {
        c->stk_base=(BYTE *)malloc(1<<ARC_MAX_BITS);
        c->stk_ptr=c->stk_base;
    }
    if (compression_type==CT_7_BIT)
        c->min_bits=7;
    else
        c->min_bits=8;
    c->min_table_entry=1<<c->min_bits;
    c->free_index=c->min_table_entry;
    c->next_bits_in_use=c->min_bits+1;
    c->free_limit=1<<c->next_bits_in_use;
    c->saved_basecode=0xFFFFFFFFl;
    c->entry_used=TRUE;
    ArcEntryGet(c);
    c->entry_used=TRUE;
    return c;
}

void ArcCtrlDel(CArcCtrl *c)
{
    free(c->stk_base);
    free(c);
}

BYTE *ExpandBuf(CArcCompress *arc)
{
    CArcCtrl *c;
    BYTE *result;

    if (!(CT_NONE<=arc->compression_type && arc->compression_type<=CT_8_BIT) ||
            arc->expanded_size>=0x20000000l)
        return NULL;

    result=(BYTE *)malloc(arc->expanded_size+1);
    result[arc->expanded_size]=0; //terminate
    switch (arc->compression_type) {
        case CT_NONE:
            memcpy(result,arc->body,arc->expanded_size);
            break;
        case CT_7_BIT:
        case CT_8_BIT:
            c=ArcCtrlNew(TRUE,arc->compression_type);
            c->src_size=arc->compressed_size*8;
            c->src_pos=(sizeof(CArcCompress)-1)*8;
            c->src_buf=(BYTE *)arc;
            c->dst_size=arc->expanded_size;
            c->dst_buf=result;
            c->dst_pos=0;
            ArcExpandBuf(c);
            ArcCtrlDel(c);
            break;
    }
    return result;
}

long FSize(FILE *f)
{
    long	result,original=ftell(f);
    fseek(f,0,SEEK_END);
    result=ftell(f);
    fseek(f,original,SEEK_SET);
    return result;
}

// Sets decompressed to point to the allocated byte array
// Returns the number of bytes in that array
long decompress(BYTE *compressed, long compressed_size, BYTE**decompressed){
    DWORD out_size;
    CArcCompress *arc;
    BYTE *out_buf;
    arc=(CArcCompress *)malloc(compressed_size);
    memcpy(arc, compressed, compressed_size);
    out_size=arc->expanded_size;
    if (arc->compressed_size==compressed_size &&
            arc->compression_type && arc->compression_type<=3) {
        if (out_buf=ExpandBuf(arc)) {
            // Decompression was successful
        }
    }
    *decompressed = out_buf;

    free(arc);
    return out_size;
}

// DECOMPRESS STUFF copied from Compress.cpp
long ArcDetermineCompressionType(BYTE *src, long size)
{
  while (size--)
    if (*src++&0x80)
      return CT_8_BIT;
  return CT_7_BIT;
}

    /*
      From KernelB.html and KUtils.html in TempleOS source
    
      public _extern _BIT_FIELD_EXTRACT_U32 U32 BFieldExtU32(U8 *bit_field,I64 bit,I64 size); 
      //Extract U32 from bit field.

      _BIT_FIELD_OR_U32::
        PUSH    RBP
        MOV     RBP,RSP                 ; Some stack pointer setup stuff
        MOV     RBX,U64 SF_ARG2[RBP]    ; POS in RBX
        SHR     RBX,3                   ; SHIFT POS to the right 3 times (to get byte in array)
        ADD     RBX,U64 SF_ARG1[RBP]    ; ADD the byte pointer(arg1) to RBX (to get the correct byte in the array)
        MOV     RAX,U64 SF_ARG3[RBP]    ; Move the basecode(arg3) to RAX
        MOV     RCX,U64 SF_ARG2[RBP]    ; Move the POS to RCX
        AND     CL,7                    ; AND CL with 7 (the last 8 bits of RCX)
        SHL     RAX,CL                  ; Shift RAX CL bits to the left (so the basecode is ORed from the correct rit)
        OR      U64 [RBX],RAX           ; OR RAX with RBX, and I think the result is stored in RBX
        POP     RBP
        RET1    24
    */
void BFieldOrU32(BYTE * bit_field, long bit_num, DWORD pattern){
    bit_field += bit_num >> 3; // Increment bit_field pointer by bit_num/8 
    pattern <<= bit_num & 7; // Shift pattern bit_num % 8 to the left
    *(DWORD *)bit_field |= pattern; // OR the pattern on the bit_field.
}

void ArcCompressBuf(CArcCtrl *c)
{//Use $LK,"CompressBuf",A="MN:CompressBuf"$() unless doing more than one buf.
  CArcEntry *temp,*temp1;
  long ch,basecode;
  BYTE *src_ptr,*src_limit;

  src_ptr=c->src_buf+c->src_pos;
  src_limit=c->src_buf+c->src_size;

  if (c->saved_basecode==MAX_INT)
    basecode=*src_ptr++;
  else
    basecode=c->saved_basecode;

  while (src_ptr<src_limit && c->dst_pos+c->cur_bits_in_use<=c->dst_size) {
    ArcEntryGet(c);
ac_start:
    if (src_ptr>=src_limit) goto ac_done;
    ch=*src_ptr++;
    if (temp=c->hash[basecode])
      do {
	if (temp->ch==ch) {
	  basecode=temp-&c->compress[0];
	  goto ac_start;
	}
      } while (temp=temp->next);


    BFieldOrU32(c->dst_buf,c->dst_pos,basecode);
    c->dst_pos+=c->cur_bits_in_use;

    c->entry_used=TRUE;
    temp=c->cur_entry;
    temp->basecode=basecode;
    temp->ch=ch;
    temp1=&c->hash[basecode];
    temp->next=temp1->next;
    temp1->next=temp;

    basecode=ch;
  }
ac_done:
  c->saved_basecode=basecode;
  c->src_pos=src_ptr-c->src_buf;
}

BOOL ArcFinishCompression(CArcCtrl *c)
{//Do closing touch on archivew ctrl struct.
  if (c->dst_pos+c->cur_bits_in_use<=c->dst_size) {
    BFieldOrU32(c->dst_buf,c->dst_pos,c->saved_basecode);
    c->dst_pos+=c->next_bits_in_use;
    return TRUE;
  } else
    return FALSE;
}

long compress(BYTE ** compressed, BYTE *src,long size)
{//See $LK,"::/Demo/Dsk/SerializeTree.CPP"$.
  CArcCompress *arc;
  long size_out,compression_type=ArcDetermineCompressionType(src,size);
  CArcCtrl *c=ArcCtrlNew(FALSE,compression_type);
  c->src_size=size;
  c->src_buf=src;
  c->dst_size=(size+sizeof(CArcCompress))<<3;
  c->dst_buf=calloc(c->dst_size>>3, 1);
  c->dst_pos=(sizeof(CArcCompress) - 1)<<3;
  ArcCompressBuf(c);
  if (ArcFinishCompression(c) && c->src_pos==c->src_size) {
    size_out=(c->dst_pos+7)>>3;
    arc=malloc(size_out);
    memcpy(arc,c->dst_buf,size_out);
    arc->compression_type=compression_type;
    arc->compressed_size=size_out;
  } else {
    arc=malloc(size+sizeof(CArcCompress));
    memcpy(&arc->body,src,size);
    arc->compression_type=CT_NONE;
    arc->compressed_size=size+sizeof(CArcCompress);
  }

  arc->expanded_size=size;

  /*
  BYTE * pointer = (BYTE *) arc;
  int i;
  for (i =size_out - 196; i <= size_out; i++){
      printf("%02x", pointer[i]);
      if (i % 2){
          printf(" ");
      }
      if ((i + 1) % 16 == 0) {
          printf("\n");
      }
  }
  printf("\n");
  */

  free(c->dst_buf);
  ArcCtrlDel(c);
  *compressed = (BYTE*) arc;
  return arc->compressed_size;
}
