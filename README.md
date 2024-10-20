# Re-create Dangerous Dave ID Software

This guide provides steps to recreate Dangerous Dave using mingw32-make, SDL, and unlzexe.exe.

## Website material
<https://moddingwiki.shikadi.net/wiki/Dangerous_Dave>
<https://moddingwiki.shikadi.net/wiki/Dangerous_Dave_Tileset_Format>

## Requirements

* mingw32-make
* SDL
* unzlexe
* DAVE.EXE

## Installation

1. Install MinGW-W64:

* [MinGW-W64 Online Installer](https://github.com/niXman/mingw-builds-binaries)
* Install to C:/mingw64
* Add the installation directory to the system PATH
* Verify installation by running gcc in the terminal

2. Download SDL2:

* [SDL2-devel-2.30.8-mingw.zip](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.8 )
* Unzip to C:/MinGW_SDL

3. Download unzlexe.exe

* [Unlzexe Windows 32-bit newest](https://keenwiki.shikadi.net/wiki/UNLZEXE)

4. Download DAVE.EXE

* [Dangerous Dave executable](https://www.dosgamesarchive.com/file/dangerous-dave/gameedge)

## Steps

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

3. Copy files to root folder
copy tileset/tileX.bmp to root folder, remove folder
copy tilemap/levelX.dat to root folder, remove folder

1. Run the game
```
./IMDAVE
```

## Commit by Commit

### 1. pull graphics assets from Dangerous Dave executable
 
*Optional knowledge (not used)*: Dissassemble Dave.exe with IDA Pro
[IDA Pro](https://hex-rays.com/ida-pro). 
Open IDA Pro. Move Dave.exe to window. Keep default settings -> OK
"Possible packed file, continue?" (Dave.exe is compressed with LZW) -> OK
At top - No instructions areas was found. except entry point. to depack it.

#### 1.1. Decompress Dave.exe
To decompress Dave.exe use [Unlzexe Windows 32-bit newest](https://keenwiki.shikadi.net/wiki/UNLZEXE). Download and put into root folder. Open a terminal, locate root folder and run `unlzexe.exe DAVE.EXE`. Should print `unpacked file 'dave.exe' is generated`.

#### 1.2. Minw64 in C:/
For mingw64 use [MinGW-W64 Online Installer](https://github.com/niXman/mingw-builds-binaries). Download, open and click "run anyway". Installation process: 
```
version: 14.2.0
architecture: 64 bit
thread model: win32
build revision: rev0
C runtime: msvcrt
directory: C:/
``` 
Click "process". Wait for extracting. "C:/mingw64" should be created.
"

##### 1.2.1. Add system PATH
Click "windows key" -> "edit the system environment variables". Environment varibles.
In system variables click "Path" -> "Edit" -> "Add" -> "C:/mingw64/bin"

##### 1.2.2. Verify gcc in the terminal
Open terminal. Type `gcc`. should print out a fatal error:
```
C:\> gcc
gcc: fatal error: no input files
compilation terminated.
```

#### 1.3. SDL with MinGW-W64
Create folder `C:/MinGW_SDL.`. unzip [SDL2-devel-2.30.8-mingw.zip](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.8) to C:/MinGW_SDL.

#### 1.4. Create Makefile and "c/tiles.c" in root folder

1. ...
2. ...
3. ...
4. ...
```