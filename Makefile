ASM = sdasz80
LD = sdldz80
OBJCOPY = objcopy

BUILD_DIR = build
TARGET = $(BUILD_DIR)/paku
ASM_SOURCES = $(wildcard src/*.asm) $(wildcard src/*.inc)

.PHONY: all clean run

all: $(TARGET).rom

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET).rel: $(ASM_SOURCES) | $(BUILD_DIR)
	$(ASM) -plosgff -o $@ $<

$(TARGET).ihx: $(TARGET).rel
	$(LD) -n -i $(TARGET) $<

$(TARGET).rom: $(TARGET).ihx
	$(OBJCOPY) -I ihex -O binary --change-addresses -0x4000 --pad-to 0x4000 --gap-fill 0 $< $@

run: $(TARGET).rom
	openmsx -cart $<

clean:
	rm -rf $(BUILD_DIR)
