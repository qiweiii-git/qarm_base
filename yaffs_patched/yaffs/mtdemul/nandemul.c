/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system. 
 *
 * Copyright (C) 2002 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/pagemap.h>
#include <linux/mtd/mtd.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/locks.h>

#include <asm/uaccess.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>

#define T(x) printk x
#define ALLOCATE(x) kmalloc(x,GFP_KERNEL)
#define FREE(x)     kfree(x)

#define DEFAULT_SIZE_IN_MB 16

#define NAND_SHIFT 9


static struct mtd_info nandemul_mtd;

typedef struct
{
	__u8 data[528]; // Data + spare
	int count[3];   // The programming count for each area of
			// the page (0..255,256..511,512..527
	int empty;      // is this empty?
} nandemul_Page;

typedef struct
{
	nandemul_Page page[32]; // The pages in the block
	__u8 damaged; 		// Is the block damaged?
	
} nandemul_Block;



typedef struct
{
	nandemul_Block **block;
	int nBlocks;
} nandemul_Device;

nandemul_Device device;

int sizeInMB = DEFAULT_SIZE_IN_MB;

int nandemul_CalcNBlocks(void)
{
        switch(sizeInMB)
        {
        	case 8:
        	case 16:
        	case 32:
        	case 64:
        	case 128:
        	case 256:
        	case 512:
        		break;
        	default:
        		sizeInMB = DEFAULT_SIZE_IN_MB;
        }
	return sizeInMB * 64;
}


static void nandemul_ReallyEraseBlock(int blockNumber)
{
	int i;
	
	nandemul_Block *theBlock = device.block[blockNumber];
	
	for(i = 0; i < 32; i++)
	{
		memset(theBlock->page[i].data,0xff,528);
		theBlock->page[i].count[0] = 0;
		theBlock->page[i].count[1] = 0;
		theBlock->page[i].count[2] = 0;
		theBlock->page[i].empty = 1;
	}

}

static int nandemul_DoErase(int blockNumber)
{
	if(blockNumber < 0 || nandemul_CalcNBlocks())
	{
		T(("Attempt to erase non-existant block %d\n",blockNumber));
	}
	else if(device.block[blockNumber]->damaged)
	{
		T(("Attempt to erase damaged block %d\n",blockNumber));
	}
	else
	{
		nandemul_ReallyEraseBlock(blockNumber);
	}

	return 1;
	
}


int nandemul_Initialise(void)
{
	int i;
	int fail = 0;
	int nBlocks = nandemul_CalcNBlocks();
	int nAllocated = 0;

	device.block = ALLOCATE (sizeof(nandemul_Block *) * nBlocks);

	if(!device.block) return 0;

	for(i=0; i <nBlocks; i++)
	{
		device.block[i] = NULL;
	}
	
	for(i=0; i <nBlocks && !fail; i++)
	{
		if((device.block[i] = ALLOCATE(sizeof(nandemul_Block))) == 0)
		{
			fail = 1;
		}
		else
		{
			nandemul_ReallyEraseBlock(i);
			device.block[i]->damaged = 0;
			nAllocated++;
		}
	}
	
	if(fail)
	{
		for(i = 0; i < nAllocated; i++)
		{
			FREE(device.block[i]);
		}
		FREE(device.block);

		T(("Allocation failed, could only allocate %dMB of %dMB requested.\n",
		   nAllocated/64,sizeInMB));
		return 0;
	}

	device.nBlocks = nBlocks;
	return 1;
}

int nandemul_DeInitialise(void)
{
	int i;
	for(i = 0; i < device.nBlocks; i++)
	{
		FREE(device.block[i]);
		device.block[i] = NULL;
	}
	
	FREE(device.block);
	device.block = NULL;
	return 1;
}

int nandemul_GetNumberOfBlocks(__u32 *nBlocks)
{
	*nBlocks = device.nBlocks;
	
	return 1;
}

int nandemul_Reset(void)
{
	// Do nothing
	return 1;
}

int nandemul_Read(__u8 *buffer, __u32 pageAddress,__u32 pageOffset, __u32 nBytes)
{
	unsigned blockN, pageN;
	
	blockN = pageAddress/32;
	pageN =  pageAddress % 32;
	
	// TODO: Do tests for valid blockN, pageN, pageOffset

	memcpy(buffer,&device.block[blockN]->page[pageN].data[pageOffset],nBytes);
	
	return 1;
		
}

int nandemul_Program(const __u8 *buffer, __u32 pageAddress,__u32 pageOffset, __u32 nBytes)
{
	unsigned blockN, pageN, pageO;
	
	int p0, p1, p2;
	int i;
	
	blockN = pageAddress/32;
	pageN =  pageAddress % 32;
	p0 = 0;
	p1 = 0;
	p2 = 0;
	
	// TODO: Do tests for valid blockN, pageN, pageOffset


    for(i = 0,pageO = pageOffset; i < nBytes; i++, pageO++)
    {
    	device.block[blockN]->page[pageN].data[pageO] &= buffer[i];
	
		if(pageO < 256) p0 = 1;
		else if(pageO <512) p1 = 1;
		else p2 = 1;
    }
	
	device.block[blockN]->page[pageN].empty = 0;
	device.block[blockN]->page[pageN].count[0] += p0;
	device.block[blockN]->page[pageN].count[1] += p1;
	device.block[blockN]->page[pageN].count[2] += p2;
	
	if(device.block[blockN]->page[pageN].count[0] > 1)
	{
		T(("block %d page %d first half programmed %d times\n",
		    blockN,pageN,device.block[blockN]->page[pageN].count[0]));
	}
	if(device.block[blockN]->page[pageN].count[1] > 1)
	{
		T(("block %d page %d second half programmed %d times\n",
		    blockN,pageN,device.block[blockN]->page[pageN].count[1]));
	}
	if(device.block[blockN]->page[pageN].count[2] > 3)
	{
		T(("block %d page %d spare programmed %d times\n",
		    blockN,pageN,device.block[blockN]->page[pageN].count[2]));
	}

	return 1;
	
}

int nandemul_CauseBitErrors( __u32 pageAddress, __u32 pageOffset, __u8 xorPattern)
{
	unsigned blockN, pageN;

	
	blockN = pageAddress/32;
	pageN =  pageAddress % 32;

	
	// TODO: Do tests for valid blockN, pageN, pageOffset

    device.block[blockN]->page[pageN].data[pageOffset] ^= xorPattern;
	

	return 1;
	
}


int nandemul_BlockErase(__u32 pageAddress)
{
	unsigned blockN;
	
	blockN = pageAddress/32;

	// TODO: Do tests for valid blockN
	// TODO: Test that the block has not failed

	return nandemul_DoErase(blockN);
	
}

int nandemul_FailBlock(__u32 pageAddress)
{
	unsigned blockN;
	
	blockN = pageAddress/32;

	// TODO: Do tests for valid blockN
	// TODO: Test that the block has not failed
	
	nandemul_DoErase(blockN);
	return 1;
}

int nandemul_ReadId(__u8 *vendorId, __u8 *deviceId)
{
	*vendorId = 0xEC;
	*deviceId = 0x75;
	
	return 1;
}

int nandemul_CopyPage(__u32 fromPageAddress, __u32 toPageAddress)
{
	__u8 copyBuffer[528];
	
	// TODO: Check the bitplane issue.
	nandemul_Read(copyBuffer, fromPageAddress,0,528);
	nandemul_Program(copyBuffer, toPageAddress,0,528);
	
	return 1;
}

int nandemul_ReadStatus(__u8 *status)
{
		*status = 0;
		return 1;
}


#ifdef CONFIG_MTD_NAND_ECC
#include <linux/mtd/nand_ecc.h>
#endif

/*
 * NAND low-level MTD interface functions
 */
static int nand_read (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf);
static int nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
				size_t *retlen, u_char *buf, u_char *ecc_code);
static int nand_read_oob (struct mtd_info *mtd, loff_t from, size_t len,
				size_t *retlen, u_char *buf);
static int nand_write (struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char *buf);
static int nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf,
				u_char *ecc_code);
static int nand_write_oob (struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf);
static int nand_writev (struct mtd_info *mtd, const struct iovec *vecs,
				unsigned long count, loff_t to, size_t *retlen);
static int nand_erase (struct mtd_info *mtd, struct erase_info *instr);
static void nand_sync (struct mtd_info *mtd);



/*
 * NAND read
 */
static int nand_read (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf)
{
	return nand_read_ecc (mtd, from, len, retlen, buf, NULL);
}

/*
 * NAND read with ECC
 */
static int nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
				size_t *retlen, u_char *buf, u_char *ecc_code)
{
	int 	start, page;
	int n = len;
	int nToCopy;



	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size) {
		*retlen = 0;
		return -EINVAL;
	}


	/* Initialize return value */
	*retlen = 0;

	while(n > 0)
	{

		/* First we calculate the starting page */
		page = from >> NAND_SHIFT;

		/* Get raw starting column */

		start = from & (mtd->oobblock-1);

		// OK now check for the curveball where the start and end are in
		// the same page
		if((start + n) < mtd->oobblock)
		{
			nToCopy = n;
		}
		else
		{
			nToCopy =  mtd->oobblock - start;
		}

		nandemul_Read(buf, page, start, nToCopy);

		n -= nToCopy;
		from += nToCopy;
		buf += nToCopy;
		*retlen += nToCopy;

	}


	return 0;
}

/*
 * NAND read out-of-band
 */
static int nand_read_oob (struct mtd_info *mtd, loff_t from, size_t len,
				size_t *retlen, u_char *buf)
{
	int col, page;

	DEBUG (MTD_DEBUG_LEVEL3,
		"nand_read_oob: from = 0x%08x, len = %i\n", (unsigned int) from,
		(int) len);

	/* Shift to get page */
	page = ((int) from) >> NAND_SHIFT;

	/* Mask to get column */
	col = from & 0x0f;

	/* Initialize return length value */
	*retlen = 0;

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_read_oob: Attempt read beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}

	nandemul_Read(buf,page,512 + col,len);

	/* Return happy */
	*retlen = len;
	return 0;
}

/*
 * NAND write
 */
static int nand_write (struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char *buf)
{
	return nand_write_ecc (mtd, to, len, retlen, buf, NULL);
}

/*
 * NAND write with ECC
 */
static int nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf,
				u_char *ecc_code)
{

	int 	start, page;
	int n = len;
	int nToCopy;



	/* Do not allow reads past end of device */
	if ((to + len) > mtd->size) {
		*retlen = 0;
		return -EINVAL;
	}


	/* Initialize return value */
	*retlen = 0;

	while(n > 0)
	{

		/* First we calculate the starting page */
		page = to >> NAND_SHIFT;

		/* Get raw starting column */

		start = to & (mtd->oobblock - 1);

		// OK now check for the curveball where the start and end are in
		// the same page
		if((start + n) < mtd->oobblock)
		{
			nToCopy = n;
		}
		else
		{
			nToCopy =  mtd->oobblock - start;
		}

		nandemul_Program(buf, page, start, nToCopy);

		n -= nToCopy;
		to += nToCopy;
		buf += nToCopy;
		*retlen += nToCopy;

	}


	return 0;
}

/*
 * NAND write out-of-band
 */
static int nand_write_oob (struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf)
{
	int col, page;


	DEBUG (MTD_DEBUG_LEVEL3,
		"nand_read_oob: to = 0x%08x, len = %i\n", (unsigned int) to,
		(int) len);

	/* Shift to get page */
	page = ((int) to) >> NAND_SHIFT;

	/* Mask to get column */
	col = to & 0x0f;

	/* Initialize return length value */
	*retlen = 0;

	/* Do not allow reads past end of device */
	if ((to + len) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_read_oob: Attempt read beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}

	nandemul_Program(buf,page,512 + col,len);

	/* Return happy */
	*retlen = len;
	return 0;

}

/*
 * NAND write with iovec
 */
static int nand_writev (struct mtd_info *mtd, const struct iovec *vecs,
				unsigned long count, loff_t to, size_t *retlen)
{
	return -EINVAL;
}

/*
 * NAND erase a block
 */
static int nand_erase (struct mtd_info *mtd, struct erase_info *instr)
{
	int i, nBlocks,block;

	DEBUG (MTD_DEBUG_LEVEL3,
		"nand_erase: start = 0x%08x, len = %i\n",
		(unsigned int) instr->addr, (unsigned int) instr->len);

	/* Start address must align on block boundary */
	if (instr->addr & (mtd->erasesize - 1)) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_erase: Unaligned address\n");
		return -EINVAL;
	}

	/* Length must align on block boundary */
	if (instr->len & (mtd->erasesize - 1)) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_erase: Length not block aligned\n");
		return -EINVAL;
	}

	/* Do not allow erase past end of device */
	if ((instr->len + instr->addr) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0,
			"nand_erase: Erase past end of device\n");
		return -EINVAL;
	}

	nBlocks = instr->len >> (NAND_SHIFT + 5);
	block = instr->addr >> (NAND_SHIFT + 5);

	for(i = 0; i < nBlocks; i++)
	{
		nandemul_DoErase(block);
		block++;
	}



	return 0;


}

/*
 * NAND sync
 */
static void nand_sync (struct mtd_info *mtd)
{
	DEBUG (MTD_DEBUG_LEVEL3, "nand_sync: called\n");

}

/*
 * Scan for the NAND device
 */
int nand_scan (struct mtd_info *mtd,int maxchips)
{
	mtd->oobblock = 512;
	mtd->oobsize = 16;
	mtd->erasesize = 512 * 32;
	mtd->size = sizeInMB * 1024*1024;



	/* Fill in remaining MTD driver data */
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH;
	mtd->owner = THIS_MODULE;
	mtd->ecctype = MTD_ECC_NONE;
	mtd->erase = nand_erase;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->read = nand_read;
	mtd->write = nand_write;
	mtd->read_ecc = nand_read_ecc;
	mtd->write_ecc = nand_write_ecc;
	mtd->read_oob = nand_read_oob;
	mtd->write_oob = nand_write_oob;
	mtd->readv = NULL;
	mtd->writev = nand_writev;
	mtd->sync = nand_sync;
	mtd->lock = NULL;
	mtd->unlock = NULL;
	mtd->suspend = NULL;
	mtd->resume = NULL;

	/* Return happy */
	return 0;
}

#if 0
#ifdef MODULE
MODULE_PARM(sizeInMB, "i");

__setup("sizeInMB=",sizeInMB);
#endif
#endif

/*
 * Define partitions for flash devices
 */

static struct mtd_partition nandemul_partition[] =
{
	{ name: "NANDemul partition 1",
	  offset:  0,
	  size: 0 },
};

static int nPartitions = sizeof(nandemul_partition)/sizeof(nandemul_partition[0]);

/*
 * Main initialization routine
 */
int __init nandemul_init (void)
{

	// Do the nand init

	nand_scan(&nandemul_mtd,1);

	nandemul_Initialise();

	// Build the partition table

	nandemul_partition[0].size = sizeInMB * 1024 * 1024;

	// Register the partition
	add_mtd_partitions(&nandemul_mtd,nandemul_partition,nPartitions);

	return 0;

}

module_init(nandemul_init);

/*
 * Clean up routine
 */
#ifdef MODULE
static void __exit nandemul_cleanup (void)
{

	nandemul_DeInitialise();

	/* Unregister partitions */
	del_mtd_partitions(&nandemul_mtd);

	/* Unregister the device */
	del_mtd_device (&nandemul_mtd);

}
module_exit(nandemul_cleanup);
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Charles Manning <manningc@aleph1.co.uk>");
MODULE_DESCRIPTION("NAND emulated in RAM");




