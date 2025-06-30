# Hero of Law Patch Creator

Setup/usage:
1. Make sure the `7z` command is available in your environment
2. Create `tools` directory, and put `xdelta3-3.1.0-i686.exe` (available from [here](https://github.com/jmacd/xdelta-gpl/releases/tag/v3.1.0)) in it
2. Create `in_roms` directory, and put the base roms (`CZLE_1.0.z64`, etc.) there (both `z64`s and `wad`s)
3. Create `in_pdfs` directory, and put PDFs you want to include in the offline download package there
4. Run: `python3 make_patches.py` (uses `build-yaz.z64` and `build-yaz.wad` from the Build folder)
5. Output: `HeroOfLaw_web_patches` directory and `HeroOfLaw.7z` file
