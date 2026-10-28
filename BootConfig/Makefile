ARCH              = arm64
TARGET            = aarch64-linux-gnu
CROSS_COMPILE     = $(TARGET)-
CC                = $(CROSS_COMPILE)gcc
OBJCOPY           = $(CROSS_COMPILE)objcopy

.PHONY: FORCE
FORCE:

BootConfigShim.bin: BootConfigShim.elf
	$(OBJCOPY) -O binary $< $@

BootConfigShim.elf: BootConfigShim.S FORCE
	$(CC) -c $< -o $@ \
		-DBOOT_CONFIG_SIZE=$(BOOT_CONFIG_SIZE) \
		-DBOOT_CONFIG_SHIM_SIZE=$(BOOT_CONFIG_SHIM_SIZE) \
		-DUEFI_FD_BASE=$(UEFI_FD_BASE) \
		-DUEFI_FD_SIZE=$(UEFI_FD_SIZE)
