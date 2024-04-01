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
	mov	x10, x0
	ldr	x9, [x10, 16]!
	cmp	x9, x10
	beq	_Payload
	mov	x3, 0
	mov	x4, 0
	mov	w8, 52138
	movk	w8, 0xd518, lsl 16
	mov	x7, 3
	movk	x7, 0x9, lsl 16
	movk	x7, 0x16, lsl 32
	movk	x7, 0x9, lsl 48
	b	.L9
.L5:
	ldr	x9, [x9]
	cmp	x9, x10
	beq	.L13
.L9:
	ldr	x2, [x9, 48]
	ldr	w6, [x9, 64]
	add	x6, x6, x2
	cmp	x2, x6
	bcs	.L5
.L8:
	ldr	w5, [x2]
	cmp	w5, w8
	csel	x4, x4, x2, ne
	ldr	x5, [x2]
	cmp	x5, x7
	csel	x3, x3, x2, ne
	cmp	x4, 0
	ccmp	x3, 0, 4, ne
	bne	.L5
	add	x2, x2, 4
	cmp	x2, x6
	bcc	.L8
	b	.L5
.L13:
	cmp	x4, 0
	ccmp	x3, 0, 4, ne
	beq	_Payload
	sub	x2, x3, #168
	sub	x2, x2, x4
	ubfx	w2, w2, 2, 26
	mov	w5, -1811939328
	orr	w2, w2, w5
	str	w2, [x4]
	mov	x2, 21491
	movk	x2, 0xa9bd, lsl 16
	movk	x2, 0x5bf5, lsl 32
	movk	x2, 0xa901, lsl 48
	str	x2, [x3, -168]
	mov	x2, 5118
	movk	x2, 0xf900, lsl 16
	movk	x2, 0x24a, lsl 32
	movk	x2, 0xb640, lsl 48
	str	x2, [x3, -160]
	mov	x2, 63830
	movk	x2, 0x9257, lsl 16
	movk	x2, 0x13, lsl 32
	movk	x2, 0x5280, lsl 48
	str	x2, [x3, -152]
	mov	x2, 161
	movk	x2, 0xd538, lsl 16
	movk	x2, 0x2c35, lsl 32
	movk	x2, 0xd348, lsl 48
	str	x2, [x3, -144]
	mov	x2, 20
	movk	x2, 0xd280, lsl 16
	movk	x2, 0x29f, lsl 32
	movk	x2, 0xeb15, lsl 48
	str	x2, [x3, -136]
	mov	x2, 192
	movk	x2, 0x5400, lsl 16
	movk	x2, 0x5e68, lsl 32
	movk	x2, 0xd378, lsl 48
	str	x2, [x3, -128]
	mov	x2, 264
	movk	x2, 0xaa16, lsl 16
	movk	x2, 0x10a, lsl 32
	movk	x2, 0xb240, lsl 48
	str	x2, [x3, -120]
	mov	x2, 52138
	movk	x2, 0xd518, lsl 16
	movk	x2, 0x3f9f, lsl 32
	movk	x2, 0xd503, lsl 48
	str	x2, [x3, -112]
	mov	x2, 1651
	movk	x2, 0x1100, lsl 16
	movk	x2, 0x227f, lsl 32
	movk	x2, 0x7100, lsl 48
	str	x2, [x3, -104]
	mov	x2, 1684
	movk	x2, 0x9100, lsl 16
	movk	x2, 0xfec3, lsl 32
	movk	x2, 0x54ff, lsl 48
	str	x2, [x3, -96]
	mov	x2, 3
	movk	x2, 0x1400, lsl 16
	movk	x2, 0xcbaa, lsl 32
	movk	x2, 0xd518, lsl 48
	str	x2, [x3, -88]
	mov	x2, 16287
	movk	x2, 0xd503, lsl 16
	movk	x2, 0x13fe, lsl 32
	movk	x2, 0xf940, lsl 48
	str	x2, [x3, -80]
	mov	x2, 23541
	movk	x2, 0xa941, lsl 16
	movk	x2, 0x53f3, lsl 32
	movk	x2, 0xa8c3, lsl 48
	str	x2, [x3, -72]
	mov	w2, 960
	movk	w2, 0xd65f, lsl 16
	str	w2, [x3, -64]

.text
.align 4

_Payload:
	/* Your code will get ran right after this binary */
