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
	mov	x20, x0
	ldr	x7, [x20, 16]!
	cmp	x7, x20
	beq	_Payload
	mov	w5, 52138
	movk	w5, 0xd518, lsl 16
	mov	w19, 26
	movk	w19, 0x1400, lsl 16
	mov	w30, 40202
	movk	w30, 0xd368, lsl 16
	mov	w18, 168
	movk	w18, 0xd538, lsl 16
	mov	w17, 11
	movk	w17, 0xd280, lsl 16
	mov	w16, 383
	movk	w16, 0xeb08, lsl 16
	mov	w15, 160
	movk	w15, 0x5400, lsl 16
	mov	w14, 56681
	movk	w14, 0xd378, lsl 16
	mov	w13, 297
	movk	w13, 0xaa0a, lsl 16
	mov	w12, 52137
	movk	w12, 0xd518, lsl 16
	mov	w11, 16287
	movk	w11, 0xd503, lsl 16
	mov	w10, 363
	movk	w10, 0x9104, lsl 16
	mov	w9, 383
	movk	w9, 0xf120, lsl 16
	b	.L8
.L6:
	add	x2, x2, 4
	cmp	x2, x4
	bcs	.L5
.L7:
	ldr	w3, [x2]
	cmp	w3, w5
	bne	.L6
	str	w19, [x2, -8]
	str	w30, [x2, 96]
	str	w18, [x2, 100]
	str	w17, [x2, 104]
	str	w16, [x2, 108]
	str	w15, [x2, 112]
	str	w14, [x2, 116]
	str	w13, [x2, 120]
	str	w12, [x2, 124]
	str	w11, [x2, 128]
	str	w10, [x2, 132]
	str	w9, [x2, 136]
	str	w8, [x2, 140]
	str	w6, [x2, 144]
	str	w6, [x2, 148]
	b	.L6
.L5:
	ldr	x7, [x7]
	cmp	x7, x20
	b	_Payload
.L8:
	ldr	x2, [x7, 48]
	ldr	w4, [x7, 64]
	add	x4, x4, x2
	cmp	x2, x4
	bcs	.L5
	mov	w8, 65281
	movk	w8, 0x54ff, lsl 16
	mov	w6, 8223
	movk	w6, 0xd503, lsl 16
	b	.L7

_Dead:
	/* We should never get here */
	b		_Dead

.text
.align 4

_Payload:
	/* Your code will get ran right after this binary */
