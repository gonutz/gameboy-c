# GameBoy Emulator

This is a fork of https://github.com/deltabeard/gameboy-c.

This fork provides a way to compile on Windows without external dependencies.
It bundles [Zig](https://ziglang.org) as the C compiler and includes
[SDL](https://github.com/libsdl-org/SDL) (as a Git subtree).

To build on 64 bit Windows, call:

    build.bat

which results in `gameboy.exe` in the top-level folder.

To build and run in one step, you can use `run.bat`:

    run.bat -f path/to/tetris.gb

which will build `gameboy.exe` and (on success) run it with the given
arguments.

Note that the first build takes some time because `build.bat` extracts Zig and
rebuilds the whole project and its dependencies. After the first build, things
will be much faster since Zig is already extracted and it has a caching
mechanism that only builds files that have changed.

Here is the original readme:

## Features:
- Very small (about 36KiB compiled).
- SDL and X11 only dependencies on Linux.
- Preliminary sound support (disabled by default).

Default input can be changed in platform.c.

## Default Keys:
#### Player 1:

Arrow keys

z

x

Right_Shift

Return


#### Player 2:

WASD keys

Space

Backspace

Left_Shift

Escape


#### Special:

0 = Set framskip to zero (Default)

1 = Set frameskip to one (~3x speedup)

2 = Set frameskip to two (~6x speedup)

3 = Set frameskip to two (~9x speedup)

9 = Set frameskip to two (~25x speedup)

r = Use different colour pallete (Red)

g = Use different colour pallete (Green)

b = Use different colour pallete (Blue)

o = Use different colour pallete (Yellow)

y = Use different colour pallete (Grey - Default)

ctrl+q = Quit
