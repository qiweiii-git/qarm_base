/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 * yaffsfsd.c: FSD interface funtions for WinCE.
 *
 * Copyright (C) 2002 Trimble Navigation Ltd.
 *
 * Created by Charles Manning <charles.manning@trimble.co.nz>
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
 * Acknowledgements:
 *  Various clean-ups and WinCE4 support by Steve Fogle and Lynn Winter
 * $Id: yaffsfsd.c,v 1.4 2003/01/31 03:30:33 charles Exp $
 */
#include <windows.h>
#include <extfile.h>
#include <yaffs_guts.h>
#include <ynandif.h>
//slf021104b begin
#include <diskio.h>
//slf021104b end

#define MAX_WIN_FILE	200
#define YFSD_NAME_LENGTH 128
#define YFSD_FULL_PATH_NAME_SIZE 500


#define YFSD_DISK_NAME L"Disk"
#define YFSD_BOOT_NAME L"Boot"

#define PARTITION_START_NUMBER (1280)	


// if this is defined the there will be a constant message box raised to display status
//#define MSGBOX_DISPLAY
	

//#define MSGSTATE 1
#define MSGSTATE 0
//#define DISABLE_BOOT_PARTITION
//slf021105a begin
// Define DO_PARTITION_TABLE to cause the partition table 
// information to be retrieved from the block driver.
// Can define this in your sources file.
//#define DO_PARTITION_TABLE
// How many partitions the disk might have.  2 gives 
// space for the "Disk" and "Boot" partitions.
#define MAXPARTITIONS (2)
//slf021105a end

//unsigned yaffs_traceMask=0xffffffff;
unsigned yaffs_traceMask=0;


typedef struct
{
	yaffs_Device dev;
	DWORD	hdsk;
	DWORD    mgrVolume; // The volume id from the manager issed when we registered it - it is an HVOL
	BOOL	isMounted;
	BOOL	configured;
//	DWORD	guard0[100];
//	DWORD	guard1[100];
	SHELLFILECHANGEFUNC_t shellFunction;
	PWSTR volName;
} yfsd_Volume;

typedef struct 
{
	yaffs_Object *obj;
	DWORD offset;
	BOOL isopen;
	BOOL dirty;
	WCHAR *fullName;
	yfsd_Volume *myVolume;
	BOOL writePermitted;
	BOOL readPermitted;
	BOOL shareRead;
	BOOL shareWrite;

}	yfsd_WinFile;

struct yfsd_FoundObjectStruct
{
  yaffs_Object *obj;
  struct yfsd_FoundObjectStruct *next;
};

typedef struct yfsd_FoundObjectStruct yaffs_FoundObject;

typedef struct 
{
	yaffs_Object *dir;
	char pattern[YFSD_NAME_LENGTH+1];
	yaffs_FoundObject *foundObjects;
}	yfsd_WinFind;



#define PSEARCH yfsd_WinFind*

#define PVOLUME yfsd_Volume*
#define PFILE   yfsd_WinFile*

#define FSD_API	YFSD

#include <fsdmgr.h>

//slf021105a begin
//static yfsd_Volume disk_volume;
//static yfsd_Volume boot_volume;
static yfsd_Volume * disk_volumes[MAXPARTITIONS];
//slf021105a end;

static CRITICAL_SECTION yaffsLock;
static CRITICAL_SECTION winFileLock;

static int yaffsLockInited = 0;

static yfsd_WinFile yfsd_winFile[MAX_WIN_FILE];

#if 0
static yfsd_SetGuards(void)
{
	int i;
	for(i = 0; i < 100; i++)
	{
		yfsd_volume.guard0[i] = yfsd_volume.guard1[i] = i;
	}
}

static void yfsd_CheckGuards(void)
{
	int i;
	int found;
	for(i = found = 0; i < 100 && !found; i++)
	{
			if(yfsd_volume.guard0[i] != i)
			{
					RETAILMSG (MSGSTATE, (L"YAFFS:: guard 0 %d brocken\r\n",i));
					found = 1;
			}
			if(yfsd_volume.guard1[i] != i)
			{
					RETAILMSG (MSGSTATE, (L"YAFFS:: guard 0 %d brocken\r\n",i));
					found = 1;
			}
	}
}
#endif


#ifdef MSGBOX_DISPLAY
DWORD WINAPI yfsd_MessageThread(LPVOID param)
{
    yaffs_Device *dev = (yaffs_Device *)param;
    TCHAR dataBuffer[1000];
    Sleep(10000);

    // note : if the device gets free'd from under us, we will cause an exception in the loop
    while (1)
    {
        wsprintf(dataBuffer, L"nShortOpCaches %i\r\n"
                             L"nErasedBlocks %i\r\n"
                             L"allocationBlock %i\r\n"
                             L"allocationPage %i\r\n"
                             L"garbageCollectionRequired %i\r\n"
                             L"nRetiredBlocks %i\r\n"
                             L"cacheHits %i\r\n"
                             L"eccFixed %i\r\n"
                             L"eccUnfixed %i\r\n"
                             L"tagsEccFixed %i\r\n"
                             L"tagsEccUnfixed %i\r\n",
                             dev->nShortOpCaches, 
                             dev->nErasedBlocks,
                             dev->allocationBlock,
                             dev->allocationPage,
                             dev->garbageCollectionRequired,
                             dev->nRetiredBlocks,
                             dev->cacheHits,
                             dev->eccFixed,
                             dev->eccUnfixed,
                             dev->tagsEccFixed,
                             dev->tagsEccUnfixed);

        MessageBox(NULL,
                   dataBuffer,
                   L"YAFFS PROC INFO",
                   MB_OK);
        Sleep(1);
    }
}
#endif

void yfsd_LockWinFiles(void)
{
	//RETAILMSG (MSGSTATE, (L"YAFFS::LockWinfiles\r\n"));
	EnterCriticalSection(&winFileLock);
}
void yfsd_UnlockWinFiles(void)
{
	//RETAILMSG (MSGSTATE, (L"YAFFS::UnlockWinFiles\r\n"));
	LeaveCriticalSection(&winFileLock);
}

int lockwaits;

void yfsd_LockYAFFS(void)
{
	//yfsd_CheckGuards();
	//RETAILMSG (MSGSTATE, (L"YAFFS::LockYAFFS %d ",lockwaits));
	lockwaits++;
	EnterCriticalSection(&yaffsLock);
	//RETAILMSG (MSGSTATE, (L" locked\r\n"));
}
void yfsd_UnlockYAFFS(void)
{
	//yfsd_CheckGuards();
	//RETAILMSG (MSGSTATE, (L"YAFFS::UnLockYAFFS "));
	LeaveCriticalSection(&yaffsLock);
	lockwaits--;
	//RETAILMSG (MSGSTATE, (L" unlocked\r\n"));
}


void yfsd_InitialiseWinFiles(void)
{
	int i;
	
	RETAILMSG (MSGSTATE, (L"YAFFS::InitWinFiles\r\n"));

	InitializeCriticalSection(&winFileLock);

	yfsd_LockWinFiles();
	for(i = 0; i < MAX_WIN_FILE; i++)
	{
			yfsd_winFile[i].isopen = 0;
	}
	yfsd_UnlockWinFiles();
}

yfsd_WinFile * yfsd_GetWinFile(void)
{
	int i;
	RETAILMSG (MSGSTATE, (L"YAFFS::GetWinFiles\r\n"));

	yfsd_LockWinFiles();

	for(i = 0; i < MAX_WIN_FILE; i++)
	{
		if(!yfsd_winFile[i].isopen)
		{
			yfsd_winFile[i].isopen = 1;
			yfsd_winFile[i].writePermitted = 0;
			yfsd_winFile[i].readPermitted = 0;
			yfsd_winFile[i].shareRead = 0;
			yfsd_winFile[i].shareWrite = 0;
			yfsd_winFile[i].dirty = 0;
			yfsd_winFile[i].fullName = NULL;
			yfsd_winFile[i].obj = NULL;

			yfsd_UnlockWinFiles();
			return &yfsd_winFile[i];
		}
	}

	yfsd_UnlockWinFiles();

	RETAILMSG (MSGSTATE, (L"YAFFS::GetWinFiles did not find a handle. Too many open.\r\n"));

	return NULL;
}

void yfsd_PutWinFile(yfsd_WinFile *f)
{
	RETAILMSG (MSGSTATE, (L"YAFFS::PutWinFile\r\n"));
	yfsd_LockWinFiles();
	f->isopen = 0;
	f->obj = NULL;
	if(f->fullName)
	{
		free(f->fullName);
		f->fullName = NULL;
	}

	yfsd_UnlockWinFiles();
}



void yfsd_FlushAllFiles(void)
{
	int i;
	RETAILMSG (MSGSTATE, (L"YAFFS::FlushAllFiles\r\n"));

	yfsd_LockYAFFS();
	yfsd_LockWinFiles();
	for(i = 0; i < MAX_WIN_FILE; i++)
	{
		if(yfsd_winFile[i].isopen &&
		   yfsd_winFile[i].obj)
		{
			yaffs_FlushFile(yfsd_winFile[i].obj,1);
		}
	}
	yfsd_UnlockWinFiles();
	yfsd_UnlockYAFFS();
}

//slf021104d begin
//////////////////////////////////////////////////////////////////////
// Search through winFiles to see if any are open.  

BOOL yfsd_FilesOpen(void)
{
	int i;
	BOOL rval;
	RETAILMSG (MSGSTATE, (L"YAFFS::FilesOpen?\r\n"));

	yfsd_LockWinFiles();
	for(i = 0, rval = FALSE; i < MAX_WIN_FILE; i++)
	{
		if(yfsd_winFile[i].isopen)
		{
			rval = TRUE;
			break;
		}
	}
	yfsd_UnlockWinFiles();
	return rval;
}
//slf021104d end

PWSTR yfsd_FullPathName(PVOLUME vol, PWSTR fpn,int slength,PCWSTR pathName)
{

	// todo check for bounds
	//slf021104b begin
	//volName already has the initial backslash if it needs it.
	//wcscpy(fpn,L"\\");
	//wcscat(fpn,vol->volName);
	wcscpy(fpn,vol->volName);
	//slf021104b end
	if(pathName[0] != '\\')
	{
		wcscat(fpn,L"\\");
	}
	wcscat(fpn,pathName);

	return fpn;

}


// FILETIME is a 64-bit value as 100-nanosecond intervals since January 1, 1601.

void yfsd_U32sToWinFileTime(__u32 target[2], FILETIME *wft)
{
	
	wft->dwLowDateTime = target[0];
	wft->dwHighDateTime = target[1];

}

void yfsd_NullWinFileTime(FILETIME *wft)
{
	wft->dwLowDateTime = 0;
	wft->dwHighDateTime = 0;
}

void yfsd_WinFileTimeToU32s(const FILETIME *wft, __u32 target[2])
{
	target[0] = wft->dwLowDateTime;
	target[1] = wft->dwHighDateTime;
}

void  yfsd_WinFileTimeNow(__u32 target[2])
{
	SYSTEMTIME st;
	FILETIME ft;

	GetSystemTime(&st);
	SystemTimeToFileTime(&st,&ft);
	yfsd_WinFileTimeToU32s(&ft,target);
}

// Cut down the name to the parent directory, then inform the shell of
// a change to the directory.
void yfsd_ShellDirectoryChanged(PVOLUME pVolume, PWSTR fullPathName)
{
	WCHAR str[500];
	int i;
	wcscpy(str,fullPathName);

	i = wcslen(str) - 1;
	
	if(i > 0)
	{
		str[i] = 0;
		i--;
	}

	// Curveball if the name is a directory (ie. we're doing an update of
	// a directory because we added a directory item). , then it might end in a \
	// which we must toss first
	if(i >= 0 && (str[i] == '\\' || str[i] == '/'))
	{
		str[i] = 0;
		i--;
	}

	// Ok, now strip back...

	while(i >= 0 && str[i] != '\\' && str[i] != '/')
	{
		str[i] = 0;
		i--;
	}

	if(pVolume->shellFunction)
	{
			FILECHANGEINFO fc;
			
			fc.cbSize = sizeof(FILECHANGEINFO);
			fc.wEventId = SHCNE_UPDATEDIR;
			fc.uFlags = SHCNF_PATH;
			fc.dwItem1 = (DWORD)str;
			fc.dwItem2 = 0;
			fc.dwAttributes = 0;
			yfsd_NullWinFileTime(&fc.ftModified);
			fc.nFileSize = 0;

			pVolume->shellFunction(&fc);
			RETAILMSG (MSGSTATE, (L"YAFFS:: directory changed %s\r\n",str));

	}


}


// Minimal name test for now
BOOL yfsd_NameIsValid (const char *name)
{
	int length = strlen(name);

	return (length > 0 && length <= YFSD_NAME_LENGTH);

}

// File attributes:
// Wince understands the following attributes of any use to YAFFS:
//  
//   ARCHIVE
//   HIDDEN
//   READONLY
//   SYSTEM
//   TEMPORARY
//
//	 Also, FILE_ATTRIBUTE_DIRECTORY is used to mark directories.
//
//   It also understands NORMAL. If no other flag is set, then set NORMAL.
//   If any of the above are set, then NORMAL must **not** be set.
//	 Ignore this and the WinCE Explorer barfs the file.
//
//
// in addition, GetAttributes also returns FILE_ATTRIBUTE_DIRECTORY

// The following are valid ones we get presented with,
// but must filter out the stuff we don't unserstand
//#define FILE_ATTRIBUTE_READONLY             0x00000001  
//#define FILE_ATTRIBUTE_HIDDEN               0x00000002  
//#define FILE_ATTRIBUTE_SYSTEM               0x00000004  
//#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  
//#define FILE_ATTRIBUTE_ARCHIVE              0x00000020  
//#define FILE_ATTRIBUTE_INROM				  0x00000040
//#define FILE_ATTRIBUTE_ENCRYPTED            0x00000040  
//#define FILE_ATTRIBUTE_NORMAL               0x00000080  
//#define FILE_ATTRIBUTE_TEMPORARY            0x00000100  
//#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200  
//#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400  
//#define FILE_ATTRIBUTE_COMPRESSED           0x00000800  
//#define FILE_ATTRIBUTE_OFFLINE              0x00001000  
//#define FILE_ATTRIBUTE_ROMSTATICREF		  0x00001000
//#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000  
//#define FILE_ATTRIBUTE_ROMMODULE			  0x00002000


BOOL yfsd_CheckValidAttributes(DWORD attribs)
{

	RETAILMSG (MSGSTATE, (L"Attributes:%X\r\n", attribs));

#if 0
		// If NORMAL, then nothing else
		if(attribs & FILE_ATTRIBUTE_NORMAL && attribs != FILE_ATTRIBUTE_NORMAL)
			return FALSE;
		if(attribs == FILE_ATTRIBUTE_NORMAL) 
			return TRUE;
#endif
		// Check that the bits are in the valid set
		if(attribs & ~(0x3FE7))
			return FALSE;

		return TRUE;

}
DWORD yfsd_GetObjectWinAttributes(yaffs_Object *obj)
{

		DWORD result;
		
		result = obj->st_mode & 
					(FILE_ATTRIBUTE_READONLY | 
					 FILE_ATTRIBUTE_ARCHIVE | 
					 FILE_ATTRIBUTE_HIDDEN |
					 FILE_ATTRIBUTE_SYSTEM);

		if(obj->variantType == YAFFS_OBJECT_TYPE_DIRECTORY) result |= FILE_ATTRIBUTE_DIRECTORY;

		if(result & ~FILE_ATTRIBUTE_NORMAL)
		{ 
			result &= ~FILE_ATTRIBUTE_NORMAL;
		}
		else 
		{
			result = FILE_ATTRIBUTE_NORMAL;
		}


		return result;
}



/*
*	Runs over input until a '\' is found, or '\0' is found, or outSize - 1 is
*	reached.  Characters are copied from input into output until the above stop
*	condition is reached - output is then given a '\0'.  output must be at least
*	as large as outSize
*/
static int parseToNextSlash(const unsigned short *input, char *output, int outSize)
{
	int counter = 0;
	char *t = output;
	/* strip any starting \'s */
	//RETAILMSG(1, (L"\r\nParsing.. "));
	while (*input == '\\' || *input == '/') input++, counter++;

	for (; counter < outSize - 1; counter++)
	{
		if (*input == '\0' ||
			((*input == '\\' || *input == '/') && input[1] == '\0'))   // special case: if the string ends in a '\', then toss the '\'
		{
			counter = -1;   // break & tell people we've run to the end
			break;
		}
		if (*input == '\\' || *input == '/')
			break;
		//RETAILMSG(1, (L"%c", *input));
		*output = (char) (*input);
		input++;
		output++;
	}
	*output++ = '\0';
	*output  = '\0';
//	RETAILMSG(1, (L"\r\nOut %a\r\n", t));
	
	return counter;
}

/*
*	Since the notion of paths as WinCE sees them and as YAFFS sees them
*	is different, we needed a helper function to search from the root of
*	device along the string in path.  The processed pointer is set to where
*	abouts in the string the function was able to search up to.
*/
yaffs_Object *yfsd_FindDirectoryByWinPath(yaffs_Device *device, const wchar_t *path, char *processed, int length)
{
	// a buffer to keep the current chunk of path we're talking about it
	char pathChunk[255];
	int chunkSize;
	int counter;
	// the current object we are at
	yaffs_Object *current;

	RETAILMSG (MSGSTATE, (L"YAFFS::FindByWinPath (%s) : ", path));
	// start at the root of this device
	current = yaffs_Root(device);
	*processed = '\0';

	do
	{
	//	parse chunks until we run out
		chunkSize = parseToNextSlash(path, pathChunk, 255);
//		RETAILMSG(1, (L"Chunk %a\r\n", pathChunk));
		if (chunkSize == -1)
			break;
	//	move the path pointer along
		path += chunkSize;
	//	try and find the next yaffs object by chunkname	
		current = yaffs_FindObjectByName(current, pathChunk);
		if (current == 0)
		{
			processed[0] = '\0';
			return 0;
		}
	} while (1);

	for (counter = 0; counter < length; counter++)
	{
		// Get the rest of the string
		processed[counter] = pathChunk[counter];
		if (pathChunk[counter] == '\0')
			break;
	}

	RETAILMSG (MSGSTATE, (L"YAFFS::FindDirectoryByWinPath parent:%X name:%a\r\n", current,processed));

	return current;
}


yaffs_Object *yfsd_FindObjectByWinPath(yaffs_Device *dev, PCWSTR pwsFileName )
{
	yaffs_Object *obj = NULL;
	yaffs_Object *parent = NULL;
	char name[YFSD_NAME_LENGTH+1];

	RETAILMSG (MSGSTATE, (L"YAFFS::FindObjByWinPath\r\n"));

	parent = yfsd_FindDirectoryByWinPath(dev,pwsFileName,name,YFSD_NAME_LENGTH);

	if(parent && yfsd_NameIsValid(name))
	{
		obj = yaffs_FindObjectByName(parent,name);
	}

	RETAILMSG (MSGSTATE, (L"YAFFS::FindObjectByWinPath parent:%X obj:%X\r\n", parent,obj));

	return obj;
}

BOOL YFSD_InitVolume(HDSK hdsk, yfsd_Volume *vol, int startBlock, int endBlock, PWSTR volName)
{
	//slf021104b Begin
	WCHAR szName[MAX_PATH];
    DWORD dwAvail;
	//slf021104b end
	RETAILMSG (MSGSTATE, (L"YAFFS::InitVolume\r\n"));
	//slf021104b Begin filled in later.
	//vol->volName = volName;
	//slf021104b end


	yfsd_LockYAFFS();
	
	//slf021220a Begin Cleanup block driver interface
#if _WINCEOSVER >= 400
	// For Win'CE 4.0 and later pass the hdsk for use by the yandif layer.
	vol->dev.genericDevice = (PVOID)hdsk;
#endif
	//slf021220a End Cleanup block driver interface

	//Mount/initialise YAFFs here
	//slf021127a begin check for error returns!
	if (ynandif_InitialiseNAND(&vol->dev))  
	{
	//slf021127a end check for error returns!
		vol->dev.writeChunkToNAND = ynandif_WriteChunkToNAND;
		vol->dev.readChunkFromNAND = ynandif_ReadChunkFromNAND;
		vol->dev.eraseBlockInNAND = ynandif_EraseBlockInNAND;
		vol->dev.initialiseNAND = ynandif_InitialiseNAND;
		vol->dev.startBlock = startBlock;
		if (endBlock != -1)
			vol->dev.endBlock = endBlock;
		vol->dev.nShortOpCaches = 10; // a nice number of caches.
		vol->dev.nChunksPerBlock = YAFFS_CHUNKS_PER_BLOCK;
		vol->dev.nBytesPerChunk = YAFFS_BYTES_PER_CHUNK;
		vol->dev.nReservedBlocks = 5; // a nice reserve size


		// nBlocks is set the total size of the disk, not the partition
	//	vol->dev.nBlocks = endBlock - startBlock + 1;

	//	qnand_EraseAllBlocks(&vol->dev);

		//slf021127a begin check for error returns!
		if (yaffs_GutsInitialise(&vol->dev))
		{
		//slf021127a end check for error returns!
			RETAILMSG(1, (L"YAFFS::Done yaffs_GutsInitialise\r\n"));

			RETAILMSG(1, (L"Blocks start %d end %d Group size %d bits %d\r\n",
							vol->dev.startBlock,vol->dev.endBlock,
							vol->dev.chunkGroupSize,vol->dev.chunkGroupBits));


#if 0
			for(i = vol->dev.startBlock; i <= vol->dev.endBlock; i++)
			{
					switch(vol->dev.blockInfo[i].blockState)
					{
						case YAFFS_BLOCK_STATE_DEAD:
				
							RETAILMSG(1, (L"YAFFS::Dead block %d\r\n",i));
							deadBlox++;
							break;
						case YAFFS_BLOCK_STATE_EMPTY: emptyBlox++; break;
						case YAFFS_BLOCK_STATE_FULL: fullBlox++; break;
						case YAFFS_BLOCK_STATE_ALLOCATING: allocatingBlox++; break;
						case YAFFS_BLOCK_STATE_DIRTY: dirtyBlox++; break;
						default:
							RETAILMSG(1, (L"YAFFS::Block %d has goofus state %d\r\n",i,vol->dev.blockInfo[i].blockState));
							break;
					}
			}

			RETAILMSG(1, (L"Blocks dead  %d empty %d full %d allocating %d dirty %d\r\n",
							deadBlox,emptyBlox,fullBlox,allocatingBlox,dirtyBlox));

#endif

//slf021127a begin check for error returns!
			vol->isMounted = 1;
		}
	}
//slf021127a begin check for error returns!
	
	yfsd_UnlockYAFFS();

//slf021127a begin check for error returns!
//	vol->isMounted = 1;
//slf021127a begin check for error returns!
	
	//slf021104b begin
	//vol->mgrVolume = FSDMGR_RegisterVolume(hdsk,vol->volName,vol);
	// If the caller passed a volume name use it.
	if (volName[0])
        wcscpy( szName, volName);
#if WINCEOSVER >= 400
	// The user passed an empty volume name.  On CE 4.xx try to get
	// if from the block driver (which got it from the registry).
	else if (!FSDMGR_DiskIoControl(hdsk, DISK_IOCTL_GETNAME, NULL, 0, (LPVOID)szName, sizeof(szName), &dwAvail, NULL)) 
#else
	else
#endif
	{ 
		// Didn't get a volume name so use "Disk" by default.
        wcscpy( szName, YFSD_DISK_NAME);
    }    
 	vol->mgrVolume = FSDMGR_RegisterVolume(hdsk,szName,vol);
	//slf021104b end

	if(vol->mgrVolume)
	{
		//slf021104b Begin
		// Get some space for the volume name.
        vol->volName = malloc( MAX_PATH * sizeof(WCHAR));
        if (vol->volName) 
		{
#if WINCEOSVER >= 400
			// Get the name we were really mounted under.
            FSDMGR_GetVolumeName(vol->mgrVolume, vol->volName, MAX_PATH);

			// If we got mounted as root then throw away the backslash
			// so we won't get a double backslash when volName is
			// prepended to the path in the full path name calculation
			// that is used for shell callbacks.
			if (0 == wcscmp(vol->volName,L"\\"))
				vol->volName[0] = 0;
#else
			// Use the name we asked to be mounted under for
			// our root.  
			wcscpy(vol->volName,L"\\");
			wcscat(vol->volName, szName);
#endif
		}
		//slf021104b end
		return TRUE;
	}
	else
	{
		vol->isMounted = 0;
		SetLastError(ERROR_OUTOFMEMORY);
		return FALSE;
	}	
}


BOOL YFSD_MountDisk(HDSK hdsk)
{
//slf021105a begin
#ifdef DO_PARTITION_TABLE
	ynandif_partition PartTable[MAXPARTITIONS];
	DWORD dwAvail;
	int i;
	BOOL rval = FALSE;
#endif
//slf021105a end
	int deadBlox=0,emptyBlox=0,fullBlox=0,allocatingBlox=0,dirtyBlox=0;
	//int i;
	// Called to mount a disk.
	// NB THis call might happen redundantly.
	//
	//
	// If yaffs is not initialised, then call the 
	// initialisation function
	//
	RETAILMSG (MSGSTATE, (L"YAFFS::MountDisk\r\n"));

	if (!yaffsLockInited)
	{
		InitializeCriticalSection(&yaffsLock);
		yfsd_InitialiseWinFiles();
		yaffsLockInited = 1;
	}

	//slf021105a begin
	memset(disk_volumes,0,sizeof(disk_volumes));
#ifdef DO_PARTITION_TABLE
	memset(&PartTable,0,sizeof(PartTable));
	// Call the block driver to get the partition table from it.
    if (FSDMGR_DiskIoControl(hdsk, YNANDIF_GETPARTITIONS, NULL, 0, (LPVOID)&PartTable, sizeof(PartTable), &dwAvail, NULL)) 
	{
		// Scan throught the table it return.
		for (i=0; i<MAXPARTITIONS; i++)
		{
			// At the very lease check that the end is later than the beginning
			// and don't let it start at 0.  
			// Probably could do more thorough checking but I trust the block
			// driver.
			if (PartTable[i].startBlock && (PartTable[i].endBlock > PartTable[i].startBlock))
			{
				// Found a partion.  Get a volume structure to hold it.
				disk_volumes[i] = malloc(sizeof(yfsd_Volume));
				if (disk_volumes[i])
				{
					memset(disk_volumes[i],0,sizeof(yfsd_Volume));
					// Go init the volume.  Note that if the block driver wants the
					// name to come from the registry it will have returned an
					// empty name string.
					YFSD_InitVolume(hdsk,disk_volumes[i],PartTable[i].startBlock,PartTable[i].endBlock,PartTable[i].volName);
					if (disk_volumes[i]->isMounted)
						rval = TRUE; //Hey, we found at least on partition.
				}
			}
		}
	}

	return rval;

#else
#ifdef DISABLE_BOOT_PARTITION
	// Only want disk volume
	disk_volumes[0] = malloc(sizeof(yfsd_Volume));
	if (disk_volumes[0])
	{
		memset(disk_volumes[0],0,sizeof(yfsd_Volume));
		YFSD_InitVolume(hdsk, disk_volumes[0], 1, -1, YFSD_DISK_NAME);

		if(disk_volumes[0].isMounted)
		{
			return TRUE;
		}
	}
	if (disk_volumes[0])
	{
		free(disk_volumes[0];
		disk_volumes[0] = NULL;
	}
#else
	// Want both boot and disk
	disk_volumes[0] = malloc(sizeof(yfsd_Volume));
	disk_volumes[1] = malloc(sizeof(yfsd_Volume));
	if (disk_volumes[0] && disk_volumes[1])
	{
		memset(disk_volumes[0],0,sizeof(yfsd_Volume));
		memset(disk_volumes[1],0,sizeof(yfsd_Volume));
		YFSD_InitVolume(hdsk, disk_volumes[0], PARTITION_START_NUMBER+1, -1, YFSD_DISK_NAME);
		YFSD_InitVolume(hdsk, disk_volumes[1], 1, PARTITION_START_NUMBER, YFSD_BOOT_NAME);

#ifdef MSGBOX_DISPLAY
        // pass the device we are sniffing to the thread
        CreateThread(NULL, 0, yfsd_MessageThread, (LPVOID)&disk_volumes[0]->dev, 0, NULL);
#endif

		if(disk_volumes[0]->isMounted && disk_volumes[1]->isMounted)
		{
			return TRUE;
		}
	}

	// If we got this far something went wrong.  Make sure to 
	// free any memory we allocated.
	if (disk_volumes[0])
	{
		if (disk_volumes[0]->volName)
		{
			free(disk_volumes[0]->volName);
		}
		free(disk_volumes[0]);
		disk_volumes[0] = NULL;
	}
	if (disk_volumes[1])
	{
		if (disk_volumes[1]->volName)
		{
			free(disk_volumes[1]->volName);
		}
		free(disk_volumes[1]);
		disk_volumes[1] = NULL;
	}
#endif

	return FALSE;

	// Only want disk volume
//	YFSD_InitVolume(hdsk, &disk_volume, 1, -1, YFSD_DISK_NAME);
//
//	
//	if(disk_volume.isMounted)
//	{
//		return TRUE;
//	}
//#else
//	// Want both boot and disk
//	YFSD_InitVolume(hdsk, &disk_volume, PARTITION_START_NUMBER+1, -1, YFSD_DISK_NAME);
//	YFSD_InitVolume(hdsk, &boot_volume, 1, PARTITION_START_NUMBER, YFSD_BOOT_NAME);
//
//	
//	if(disk_volume.isMounted && boot_volume.isMounted)
//	{
//		return TRUE;
//	}
//#endif
//
//	return FALSE;
#endif
//slf021105a end

//	yfsd_SetGuards();

	// todo - get name from registry

}


BOOL YFSD_UnmountDisk(HDSK hdsk)
{
//slf021105a begin
	int i;
//slf021105a end
	RETAILMSG (MSGSTATE, (L"YAFFS::UnmountDisk\r\n"));
	
	//slf021104d begin
	// If there are any files open don't let them dismount
	// it or the system will get very confused.  
	if (yfsd_FilesOpen())
		return FALSE;

	//yfsd_FlushAllFiles();
	//slf021104d end

	yfsd_LockYAFFS();
//slf021105a begin
//	yaffs_Deinitialise(&disk_volume.dev);
//	yaffs_Deinitialise(&boot_volume.dev);
//	yfsd_UnlockYAFFS();
//
//	FSDMGR_DeregisterVolume(disk_volume.mgrVolume);
//	FSDMGR_DeregisterVolume(boot_volume.mgrVolume);

	// Walk through the partions deinitializing, deregistering
	// and freeing them.
	for (i=0; i<MAXPARTITIONS; i++)
	{
		if (disk_volumes[i])
		{
			yaffs_Deinitialise(&(disk_volumes[i]->dev));
//slf021220a Begin Cleanup block driver interface
			ynandif_DeinitialiseNAND(&(disk_volumes[i]->dev));
//slf021220a end Cleanup block driver interface
			FSDMGR_DeregisterVolume(disk_volumes[i]->mgrVolume);
			if (disk_volumes[i]->volName)
			{
				free(disk_volumes[i]->volName);
			}
			free(disk_volumes[i]);
			disk_volumes[i] = NULL;
		}
	}
	yfsd_UnlockYAFFS();
//slf021105a end
	return TRUE;
}


BOOL YFSD_CreateDirectoryW(PVOLUME pVolume, PCWSTR pathName, PSECURITY_ATTRIBUTES pSecurityAttributes)
{
	// security attributes are ignored (should be NULL)

	yaffs_Object *newDir = NULL;
	yaffs_Object *parent = NULL;
	char name[YFSD_NAME_LENGTH+1];
	ULONG objSize;
	DWORD attribs;
	unsigned modifiedTime[2];

	RETAILMSG (MSGSTATE, (L"YAFFS::CreateDirectory (%s)\r\n", pathName));

	yfsd_LockYAFFS();

	parent = yfsd_FindDirectoryByWinPath(&pVolume->dev,pathName,name,YFSD_NAME_LENGTH);

	//slf021101b begin 
	if (parent)
	{
		if(yfsd_NameIsValid(name))
		{
			newDir = yaffs_MknodDirectory(parent,name,0,0,0);
			if(newDir)
			{
				objSize = yaffs_GetObjectFileLength(newDir);
				attribs = yfsd_GetObjectWinAttributes(newDir);
				modifiedTime[0] = newDir->win_mtime[0];
                                modifiedTime[1] = newDir->win_mtime[1];
			}
			else
			{
				if(yaffs_FindObjectByName(parent,name))
					SetLastError(ERROR_ALREADY_EXISTS);
				else
					SetLastError(ERROR_DISK_FULL);
			}
		}
		else
			SetLastError(ERROR_INVALID_NAME);
	}
	else
	{
		SetLastError(ERROR_PATH_NOT_FOUND);
	}
    //slf021101b end

	yfsd_UnlockYAFFS();

	// Call shell function to tell of new directory
	if(newDir && pVolume->shellFunction)
	{
			FILECHANGEINFO fc;
			WCHAR fpn[YFSD_FULL_PATH_NAME_SIZE];

			fc.cbSize = sizeof(FILECHANGEINFO);
			fc.wEventId = SHCNE_MKDIR;
			fc.uFlags = SHCNF_PATH;
			fc.dwItem1 = (DWORD)yfsd_FullPathName(pVolume, fpn,YFSD_FULL_PATH_NAME_SIZE,pathName);
			fc.dwItem2 = 0;
			fc.dwAttributes = attribs; 
			yfsd_U32sToWinFileTime(modifiedTime,&fc.ftModified);
			fc.nFileSize = objSize;

			pVolume->shellFunction(&fc);
			RETAILMSG (MSGSTATE, (L"YAFFS::shell function called\r\n"));

			//yfsd_ShellDirectoryChanged(pVolume,fpn);

	}

//slf021101b begin 
//	if(parent && !newDir)
//	{
//			SetLastError(ERROR_DISK_FULL);
//	}
//slf021101b end

	return newDir ? TRUE : FALSE;
}


BOOL YFSD_RemoveDirectoryW(PVOLUME pVolume, PCWSTR pathName)
{
	int result = FALSE;
	yaffs_Object *parent = NULL;
	yaffs_Object *obj;
	char name[YFSD_NAME_LENGTH+1];

	RETAILMSG (MSGSTATE, (L"YAFFS::RemoveDirectory (%s)\r\n", pathName));
	
	yfsd_LockYAFFS();

	obj = yfsd_FindObjectByWinPath(&pVolume->dev,pathName);
	if(!obj)
	{
		SetLastError(ERROR_PATH_NOT_FOUND);
		result = FALSE;
	}
	else if (obj->variantType != YAFFS_OBJECT_TYPE_DIRECTORY)
	{
		SetLastError(ERROR_ACCESS_DENIED);
		result = FALSE;
	}
	else if(obj->st_mode & FILE_ATTRIBUTE_READONLY)
	{
		SetLastError(ERROR_ACCESS_DENIED);
		result = FALSE;
	}
	else if(obj->inUse)
	{
		SetLastError(ERROR_ACCESS_DENIED);
		result = FALSE;
	}
	else
	{

		parent = yfsd_FindDirectoryByWinPath(&pVolume->dev,pathName,name,YFSD_NAME_LENGTH);

		if(parent && yfsd_NameIsValid(name))
		{
			result = yaffs_Unlink(parent,name);
			if(!result)
				SetLastError(ERROR_DIR_NOT_EMPTY);
		}
	}

	yfsd_UnlockYAFFS();

	if(result && pVolume->shellFunction)
	{
			FILECHANGEINFO fc;
			WCHAR fpn[YFSD_FULL_PATH_NAME_SIZE];

			fc.cbSize = sizeof(FILECHANGEINFO);
			fc.wEventId = SHCNE_RMDIR;
			fc.uFlags = SHCNF_PATH;
			fc.dwItem1 = (DWORD)yfsd_FullPathName(pVolume,fpn,YFSD_FULL_PATH_NAME_SIZE,pathName);
			fc.dwItem2 = 0;
			fc.dwAttributes = 0;
			yfsd_NullWinFileTime(&fc.ftModified);
			fc.nFileSize = 0;

			pVolume->shellFunction(&fc);
			RETAILMSG (MSGSTATE, (L"YAFFS::shell function called\r\n"));

			yfsd_ShellDirectoryChanged(pVolume,fpn);
	}
	
	return result ? TRUE : FALSE;
}


DWORD YFSD_GetFileAttributesW(PVOLUME pVolume, PCWSTR pwsFileName )
{
	yaffs_Object *obj = NULL;

	DWORD result = 0xFFFFFFFF;

	RETAILMSG (MSGSTATE, (L"YAFFS::GetFileAttributes\r\n"));

	yfsd_LockYAFFS();

	obj = yfsd_FindObjectByWinPath(&pVolume->dev,pwsFileName);

	if(obj)
	{
		result = yfsd_GetObjectWinAttributes(obj);
	}
	else
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
	}

	yfsd_UnlockYAFFS();
	
	RETAILMSG (MSGSTATE, (L"YAFFS::GetFileAttributes for %s returning %X\r\n",pwsFileName,result));
	return result;

	
}

BOOL YFSD_SetFileAttributesW( PVOLUME pVolume,PCWSTR pwsFileName, DWORD dwFileAttributes )
{
	yaffs_Object *obj = NULL;
	DWORD mtime[2];
	DWORD attribs;
	DWORD objSize;

	int result = 0;

	RETAILMSG (MSGSTATE, (L"YAFFS::SetFileAttributes %X\r\n",dwFileAttributes));

	if(!yfsd_CheckValidAttributes(dwFileAttributes))
	{
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
	}

	yfsd_LockYAFFS();

	obj = yfsd_FindObjectByWinPath(&pVolume->dev,pwsFileName);

	if(obj)
	{
		obj->st_mode = dwFileAttributes;
		obj->dirty = 1;
		result = yaffs_FlushFile(obj,0);
		attribs = yfsd_GetObjectWinAttributes(obj);
		objSize = yaffs_GetObjectFileLength(obj);
		mtime[0] = obj->win_mtime[0];
		mtime[1] = obj->win_mtime[1];
	}
	else
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
	}

	yfsd_UnlockYAFFS();

	if(result && pVolume->shellFunction)
	{
			FILECHANGEINFO fc;
			WCHAR fpn[YFSD_FULL_PATH_NAME_SIZE];

			fc.cbSize = sizeof(FILECHANGEINFO);
			fc.wEventId = SHCNE_ATTRIBUTES;
			fc.uFlags = SHCNF_PATH;
			fc.dwItem1 = (DWORD)yfsd_FullPathName(pVolume,fpn,YFSD_FULL_PATH_NAME_SIZE,pwsFileName);
			fc.dwItem2 = 0;
			fc.dwAttributes =  attribs;
			yfsd_U32sToWinFileTime(mtime,&fc.ftModified);
			fc.nFileSize = objSize;

			pVolume->shellFunction(&fc);
			RETAILMSG (MSGSTATE, (L"YAFFS::shell function called\r\n"));

			//yfsd_ShellDirectoryChanged(pVolume,fpn);
	}
	

	return result;

}

BOOL YFSD_DeleteFileW( PVOLUME pVolume, PCWSTR pwsFileName )
{
	int result = FALSE;
	yaffs_Object *parent = NULL;
	yaffs_Object *obj;
	char name[YFSD_NAME_LENGTH+1];

	RETAILMSG (MSGSTATE, (L"YAFFS::DeleteFileW (%s)\r\n", pwsFileName));

	yfsd_LockYAFFS();

	obj = yfsd_FindObjectByWinPath(&pVolume->dev,pwsFileName);
	if(!obj)
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		result = FALSE;
	}
	else if (obj->variantType != YAFFS_OBJECT_TYPE_FILE)
	{
		SetLastError(ERROR_ACCESS_DENIED);
		result = FALSE;
	}
	else if(obj->st_mode & FILE_ATTRIBUTE_READONLY)
	{
		SetLastError(ERROR_ACCESS_DENIED);
		result = FALSE;
	}
	else if(obj->inUse)
	{
		SetLastError(ERROR_ACCESS_DENIED);
		result = FALSE;
	}
	else
	{

		parent = yfsd_FindDirectoryByWinPath(&pVolume->dev,pwsFileName,name,YFSD_NAME_LENGTH);

		if(parent && yfsd_NameIsValid(name))
		{
			result = yaffs_Unlink(parent,name);
			if(!result)
				SetLastError(ERROR_ACCESS_DENIED);
		}
	}

	yfsd_UnlockYAFFS();

	if(result && pVolume->shellFunction)
	{
			FILECHANGEINFO fc;
			WCHAR fpn[YFSD_FULL_PATH_NAME_SIZE];

			fc.cbSize = sizeof(FILECHANGEINFO);
			fc.wEventId = SHCNE_DELETE;
			fc.uFlags = SHCNF_PATH;
			fc.dwItem1 = (DWORD)yfsd_FullPathName(pVolume,fpn,YFSD_FULL_PATH_NAME_SIZE,pwsFileName);
			fc.dwItem2 = 0;
			fc.dwAttributes = -1;
			yfsd_NullWinFileTime(&fc.ftModified);
			fc.nFileSize = 0;

			pVolume->shellFunction(&fc);
			RETAILMSG (MSGSTATE, (L"YAFFS::shell function called\r\n"));

			yfsd_ShellDirectoryChanged(pVolume,fpn);
	}

	return result ? TRUE : FALSE;
}

BOOL YFSD_MoveFileW(PVOLUME pVolume,PCWSTR pwsOldFileName, PCWSTR pwsNewFileName )
{
	yaffs_Object *newParent = NULL;
	yaffs_Object *oldParent = NULL;
	yaffs_Object *obj = NULL;
	char oldName[YFSD_NAME_LENGTH+1];
	char newName[YFSD_NAME_LENGTH+1];
	int result = 0;
	int objIsDir = 0;
	DWORD attribs;
	DWORD objSize;
	DWORD mtime[2];

	RETAILMSG (MSGSTATE, (L"YAFFS::MoveFile\r\n"));

	yfsd_LockYAFFS();

	oldParent = yfsd_FindDirectoryByWinPath(&pVolume->dev,pwsOldFileName,oldName,YFSD_NAME_LENGTH);
	newParent = yfsd_FindDirectoryByWinPath(&pVolume->dev,pwsNewFileName,newName,YFSD_NAME_LENGTH);

	if(oldParent  && yfsd_NameIsValid(oldName) && newParent && yfsd_NameIsValid(newName))
	{
		result = yaffs_RenameObject(oldParent,oldName,newParent,newName);
		if(!result)
		{
			SetLastError(ERROR_FILE_NOT_FOUND);
		}

		obj = yfsd_FindObjectByWinPath(&pVolume->dev,pwsNewFileName);
		if(obj)
		{
			objIsDir = (obj->variantType == YAFFS_OBJECT_TYPE_DIRECTORY);
			attribs = yfsd_GetObjectWinAttributes(obj);
			objSize = yaffs_GetObjectFileLength(obj);
			mtime[0] = obj->win_mtime[0];
			mtime[1] = obj->win_mtime[1];
		}
	}
	else
	{
		SetLastError(ERROR_PATH_NOT_FOUND);
	}

	yfsd_UnlockYAFFS();


	if(result && pVolume->shellFunction)
	{
			FILECHANGEINFO fc;
			WCHAR fpn1[YFSD_FULL_PATH_NAME_SIZE];
			WCHAR fpn2[YFSD_FULL_PATH_NAME_SIZE];

			fc.cbSize = sizeof(FILECHANGEINFO);
			fc.wEventId = objIsDir ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM;
			fc.uFlags = SHCNF_PATH;
			fc.dwItem1 = (DWORD)yfsd_FullPathName(pVolume,fpn1,YFSD_FULL_PATH_NAME_SIZE,pwsOldFileName);
			fc.dwItem2 = (DWORD)yfsd_FullPathName(pVolume,fpn2,YFSD_FULL_PATH_NAME_SIZE,pwsNewFileName);
			fc.dwAttributes = attribs;
			yfsd_U32sToWinFileTime(mtime,&fc.ftModified);
			fc.nFileSize = objSize;

			pVolume->shellFunction(&fc);
			RETAILMSG (MSGSTATE, (L"YAFFS::shell function called\r\n"));

			yfsd_ShellDirectoryChanged(pVolume,fpn1);
			yfsd_ShellDirectoryChanged(pVolume,fpn2);
	}


	return result ? TRUE : FALSE;

}

BOOL YFSD_DeleteAndRenameFileW(PVOLUME pVolume, PCWSTR pwsOldFileName, PCWSTR pwsNewFileName )
{
	//slf021104c begin
    BOOL fSuccess;
	//slf021104c end

	RETAILMSG (MSGSTATE, (L"YAFFS::DeleteAndRename\r\n"));

	//slf021104c begin
    if (fSuccess = YFSD_DeleteFileW(pVolume, pwsOldFileName))
        fSuccess = YFSD_MoveFileW(pVolume, pwsNewFileName, pwsOldFileName);
	return fSuccess;
	//return FALSE;
	//slf021104c end
}

BOOL YFSD_GetDiskFreeSpaceW( PVOLUME pVolume, PCWSTR pwsPathName, PDWORD pSectorsPerCluster,PDWORD pBytesPerSector, PDWORD pFreeClusters, PDWORD pClusters )
{

	int nChunks;

	RETAILMSG (MSGSTATE, (L"YAFFS::GetFreeSpace\r\n"));

	yfsd_LockYAFFS();
	nChunks = yaffs_GetNumberOfFreeChunks(&pVolume->dev);
	yfsd_UnlockYAFFS();

	if(nChunks >= 0)
	{
		// Let's pretentd our clusters are the same size as eraseable blocks...
		*pBytesPerSector = 512;
		*pSectorsPerCluster  =32;
		*pFreeClusters = nChunks/32;
		*pClusters = pVolume->dev.endBlock - pVolume->dev.startBlock + 1;
	}

	return (nChunks >= 0)? TRUE : FALSE;



}

void YFSD_Notify(PVOLUME pVolume, DWORD dwFlags )
{
	// Flags can be one of:
	// FSNOTIFY_POWER_ON: no action required
	// FSNOTIFY_POWER_OFF: flush all files
	// FSNOTIFY_DEVICE_ON: no action required

	RETAILMSG (MSGSTATE, (L"YAFFS::Notify\r\n"));
	if(dwFlags == FSNOTIFY_POWER_OFF)
	{
		yfsd_FlushAllFiles();
	}

}


BOOL YFSD_RegisterFileSystemFunction(PVOLUME pVolume,SHELLFILECHANGEFUNC_t pfn )
{
	RETAILMSG (MSGSTATE, (L"YAFFS::RegisterFileSysFunction\r\n"));
	
	pVolume->shellFunction = pfn;

	return TRUE;
}





int iMatch(const char a, const char b)
{
	if (a == '?' || b == '?')
		return 1;
	return (toupper(a) == toupper(b));
}

void pString(const char *inp)
{
	while (*inp) RETAILMSG(1, (L"%c", *inp++));
}

int regularMatch(const char *regexp, const char *str)
{
//	pString(regexp);
//	RETAILMSG(1, (L" "));
//	pString(str);
//	RETAILMSG(1, (L"\r\n"));

	if (*regexp == 0 && *str == 0)
	{
		//RETAILMSG(1, (L"Match!\r\n"));
		return 1;
	}
	if (*regexp == '*')			
	{
		regexp++;
		if (*regexp == 0)   // end of the expression is a *, we must match
		{
			//RETAILMSG(1, (L"Match!\r\n"));
			return 1;
		}
		while (!iMatch(*regexp, *str)) // throw away chars from str until we match
		{
			if (*str == 0)  // if we're not at the end
			{
				// if we have .* left to match, but the str is finished then match it OK
				if (regexp[0] == '.' && regexp[1] == '*')
				{
					//RETAILMSG(1, (L"Match!\r\n"));
					return 1;
				}
				else
				{
				// the extension failed the match
					//RETAILMSG(1, (L"No Match!\r\n"));
					return 0;
				}
			}
			str++;
		} 
		// right now we should either eat more characters, or try to match
		return (regularMatch(regexp, str) || regularMatch(--regexp, ++str));
	}
//  compare chars until we hit another *, or we fail
	while (iMatch(*regexp, *str))
	{
		if (*regexp == 0 && *str == 0)
		{
			//RETAILMSG(1, (L"Match!\r\n"));
			return 1;
		}
		regexp++;
		str++;
	}

	if (*regexp == 0 && *str == 0)
	{
		//RETAILMSG(1, (L"Match!\r\n"));
		return 1;
	}

	if (*regexp == '*')
		return regularMatch(regexp, str);

	//RETAILMSG(1, (L"No Match!\r\n"));
	return 0;
}


void yfsd_DeleteFinder(PSEARCH pSearch)
{
  if(pSearch->foundObjects) //If we found some objects we must clean up the cached linked list.
  {
    yaffs_FoundObject *it;
    yaffs_FoundObject *temp;

    it = pSearch->foundObjects;

    while(it != NULL)
    {
      temp = it;
      it = it->next;
      
      free(temp);
    }

    pSearch->foundObjects = NULL;
  }

		pSearch->dir->inUse--;
		free(pSearch);
}

BOOL yfsd_ObjectAlreadyFound(PSEARCH pSearch, yaffs_Object *l)
{
  //Iterate through the current list of objs already found and return true if already exists.
  //If the object hasn't already been found then add it to the list (SIDE-EFFECT alert) and return false.
  BOOL found = FALSE;

  yaffs_FoundObject *it;
  it = pSearch->foundObjects;

  
  while(it->next != NULL) //iterate through singly linked list.
  {
    if(it->obj == l)
    {
      found = TRUE;
      break;
    }
    it = it->next;
  }

  if(!found)
  {
    //Add the item to the list.
    //'it' will currently be pointing to the last of the list nodes. i.e node->next == NULL
    it->next = malloc(sizeof(yaffs_FoundObject));
    it->next->next = NULL;
    it->next->obj = 0;

    it->obj = l;
  }

  return found;
}

#if 0
// slower one
BOOL yfsd_DoFindFile(PSEARCH pSearch, PWIN32_FIND_DATAW pfd)
{

	struct list_head *i;
	int pos;
	yaffs_Object *l;
	BOOL found = 0;

	char name[YAFFS_MAX_NAME_LENGTH+1];

  if(!pSearch->foundObjects)
  {
    pSearch->foundObjects = malloc(sizeof(yaffs_FoundObject));
    pSearch->foundObjects->next = NULL;
    pSearch->foundObjects->obj = 0;
  }


	yfsd_LockYAFFS();

	pos = 0;
	list_for_each(i,&pSearch->dir->variant.directoryVariant.children)
	{

		l = list_entry(i, yaffs_Object,siblings);

		yaffs_GetObjectName(l,name,YAFFS_MAX_NAME_LENGTH+1);

		if(regularMatch(pSearch->pattern,name))
		{
			if(!yfsd_ObjectAlreadyFound(pSearch, l))//pos == pSearch->currentPos)
			{		

				
				found = 1;
				//pSearch->currentPos++;

				// fill out find data

				pfd->dwFileAttributes = yfsd_GetObjectWinAttributes(l);

				yfsd_U32sToWinFileTime(l->win_ctime,&pfd->ftCreationTime);
				yfsd_U32sToWinFileTime(l->win_atime,&pfd->ftLastAccessTime);
				yfsd_U32sToWinFileTime(l->win_mtime,&pfd->ftLastWriteTime);

				pfd->nFileSizeHigh = 0;
				pfd->nFileSizeLow = yaffs_GetObjectFileLength(l);
				pfd->dwOID = (CEOID)(INVALID_HANDLE_VALUE); // wtf is this???

				MultiByteToWideChar(CP_ACP,0,name,-1,pfd->cFileName,YFSD_NAME_LENGTH);

				RETAILMSG(MSGSTATE,(L"File %s id %d header %d nDataChunks %d scannedLength %d\r\n",
							pfd->cFileName,l->objectId, l->chunkId, l->nDataChunks,
							l->variant.fileVariant.scannedFileSize));
				goto out_of_here;
			}
			else
			{
				pos++;
			}
		}
	}

out_of_here:
	yfsd_UnlockYAFFS();


	if(!found)
	{
		SetLastError(ERROR_NO_MORE_FILES);
	}
	return found;
	
}

#else
// faster one
BOOL yfsd_DoFindFile(PSEARCH pSearch, PWIN32_FIND_DATAW pfd)
{

	struct list_head *i;
	yaffs_Object *l;
	BOOL found = 0;

	char name[YAFFS_MAX_NAME_LENGTH+1];

  if(!pSearch->foundObjects)
  {
    pSearch->foundObjects = malloc(sizeof(yaffs_FoundObject));
    pSearch->foundObjects->next = NULL;
    pSearch->foundObjects->obj = 0;
  }


	yfsd_LockYAFFS();

	list_for_each(i,&pSearch->dir->variant.directoryVariant.children)
	{

		l = list_entry(i, yaffs_Object,siblings);
		if(!yfsd_ObjectAlreadyFound(pSearch,l))
		{
			// Only look at things we have not looked at already
			yaffs_GetObjectName(l,name,YAFFS_MAX_NAME_LENGTH+1);

			if(regularMatch(pSearch->pattern,name))
			{
				found = 1;
				// fill out find data

				pfd->dwFileAttributes = yfsd_GetObjectWinAttributes(l);

				yfsd_U32sToWinFileTime(l->win_ctime,&pfd->ftCreationTime);
				yfsd_U32sToWinFileTime(l->win_atime,&pfd->ftLastAccessTime);
				yfsd_U32sToWinFileTime(l->win_mtime,&pfd->ftLastWriteTime);

				pfd->nFileSizeHigh = 0;
				pfd->nFileSizeLow = yaffs_GetObjectFileLength(l);
				pfd->dwOID = (CEOID)(INVALID_HANDLE_VALUE); // wtf is this???

				MultiByteToWideChar(CP_ACP,0,name,-1,pfd->cFileName,YFSD_NAME_LENGTH);

				RETAILMSG(MSGSTATE,(L"File %s id %d header %d nDataChunks %d scannedLength %d\r\n",
							pfd->cFileName,l->objectId, l->chunkId, l->nDataChunks,
							l->variant.fileVariant.scannedFileSize));
				goto out_of_here;
			}

		}


	}

out_of_here:
	yfsd_UnlockYAFFS();


	if(!found)
	{
		SetLastError(ERROR_NO_MORE_FILES);
	}
	return found;
	
}
#endif

HANDLE YFSD_FindFirstFileW(PVOLUME pVolume, HANDLE hProc,PCWSTR pwsFileSpec, PWIN32_FIND_DATAW pfd )
{

	// Create a search context, register it, and do the first search

	PSEARCH pSearch;
	HANDLE h = INVALID_HANDLE_VALUE;
	BOOL found = 0;

	RETAILMSG (MSGSTATE, (L"YAFFS::FindFirst\r\n"));

	pSearch = malloc(sizeof(yfsd_WinFind));
	if(!pSearch)
	{
		SetLastError(ERROR_OUTOFMEMORY);
	}

	yfsd_LockYAFFS();

	if(pSearch)
	{
		pSearch->foundObjects = NULL; //pSearch->currentPos = 0;
		pSearch->dir = yfsd_FindDirectoryByWinPath(&pVolume->dev,pwsFileSpec,pSearch->pattern,YFSD_NAME_LENGTH);
		if(pSearch->dir)
		{
				pSearch->dir->inUse++;
		}
		else
		{
			free(pSearch);
			pSearch = NULL;
			SetLastError(ERROR_PATH_NOT_FOUND);
		}
	}

	yfsd_UnlockYAFFS();



	if(pSearch)
	{
		found = yfsd_DoFindFile(pSearch,pfd);

		if(found)
		{
			h = FSDMGR_CreateSearchHandle(pVolume->mgrVolume,hProc,pSearch);
			if(h == INVALID_HANDLE_VALUE)
			{
				SetLastError(ERROR_NO_MORE_SEARCH_HANDLES);
			}
		}
		else
		{
			SetLastError(ERROR_FILE_NOT_FOUND);
		}

		if(h == INVALID_HANDLE_VALUE)
		{
			yfsd_DeleteFinder(pSearch);
		}

	}


	return h;
}

BOOL YFSD_FindNextFileW(PSEARCH pSearch, PWIN32_FIND_DATAW pfd )
{
	RETAILMSG (MSGSTATE, (L"YAFFS::FindNext\r\n"));
	if(!pSearch)
	{
		return FALSE;
	}
	return yfsd_DoFindFile(pSearch,pfd);
}

BOOL YFSD_FindClose( PSEARCH pSearch )
{	
	RETAILMSG (MSGSTATE, (L"YAFFS::FindClose\r\n"));
	if(!pSearch)
	{
		return FALSE;
	}
	yfsd_DeleteFinder(pSearch);
	return TRUE;
}


HANDLE YFSD_CreateFileW( 
	PVOLUME pVolume, 
	HANDLE hProc, 
	PCWSTR pwsFileName, 
	DWORD dwAccess, 
	DWORD dwShareMode,
	PSECURITY_ATTRIBUTES pSecurityAttributes, // ignore
	DWORD dwCreate,
	DWORD dwFlagsAndAttributes, 
	HANDLE hTemplateFile ) // ignore
{


	yaffs_Object *parent = NULL;
	yaffs_Object *obj = NULL;
	char name[YFSD_NAME_LENGTH+1];
	int mode;
	yfsd_WinFile *f = NULL;
	HANDLE handle = INVALID_HANDLE_VALUE;
	unsigned modifiedTime[2];
	unsigned objSize;

	BOOL writePermitted = (dwAccess & GENERIC_WRITE) ? TRUE : FALSE;
	BOOL readPermitted = (dwAccess & GENERIC_READ) ? TRUE : FALSE;
	BOOL shareRead = (dwShareMode & FILE_SHARE_READ) ? TRUE : FALSE;
	BOOL shareWrite = (dwShareMode & FILE_SHARE_WRITE) ? TRUE : FALSE;

	BOOL openRead, openWrite, openReadAllowed, openWriteAllowed;

	BOOL fileCreated = FALSE;
	
	BOOL fAlwaysCreateOnExistingFile = FALSE;
	BOOL fTruncateExistingFile = FALSE;


	mode = dwFlagsAndAttributes & 0x00FFFFFF;  // ding off the flags
	RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile (%s) flags %X mode %X\r\n", pwsFileName,dwFlagsAndAttributes,mode));
	if(writePermitted)
	{
		RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile write permitted\r\n"));
	}
	else
	{
		RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile write not permitted\r\n"));
	}

	if(!yfsd_CheckValidAttributes(mode))
	{
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
	}

	yfsd_LockYAFFS();


	parent = yfsd_FindDirectoryByWinPath(&pVolume->dev,pwsFileName,name,YFSD_NAME_LENGTH);


	if(parent && yfsd_NameIsValid(name))
	{

		//slf021220b begin Fix still more bugs in CreateFile.
		// Get the object for this file if it exists (only once).
		obj = yfsd_FindObjectByWinPath(&pVolume->dev,pwsFileName);
		//slf021220b end Fix still more bugs in CreateFile.
		if(dwCreate == CREATE_NEW)
		{
			RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile creating file in CREATE_NEW\r\n"));

			//slf021101c begin
			//slf021220b begin Fix still more bugs in CreateFile.
			// got above. obj = yfsd_FindObjectByWinPath(&pVolume->dev,pwsFileName);
			//slf021220b end Fix still more bugs in CreateFile.
			if(!obj)
			{
				obj = yaffs_MknodFile(parent,name,mode,0,0);
				if(!obj)
					SetLastError(ERROR_DISK_FULL);
				fileCreated = TRUE;
			}
			//slf021220b begin Fix still more bugs in CreateFile.
			else if (obj->variantType == YAFFS_OBJECT_TYPE_DIRECTORY)
			{
				obj = NULL;
				SetLastError(ERROR_ALREADY_EXISTS);
			}
			//slf021220b end Fix still more bugs in CreateFile.
			else
			{
				obj = NULL;
				//slf021220b begin Fix still more bugs in CreateFile.
				//Match CE FAT error return SetLastError(ERROR_ALREADY_EXISTS);
				SetLastError(ERROR_FILE_EXISTS);
				//slf021220b begin Fix still more bugs in CreateFile.
			}
			//slf021101c end
		}
		else if( dwCreate == OPEN_ALWAYS)
		{
			//slf021220b begin Fix still more bugs in CreateFile.
			// got above. obj = yfsd_FindObjectByWinPath(&pVolume->dev,pwsFileName);
			//slf021220b end Fix still more bugs in CreateFile.
			if(!obj)
			{
				RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile creating file in OPEN_ALWAYS\r\n"));
				obj = yaffs_MknodFile(parent,name,mode,0,0);
				if(!obj)
					SetLastError(ERROR_DISK_FULL);
				fileCreated = TRUE;

			}
			//slf021220b begin Fix still more bugs in CreateFile.
			else if (obj->variantType == YAFFS_OBJECT_TYPE_DIRECTORY)
			{
				obj = NULL;
				SetLastError(ERROR_ACCESS_DENIED);
			}
			//slf021220b end Fix still more bugs in CreateFile.
			else
			{
				RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile opening existing file in OPEN_ALWAYS\r\n"));
			}
		}
		else if(dwCreate == OPEN_EXISTING)
		{
			RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile opening file in OPEN_EXISTING\r\n"));
			//slf021220b begin Fix still more bugs in CreateFile.
			// got above. obj = yfsd_FindObjectByWinPath(&pVolume->dev,pwsFileName);
			//slf021220b end Fix still more bugs in CreateFile.
			if(!obj)
				SetLastError(ERROR_FILE_NOT_FOUND);
		//slf021220b begin Fix still more bugs in CreateFile.
            //slf021101c begin
	    //			else
	    //				if (yfsd_GetObjectWinAttributes(obj) & FILE_ATTRIBUTE_DIRECTORY)
	    //				{
	    //					SetLastError(ERROR_ACCESS_DENIED);
	    //					obj = NULL;
	    //				}
            //slf021101c end
			else if (obj->variantType == YAFFS_OBJECT_TYPE_DIRECTORY)
			{
				SetLastError(ERROR_ACCESS_DENIED);
				obj = NULL;
			}
		//slf021220b end Fix still more bugs in CreateFile.
		}
		else if(dwCreate == TRUNCATE_EXISTING)
		{
			RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile opening file in TRUNCATE_EXISTING\r\n"));
			//slf021220b begin Fix still more bugs in CreateFile.
			// got above. obj = yfsd_FindObjectByWinPath(&pVolume->dev,pwsFileName);
			//if(obj)
			if (!writePermitted || (obj  && (obj->variantType == YAFFS_OBJECT_TYPE_DIRECTORY)))
			{
				obj = NULL;
				SetLastError(ERROR_ACCESS_DENIED);
			}
			else if(obj)
			//slf021220b end Fix still more bugs in CreateFile.
			{
				// Indicate that file is to be truncated.  This will happen later on assuming
				// that a sharing violation does not occur and that we can get a file handle.
				fTruncateExistingFile = TRUE;
			}
			else 
			{
				SetLastError(ERROR_FILE_NOT_FOUND);
			}
		}
		else if(dwCreate == CREATE_ALWAYS)
		{
			//slf021220b begin Fix still more bugs in CreateFile.
			// got above. obj = yfsd_FindObjectByWinPath(&pVolume->dev,pwsFileName);
			//slf021220b end Fix still more bugs in CreateFile.

			if(!obj)
			{
				RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile creating file parent %X, name %a in CREATE_ALWAYS\r\n",parent,name));
				obj = yaffs_MknodFile(parent,name,mode,0,0);
				if(!obj)
					SetLastError(ERROR_DISK_FULL);
				fileCreated = TRUE;
			}
			//slf021220b begin Fix still more bugs in CreateFile.
			else if (obj->variantType == YAFFS_OBJECT_TYPE_DIRECTORY)
			{
				obj = NULL;
				SetLastError(ERROR_ACCESS_DENIED);
			}
			//slf021220b end Fix still more bugs in CreateFile.
			else
			{			
				RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile in CREATE_ALWAYS (already exists)\r\n"));
				// Indicate that file is to be recreated.  This will happen later on assuming
				// that a sharing violation does not occur and that we can get a file handle.
				fAlwaysCreateOnExistingFile = TRUE;
			}
		}
		else
		{
				RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile called with unknown flags %x\r\n", dwCreate));
				SetLastError(ERROR_INVALID_PARAMETER);
		}
	}
	else
	{
		RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile unable to get parent node\r\n"));
		SetLastError(ERROR_PATH_NOT_FOUND);
	}

	if(obj)
	{
			int i;
			yfsd_WinFile *p;
			openRead = openWrite =0;
			openReadAllowed = openWriteAllowed = 1;

			for(i = 0; i < MAX_WIN_FILE; i++)
			{
					p = &yfsd_winFile[i];

					if(p->obj == obj)
					{
						if (p->readPermitted) openRead = 1;
						if (p->writePermitted) openWrite = 1;
						if (!p->shareRead) openReadAllowed = 0;
						if (!p->shareWrite) openWriteAllowed = 0;
					}

			}

			// Now we test if the share works out.

			if((openRead && !shareRead) ||	 // already open for read, but we are not prepared to share it for read
			   (openWrite && !shareWrite) || // already open for write, but we are not prepared to share it for write
			   (!openReadAllowed && readPermitted) || // already open with read sharing not permitted
			   (!openWriteAllowed && writePermitted)) // same... write
			{
				//slf021220c begin Fix error code for new sharing mode check code.
				SetLastError(ERROR_SHARING_VIOLATION);
				//slf021220c end Fix error code for new sharing mode check code.
				obj = NULL;
			}


	}
	if(obj)
	{
		RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile - we have an object\r\n"));
		f = yfsd_GetWinFile();
	}
	else
	{
		RETAILMSG (MSGSTATE, (L"YAFFS::Creatfile - no object\r\n"));
	}

	if(f)
	{

		handle = FSDMGR_CreateFileHandle(pVolume->mgrVolume,hProc,f);

		if(handle != INVALID_HANDLE_VALUE)
		{
			RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile - we have an fsdmgr handle\r\n"));

			if (fTruncateExistingFile)
			{
				RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile - TRUNCATE_EXISTING - truncating existing file\r\n"));
				yaffs_ResizeFile(obj,0);
			}
			
			if (fAlwaysCreateOnExistingFile)
			{
				RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile - CREATE_ALWAYS - zapping existing file\r\n"));
				obj->st_mode = mode;
				obj->dirty = 1;
				yaffs_ResizeFile(obj,0);
				yaffs_FlushFile(obj,1);
			}
						
			f->obj = obj;
			f->offset = 0;
			f->writePermitted = writePermitted;
			//slf021220d begin oops typo.
			f->readPermitted = readPermitted;
			//slf021220d end oops typo.
			f->shareRead= shareRead;
			f->shareWrite = shareWrite;
			f->myVolume = pVolume;
			obj->inUse++;

			modifiedTime[0] = obj->win_mtime[0];
			modifiedTime[1] = obj->win_mtime[1];
			objSize = yaffs_GetObjectFileLength(obj);
			RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile - file size %d\r\n",objSize));
		}
		else
		{
			yfsd_PutWinFile(f);
			RETAILMSG (MSGSTATE, (L"YAFFS::CreateFile - we have no fsdmgr handle\r\n"));
		}

	}

	yfsd_UnlockYAFFS();

	if(handle != INVALID_HANDLE_VALUE && 
	   fileCreated &&
	   pVolume->shellFunction)
	{
			FILECHANGEINFO fc;
			WCHAR fpn[YFSD_FULL_PATH_NAME_SIZE];

			fc.cbSize = sizeof(FILECHANGEINFO);
			fc.wEventId = SHCNE_CREATE;
			fc.uFlags = SHCNF_PATH;
			fc.dwItem1 = (DWORD)yfsd_FullPathName(pVolume,fpn,YFSD_FULL_PATH_NAME_SIZE,pwsFileName);
			fc.dwItem2 = 0;
			fc.dwAttributes = mode;
			yfsd_U32sToWinFileTime(modifiedTime,&fc.ftModified);
			fc.nFileSize = objSize;

			pVolume->shellFunction(&fc);
			RETAILMSG (MSGSTATE, (L"YAFFS::shell function called\r\n"));

			yfsd_ShellDirectoryChanged(pVolume,fpn);
	}

	if(handle != INVALID_HANDLE_VALUE && (fileCreated || writePermitted))
	{
			// Remember the name

			WCHAR fpn[YFSD_FULL_PATH_NAME_SIZE];
			int slen;

			yfsd_FullPathName(pVolume,fpn,YFSD_FULL_PATH_NAME_SIZE,pwsFileName);
			slen = wcslen(fpn);
			f->fullName = malloc((slen+1)* sizeof(WCHAR));
			if(f->fullName)
			{
				wcscpy(f->fullName,fpn);
			}

	}


	return handle;

}

BOOL yfsd_DoReadFile( 
	PFILE pFile, 
	PVOID pBuffer, 
	DWORD cbRead, 
	PDWORD pcbRead)
{
	
	DWORD maxRead;
	int nread = 0;
	yaffs_Object *obj = NULL;


	if(pcbRead)
	{
		*pcbRead = 0;
	}
	else
	{
		RETAILMSG (MSGSTATE, (L"YAFFS::DoReadFile pcbRead was NULL. naughty.\r\n"));
	}

	RETAILMSG (MSGSTATE, (L"YAFFS::DoReadFile %d bytes\r\n",cbRead));

	if(!pFile || !pFile->obj)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}
	
	obj = pFile->obj;

	if(yaffs_GetObjectFileLength(obj) > pFile->offset)
	{
		maxRead = yaffs_GetObjectFileLength(obj) - pFile->offset;
	}
	else
	{
		maxRead = 0;
	}

	if(cbRead > maxRead)
	{
		cbRead = maxRead;
	}
	
	if(maxRead > 0)
	{
		nread = yaffs_ReadDataFromFile(obj,pBuffer,pFile->offset,cbRead);
		if(nread > 0)
		{
			pFile->offset += nread;

			if(pcbRead)
			{
				*pcbRead = nread;
			}
		}
	}
	else
	{
		if(pcbRead) 
		{
			*pcbRead = maxRead;
		}
	}


	return nread < 0? FALSE : TRUE; 

}

BOOL YFSD_ReadFile( 
	PFILE pFile, 
	PVOID pBuffer, 
	DWORD cbRead, 
	PDWORD pcbRead, 
	OVERLAPPED *pOverlapped ) //ignore
{
	BOOL result;

	RETAILMSG (MSGSTATE, (L"YAFFS::ReadFile\r\n"));

	if(!pFile || !pFile->obj)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	yfsd_LockYAFFS();

	result = yfsd_DoReadFile(pFile,pBuffer,cbRead,pcbRead);

	yfsd_UnlockYAFFS();

	return result;
}

BOOL YFSD_ReadFileWithSeek( 
	PFILE pFile, 
	PVOID pBuffer, 
	DWORD cbRead, 
	PDWORD pcbRead, 
	OVERLAPPED *pOverlapped, 
	DWORD dwLowOffset, 
	DWORD dwHighOffset )
{
	BOOL result;
	DWORD rememberedOffset;

	RETAILMSG (MSGSTATE, (L"YAFFS::ReadFileWithSeek %d bytes at %d high %d pcbRead %X\r\n",cbRead,dwLowOffset,dwHighOffset,pcbRead));

	// To determine if paging is supported, the kernel calls this with all parameters except pFile
	// being zero.
	if(!pBuffer && !cbRead && !pcbRead && !pOverlapped && !dwLowOffset && !dwHighOffset)
	{
		return TRUE; // paging suppported
		//return FALSE; // paging not supported
	}

	if(!pFile || !pFile->obj)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	yfsd_LockYAFFS();

	rememberedOffset = pFile->offset;

	pFile->offset = dwLowOffset;
	// ignore high offset for now

	result = yfsd_DoReadFile(pFile,pBuffer,cbRead,pcbRead);

	//pFile->offset = rememberedOffset;

	yfsd_UnlockYAFFS();

	return result;


}


BOOL yfsd_DoWriteFile( 
	PFILE pFile, 
	PCVOID pBuffer, 
	DWORD cbWrite, 
	PDWORD pcbWritten)
{
	int nwritten = 0;
	yaffs_Object *obj = NULL;
	
	RETAILMSG (MSGSTATE, (L"YAFFS::DoWriteFile size %d\r\n",cbWrite));
	
	if(!pFile || !pFile->obj)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	if(!pFile->writePermitted)
	{
			*pcbWritten = 0;
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
	}

	obj = pFile->obj;

	*pcbWritten = 0;


		nwritten = yaffs_WriteDataToFile(obj,pBuffer,pFile->offset,cbWrite);
		if(nwritten >= 0)
		{
			pFile->offset += nwritten;
			*pcbWritten = nwritten;
		}
		if(nwritten != cbWrite)
		{
			SetLastError(ERROR_DISK_FULL);
		}


	return nwritten != cbWrite? FALSE : TRUE; 
}


BOOL YFSD_WriteFile( 
	PFILE pFile, 
	PCVOID pBuffer, 
	DWORD cbWrite, 
	PDWORD pcbWritten, 
	OVERLAPPED *pOverlapped )
{
	BOOL result;

	yfsd_LockYAFFS();
	RETAILMSG (MSGSTATE, (L"YAFFS::WriteFile\r\n"));

	result = yfsd_DoWriteFile(pFile,pBuffer,cbWrite,pcbWritten);

	yfsd_UnlockYAFFS();

	return result;
}

BOOL YFSD_WriteFileWithSeek( 
	PFILE pFile, 
	PCVOID pBuffer, 
	DWORD cbWrite, 
	PDWORD pcbWritten, 
	OVERLAPPED *pOverlapped,
	DWORD dwLowOffset, 
	DWORD dwHighOffset )
{
	BOOL result;
	DWORD rememberedOffset;
	RETAILMSG (MSGSTATE, (L"YAFFS::WriteFileWithSeek %d bytes at %d,%d pcbWritten %X\r\n",cbWrite,dwHighOffset,dwLowOffset,pcbWritten));

	

	if(!pFile || !pFile->obj)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	yfsd_LockYAFFS();

	rememberedOffset = pFile->offset;

	pFile->offset = dwLowOffset;
	// ignore high offset for now

	result = yfsd_DoWriteFile(pFile,pBuffer,cbWrite,pcbWritten);

	//pFile->offset = rememberedOffset;

	yfsd_UnlockYAFFS();

	return result;
}

DWORD YFSD_SetFilePointer( 
	PFILE pFile, 
	LONG lDistanceToMove, 
	PLONG pDistanceToMoveHigh, 
	DWORD dwMoveMethod )
{
	// ignore high offset for now

	DWORD offset = 0xFFFFFFFF;
	DWORD oldPos;
	int fileSize;
	int seekNegative = 0;


	if(!pFile || !pFile->obj)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return offset;
	}

	yfsd_LockYAFFS();


	oldPos = pFile->offset;

	if(dwMoveMethod == FILE_BEGIN)
	{
		if(lDistanceToMove >= 0)
		{	
			offset = pFile->offset = lDistanceToMove;
		}
		else
		{
			seekNegative = 1;
		}
	}
	else if(dwMoveMethod == FILE_END)
	{
		fileSize = yaffs_GetObjectFileLength(pFile->obj);
		if(fileSize >= 0 &&
		   (fileSize + lDistanceToMove) >= 0)
		{
			offset = pFile->offset = fileSize + lDistanceToMove;
		}
		else
		{
			seekNegative = 1;
		}
	}
	else if(dwMoveMethod == FILE_CURRENT)
	{
		if(pFile->offset + lDistanceToMove >= 0)
		{
			offset = pFile->offset = pFile->offset + lDistanceToMove;		
		}
		else
		{
				seekNegative = 1;
		}
	}

	if(seekNegative)
	{
			SetLastError(ERROR_NEGATIVE_SEEK);
			
	}

	yfsd_UnlockYAFFS();

	RETAILMSG (MSGSTATE, (L"YAFFS::SetFilePtr method %d distance %d high %X oldpos %d newpos %d\r\n",
		                  dwMoveMethod,lDistanceToMove,pDistanceToMoveHigh,oldPos,offset));

	return offset;

}

DWORD YFSD_GetFileSize( 
	PFILE pFile, 
	PDWORD pFileSizeHigh )
{
	int fileSize;
	
	RETAILMSG (MSGSTATE, (L"YAFFS::GetFileSize high %X\r\n",pFileSizeHigh));
	

	if(!pFile || !pFile->obj)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return -1;
	}

	yfsd_LockYAFFS();

	fileSize = yaffs_GetObjectFileLength(pFile->obj);

	yfsd_UnlockYAFFS();
	if(pFileSizeHigh)
		 *pFileSizeHigh = 0;

	return fileSize;

}


BOOL YFSD_GetFileInformationByHandle( 
	PFILE pFile,
	PBY_HANDLE_FILE_INFORMATION pFileInfo )
{
	RETAILMSG (MSGSTATE, (L"YAFFS::GetFileInfoByHandle\r\n"));

	if(!pFile || !pFile->obj || !pFileInfo)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	yfsd_LockYAFFS();

	pFileInfo->dwFileAttributes = yfsd_GetObjectWinAttributes(pFile->obj);
	yfsd_U32sToWinFileTime(pFile->obj->win_ctime,&pFileInfo->ftCreationTime);
	yfsd_U32sToWinFileTime(pFile->obj->win_atime,&pFileInfo->ftLastAccessTime);
	yfsd_U32sToWinFileTime(pFile->obj->win_mtime,&pFileInfo->ftLastWriteTime);
	pFileInfo->dwVolumeSerialNumber = 0; //todo is this OK? 
	pFileInfo->nFileSizeHigh = 0;
	pFileInfo->nFileSizeLow = yaffs_GetObjectFileLength(pFile->obj); 
	pFileInfo->nNumberOfLinks = 1; // only primary link supported like FAT
	pFileInfo->nFileIndexHigh = 0; 
	pFileInfo->nFileIndexLow = pFile->obj->objectId;
	pFileInfo->dwOID = (CEOID)(INVALID_HANDLE_VALUE);

	yfsd_UnlockYAFFS();

	return TRUE;
}

BOOL YFSD_FlushFileBuffers(PFILE pFile )
{
	WCHAR fpn[YFSD_FULL_PATH_NAME_SIZE];
	int nameExists = 0;
	yfsd_Volume *vol = NULL;
	DWORD attribs = 0;
	DWORD objSize = 0;
	DWORD mtime[2];


	RETAILMSG (MSGSTATE, (L"YAFFS::FlushFileBuffers\r\n"));

	if(!pFile || !pFile->obj)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	yfsd_LockYAFFS();

	yaffs_FlushFile(pFile->obj,1);
	attribs = yfsd_GetObjectWinAttributes(pFile->obj);
	objSize = yaffs_GetObjectFileLength(pFile->obj);
	mtime[0] = pFile->obj->win_mtime[0];
	mtime[1] = pFile->obj->win_mtime[1];
	if(pFile->fullName)
	{
		wcscpy(fpn,pFile->fullName);
		nameExists = 1;
	}
	vol = pFile->myVolume;

	yfsd_UnlockYAFFS();
	
	if(vol && vol->shellFunction && nameExists)
	{
			FILECHANGEINFO fc;
		
			fc.cbSize = sizeof(FILECHANGEINFO);
			fc.wEventId = SHCNE_UPDATEITEM;
			fc.uFlags = SHCNF_PATH;
			fc.dwItem1 = (DWORD)fpn;
			fc.dwItem2 = 0;
			fc.dwAttributes = attribs;
			yfsd_U32sToWinFileTime(mtime,&fc.ftModified);
			fc.nFileSize = objSize;

			vol->shellFunction(&fc);
			RETAILMSG (MSGSTATE, (L"YAFFS::shell function called\r\n"));
			//yfsd_ShellDirectoryChanged(vol,fpn);
	}

	
	return TRUE;
}

BOOL YFSD_GetFileTime( 
	PFILE pFile, 
	FILETIME *pCreation, 
	FILETIME *pLastAccess, 
	FILETIME *pLastWrite )
{

	RETAILMSG (MSGSTATE, (L"YAFFS::GetFileTime\r\n"));
	if(!pFile || !pFile->obj)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	yfsd_LockYAFFS();

	if(pCreation) yfsd_U32sToWinFileTime(pFile->obj->win_ctime,pCreation);
	if(pLastAccess) yfsd_U32sToWinFileTime(pFile->obj->win_atime,pLastAccess);
	if(pLastWrite) yfsd_U32sToWinFileTime(pFile->obj->win_mtime,pLastWrite);

	yfsd_UnlockYAFFS();

	return TRUE;
}

BOOL YFSD_SetFileTime( 
	PFILE pFile, 
	CONST FILETIME *pCreation, 
	CONST FILETIME *pLastAccess, 
	CONST FILETIME *pLastWrite )
{
	WCHAR fpn[YFSD_FULL_PATH_NAME_SIZE];
	int nameExists = 0;
	int result = FALSE;
	yfsd_Volume *vol = NULL;
	DWORD attribs = 0;
	DWORD objSize = 0;
	DWORD mtime[2];

	
	RETAILMSG (MSGSTATE, (L"YAFFS::SetFileTime\r\n"));

	if(!pFile || !pFile->obj)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}
	
	
	yfsd_LockYAFFS();

	if(pCreation) 
	{
		 yfsd_WinFileTimeToU32s(pCreation,pFile->obj->win_ctime);
		pFile->obj->dirty = 1;
	}
	if(pLastAccess)
	{
		yfsd_WinFileTimeToU32s(pLastAccess,pFile->obj->win_atime);
		pFile->obj->dirty = 1;
	}
	if(pLastWrite)
	{
		yfsd_WinFileTimeToU32s(pLastWrite,pFile->obj->win_mtime);
		pFile->obj->dirty = 1;
	}
	if(pCreation || pLastAccess || pLastWrite)
	{
		result = yaffs_FlushFile(pFile->obj,0);
	}

	if(result)
	{
		attribs = yfsd_GetObjectWinAttributes(pFile->obj);
		objSize = yaffs_GetObjectFileLength(pFile->obj);
		mtime[0] = pFile->obj->win_mtime[0];
		mtime[1] = pFile->obj->win_mtime[1];
		if(pFile->fullName)
		{
			wcscpy(fpn,pFile->fullName);
			nameExists = 1;
		}
		vol = pFile->myVolume;
	}

	yfsd_UnlockYAFFS();

	// Call shell function
	if(nameExists && result && vol && vol->shellFunction)
	{
			FILECHANGEINFO fc;
		
			fc.cbSize = sizeof(FILECHANGEINFO);
			fc.wEventId = SHCNE_UPDATEITEM;
			fc.uFlags = SHCNF_PATH;
			fc.dwItem1 = (DWORD)fpn;
			fc.dwItem2 = 0;
			fc.dwAttributes = attribs;
			yfsd_U32sToWinFileTime(mtime,&fc.ftModified);
			fc.nFileSize = objSize;

			vol->shellFunction(&fc);
			RETAILMSG (MSGSTATE, (L"YAFFS::shell function called\r\n"));
			//yfsd_ShellDirectoryChanged(vol,fpn);
	}

	return TRUE;
}
   
BOOL YFSD_SetEndOfFile( 
PFILE pFile )
{

	WCHAR fpn[YFSD_FULL_PATH_NAME_SIZE];
	int nameExists = 0;
	yfsd_Volume *vol = NULL;
	DWORD attribs = 0;
	DWORD objSize = 0;
	DWORD mtime[2];
	static unsigned char zeros[512];

	int result;
	BOOL retVal = FALSE;

	RETAILMSG (MSGSTATE, (L"YAFFS::SetEOF\r\n"));

	if(!pFile || !pFile->obj)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	yfsd_LockYAFFS();
	result = yaffs_ResizeFile(pFile->obj,pFile->offset);

	RETAILMSG (MSGSTATE, (L"YAFFS::SetEOF resizing to %d, result %d\r\n",pFile->offset,result));

	// Resize only works if we're shortening the file.
	// If the result is shorter than the offset, then we need to write zeros....
	// 
	if(result != pFile->offset)
	{
		if(result < pFile->offset)
		{

			int nBytes = pFile->offset - result;
			int thisWriteSize;
			int written;
			BOOL ok = TRUE;

			memset(zeros,0,512);

			pFile->offset = result;
			RETAILMSG (MSGSTATE, (L"YAFFS::SetEOF expanding file by %d bytes\r\n",nBytes));
			while(nBytes > 0 && ok)
			{
				thisWriteSize = (nBytes > 512) ? 512  : nBytes;

				ok = yfsd_DoWriteFile(pFile,zeros,thisWriteSize,&written);	
				if(written != thisWriteSize)
				{
					ok = FALSE;
				}

				nBytes -= thisWriteSize;
			}

			retVal = ok;
		}
		else
		{

			SetLastError(ERROR_ACCESS_DENIED);
			retVal = FALSE;
		}
	}
	else
	{
		retVal = TRUE;
	}
	if(retVal)
	{
		attribs = yfsd_GetObjectWinAttributes(pFile->obj);
		objSize = yaffs_GetObjectFileLength(pFile->obj);
		mtime[0] = pFile->obj->win_mtime[0];
		mtime[1] = pFile->obj->win_mtime[1];
		if(pFile->fullName)
		{
			wcscpy(fpn,pFile->fullName);
			nameExists = 1;
		}
		vol = pFile->myVolume;
	}


	yfsd_UnlockYAFFS();

	if(nameExists && retVal && vol && vol->shellFunction)
	{
			FILECHANGEINFO fc;
		
			fc.cbSize = sizeof(FILECHANGEINFO);
			fc.wEventId = SHCNE_UPDATEITEM;
			fc.uFlags = SHCNF_PATH;
			fc.dwItem1 = (DWORD)fpn;
			fc.dwItem2 = 0;
			fc.dwAttributes = attribs;
			yfsd_U32sToWinFileTime(mtime,&fc.ftModified);
			fc.nFileSize = objSize;

			vol->shellFunction(&fc);
			RETAILMSG (MSGSTATE, (L"YAFFS::shell function called\r\n"));
			//yfsd_ShellDirectoryChanged(vol,fpn);
	}

	RETAILMSG (MSGSTATE, (L"YAFFS::SetEOF file size %d\r\n",yaffs_GetObjectFileLength(pFile->obj)));


	
	return retVal;
}

BOOL YFSD_DeviceIoControl( 
	PFILE pFile, 
	DWORD dwIoControlCode, 
	PVOID pInBuf, 
	DWORD nInBufSize, 
	PVOID pOutBuf, 
	DWORD nOutBufSize, 
	PDWORD pBytesReturned, 
	OVERLAPPED *pOverlapped )
{
	RETAILMSG (MSGSTATE, (L"YAFFS::DeviceIoControl\r\n"));

	return FALSE;
}

BOOL YFSD_CloseFile( PFILE pFile )
{
	WCHAR fpn[YFSD_FULL_PATH_NAME_SIZE];
	int nameExists = 0;
	yfsd_Volume *vol = NULL;
	DWORD attribs = 0;
	DWORD objSize = 0;
	DWORD mtime[2];

	RETAILMSG (MSGSTATE, (L"YAFFS::CloseFile %X\r\n",pFile));

	yfsd_LockYAFFS();

	if(!pFile)
	{
		RETAILMSG (MSGSTATE, (L"YAFFS::CloseFile null pFile\r\n"));
	}
	else
	{
		if(pFile->obj)
		{
			pFile->obj->inUse--;
			RETAILMSG (MSGSTATE, (L"YAFFS::CloseFile on obj\r\n"));
			yaffs_FlushFile(pFile->obj,1);
			attribs = yfsd_GetObjectWinAttributes(pFile->obj);
			objSize = yaffs_GetObjectFileLength(pFile->obj);
			mtime[0] = pFile->obj->win_mtime[0];
			mtime[1] = pFile->obj->win_mtime[1];
			RETAILMSG (MSGSTATE, (L"YAFFS::CloseFile on obj done, size is %d\r\n",objSize));
			if(pFile->fullName)
			{
				wcscpy(fpn,pFile->fullName);
				nameExists = 1;
			}
			vol = pFile->myVolume;
			yfsd_PutWinFile(pFile);
		}
		else
		{
			RETAILMSG (MSGSTATE, (L"YAFFS::CloseFile null obj\r\n"));
		}

	}
	yfsd_UnlockYAFFS();


	if(nameExists && vol && vol->shellFunction)
	{
			FILECHANGEINFO fc;
		
			fc.cbSize = sizeof(FILECHANGEINFO);
			fc.wEventId = SHCNE_UPDATEITEM;
			fc.uFlags = SHCNF_PATH;
			fc.dwItem1 = (DWORD)fpn;
			fc.dwItem2 = 0;
			fc.dwAttributes = attribs;
			yfsd_U32sToWinFileTime(mtime,&fc.ftModified);
			fc.nFileSize = objSize;

			vol->shellFunction(&fc);
			RETAILMSG (MSGSTATE, (L"YAFFS::shell function called\r\n"));
			//yfsd_ShellDirectoryChanged(vol,fpn);
	}

	

	RETAILMSG (MSGSTATE, (L"YAFFS::CloseFile done\r\n"));

	return TRUE;

}


BOOL YFSD_CloseVolume(PVOLUME pVolume )
{
	RETAILMSG (MSGSTATE, (L"YAFFS::CloseVolume\r\n"));
	yfsd_FlushAllFiles();
	return TRUE;
}

