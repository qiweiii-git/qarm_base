/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system. 
 * ywinceenv.h: Port of functions to WinCE
 *
 * Copyright (C) 2002 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 *
 * $Id: ywinceenv.h,v 1.3 2004/10/10 18:03:35 charles Exp $
 *
 */
 
#ifndef __YWINCEENV_H__
#define __YWINCEENV_H__

// CONFIG_YAFFS_WINCE
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "devextras.h"

#define CONFIG_YAFFS_CASE_INSENSITIVE

#define YMALLOC(x) malloc(x)
#define YFREE(x)   free(x)


#define YINFO(s) YPRINTF(( __FILE__ " %d %s\n",__LINE__,s))
#define YALERT(s) YINFO(s)

#include <windows.h>

#define YAFFS_LOSTNFOUND_NAME		"Lost Clusters"
#define YAFFS_LOSTNFOUND_PREFIX		"OBJ"

#define YPRINTF(x)	RETAILMSG(1,x)

// Always pass the sum compare to overcome the case insensitivity issue
#define yaffs_SumCompare(x,y) ((x) == (y))
#define yaffs_strcmp(a,b) _stricmp(a,b)


#define u_char unsigned char
#define loff_t int
#define S_IFDIR						04000

#define S_ISFIFO(x) 0
#define S_ISCHR(x) 0
#define S_ISBLK(x) 0
#define S_ISSOCK(x) 0

extern unsigned yfsd_U32FileTimeNow(void);

#define Y_CURRENT_TIME				 yfsd_U32FileTimeNow()
#define Y_TIME_CONVERT(x) (x)

#define YAFFS_ROOT_MODE				FILE_ATTRIBUTE_ARCHIVE
#define YAFFS_LOSTNFOUND_MODE		FILE_ATTRIBUTE_HIDDEN


#define TENDSTR L"\r\n"
#define TSTR(x) TEXT(x)
#define TOUT(x) RETAILMSG(1, x)

#define YBUG() T(YAFFS_TRACE_BUG,(TSTR("==>> yaffs bug: %s %d" TENDSTR),TEXT(__FILE__),__LINE__))

#endif


