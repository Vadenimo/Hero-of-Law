make clean
make

if command -v py; then
    py -3 ../../../../tool/inject_file.py code1.bin "../../code.bin" 0x6DDBC 0x600
else
    python3 ../../../../tool/inject_file.py code1.bin "../../code.bin" 0x6DDBC 0x600
fi
