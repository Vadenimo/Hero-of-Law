PATH_TO_DECOMP="/mnt/d/Nintendo 64/Decomp/oot-1.0/oot"
BUILDFILE="Build"
eval $(echo mips64-gcc -c -G 0 -Os -I\'"${PATH_TO_DECOMP}"\/include/libc\' -I\'"${PATH_TO_DECOMP}"\/include/ultra64\' -I\'"${PATH_TO_DECOMP}"\/include\' ${BUILDFILE}.c -nostdinc -DF3DLP_GBI -DF3DEX_GBI_2)
eval $(echo mips64-ld --emit-relocs -o ${BUILDFILE}.elf ${BUILDFILE}.o -T ${BUILDFILE}.ld)
eval $(echo mips64-objcopy -R .MIPS.abiflags -O binary ${BUILDFILE}.elf ${BUILDFILE}.zobj)
eval $(echo mips64-objdump --syms ${BUILDFILE}.elf) >> symbols.txt