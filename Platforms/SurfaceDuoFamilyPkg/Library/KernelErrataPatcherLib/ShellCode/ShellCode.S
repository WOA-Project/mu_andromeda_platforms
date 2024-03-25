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
	ldr	x3, [x0, 16]
	ldr	x2, [x3, 48]
	ldr	w4, [x3, 64]
	add	x4, x4, x2
	cmp	x2, x4
	bcs	_Payload
	mov	w5, 52138
	movk	w5, 0xd518, lsl 16
	mov	w30, 26
	movk	w30, 0x1400, lsl 16
	mov	w18, 40202
	movk	w18, 0xd368, lsl 16
	mov	w17, 168
	movk	w17, 0xd538, lsl 16
	mov	w16, 11
	movk	w16, 0xd280, lsl 16
	mov	w15, 383
	movk	w15, 0xeb08, lsl 16
	mov	w14, 160
	movk	w14, 0x5400, lsl 16
	mov	w13, 56681
	movk	w13, 0xd378, lsl 16
	mov	w12, 297
	movk	w12, 0xaa0a, lsl 16
	mov	w11, 52137
	movk	w11, 0xd518, lsl 16
	mov	w10, 16287
	movk	w10, 0xd503, lsl 16
	mov	w9, 363
	movk	w9, 0x9104, lsl 16
	mov	w8, 383
	movk	w8, 0xf120, lsl 16
	mov	w7, 65281
	movk	w7, 0x54ff, lsl 16
	mov	w6, 8223
	movk	w6, 0xd503, lsl 16
	b	.L6
.L5:
	add	x2, x2, 4
	cmp	x2, x4
	bcs	_Payload
.L6:
	ldr	w3, [x2]
	cmp	w3, w5
	bne	.L5
	str	w30, [x2, -8]
	add	w3, w2, 96
	str	w18, [x3]
	add	w3, w2, 100
	str	w17, [x3]
	add	w3, w2, 104
	str	w16, [x3]
	add	w3, w2, 108
	str	w15, [x3]
	add	w3, w2, 112
	str	w14, [x3]
	add	w3, w2, 116
	str	w13, [x3]
	add	w3, w2, 120
	str	w12, [x3]
	add	w3, w2, 124
	str	w11, [x3]
	add	w3, w2, 128
	str	w10, [x3]
	add	w3, w2, 132
	str	w9, [x3]
	add	w3, w2, 136
	str	w8, [x3]
	add	w3, w2, 140
	str	w7, [x3]
	add	w3, w2, 144
	str	w6, [x3]
	add	w3, w2, 148
	str	w6, [x3]
	b	.L5
	b	_Payload

_Dead:
	/* We should never get here */
	b		_Dead

.text
.align 4

_Payload:
	/* Your code will get ran right after this binary */

