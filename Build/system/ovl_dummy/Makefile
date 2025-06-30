BUILDDIR = build
SOURCEDIR = source

PROGRAM = dummy
OVLOUT = dummy.zovl
ADDRESS = 0x80800000

ELF = $(BUILDDIR)/$(PROGRAM).elf
SRCS := $(shell find $(SOURCEDIR) -name '*.c')
PARTS:=$(SRCS:.c=.o)
OBJECTS := $(addprefix $(BUILDDIR)/,$(notdir $(PARTS)))

CC   = mips64-gcc
LD   = mips64-ld
NOVL = novl

CFLAGS = -G 0 $(OPTIMIZATION) $(Z64HDR) --std=gnu99 -mtune=vr4300 -mabi=32 -mips3 -mno-split-addresses -fno-builtin -fno-reorder-blocks -ffast-math -mno-explicit-relocs -fno-unsafe-math-optimizations -fno-optimize-sibling-calls -mno-memcpy -mno-check-zero-division -Wmissing-field-initializers -Wall -Wno-unknown-pragmas
NOVLFLAGS = -c -A $(ADDRESS) -o $(OVLOUT)

default: 10

10: OPTIMIZATION = -Os
10: Z64HDR = -I/opt/n64/mips64/include/z64hdr/include -I/opt/n64/mips64/include/z64hdr/oot_u10 -I/opt/n64/mips64/include/
10: LDFLAGS = -L/opt/n64/mips64/include/z64hdr/common -L/opt/n64/mips64/include/z64hdr/oot_u10/ -T /opt/n64/mips64/include/z64hdr/common/z64hdr.ld --emit-relocs
10: create_dir 
10: $(PROGRAM).ovl
10: copy

10D: OPTIMIZATION = -Os
10D: Z64HDR = -I/opt/n64/mips64/include/z64hdr/include -I/opt/n64/mips64/include/z64hdr/oot_u10 -I/opt/n64/mips64/include/
10D: LDFLAGS = -L/opt/n64/mips64/include/z64hdr/common -L/opt/n64/mips64/include/z64hdr/oot_u10/ -T /opt/n64/mips64/include/z64hdr/common/z64hdr.ld --emit-relocs
10D: create_dir
10D: $(PROGRAM).ovl
10D: copy

clean: 
	rm -fv $(ELF) $(OVLOUT) $(OBJECTS)
	rm -fv ../ovl_file_choose/ovl_file_choose.ovl
	rm -fv ../ovl_kaleido_scope/ovl_kaleido_scope.ovl
	rm -fv ../ovl_map_mark_data/ovl_map_mark_data.ovl
	rm -fv ../ovl_select/ovl_select.ovl
	rm -fv ../ovl_player_actor/ovl_player_actor.ovl	
	
create_dir:
	@mkdir -p $(BUILDDIR)
	
copy:
	@cp $(OVLOUT) ../ovl_file_choose/ovl_file_choose.zovl
	@cp $(OVLOUT) ../ovl_kaleido_scope/ovl_kaleido_scope.zovl
	@cp $(OVLOUT) ../ovl_map_mark_data/ovl_map_mark_data.zovl
	@cp $(OVLOUT) ../ovl_select/ovl_select.zovl
	@cp $(OVLOUT) ../ovl_player_actor/ovl_player_actor.zovl

	
$(PROGRAM).ovl: $(PARTS)
	$(LD) -o $(ELF) $(OBJECTS) $(LDFLAGS)
	$(NOVL) $(NOVLFLAGS) $(BUILDDIR)/$(PROGRAM).elf
	
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $(BUILDDIR)/$(notdir $@)