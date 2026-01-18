ARCH              = arm64
TARGET            = aarch64-linux-gnu
CC                = clang
CFLAGS            = --target=$(TARGET)
LDFLAGS           = -nostdlib -nostartfiles -Wl,-Tstage1.ld
OBJCOPY           = llvm-objcopy

all: stage1.elf stage1.bin

stage1.bin: stage1.elf
	$(OBJCOPY) -O binary $< $@

stage1.elf: stage1.S
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -f stage1.elf stage1.bin
