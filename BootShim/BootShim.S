_head:
	/* Set current address */
	adr x1, #0

	/* Jump to the real code */
	b _start
	
	/* Fake Linux kernel header */

	/* Text Offset */
	.quad 0x80000

	/* Image Size */
	.quad 0x200000

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

_start:
	/* Your code will get ran right after this binary */