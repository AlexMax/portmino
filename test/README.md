cmocka is used for unit testing.  Compile cmocka using the following
invocation:

    cmake -DCMAKE_INSTALL_PREFIX=~/.local -DCMAKE_BUILD_TYPE=Debug -DWITH_STATIC_LIB=true ..

Then, make sure `BUILD_TESTING` is enabled when building portmino.  Currently,
unit tests have only been tested to work on Linux.
