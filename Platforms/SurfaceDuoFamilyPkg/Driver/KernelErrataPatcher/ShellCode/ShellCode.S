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
	mov	x14, x0
	ldr	x8, [x14, 16]!
	cmp	x8, x14
	beq	_Payload
	mov	w6, 14549
	movk	w6, 0x2810, lsl 16
	mov	w7, 6357
	movk	w7, 0x2910, lsl 16
	mov	w10, 15317
	movk	w10, 0xa8d2, lsl 16
	mov	w12, 7125
	movk	w12, 0xa9d2, lsl 16
	mov	x13, 32978
	movk	x13, 0x300, lsl 16
	movk	x13, 0x18, lsl 32
	movk	x13, 0xd502, lsl 48
	mov	x15, 32978
	movk	x15, 0x200, lsl 16
	movk	x15, 0x80d2, lsl 32
	movk	x15, 0x300, lsl 48
	mov	x17, 24
	movk	x17, 0x4002, lsl 16
	movk	x17, 0x80d2, lsl 32
	movk	x17, 0x100, lsl 48
	mov	w18, 24534
	movk	w18, 0xc003, lsl 16
	mov	x16, 32978
	movk	x16, 0x100, lsl 16
	movk	x16, 0x80d2, lsl 32
	movk	x16, 0x200, lsl 48
	mov	w11, 981
	movk	w11, 0x1f20, lsl 16
	mov	w9, 32978
	movk	w9, 0x800, lsl 16
	b	.L13
.L17:
	str	w9, [x2]
.L7:
	add	x2, x2, 4
	cmp	x2, x5
	bcs	.L5
.L12:
	ldr	w3, [x2]
	cmp	w3, w6
	beq	.L17
	cmp	w3, w7
	beq	.L18
	cmp	w3, w10
	beq	.L19
	cmp	w3, w12
	beq	.L20
	ldr	x3, [x2]
	cmp	x3, x13
	beq	.L21
	cmp	x3, x15
	bne	.L7
	ldr	x3, [x2, 8]
	cmp	x3, x17
	bne	.L7
	str	w18, [x2, -28]
	b	.L7
.L18:
	str	w11, [x2]
	b	.L7
.L19:
	str	w9, [x2]
	b	.L7
.L20:
	str	w11, [x2]
	b	.L7
.L21:
	ldr	x3, [x2, 8]
	cmp	x3, x16
	bne	.L7
	str	w18, [x2, -32]
	b	.L7
.L5:
	ldr	x8, [x8]
	cmp	x8, x14
	beq	_Payload
.L13:
	ldr	x2, [x8, 48]
	ldr	w5, [x8, 64]
	add	x5, x5, x2
	cmp	x2, x5
	bcc	.L12
	b	.L5

_Dead:
	/* We should never get here */
	b		_Dead

.text
.align 4

_Payload:
	/* Your code will get ran right after this binary */

