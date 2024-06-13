#define NOB_IMPLEMENTATION
#include "src/ext/nob.h"

static const char *modules[] = {
    "rcore",
    "raudio",
    "rglfw",
    "rmodels",
    "rshapes",
    "rtext",
    "rtextures",
    "utils",
};

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    Nob_Cmd cmd = {0};
    Nob_File_Paths obj_files = {0};

    /* build raylib */
    if (!nob_mkdir_if_not_exists("build"))                  return false;
    if (!nob_mkdir_if_not_exists("build/raylib"))           return false;
    Nob_Procs procs = {0};
    for (size_t i = 0; i < NOB_ARRAY_LEN(modules); i++) {
        const char *in  = nob_temp_sprintf("./src/ext/raylib-5.0/src/%s.c", modules[i]);
        const char *out = nob_temp_sprintf("./build/raylib/%s.o", modules[i]);
        nob_da_append(&obj_files, out);
        if (nob_needs_rebuild(out, &in, 1)) {
            cmd.count = 0;
            nob_cmd_append(&cmd, "cc");
            nob_cmd_append(&cmd, "-g");
            nob_cmd_append(&cmd, "-DPLATFORM_DESKTOP", "-fPIC");
            nob_cmd_append(&cmd, "-I./src/ext/raylib-5.0/src/external/glfw/include");
            nob_cmd_append(&cmd, "-c", in);
            nob_cmd_append(&cmd, "-o", out);

            Nob_Proc proc = nob_cmd_run_async(cmd);
            nob_da_append(&procs, proc);
        }
    }
    if (!nob_procs_wait(procs)) return false;

    /* archive raylib as static library */
    cmd.count = 0;
    const char *libraylib_path = "./build/raylib/libraylib.a";
    if (nob_needs_rebuild(libraylib_path, obj_files.items, obj_files.count)) {
        nob_cmd_append(&cmd, "ar", "-crs", libraylib_path);
        for (size_t i = 0; i < NOB_ARRAY_LEN(modules); i++) {
            const char *in = nob_temp_sprintf("./build/raylib/%s.o", modules[i]);
            nob_cmd_append(&cmd, in);
        }
        if (!nob_cmd_run_sync(cmd)) return false;
    }

    /* build executable for raylib example */
    cmd.count = 0;
    nob_cmd_append(&cmd, "cc");
    nob_cmd_append(&cmd, "-Werror", "-Wall", "-Wextra", "-g");
    nob_cmd_append(&cmd, "-I./src/ext/raylib-5.0/src");
    nob_cmd_append(&cmd, "-o", "main", "src/main.c");
    nob_cmd_append(&cmd, "-L./build/raylib", "-l:libraylib.a");
    nob_cmd_append(&cmd, "-lm", "-ldl", "-lpthread");
    if (!nob_cmd_run_sync(cmd)) return 1;

    /* run after building*/
    cmd.count = 0;
    nob_cmd_append(&cmd, "./main");
    if (!nob_cmd_run_sync(cmd)) return 1;

    return 0;
}
