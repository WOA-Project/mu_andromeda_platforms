.text
.align 4

_BootConfigShimEntry:
	movz	x0, #((UEFI_FD_BASE + UEFI_FD_SIZE) & 0xffff)
	movk	x0, #(((UEFI_FD_BASE + UEFI_FD_SIZE) >> 16) & 0xffff), lsl #16
	movz	x1, #(BOOT_CONFIG_SIZE & 0xffff)
	movk	x1, #((BOOT_CONFIG_SIZE >> 16) & 0xffff), lsl #16
	movz	x9, #(UEFI_FD_BASE & 0xffff)
	movk	x9, #((UEFI_FD_BASE >> 16) & 0xffff), lsl #16
	br		x9

_BootConfigShimEnd:
	.space	(BOOT_CONFIG_SHIM_SIZE - (_BootConfigShimEnd - _BootConfigShimEntry))
