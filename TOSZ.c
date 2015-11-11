#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "TOSZ.h"

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

int Bt(int bit_num, BYTE *bit_field)
{
    bit_field+=bit_num>>3;
    bit_num&=7;
    return (*bit_field & (1<<bit_num)) ? 1:0;
}

int Bts(int bit_num, BYTE *bit_field)
{
    int result;
    bit_field+=bit_num>>3;
    bit_num&=7;
    result=*bit_field & (1<<bit_num);
    *bit_field|=(1<<bit_num);
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
    memset(c,0,sizeof(CArcCtrl));
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
long uncompress(BYTE *compressed, long compressed_size, BYTE**decompressed){
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
