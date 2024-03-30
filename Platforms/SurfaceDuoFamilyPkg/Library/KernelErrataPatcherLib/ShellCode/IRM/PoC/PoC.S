	.arch armv8-a
	.file	"PoC.c"
	.text
	.align	2
	.global	PreOslArm64TransferToKernel
	.type	PreOslArm64TransferToKernel, %function
PreOslArm64TransferToKernel:
	.cfi_startproc

				ubfiz		x8, x0, #0x18, #4       // 
				orr			x10, x8, #1             // 
				mrs			x20, mpidr_el1          // 
				and			x8, x20, #0xf00         // 
				add			x20, x10, #0x80,lsl#12  // 
				orr			x21, x10, x8,lsl#8      // 
				b			.loop_entry             // 
.loop:
				cmp			x10, x21                // 
				b.eq		.skip                   // 
				msr			icc_sgi1r_el1, x10      // 
				dsb			sy                      // 
.skip:
				add			x10, x10, #0x10,lsl#12  // 
.loop_entry:
				cmp			x10, x20                // 
				b.cc		.loop                   // 

	.cfi_endproc
.LFE0:
	.size	PreOslArm64TransferToKernel, .-PreOslArm64TransferToKernel
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
