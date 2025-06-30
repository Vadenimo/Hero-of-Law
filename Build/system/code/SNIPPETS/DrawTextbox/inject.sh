if command -v py; then
    py -3 ../../../../tool/inject_file.py code1.bin "../../code.bin" 0xCC4E4 0x4A0
else
    python3 ../../../../tool/inject_file.py code1.bin "../../code.bin" 0xCC4E4 0x4A0
fi
