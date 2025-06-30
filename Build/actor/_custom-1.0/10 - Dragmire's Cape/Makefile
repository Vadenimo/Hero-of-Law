BUILDDIR = build
SOURCEDIR = source

PROGRAM = voiceMgr
OVLOUT = actor.zovl
ADDRESS = 0x80800000

ELF = $(BUILDDIR)/$(PROGRAM).elf
SRCS := $(shell find $(SOURCEDIR) -name '*.c')
PARTS:=$(SRCS:.c=.o)
OBJECTS := $(addprefix $(BUILDDIR)/,$(notdir $(PARTS)))

CC   = mips64-gcc
LD   = mips64-ld
NOVL = novl

CFLAGS = -G 0 $(OPTIMIZATION) $(Z64HDR) --std=gnu99 -Wno-multichar -Wno-overflow -mtune=vr4300 -mabi=32 -mips3 -fno-builtin -mno-split-addresses -fno-reorder-blocks -ffast-math -mno-explicit-relocs -fno-unsafe-math-optimizations -fno-optimize-sibling-calls -mno-memcpy -mno-check-zero-division -Wmissing-field-initializers -Wall -Wno-unknown-pragmas
NOVLFLAGS = -c -A $(ADDRESS) -o $(OVLOUT)
DEFINES = 

default: 10

10: OPTIMIZATION = -Os
10: Z64HDR = -I/opt/n64/mips64/include/z64hdr/include -I/opt/n64/mips64/include/z64hdr/oot_u10 -I/opt/n64/mips64/include
10: LDFLAGS = -L/opt/n64/mips64/include/z64hdr/common -L/opt/n64/mips64/include/z64hdr/oot_u10/ -T /opt/n64/mips64/include/z64hdr/common/z64hdr.ld --emit-relocs
10: create_dir $(PROGRAM).ovl

10D: OPTIMIZATION = -Os
10D: Z64HDR = -I/opt/n64/mips64/include/z64hdr/include -I/opt/n64/mips64/include/z64hdr/oot_u10 -I/opt/n64/mips64/include
10D: LDFLAGS = -L/opt/n64/mips64/include/z64hdr/common -L/opt/n64/mips64/include/z64hdr/oot_u10/ -T /opt/n64/mips64/include/z64hdr/common/z64hdr.ld --emit-relocs
10D: create_dir $(PROGRAM).ovl


clean: 
	rm -fv $(ELF) $(OVLOUT) $(OBJECTS)
	



create_dir:
	@mkdir -p $(BUILDDIR)
	
$(PROGRAM).ovl: $(PARTS)
	$(LD) -o $(ELF) $(OBJECTS) $(LDFLAGS)
	$(NOVL) $(NOVLFLAGS) $(BUILDDIR)/$(PROGRAM).elf
	
%.o: %.c
	$(CC) -c $(CFLAGS) $< $(DEFINES) -o $(BUILDDIR)/$(notdir $@)