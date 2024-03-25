	.arch armv8-a
	.file	"PoC.c"
	.text
	.align	2
	.global	PreOslArm64TransferToKernel
	.type	PreOslArm64TransferToKernel, %function
PreOslArm64TransferToKernel:
	.cfi_startproc

	lsl   x10, x8, #0x18    // shift the irq number into the correct position
	mrs   x8, mpidr_el1     // get the mpidr_el1 register of the current cpu
	mov   x11, #0x0         // setup a counter

.loop:
	cmp   x11, x8           // compare the aff1 bit of the current cpu with the aff1 bit of the cpu we're sending the sgi to
	b.eq  .skip             // if they match, skip the current cpu
	lsl   x9, x11, #0x8     // shift the aff1 bit into the correct position
	orr   x9, x9, x10       // add the aff1 bit to the sgir register value
	msr   icc_sgi1r_el1, x9 // send the sgi
	dsb   sy                // ensure the sgi is sent

.skip:
	add   x11, x11, #0x100  // increment the counter
	cmp   x11, #0x800       // check if we've iterated through all cpus (max: 8)
	b.ne  .loop             // if we have not, loop

	.cfi_endproc
.LFE0:
	.size	PreOslArm64TransferToKernel, .-PreOslArm64TransferToKernel
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
