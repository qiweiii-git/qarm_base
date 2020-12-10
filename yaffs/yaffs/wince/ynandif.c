/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 * ynandif.c: NAND interface functions for the WinCE port of YAFFS.
 *
 * Copyright (C) 2002-2003 Trimble Navigation Ltd.
 *
 * Created by Brad Beveridge <brad.beveridge@trimble.co.nz>
 * Modified for CE 4.x by Steve Fogle <stevef@atworkcom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
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
 * $Id: ynandif.c,v 1.2 2003/01/31 00:52:53 charles Exp $
 */
#include <windows.h>
#include <fsdmgr.h>
#include "ynandif.h"

//slf021220a Begin Cleanup block driver interface
#if _WINCEOSVER < 400
// For Win'CE 4.0 FSDMGR instead of direct access.
HANDLE devHandle = 0;
#endif
//slf021220a end Cleanup block driver interface

/*
*	Functions that need to be provided for YAFFS
*/
int ynandif_WriteChunkToNAND(yaffs_Device *dev, int chunkInNAND,const __u8 *data, yaffs_Spare *spare)
{
//slf021220a Begin Cleanup block driver interface
#if _WINCEOSVER >= 400
	if (dev)
#else
	if (devHandle)
#endif
//slf021220a end Cleanup block driver interface
	{
		ynandif_data writeData;
		int result;

		writeData.chunk = chunkInNAND;
		writeData.data = (__u8 *)data;
		writeData.spare = (char *)spare;
		
//slf021220a Begin Cleanup block driver interface
#if _WINCEOSVER >= 400
		if (!FSDMGR_DiskIoControl((HDSK)dev->genericDevice,
#else
		if (!DeviceIoControl(devHandle,
#endif
//slf021220a end Cleanup block driver interface
			 			     YNANDIF_WRITE,
						     &writeData,
						     sizeof(ynandif_data),
						     &result,
						     sizeof(result),
						     NULL,
							 NULL))
			return YAFFS_FAIL;

		return result;
	}

	return YAFFS_FAIL;
}

int ynandif_ReadChunkFromNAND(yaffs_Device *dev,int chunkInNAND, __u8 *data, yaffs_Spare *spare)
{
//slf021220a Begin Cleanup block driver interface
#if _WINCEOSVER >= 400
	if (dev)
#else
	if (devHandle)
#endif
//slf021220a end Cleanup block driver interface
	{
		ynandif_data readData;
		int result;

		readData.chunk = chunkInNAND;
		readData.data = data;
		readData.spare = (char *)spare;
		
//slf021220a Begin Cleanup block driver interface
#if _WINCEOSVER >= 400
		if (!FSDMGR_DiskIoControl((HDSK)dev->genericDevice,
#else
		if (!DeviceIoControl(devHandle,
#endif
//slf021220a end Cleanup block driver interface
			 			     YNANDIF_READ,
						     &readData,
						     sizeof(ynandif_data),
						     &result,
						     sizeof(result),
						     NULL,
							 NULL))
			return YAFFS_FAIL;

		return result;
	}

	return YAFFS_FAIL;
}

int ynandif_EraseBlockInNAND(yaffs_Device *dev, int blockNumber)
{
//slf021220a Begin Cleanup block driver interface
#if _WINCEOSVER >= 400
	if (dev)
#else
	if (devHandle)
#endif
//slf021220a end Cleanup block driver interface
	{
		int result;
		
//slf021220a Begin Cleanup block driver interface
#if _WINCEOSVER >= 400
		if (!FSDMGR_DiskIoControl((HDSK)dev->genericDevice,
#else
	if (!DeviceIoControl(devHandle,
#endif
//slf021220a end Cleanup block driver interface
			 			     YNANDIF_ERASE,
						     &blockNumber,
						     sizeof(int),
						     &result,
						     sizeof(result),
						     NULL,
							 NULL))
			return YAFFS_FAIL;

		return result;
	}

// if we return YAFFS_FAIL, then yaffs will retire this block & mark it bad, not exactly
// what we want to do by default.
	return YAFFS_OK;
}

int ynandif_InitialiseNAND(yaffs_Device *dev)
{

//slf021220a Begin Cleanup block driver interface
#if _WINCEOSVER < 400
// For Win'CE 4.0 FSDMGR instead of direct access.
	RETAILMSG(1, (L"ynandif_InitialiseNAND\r\n"));
	devHandle = CreateFile(L"YND1:",
						   GENERIC_READ|GENERIC_WRITE,
						   0,
						   NULL,
						   OPEN_EXISTING,
						   0,
						   0);

//slf021220d Begin CreateFile returns INVALID_HANDLE_VALUE not null fix.
	if (INVALID_HANDLE_VALUE == devHandle)
	{
		devHandle = NULL;
		return 0;
	}
//	if (!devHandle)
//		return 0;
//slf021220d end CreateFile returns INVALID_HANDLE_VALUE not null fix.

	RETAILMSG(1, (L"devhandle open\r\n"));
#endif
//slf021220a Begin Cleanup block driver interface

	RETAILMSG(1, (L"DeviceIo INIT\r\n"));
//slf021220a Begin Cleanup block driver interface
	if (dev)
	{
#if _WINCEOSVER >= 400
		if (!FSDMGR_DiskIoControl((HDSK)dev->genericDevice,
#else
		if (!DeviceIoControl(devHandle,
#endif
//slf021220a end Cleanup block driver interface
 						 YNANDIF_INIT,
						 NULL,
						 0,
						 NULL,
						 0,
						 NULL,
						 NULL))
			return 0;
//slf021220a Begin Cleanup block driver interface
	}
//slf021220a end Cleanup block driver interface

	if (dev)
	{
//slf021220a Begin Cleanup block driver interface
//		int nBlocks = ynandif_GetChipSize(0xFF) / (YAFFS_CHUNKS_PER_BLOCK * YAFFS_BYTES_PER_CHUNK);
		int nBlocks = ynandif_GetChipSize(dev,0xFF) / (YAFFS_CHUNKS_PER_BLOCK * YAFFS_BYTES_PER_CHUNK);
//slf021220a end Cleanup block driver interface
		dev->startBlock = 1;  // Don't use block 0
		dev->endBlock = nBlocks - 1;
	}

	return 1;
}

//slf021220a Begin Cleanup block driver interface
void ynandif_DeinitialiseNAND(yaffs_Device *dev)
{
	RETAILMSG(1, (L"ynandif_DeinitialiseNAND\r\n"));
#if _WINCEOSVER < 400
	if (devHandle)
	{
		CloseHandle(devHandle);
		devHandle = NULL;
	}
#endif
}
//slf021220a end Cleanup block driver interface

int ynandif_EraseAllBlocks(yaffs_Device *dev)
{
	int numBlocks, counter;
//slf021220a Begin Cleanup block driver interface
//	numBlocks = ynandif_GetChipSize(0xFF) / (YAFFS_CHUNKS_PER_BLOCK * YAFFS_BYTES_PER_CHUNK);
	numBlocks = ynandif_GetChipSize(dev,0xFF) / (YAFFS_CHUNKS_PER_BLOCK * YAFFS_BYTES_PER_CHUNK);
//slf021220a end Cleanup block driver interface
	for (counter = 0; counter < numBlocks; counter++)
	{
		ynandif_EraseBlockInNAND(dev, counter);
	}
	return YAFFS_OK;
}

//slf021220a Begin Cleanup block driver interface
//int ynandif_GetChipSize(unsigned char chipNumber)
int ynandif_GetChipSize(yaffs_Device *dev, unsigned char chipNumber)
//slf021220a end Cleanup block driver interface
{
	int ret = 0;
	RETAILMSG(1, (L"DeviceIo GETSIZE\r\n"));
//slf021220a Begin Cleanup block driver interface
#if _WINCEOSVER >= 400
	if (dev)
#else
	if (devHandle)
#endif
//slf021220a end Cleanup block driver interface
	{
		RETAILMSG(1, (L"DeviceIo GETSIZE - getting ret\r\n"));
//slf021220a Begin Cleanup block driver interface
#if _WINCEOSVER >= 400
		FSDMGR_DiskIoControl((HDSK)dev->genericDevice,
#else
		DeviceIoControl(devHandle,
#endif
//slf021220a end Cleanup block driver interface
					YNANDIF_GETSIZE,
					NULL,
					0,
					&ret,
					sizeof(ret),
					NULL,
					NULL);
	}
	RETAILMSG(1, (L"DeviceIo GETSIZE ret %X\r\n", ret));



	return ret;
}
