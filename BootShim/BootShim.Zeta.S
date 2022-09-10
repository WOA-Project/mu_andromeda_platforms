_Head:
	/* Set _Entry address */
	adr x1, #0x70

	/* Jump to the real code */
	b _Start

	/* Fake Linux kernel header */

_StackBase:
	/* Text Offset */
	.quad 0x9F000000

_StackSize:
	/* Image Size */
	.quad 0x00500000

	/* Flags */
	.quad 0

	/* Reserved */
	.quad 0

	/* Reserved */
	.quad 0

	/* Reserved */
	.quad 0

	/* ARM64 Magic */
	.ascii "ARM\x64"

	/* Reserved */
	.long 0

_Start:
	ldr		x5, _StackBase
	cmp		x1, x5
	beq 	_Entry
	ldr		x6,  _StackSize

_CopyLoop:
	ldp		x2, x3, [x1], #16
	stp		x2, x3, [x5], #16
	subs	x6, x6, #16
	b.ne   _CopyLoop

_Entry:
	br       x5
	/* Your code will get ran right after this binary */
	nop
	nop

	/* We should never get here */
dead:
	b		dead