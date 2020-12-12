
#
# Makefile for the linux YAFFS filesystem routines.
#

obj-$(CONFIG_YAFFS_FS) += yaffs.o

yaffs-y := yaffs_ecc.o yaffs_fs.o yaffs_guts.o

yaffs-$(CONFIG_YAFFS_MTD_ENABLED) += yaffs_mtdif.o
yaffs-$(CONFIG_YAFFS_RAM_ENABLED) += yaffs_ramem.o

 
