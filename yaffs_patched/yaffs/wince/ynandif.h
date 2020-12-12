/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system. 
 * ynandif.h: Nand interface routines for WinCE version of YAFFS.
 *
 * Copyright (C) 2002-2003 Trimble Navigaion Ltd.
 *
 * Created by Brad Beveridge <brad.beveridge@trimble.co.nz>
 * Modified for CE 4.x by Steve Fogle <stevef@atworkcom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details. You should have received a 
 * copy of the GNU General Public License along with this program; 
 * if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
 *
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 *
 * $Id: ynandif.h,v 1.2 2003/01/31 00:52:53 charles Exp $
*/

#ifndef YNANDIF_H
#define YNANDIF_H

#include "yaffs_guts.h"

typedef struct
{
	int chunk;
	unsigned char *data;
	unsigned char *spare;
} ynandif_data;

typedef enum
{
	YNANDIF_WRITE = 0x59AFF,
	YNANDIF_READ,
	YNANDIF_ERASE,
	YNANDIF_INIT,
	YNANDIF_GETSIZE,
	//slf021105a begin
	YNANDIF_GETPARTITIONS,
	//slf021105a end
} ynandif_commands;

//slf021105a begin
typedef struct
{
	int startBlock;
	int endBlock;
	unsigned short volName[12];
} ynandif_partition;
//slf021105a end

/*
*	Functions that need to be provided for YAFFS
*/
int ynandif_WriteChunkToNAND(yaffs_Device *dev, int chunkInNAND,const __u8 *data, yaffs_Spare *spare);
int ynandif_ReadChunkFromNAND(yaffs_Device *dev,int chunkInNAND, __u8 *data, yaffs_Spare *spare);
int ynandif_EraseBlockInNAND(yaffs_Device *dev, int blockNumber);
int ynandif_InitialiseNAND(yaffs_Device *dev);
//slf021220a begin Cleanup block driver interface
void ynandif_DeinitialiseNAND(yaffs_Device *dev);
//slf021220a end Cleanup block driver interface

/*
*	Additional optional functions
*/

int ynandif_EraseAllBlocks(yaffs_Device *dev);
//slf021220a begin Cleanup block driver interface
//int ynandif_GetChipSize(unsigned char chipNumber);
int ynandif_GetChipSize(yaffs_Device *dev, unsigned char chipNumber);
//slf021220a end Cleanup block driver interface

#endif // end of file