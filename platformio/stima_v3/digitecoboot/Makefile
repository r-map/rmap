#----------------------------------------------------------------------
# Main configuration
#----------------------------------------------------------------------
SD_CS_PORT    	= PORTB	# Data Register of the SD CS pin
SD_CS_DDR     	= DDRB	# Data Direction Register of the SD CS pin
SD_CS_BIT     	= 3	# Bit of the SD CS pin
USE_LED       	= 0	# Debug with two (defined in asmfunc.S)
USE_FAT16     	= 0	# Enable Fat16 support, disabled by default. If enabled, disable Fat32 to keep size of bootloader within limits
USE_FAT32     	= 1	# Enable Fat32 support, enabled by default to allow the use of sd cards > 2GB
#AVRDUDE         = sleep 5; avrdude -v -c dragon_isp -P usb -p $(MCU_AVRDUDE) # edit this line for your programmer
#AVRDUDE         = avrdude  -v -c arduino -P /dev/ttyUSB0 -b$(BAUD_RATE) -p $(MCU_AVRDUDE) # edit this line for your programmer
AVRDUDE         = avrdude -F -v -c stk500v1 -P /dev/ttyUSB0 -b$(BAUD_RATE) -p $(MCU_AVRDUDE) # edit this line for your programmer
#AVRDUDE         = avrdude -v -c avrisp -P /dev/ttyUSB0 -b$(BAUD_RATE) -p $(MCU_AVRDUDE) # edit this line for your programmer


#----------------------------------------------------------------------
# Other configuration
#----------------------------------------------------------------------
PROGRAM		= digitecoboot
include avr_conf
include Makefile.644
include Makefile.1284
CSRC        	= optiboot.c pff/src/pff.c diskio.c
ASRC        	= asmfunc.S
DEFS        	= -DBIGBOOT -DBOOT_ADR=$(BOOT_ADR) -DF_CPU=$(F_CPU) -DUSE_LED=$(USE_LED) -DSD_CS_PORT=$(SD_CS_PORT) -DSD_CS_DDR=$(SD_CS_DDR) -DSD_CS_BIT=$(SD_CS_BIT) -DUSE_FAT16=$(USE_FAT16) -DUSE_FAT32=$(USE_FAT32) -DBAUD_RATE=$(BAUD_RATE)
OPTIMIZE    	= -Os -mcall-prologues -ffunction-sections -fdata-sections -fwhole-file -flto -funsigned-char -fno-split-wide-types -fno-inline-small-functions -ffreestanding -fno-jump-tables
LIBS        	=

ASFLAGS     	= -Wa,-adhlns=$(<:.S=.lst),-gstabs $(DEFS)
ALL_ASFLAGS 	= -mmcu=$(MCU_TARGET) -I. -x assembler-with-cpp $(ASFLAGS)
CFLAGS      	= -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -std=c99 $(DEFS)
LDFLAGS     	= -Wl,-Map,$(PROGRAM)_$(TARGET).map -Wl,--gc-sections -Wl,--section-start,.text=$(BOOT_ADR)
OBJ         	= $(CSRC:.c=.o) $(ASRC:.S=.o)

CC          	= $(AVR_DIR)/bin/avr-gcc
OBJCOPY     	= $(AVR_DIR)/bin/avr-objcopy
OBJDUMP     	= $(AVR_DIR)/bin/avr-objdump
SIZE        	= $(AVR_DIR)/bin/avr-size

MYTARGET	= $(PROGRAM)_$(TARGET)

#---------------------------------------------------------------------------
# Generic build instructions
#---------------------------------------------------------------------------
all:	clean $(MYTARGET).elf lst text bin size fuse flash lock

%.elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

# rule for programming fuse bits:
fuse:
	@[ "$(LOCK)" != "" -a "$(HFUSE)" != "" -a "$(LFUSE)" != "" -a "$(EFUSE)" != "" ] || \
		{ echo "*** Edit Makefile and choose values for FUSE!"; exit 1; }
	$(AVRDUDE) -e -U lock:w:$(UNLOCK):m -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m -U efuse:w:$(EFUSE):m

# rule for uploading firmware:
flash: $(MYTARGET).hex
	$(AVRDUDE) -U flash:w:$(MYTARGET).hex:i

# rule for lock bootloader:
lock:
	@[ "$(LOCK)" != "" ] || \
		{ echo "*** Edit Makefile and choose values for LOCK!"; exit 1; }
	$(AVRDUDE) -U lock:w:$(LOCK):m

clean:
	rm -fr *.o *.elf *.eps *.bak *.a *.bin
	rm -fr pff/src/*.o uart/*.o
	rm -fr *.lst *.map
	rm -fr *.hex
	rm -fr *.gch

size: $(MYTARGET).elf
	$(SIZE) -C --mcu=$(MCU_TARGET) $(MYTARGET).elf

lst: $(MYTARGET).lst
%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.o : %.S
	$(CC) -c $(ALL_ASFLAGS) $< -o $@

text: $(MYTARGET).hex
%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -j .fuse -O ihex $< $@

# --- make bin just to check size :)
bin: $(MYTARGET).bin
%.bin: %.hex
	$(OBJCOPY) -I ihex -O binary $< $@

print-%  : ; @echo $* = $($*)
