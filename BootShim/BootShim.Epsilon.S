_Head:
	/* Jump to the real code */
	b		_Start

_KernelStart:
	b 		#0x01B7FFFC /* b #0x01B80000 - 4 */

	/* Text Offset */
	.quad 0x00080000

	/* Image Size */
	.quad 0x0277F000

	/* Flags */
	.quad 0xA

_StackBase:
	.quad 0x9FC00000

_StackSize:
	.quad 0x00300000

_KernelSize:
	.quad 0x0217D810

	/* ARM64 Magic */
	.ascii "ARM\x64"

	/* Reserved */
	.long 0

_Start:
	mov		x4, #0x9004
	movk		x4, #0x3d7, lsl #16
	ldr		w5, [x4]
	and     w5, w5, #0x1
	cbz		w5, _CopyUEFI
	b		_KernelStart

_CopyUEFI:
	/* Copy the UEFI into the right location */
	adr		x4, _Head
	ldr		x5, _KernelSize
	add		x4, x4, x5

	ldr		x5, _StackBase
	ldr		x6, _StackSize

	bl 		_CopyLoop

_Entry:
	adr		x0, _Payload
	adr		x1, _Payload
	ldr		x5, _StackBase
	br		x5

/* x4 = SrcAddr, x5 = DstAddr, x6 = Length */
_CopyLoop:
	ldp		x2, x3, [x4], #16
	stp		x2, x3, [x5], #16
	subs		x6, x6, #16
	b.ne		_CopyLoop
	ret

_Dead:
	/* We should never get here */
	b		_Dead

.text
.align 4

_Payload:
	/* Your code will get ran right after this binary */