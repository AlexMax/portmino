Portmino
========
Portmino is an engine for falling-block puzzle games, based on and heavily
inspired by nullpomino.  It consists of a portable core that can be played
either as a libretro core using programs such as RetroArch or as a standalone
game.

Building
--------
You need a development environment capable of compiling C code, a reasonably
recent version of CMake and (for the SDL version) a recent version of SDL2's
development libraries.

Assuming you have all of those, compiling it is much like compiling any other
CMake project you've come across.  I recommend doing an out-of-tree build
using a directory in the root of the git checkout called `build`, as that's
how I have my `.gitignore` set up.

Running
-------
There are two ways to run the game.  There is a standalone SDL version of
the game, and a libretro core designed to be run through RetroArch.

Running the SDL version is simple, simply find the executable in the build
directory and run it.  Running the libretro core is simply a matter of copying
the dynamic library into the `cores` directory of your RetroArch installation.

Currently, the program assumes that it can find the `basemino.pk3` resource
pack, and bad things will happen if that is not the case.  In Linux, a variety
of locations are checked, but perhaps the most convenient place to put the
pack is in `~/.local/share/portmino/`.  On Windows, simply put the pack in
the same directory as the executable, and it will be found.

This will be made more convenient in the future.

License
-------
The source code for this engine is licensed under the GPLv3.  Any libraries
or resources not originating with this project are under their original
respective licenses.
