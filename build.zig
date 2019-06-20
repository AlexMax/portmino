const std = @import("std");
const Builder = std.build.Builder;
const Version = std.build.Version;

// Build entry point
pub fn build(b: *Builder) void {
    const mode = b.standardReleaseOptions();
    const cflags = [][]const u8{
        "-Wall",
        "-Wextra",
        "-Werror=implicit-function-declaration",
        "-DMINIZ_NO_STDIO",
    };

    // Shared common core
    var portmino_core = b.addStaticLibrary("portmino_core", null);
    portmino_core.setBuildMode(mode);
    portmino_core.addIncludeDir("src/compat");
    portmino_core.addIncludeDir("src/lib");
    portmino_core.linkSystemLibrary("c");

    portmino_core.addCSourceFile("src/audio.c", cflags);
    portmino_core.addCSourceFile("src/audioscript.c", cflags);
    portmino_core.addCSourceFile("src/board.c", cflags);
    portmino_core.addCSourceFile("src/boardscript.c", cflags);
    portmino_core.addCSourceFile("src/define.c", cflags);
    portmino_core.addCSourceFile("src/entity.c", cflags);
    portmino_core.addCSourceFile("src/environment.c", cflags);
    portmino_core.addCSourceFile("src/error.c", cflags);
    portmino_core.addCSourceFile("src/frontend.c", cflags);
    portmino_core.addCSourceFile("src/game.c", cflags);
    portmino_core.addCSourceFile("src/gametype.c", cflags);
    portmino_core.addCSourceFile("src/globalscript.c", cflags);
    portmino_core.addCSourceFile("src/ingame.c", cflags);
    portmino_core.addCSourceFile("src/input.c", cflags);
    portmino_core.addCSourceFile("src/inputscript.c", cflags);
    portmino_core.addCSourceFile("src/mainmenu.c", cflags);
    portmino_core.addCSourceFile("src/menu.c", cflags);
    portmino_core.addCSourceFile("src/pausemenu.c", cflags);
    portmino_core.addCSourceFile("src/picture.c", cflags);
    portmino_core.addCSourceFile("src/piece.c", cflags);
    portmino_core.addCSourceFile("src/piecescript.c", cflags);
    portmino_core.addCSourceFile("src/platform.c", cflags);
    portmino_core.addCSourceFile("src/playmenu.c", cflags);
    portmino_core.addCSourceFile("src/proto.c", cflags);
    portmino_core.addCSourceFile("src/protoscript.c", cflags);
    portmino_core.addCSourceFile("src/random.c", cflags);
    portmino_core.addCSourceFile("src/randomscript.c", cflags);
    portmino_core.addCSourceFile("src/render.c", cflags);
    portmino_core.addCSourceFile("src/renderscript.c", cflags);
    portmino_core.addCSourceFile("src/ruleset.c", cflags);
    portmino_core.addCSourceFile("src/rulesetmenu.c", cflags);
    portmino_core.addCSourceFile("src/screen.c", cflags);
    portmino_core.addCSourceFile("src/script.c", cflags);
    portmino_core.addCSourceFile("src/serialize.c", cflags);
    portmino_core.addCSourceFile("src/softblock.c", cflags);
    portmino_core.addCSourceFile("src/softfont.c", cflags);
    portmino_core.addCSourceFile("src/softrender.c", cflags);
    portmino_core.addCSourceFile("src/sound.c", cflags);
    portmino_core.addCSourceFile("src/vfs.c", cflags);
    portmino_core.addCSourceFile("src/lib/dr_wav.c", cflags);
    portmino_core.addCSourceFile("src/lib/miniz.c", cflags);
    portmino_core.addCSourceFile("src/lib/mpack.c", cflags);
    portmino_core.addCSourceFile("src/lib/stb_image.c", cflags);

    portmino_core.addCSourceFile("src/platform_unix.c", cflags);

    portmino_core.addIncludeDir("dep/lua-5.3.5/src");
    portmino_core.linkSystemLibrary("dep/lua-5.3.5/src/liblua.a");

    portmino_core.addIncludeDir("dep/physfs-5ebf7cddf502/src");
    portmino_core.linkSystemLibrary("dep/physfs-5ebf7cddf502/build/libphysfs.a");

    // libretro shared library
    var portmino_libretro = b.addSharedLibrary("portmino_libretro", null, b.version(0, 0, 1));
    portmino_libretro.setBuildMode(mode);
    portmino_libretro.linkLibrary(portmino_core);

    portmino_libretro.addCSourceFile("src/libretro/libretro.c", cflags);

    // SDL standalone executable
    var portmino_exe = b.addExecutable("portmino", null);
    portmino_exe.setBuildMode(mode);
    portmino_exe.addIncludeDir("src");
    portmino_exe.addIncludeDir("src/compat");
    portmino_exe.addIncludeDir("src/lib");
    portmino_exe.linkLibrary(portmino_core);

    portmino_exe.addIncludeDir("/usr/include/SDL2");
    portmino_exe.linkSystemLibrary("SDL2");

    portmino_exe.addIncludeDir("dep/lua-5.3.5/src");
    portmino_exe.linkSystemLibrary("dep/lua-5.3.5/src/liblua.a");

    portmino_exe.addIncludeDir("dep/physfs-5ebf7cddf502/src");
    portmino_exe.linkSystemLibrary("dep/physfs-5ebf7cddf502/build/libphysfs.a");

    portmino_exe.addCSourceFile("src/sdl/main.c", cflags);

    //b.default_step.dependOn(&portmino_libretro.step);
    b.default_step.dependOn(&portmino_exe.step);
}
