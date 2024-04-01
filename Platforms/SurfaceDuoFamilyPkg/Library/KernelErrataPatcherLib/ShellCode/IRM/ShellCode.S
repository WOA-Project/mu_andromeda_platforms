/** @file

  Patches NTOSKRNL to not cause a SError when reading/writing ACTLR_EL1
  Patches NTOSKRNL to not cause a SError when reading/writing AMCNTENSET0_EL0
  Patches NTOSKRNL to not cause a bugcheck when attempting to use
  PSCI_MEMPROTECT Due to an issue in QHEE

  Shell Code to patch kernel mode components before NTOSKRNL

  Copyright (c) 2022-2023 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

//VOID
//OslArm64TransferToKernel (
//  INT VOID *OsLoaderBlock, INT *KernelAddress
//  );
_Start:
	mov	x11, x0
	ldr	x10, [x11, 16]!
	cmp	x10, x11
	beq	_Payload
	mov	x4, 0
	mov	x5, 0
	mov	w9, 52138
	movk	w9, 0xd518, lsl 16
	mov	x8, 3
	movk	x8, 0x9, lsl 16
	movk	x8, 0x16, lsl 32
	movk	x8, 0x9, lsl 48
	b	.L9
.L5:
	ldr	x10, [x10]
	cmp	x10, x11
	beq	.L13
.L9:
	ldr	x2, [x10, 48]
	ldr	w7, [x10, 64]
	add	x7, x7, x2
	cmp	x2, x7
	bcs	.L5
.L8:
	mov	x3, x2
	ldr	w6, [x3], -4
	cmp	w6, w9
	csel	x5, x3, x5, eq
	ldr	x3, [x2]
	cmp	x3, x8
	csel	x4, x4, x2, ne
	cmp	x5, 0
	ccmp	x4, 0, 4, ne
	bne	.L5
	add	x2, x2, 4
	cmp	x2, x7
	bcc	.L8
	b	.L5
.L13:
	cmp	x5, 0
	ccmp	x4, 0, 4, ne
	beq	_Payload
	sub	x2, x4, #168
	sub	x2, x2, x5
	ubfx	w2, w2, 2, 26
	mov	w3, -1811939328
	orr	w2, w2, w3
	str	w2, [x5]
	mov	x2, 31741
	movk	x2, 0xa9bf, lsl 16
	movk	x2, 0x3fd, lsl 32
	movk	x2, 0x9100, lsl 48
	str	x2, [x4, -168]
	mov	x2, 1014
	movk	x2, 0x2a09, lsl 16
	movk	x2, 0xaa, lsl 32
	movk	x2, 0xd538, lsl 48
	str	x2, [x4, -160]
	mov	x2, 3402
	movk	x2, 0x9278, lsl 16
	movk	x2, 0x14, lsl 32
	movk	x2, 0xd280, lsl 48
	str	x2, [x4, -152]
	mov	x2, 277
	movk	x2, 0x5280, lsl 16
	movk	x2, 0x229f, lsl 32
	movk	x2, 0xeb4a, lsl 48
	str	x2, [x4, -144]
	mov	x2, 224
	movk	x2, 0x5400, lsl 16
	movk	x2, 0x1e88, lsl 32
	movk	x2, 0x9240, lsl 48
	str	x2, [x4, -136]
	mov	x2, 3784
	movk	x2, 0xb378, lsl 16
	movk	x2, 0xbd08, lsl 32
	movk	x2, 0xd370, lsl 48
	str	x2, [x4, -128]
	mov	x2, 265
	movk	x2, 0xb240, lsl 16
	movk	x2, 0xcba9, lsl 32
	movk	x2, 0xd518, lsl 48
	str	x2, [x4, -120]
	mov	x2, 16287
	movk	x2, 0xd503, lsl 16
	movk	x2, 0x694, lsl 32
	movk	x2, 0x9100, lsl 48
	str	x2, [x4, -112]
	mov	x2, 1717
	movk	x2, 0x5100, lsl 16
	movk	x2, 0xfed5, lsl 32
	movk	x2, 0x35ff, lsl 48
	str	x2, [x4, -104]
	mov	x2, 31741
	movk	x2, 0xa8c1, lsl 16
	movk	x2, 0x3c0, lsl 32
	movk	x2, 0xd65f, lsl 48
	str	x2, [x4, -96]

.text
.align 4

_Payload:
	/* Your code will get ran right after this binary */
