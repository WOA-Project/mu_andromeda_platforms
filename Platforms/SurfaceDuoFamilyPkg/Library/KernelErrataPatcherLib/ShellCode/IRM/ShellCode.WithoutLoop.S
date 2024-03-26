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
	mov	x15, x0
	ldr	x7, [x15, 16]!
	cmp	x7, x15
	beq	_Payload
	mov	w5, 52138
	movk	w5, 0xd518, lsl 16
	mov	w14, 26
	movk	w14, 0x1400, lsl 16
	mov	x13, 40202
	movk	x13, 0xd368, lsl 16
	movk	x13, 0xa8, lsl 32
	movk	x13, 0xd538, lsl 48
	mov	x12, 11
	movk	x12, 0xd280, lsl 16
	movk	x12, 0x17f, lsl 32
	movk	x12, 0xeb08, lsl 48
	mov	x11, 160
	movk	x11, 0x5400, lsl 16
	movk	x11, 0xdd69, lsl 32
	movk	x11, 0xd378, lsl 48
	mov	x10, 297
	movk	x10, 0xaa0a, lsl 16
	movk	x10, 0xcba9, lsl 32
	movk	x10, 0xd518, lsl 48
	mov	x9, 16287
	movk	x9, 0xd503, lsl 16
	movk	x9, 0x16b, lsl 32
	movk	x9, 0x9104, lsl 48
	mov	x8, 383
	movk	x8, 0xf120, lsl 16
	movk	x8, 0xff01, lsl 32
	movk	x8, 0x54ff, lsl 48
	mov	w6, 8223
	movk	w6, 0xd503, lsl 16
	b	.L8
.L6:
	add	x2, x2, 4
	cmp	x2, x4
	bcs	.L5
.L7:
	ldr	w3, [x2]
	cmp	w3, w5
	bne	.L6
	str	w14, [x2, -8]
	str	x13, [x2, 96]
	str	x12, [x2, 104]
	str	x11, [x2, 112]
	str	x10, [x2, 120]
	str	x9, [x2, 128]
	str	x8, [x2, 136]
	str	w6, [x2, 144]
	str	w6, [x2, 148]
	b	.L6
.L5:
	ldr	x7, [x7]
	cmp	x7, x15
	b	_Payload
.L8:
	ldr	x2, [x7, 48]
	ldr	w4, [x7, 64]
	add	x4, x4, x2
	cmp	x2, x4
	bcc	.L7
	b	.L5

_Dead:
	/* We should never get here */
	b		_Dead

.text
.align 4

_Payload:
	/* Your code will get ran right after this binary */
