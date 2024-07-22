# Tetris Bot

Call `run.bat` to build the emulator and run `tetris.gb` which must be located
in the directory where you execute `run.bat`.

If you do not happen to have a ROM of the Game Boy Tetris version, search the
internet. I cannot include the ROM in this repository because I do not own the
rights to it.

# GameBoy Emulator

This is a fork of https://github.com/deltabeard/gameboy-c.

This fork provides a way to compile on Windows without external dependencies.
It bundles [Zig](https://ziglang.org) as the C compiler and includes
[SDL](https://github.com/libsdl-org/SDL) (as a Git subtree).

To build on 64 bit Windows, call:

    build.bat

which results in `gameboy.exe` in the top-level folder.

Note that the first build takes some time because `build.bat` extracts Zig and
rebuilds the whole project and its dependencies. After the first build, things
will be much faster since Zig is already extracted and it has a caching
mechanism that only builds files that have changed.

## Keys mappings:

These can be changed in `platform.c`.

Arrow keys for Game Boy's D-Pad.

F and D for Game Boy's A and B buttons.

SPACE and ENTER for Game Boy's Select and Start buttons.

#### Special:

0 = Set framskip to zero (Default)

1 = Set frameskip to one (~3x speedup)

2 = Set frameskip to two (~6x speedup)

3 = Set frameskip to two (~9x speedup)

9 = Set frameskip to two (~25x speedup)

R = Use different colour pallete (Red)

G = Use different colour pallete (Green)

B = Use different colour pallete (Blue)

O = Use different colour pallete (Yellow)

Y = Use different colour pallete (Grey - Default)

Ctrl+q = Quit
