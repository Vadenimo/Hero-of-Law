echo off

echo BUILDUSER...
cd "actor"
cd "_custom-1.0"
cd "369 - Titlescreen"
py -3 builduser.py

echo DISABLE MUSIC DEBUG...
cd ../../../system
cd ovl_opening
py -3 musid.py 3735928559

cd ..
cd ..
cd tool
py -3 build_audio.py 

cd ..

echo y | zzrtl-hol.exe oot_build.rtl

cd tool
echo Building WAD...
cd gzinject
gzinject.exe -a inject -w "Zelda Ocarina N64 NTSC.wad" -m "../../build-yaz.z64" -o "../../build-yaz.wad" -i "NHLE" -r 1 -p "patches/hol.gzi" -k "common-key.bin" -t "Hero of Law" --cleanup

cd ..
cd ..

echo All done!
