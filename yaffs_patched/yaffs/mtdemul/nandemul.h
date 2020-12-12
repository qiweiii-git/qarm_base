#ifndef __NANDEMUL_H__
#define __NANDEMUL_H__

typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned __u32;

int nandemul_Initialise(void);
int nandemul_DeInitialise(void);

int nandemul_GetNumberOfBlocks(__u32 *nBlocks);
int nandemul_Reset(void);
int nandemul_Read(__u8 *buffer, __u32 pageAddress, __u32 pageOffset, __u32 nBytes);
int nandemul_Program(__u8 *buffer, __u32 pageAddress, __u32 pageOffset, __u32 nBytes);



int nandemul_BlockErase(__u32 address);

int nandemul_ReadId(__u8 *vendorId, __u8 *deviceId);

int nandemul_CopyPage(__u32 fromAddress, __u32 toAddress);

int nandemul_ReadStatus(__u8 *status);

int nandemul_FailBlock(__u32 pageAddress);
int nandemul_CauseBitErrors( __u32 pageAddress, __u32 pageOffset, __u8 xorPattern);

#endif



