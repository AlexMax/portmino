Portmino
========
Portmino is an engine for falling-block puzzle games, based on and heavily inspired by nullpomino.  It consists of a portable core that can be played either as a libretro core using programs such as RetroArch or as a standalone game.

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
Right now, the location of all resources is assumed to be in a directory
called `res` that is one directory up from your current working directory.
Assuming you did an out-of-tree build like I suggested above, all you have
to do is change to the `build` directory and run the program like so:

    ./src/sdl/portmino

...and the resources should be found properly.  Otherwise, the program will
almost certainly crash.

The libretro core should work as well, just copy the library into the `cores`
directory of your RetroArch installation.  However, the same warning about
the location of resources applies to the core as well, so unless you start
RetroArch from a similar directory, the core will crash.

This will be fixed at some point in the future.

License
-------
The source code for this engine is licensed under the GPLv3.  Any libraries or resources not originating with this project are under their original respective licenses.
