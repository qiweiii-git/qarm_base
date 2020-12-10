/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system. 
 * ydirectenv.h: Environment wrappers for direct.
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
 * $Id: ydirectenv.h,v 1.2 2004/10/10 17:54:59 charles Exp $
 *
 */
 
#ifndef __YDIRECTENV_H__
#define __YDIRECTENV_H__

// Direct interface

#include "devextras.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define YMALLOC(x) malloc(x)
#define YFREE(x)   free(x)


//#define YINFO(s) YPRINTF(( __FILE__ " %d %s\n",__LINE__,s))
//#define YALERT(s) YINFO(s)


#define TENDSTR "\n"
#define TSTR(x) x
#define TOUT(p) printf p


#define YAFFS_LOSTNFOUND_NAME		"lost+found"
#define YAFFS_LOSTNFOUND_PREFIX		"obj"
//#define YPRINTF(x) printf x

#include "yaffscfg.h"

#define Y_CURRENT_TIME yaffsfs_CurrentTime()
#define Y_TIME_CONVERT(x) (x)

#define YAFFS_ROOT_MODE				0666
#define YAFFS_LOSTNFOUND_MODE		0666

#define yaffs_SumCompare(x,y) ((x) == (y))
#define yaffs_strcmp(a,b) strcmp(a,b)

#endif


