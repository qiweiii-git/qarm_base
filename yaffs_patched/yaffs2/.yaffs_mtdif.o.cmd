cmd_/work/system/Development_util_ok/yaffs2/yaffs_mtdif.o := gcc -m32 -Wp,-MD,/work/system/Development_util_ok/yaffs2/.yaffs_mtdif.o.d  -nostdinc -isystem /usr/lib/gcc/i486-linux-gnu/4.1.3/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -O2 -pipe -msoft-float -mregparm=3 -freg-struct-return -mpreferred-stack-boundary=2  -march=i586 -mtune=generic -ffreestanding -maccumulate-outgoing-args   -Iinclude/asm-i386/mach-default -fomit-frame-pointer -g  -fno-stack-protector -Wdeclaration-after-statement -Wno-pointer-sign -DYAFFS_OUT_OF_TREE  -DMODULE -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(yaffs_mtdif)"  -D"KBUILD_MODNAME=KBUILD_STR(yaffs2)" -c -o /work/system/Development_util_ok/yaffs2/.tmp_yaffs_mtdif.o /work/system/Development_util_ok/yaffs2/yaffs_mtdif.c

deps_/work/system/Development_util_ok/yaffs2/yaffs_mtdif.o := \
  /work/system/Development_util_ok/yaffs2/yaffs_mtdif.c \
  /work/system/Development_util_ok/yaffs2/yportenv.h \
    $(wildcard include/config/yaffs/wince.h) \
    $(wildcard include/config/yaffs/direct.h) \
    $(wildcard include/config/yaffs/util.h) \
  /work/system/Development_util_ok/yaffs2/moduleconfig.h \
    $(wildcard include/config/h//.h) \
    $(wildcard include/config/yaffs/fs.h) \
    $(wildcard include/config/yaffs/yaffs1.h) \
    $(wildcard include/config/yaffs/yaffs2.h) \
    $(wildcard include/config/yaffs/does/ecc.h) \
    $(wildcard include/config/yaffs/ecc/wrong/order.h) \
    $(wildcard include/config/yaffs/disable/chunk/erased/check.h) \
    $(wildcard include/config/yaffs/short/names/in/ram.h) \
    $(wildcard include/config/yaffs/checkpoint/reserved/blocks.h) \
    $(wildcard include/config/yaffs/9byte/tags.h) \
  include/linux/version.h \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/numa.h) \
  /usr/lib/gcc/i486-linux-gnu/4.1.3/include/stdarg.h \
  include/linux/linkage.h \
  include/asm/linkage.h \
    $(wildcard include/config/x86/alignment/16.h) \
  include/linux/stddef.h \
  include/linux/compiler.h \
    $(wildcard include/config/enable/must/check.h) \
  include/linux/compiler-gcc4.h \
    $(wildcard include/config/forced/inlining.h) \
  include/linux/compiler-gcc.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbd.h) \
    $(wildcard include/config/lsf.h) \
    $(wildcard include/config/resources/64bit.h) \
  include/linux/posix_types.h \
  include/asm/posix_types.h \
  include/asm/types.h \
    $(wildcard include/config/highmem64g.h) \
  include/linux/bitops.h \
  include/asm/bitops.h \
  include/asm/alternative.h \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/paravirt.h) \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/ext2-non-atomic.h \
  include/asm-generic/bitops/le.h \
  include/asm/byteorder.h \
    $(wildcard include/config/x86/bswap.h) \
  include/linux/byteorder/little_endian.h \
  include/linux/byteorder/swab.h \
  include/linux/byteorder/generic.h \
  include/asm-generic/bitops/minix.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
  include/linux/mm.h \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/sysctl.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/debug/vm.h) \
    $(wildcard include/config/sparsemem.h) \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/slub.h) \
    $(wildcard include/config/shmem.h) \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/arch/populates/node/map.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/debug/pagealloc.h) \
  include/linux/errno.h \
  include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/capability.h \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
  include/linux/thread_info.h \
  include/asm/thread_info.h \
    $(wildcard include/config/4kstacks.h) \
    $(wildcard include/config/debug/stack/usage.h) \
  include/asm/page.h \
    $(wildcard include/config/x86/use/3dnow.h) \
    $(wildcard include/config/x86/pae.h) \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/highmem4g.h) \
    $(wildcard include/config/page/offset.h) \
    $(wildcard include/config/flatmem.h) \
  include/asm-generic/pgtable-nopmd.h \
  include/asm-generic/pgtable-nopud.h \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/out/of/line/pfn/to/page.h) \
  include/asm-generic/page.h \
  include/asm/processor.h \
    $(wildcard include/config/x86/ht.h) \
    $(wildcard include/config/mk8.h) \
    $(wildcard include/config/mk7.h) \
  include/asm/vm86.h \
    $(wildcard include/config/vm86.h) \
  include/asm/ptrace.h \
  include/asm/ptrace-abi.h \
  include/asm/segment.h \
  include/asm/math_emu.h \
  include/asm/sigcontext.h \
  include/asm/cpufeature.h \
  include/asm/required-features.h \
    $(wildcard include/config/x86/minimum/cpu.h) \
    $(wildcard include/config/x86/cmov.h) \
    $(wildcard include/config/x86/cmpxchg64.h) \
  include/asm/msr.h \
  include/asm/msr-index.h \
  include/asm/paravirt.h \
    $(wildcard include/config/x86/local/apic.h) \
    $(wildcard include/config/highpte.h) \
  include/linux/cpumask.h \
    $(wildcard include/config/hotplug/cpu.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/bitmap.h \
  include/linux/string.h \
  include/asm/string.h \
  include/asm/kmap_types.h \
    $(wildcard include/config/debug/highmem.h) \
  include/asm/system.h \
    $(wildcard include/config/x86/oostore.h) \
  include/asm/cmpxchg.h \
    $(wildcard include/config/x86/cmpxchg.h) \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
    $(wildcard include/config/x86.h) \
  include/asm/irqflags.h \
  include/asm/processor-flags.h \
  include/linux/cache.h \
  include/asm/cache.h \
    $(wildcard include/config/x86/l1/cache/shift.h) \
  include/asm/percpu.h \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/acpi/hotplug/memory.h) \
  include/linux/stringify.h \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/generic/hardirqs.h) \
    $(wildcard include/config/prove/locking.h) \
  include/asm/spinlock_types.h \
  include/asm/spinlock.h \
    $(wildcard include/config/x86/ppro/fence.h) \
  include/asm/atomic.h \
    $(wildcard include/config/m386.h) \
  include/asm-generic/atomic.h \
  include/asm/rwlock.h \
  include/linux/spinlock_api_smp.h \
  include/asm/current.h \
  include/linux/gfp.h \
    $(wildcard include/config/zone/dma.h) \
    $(wildcard include/config/zone/dma32.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/nodes/span/other/nodes.h) \
    $(wildcard include/config/holes/in/zone.h) \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/poison.h \
  include/linux/prefetch.h \
  include/linux/wait.h \
  include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/linux/seqlock.h \
  include/linux/nodemask.h \
  include/linux/memory_hotplug.h \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
  include/linux/notifier.h \
  include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/asm/rwsem.h \
  include/linux/srcu.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
  include/linux/smp.h \
  include/asm/smp.h \
    $(wildcard include/config/x86/io/apic.h) \
  include/asm/mpspec.h \
    $(wildcard include/config/acpi.h) \
  include/asm/mpspec_def.h \
  include/asm-i386/mach-default/mach_mpspec.h \
  include/asm/apic.h \
    $(wildcard include/config/x86/good/apic.h) \
  include/linux/pm.h \
    $(wildcard include/config/pm.h) \
  include/linux/delay.h \
  include/asm/delay.h \
  include/asm/fixmap.h \
    $(wildcard include/config/x86/visws/apic.h) \
    $(wildcard include/config/x86/f00f/bug.h) \
    $(wildcard include/config/x86/cyclone/timer.h) \
    $(wildcard include/config/pci/mmconfig.h) \
  include/asm/acpi.h \
    $(wildcard include/config/acpi/sleep.h) \
  include/acpi/pdc_intel.h \
  include/asm/apicdef.h \
  include/asm/io_apic.h \
  include/asm-i386/mach-default/mach_apicdef.h \
  include/asm/topology.h \
  include/asm-generic/topology.h \
  include/linux/rbtree.h \
  include/linux/prio_tree.h \
  include/linux/fs.h \
    $(wildcard include/config/dnotify.h) \
    $(wildcard include/config/sysfs.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/inotify.h) \
    $(wildcard include/config/security.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/block.h) \
    $(wildcard include/config/fs/xip.h) \
    $(wildcard include/config/migration.h) \
  include/linux/limits.h \
  include/linux/ioctl.h \
  include/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  include/linux/kdev_t.h \
  include/linux/dcache.h \
    $(wildcard include/config/profiling.h) \
  include/linux/rcupdate.h \
  include/linux/percpu.h \
  include/linux/slab.h \
    $(wildcard include/config/slab/debug.h) \
    $(wildcard include/config/slab.h) \
    $(wildcard include/config/debug/slab.h) \
  include/linux/slub_def.h \
  include/linux/workqueue.h \
  include/linux/timer.h \
    $(wildcard include/config/timer/stats.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/time.h \
  include/linux/jiffies.h \
  include/linux/calc64.h \
  include/asm/div64.h \
  include/linux/timex.h \
    $(wildcard include/config/time/interpolation.h) \
    $(wildcard include/config/no/hz.h) \
  include/asm/param.h \
    $(wildcard include/config/hz.h) \
  include/asm/timex.h \
    $(wildcard include/config/x86/elan.h) \
  include/asm/tsc.h \
    $(wildcard include/config/x86/tsc.h) \
    $(wildcard include/config/x86/generic.h) \
  include/linux/kobject.h \
  include/linux/sysfs.h \
  include/linux/kref.h \
  include/linux/namei.h \
  include/linux/mount.h \
  include/linux/stat.h \
  include/asm/stat.h \
  include/linux/radix-tree.h \
  include/linux/pid.h \
  include/asm/semaphore.h \
  include/linux/quota.h \
  include/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/nfs_fs_i.h \
  include/linux/nfs.h \
  include/linux/sunrpc/msg_prot.h \
  include/linux/fcntl.h \
  include/asm/fcntl.h \
  include/asm-generic/fcntl.h \
    $(wildcard include/config/64bit.h) \
  include/linux/err.h \
  include/linux/debug_locks.h \
    $(wildcard include/config/debug/locking/api/selftests.h) \
  include/linux/backing-dev.h \
  include/linux/mm_types.h \
  include/asm/pgtable.h \
  include/asm/pgtable-2level-defs.h \
  include/asm/pgtable-2level.h \
  include/asm-generic/pgtable.h \
  include/linux/page-flags.h \
    $(wildcard include/config/s390.h) \
    $(wildcard include/config/swap.h) \
  include/linux/vmstat.h \
    $(wildcard include/config/vm/event/counters.h) \
  include/linux/sched.h \
    $(wildcard include/config/detect/softlockup.h) \
    $(wildcard include/config/keys.h) \
    $(wildcard include/config/bsd/process/acct.h) \
    $(wildcard include/config/taskstats.h) \
    $(wildcard include/config/inotify/user.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/task/delay/acct.h) \
    $(wildcard include/config/blk/dev/io/trace.h) \
    $(wildcard include/config/cc/stackprotector.h) \
    $(wildcard include/config/sysvipc.h) \
    $(wildcard include/config/rt/mutexes.h) \
    $(wildcard include/config/task/xacct.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/compat.h) \
    $(wildcard include/config/fault/injection.h) \
  include/linux/auxvec.h \
  include/asm/auxvec.h \
  include/asm/mmu.h \
  include/asm/cputime.h \
  include/asm-generic/cputime.h \
  include/linux/sem.h \
  include/linux/ipc.h \
    $(wildcard include/config/ipc/ns.h) \
  include/asm/ipcbuf.h \
  include/asm/sembuf.h \
  include/linux/signal.h \
  include/asm/signal.h \
  include/asm-generic/signal.h \
  include/asm/siginfo.h \
  include/asm-generic/siginfo.h \
  include/linux/securebits.h \
  include/linux/fs_struct.h \
  include/linux/completion.h \
  include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
  include/asm/seccomp.h \
  include/linux/unistd.h \
  include/asm/unistd.h \
  include/linux/futex.h \
    $(wildcard include/config/futex.h) \
  include/linux/rtmutex.h \
    $(wildcard include/config/debug/rt/mutexes.h) \
  include/linux/plist.h \
    $(wildcard include/config/debug/pi/list.h) \
  include/linux/param.h \
  include/linux/resource.h \
  include/asm/resource.h \
  include/asm-generic/resource.h \
  include/linux/hrtimer.h \
    $(wildcard include/config/high/res/timers.h) \
  include/linux/task_io_accounting.h \
    $(wildcard include/config/task/io/accounting.h) \
  include/linux/aio.h \
  include/linux/aio_abi.h \
  include/linux/uio.h \
  include/linux/vmalloc.h \
  /work/system/Development_util_ok/yaffs2/yaffs_mtdif.h \
  /work/system/Development_util_ok/yaffs2/yaffs_guts.h \
    $(wildcard include/config/yaffs/no/yaffs1.h) \
    $(wildcard include/config/yaffs/unicode.h) \
    $(wildcard include/config/yaffs/tnode/list/debug.h) \
  /work/system/Development_util_ok/yaffs2/devextras.h \
  include/linux/mtd/mtd.h \
    $(wildcard include/config/mtd/partitions.h) \
    $(wildcard include/config/mtd/debug.h) \
    $(wildcard include/config/mtd/debug/verbose.h) \
  include/linux/module.h \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/kallsyms.h) \
  include/linux/kmod.h \
    $(wildcard include/config/kmod.h) \
  include/linux/elf.h \
  include/linux/elf-em.h \
  include/asm/elf.h \
  include/asm/user.h \
  include/asm/desc.h \
  include/asm/ldt.h \
  include/linux/moduleparam.h \
  include/asm/local.h \
  include/asm/module.h \
    $(wildcard include/config/m486.h) \
    $(wildcard include/config/m586.h) \
    $(wildcard include/config/m586tsc.h) \
    $(wildcard include/config/m586mmx.h) \
    $(wildcard include/config/mcore2.h) \
    $(wildcard include/config/m686.h) \
    $(wildcard include/config/mpentiumii.h) \
    $(wildcard include/config/mpentiumiii.h) \
    $(wildcard include/config/mpentiumm.h) \
    $(wildcard include/config/mpentium4.h) \
    $(wildcard include/config/mk6.h) \
    $(wildcard include/config/mcrusoe.h) \
    $(wildcard include/config/mefficeon.h) \
    $(wildcard include/config/mwinchipc6.h) \
    $(wildcard include/config/mwinchip2.h) \
    $(wildcard include/config/mwinchip3d.h) \
    $(wildcard include/config/mcyrixiii.h) \
    $(wildcard include/config/mviac3/2.h) \
    $(wildcard include/config/mviac7.h) \
    $(wildcard include/config/mgeodegx1.h) \
    $(wildcard include/config/mgeode/lx.h) \
  include/linux/mtd/compatmac.h \
  include/mtd/mtd-abi.h \
  include/linux/mtd/nand.h \
  include/linux/mtd/mtd.h \

/work/system/Development_util_ok/yaffs2/yaffs_mtdif.o: $(deps_/work/system/Development_util_ok/yaffs2/yaffs_mtdif.o)

$(deps_/work/system/Development_util_ok/yaffs2/yaffs_mtdif.o):
