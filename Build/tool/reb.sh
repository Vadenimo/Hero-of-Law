#!/bin/sh

rm "oot-1.0-dec.z64"
python3 "tool//patcher.py" "oot-1.0.z64" "patch//rom.txt" "oot-1.0-dec.z64"
    
python3 "tool//patcher.py" "system//code//code.bin" "patch//code.txt" "system//code//code.bin"

    
    
    