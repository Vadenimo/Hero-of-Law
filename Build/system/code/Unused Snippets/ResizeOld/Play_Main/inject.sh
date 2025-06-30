make clean
make

if command -v py; then
    py -3 ../../../../tool/inject_file.py code1.bin "../../code.bin" 0x8BA28 0x40
else
    python3 ../../../../tool/inject_file.py code1.bin "../../code.bin" 0x8BA28 0x40
fi
