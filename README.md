## Re-create Dangerous Dave ID Software

This guide provides steps to recreate Dangerous Dave using mingw32-make, SDL, and unlzexe.exe.

### Website material
https://moddingwiki.shikadi.net/wiki/Dangerous_Dave
https://moddingwiki.shikadi.net/wiki/Dangerous_Dave_Tileset_Format

### Requirements

* mingw32-make
* SDL
* unzlexe
* DAVE.EXE

### Installation
1. Install mingw32-make
Download and install MinGW-W64:
* [MinGW-W64 Online Installer](https://github.com/niXman/mingw-builds-binaries)
* Install to C:/mingw64
* Add the installation directory to the system PATH
* Verify installation by running gcc in the terminal

2. Download SDL2:
* Download [SDL2-devel-2.30.8-mingw.zip](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.8 )
* Unzip to C:/MinGW_SDL

3. Download unzlexe.exe
* Download the latest 32-bit version of unlzexe.exe

4. Obtain DAVE.EXE
Download Dangerous Dave executable from here

### Download unzlexe.exe
https://keenwiki.shikadi.net/wiki/UNLZEXE
- Unlzexe Windows 32-bit newest

### Download DAVE.EXE
Download the DAVE.EXE from the ID Software website.
https://www.dosgamesarchive.com/file/dangerous-dave/gameedge

### Steps
1. Decompress DAVE.EXE
Use unlzexe.exe to decompress the file:

```bash
unlzexe.exe DAVE.EXE
```
2. Convert DAVE.EXE to BMP
Run the following commands to convert the game tiles:
```bash
mingw32-make 
or
mingw32-make -f MakefileCpp
./TILES
./LEVEL
```
