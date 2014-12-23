
Q := @
STM32LOADER_PY  ?= stm32loader.py
STM32LOADER_ARGS ?= -e -w -v 
STM32LOADER_PORT ?= /dev/ttyUSB0
STFLASH         = $(shell which st-flash)


#my extra stuff to add to the makefiles

debug_gdb:
	arm-none-eabi-gdb -ex 'target extended-remote :4242' $(BUILDDIR)/$(PROJECT).elf

serial_bootloader_flash:
	$(STM32LOADER_PY) $(STM32LOADER_ARGS) -p $(STM32LOADER_PORT) $(BUILDDIR)/$(PROJECT).bin

#%.stlink-flash: %.bin
#        @printf "  FLASH  $<\n"
#        $(Q)$(STFLASH) write $(*).bin 0x8000000
stlink-flash:
	@printf "  FLASH  $<\n"
	$(Q)$(STFLASH) write $(*).bin 0x8000000

my_size:
	$(SZ) $(BUILDDIR)/$(PROJECT).elf 

