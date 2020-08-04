	.file	1 "bounds.c"
	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 3

 # -G value = 0, Arch = mips32r2, ISA = 33
 # GNU C (GCC) version 4.3.3 (mips-openwrt-linux-uclibc)
 #	compiled by GNU C version 4.4.3, GMP version 4.3.1, MPFR version 2.4.1.
 # GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
 # options passed:  -nostdinc -Iinclude
 # -I/home/salter.xu/auto-gpl.git/tmp/linux/linux-2.6.31/arch/mips/include
 # -I/home/salter.xu/auto-gpl.git/tmp/linux/linux-2.6.31/arch/mips/include/asm/mach-atheros
 # -I/home/salter.xu/auto-gpl.git/tmp/linux/linux-2.6.31/arch/mips/include/asm/mach-generic
 # -D__KERNEL__ -DVMLINUX_LOAD_ADDRESS=0xffffffff80002000
 # -DKBUILD_STR(s)=#s -DKBUILD_BASENAME=KBUILD_STR(bounds)
 # -DKBUILD_MODNAME=KBUILD_STR(bounds) -isystem
 # /home/salter.xu/pull-project/wndr4500v3-buildroot/staging_dir/toolchain-mips_gcc-4.3.3_uClibc-0.9.30.1/usr/lib/gcc/mips-openwrt-linux-uclibc/4.3.3/include
 # -include include/linux/autoconf.h -MD kernel/.bounds.s.d kernel/bounds.c
 # -G 0 -mno-check-zero-division -mabi=32 -mno-abicalls -msoft-float
 # -march=mips32r2 -mdspr2 -mllsc -mno-shared -auxbase-strip
 # kernel/bounds.s -Os -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs
 # -Werror-implicit-function-declaration -Wno-format-security
 # -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-aliasing
 # -fno-common -fno-delete-null-pointer-checks -fno-pic -ffreestanding
 # -fno-stack-protector -fomit-frame-pointer -fno-strict-overflow
 # -fverbose-asm
 # options enabled:  -falign-loops -fargument-alias -fauto-inc-dec
 # -fbranch-count-reg -fcaller-saves -fcprop-registers -fcrossjumping
 # -fcse-follow-jumps -fdefer-pop -fearly-inlining
 # -feliminate-unused-debug-types -fexpensive-optimizations
 # -fforward-propagate -ffunction-cse -fgcse -fgcse-lm
 # -fguess-branch-probability -fident -fif-conversion -fif-conversion2
 # -finline-functions -finline-functions-called-once
 # -finline-small-functions -fipa-pure-const -fipa-reference -fivopts
 # -fkeep-static-consts -fleading-underscore -fmath-errno -fmerge-constants
 # -fmerge-debug-strings -fmove-loop-invariants -fomit-frame-pointer
 # -foptimize-register-move -foptimize-sibling-calls -fpcc-struct-return
 # -fpeephole -fpeephole2 -fregmove -freorder-functions
 # -frerun-cse-after-loop -fsched-interblock -fsched-spec
 # -fsched-stalled-insns-dep -fschedule-insns -fschedule-insns2
 # -fsigned-zeros -fsplit-ivs-in-unroller -fsplit-wide-types -fthread-jumps
 # -ftoplevel-reorder -ftrapping-math -ftree-ccp -ftree-copy-prop
 # -ftree-copyrename -ftree-cselim -ftree-dce -ftree-dominator-opts
 # -ftree-dse -ftree-fre -ftree-loop-im -ftree-loop-ivcanon
 # -ftree-loop-optimize -ftree-parallelize-loops= -ftree-reassoc
 # -ftree-salias -ftree-scev-cprop -ftree-sink -ftree-sra -ftree-store-ccp
 # -ftree-ter -ftree-vect-loop-version -ftree-vrp -funit-at-a-time
 # -fverbose-asm -fzero-initialized-in-bss -mbranch-likely -mdivide-traps
 # -mdouble-float -mdsp -mdspr2 -meb -mexplicit-relocs -mextern-sdata
 # -mfp-exceptions -mfp32 -mfused-madd -mgp32 -mgpopt -mllsc -mlocal-sdata
 # -mlong32 -mmemcpy -mno-mips16 -mno-mips3d -msoft-float -msplit-addresses
 # -muclibc

 # Compiler executable checksum: 9ecf495b4e5e5f450e7bd06aed46ad01

	.text
	.align	2
	.globl	foo
	.ent	foo
	.type	foo, @function
foo:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
 # 16 "kernel/bounds.c" 1
	
->NR_PAGEFLAGS 23 __NR_PAGEFLAGS	 #
 # 0 "" 2
 # 17 "kernel/bounds.c" 1
	
->MAX_NR_ZONES 2 __MAX_NR_ZONES	 #
 # 0 "" 2
#NO_APP
	j	$31
	.end	foo
	.ident	"GCC: (GNU) 4.3.3"
