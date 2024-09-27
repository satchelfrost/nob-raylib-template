#define NOB_IMPLEMENTATION
#include "nob.h"

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

bool build_raylib_linux()
{
    bool result = true;
    Nob_Cmd cmd = {0};
    Nob_File_Paths obj_files = {0};

    /* build raylib */
    if (!nob_mkdir_if_not_exists("build"))        nob_return_defer(false);
    if (!nob_mkdir_if_not_exists("build/raylib")) nob_return_defer(false);
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
    if (!nob_procs_wait(procs)) nob_return_defer(false);

    /* archive raylib as static library */
    cmd.count = 0;
    const char *libraylib_path = "./build/raylib/libraylib.a";
    if (nob_needs_rebuild(libraylib_path, obj_files.items, obj_files.count)) {
        nob_cmd_append(&cmd, "ar", "-crs", libraylib_path);
        for (size_t i = 0; i < NOB_ARRAY_LEN(modules); i++) {
            const char *in = nob_temp_sprintf("./build/raylib/%s.o", modules[i]);
            nob_cmd_append(&cmd, in);
        }
        if (!nob_cmd_run_sync(cmd)) nob_return_defer(false);
    }

defer:
    nob_cmd_free(cmd);
    nob_da_free(obj_files);
    nob_da_free(procs);
    return result;
}

bool build_raylib_web()
{
    bool result = true;

    Nob_Cmd cmd = {0};
    Nob_File_Paths obj_files = {0};

    /* build raylib */
    if (!nob_mkdir_if_not_exists("build"))            nob_return_defer(false);
    if (!nob_mkdir_if_not_exists("build/raylib-web")) nob_return_defer(false);
    Nob_Procs procs = {0};

    for (size_t i = 0; i < NOB_ARRAY_LEN(modules); i++) {
        if (strcmp("rglfw", modules[i]) == 0) continue;

        const char *in  = nob_temp_sprintf("./src/ext/raylib-5.0/src/%s.c", modules[i]);
        const char *out = nob_temp_sprintf("./build/raylib-web/%s.o", modules[i]);
        nob_da_append(&obj_files, out);
        if (nob_needs_rebuild(out, &in, 1)) {
            cmd.count = 0;
            nob_cmd_append(&cmd, "emcc");
            nob_cmd_append(&cmd, "-Os", "-w");
            nob_cmd_append(&cmd, "-DPLATFORM_WEB", "-DGRAPHICS_API_OPENGL_ES2");
            nob_cmd_append(&cmd, "-c", in);
            nob_cmd_append(&cmd, "-o", out);

            Nob_Proc proc = nob_cmd_run_async(cmd);
            nob_da_append(&procs, proc);
        }
    }
    if (!nob_procs_wait(procs)) nob_return_defer(false);

    /* archive raylib as static library */
    cmd.count = 0;
    const char *libraylib_path = "./build/raylib-web/libraylib.a";
    if (nob_needs_rebuild(libraylib_path, obj_files.items, obj_files.count)) {
        nob_cmd_append(&cmd, "emar", "crs", libraylib_path);
        for (size_t i = 0; i < NOB_ARRAY_LEN(modules); i++) {
            if (strcmp("rglfw", modules[i]) == 0) continue;
            const char *in = nob_temp_sprintf("./build/raylib-web/%s.o", modules[i]);
            nob_cmd_append(&cmd, in);
        }
        if (!nob_cmd_run_sync(cmd)) nob_return_defer(false);
    }

defer:
    nob_cmd_free(cmd);
    nob_da_free(obj_files);
    nob_da_free(procs);
    return result;
}

bool build_exec_linux()
{
    bool result = true;

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "cc");
    nob_cmd_append(&cmd, "-Werror", "-Wall", "-Wextra", "-g");
    nob_cmd_append(&cmd, "-I./src/ext/raylib-5.0/src");
    nob_cmd_append(&cmd, "-o", "main", "src/main.c");
    nob_cmd_append(&cmd, "-L./build/raylib", "-l:libraylib.a");
    nob_cmd_append(&cmd, "-lm", "-ldl", "-lpthread");
    if (!nob_cmd_run_sync(cmd)) nob_return_defer(false);

defer:
    nob_cmd_free(cmd);
    return result;
}

bool build_exec_web()
{
    bool result = true;
    Nob_Cmd cmd = {0};

    nob_cmd_append(&cmd, "emcc", "-o", "index.js");
    nob_cmd_append(&cmd, "./src/main.c");
    nob_cmd_append(&cmd, "-Os", "-Wall");
    nob_cmd_append(&cmd, "-I./src/ext/raylib-5.0/src");
    nob_cmd_append(&cmd, "-L./build/raylib-web", "-l:libraylib.a");
    nob_cmd_append(&cmd, "-s", "USE_GLFW=3");
    nob_cmd_append(&cmd, "-s", "ASYNCIFY");
    if(!nob_cmd_run_sync(cmd)) nob_return_defer(false);

defer:
    nob_cmd_free(cmd);
    return result;
}

void log_usage()
{
    nob_log(NOB_INFO, "usage: ./nob <flags>");
    nob_log(NOB_INFO, "    -t <target> ([w]eb, [l]inux)");
}

typedef enum {
    TARGET_LINUX,
    TARGET_WEB,
} Target;

Target target = {0};

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    char *program = nob_shift_args(&argc, &argv);
    while (argc > 0) {
        char *arg = nob_shift_args(&argc, &argv);
        if (arg[0] - '-' == 0 && argc > 0) {
            if (strcmp(arg, "-t") == 0) {
                char *flag = nob_shift_args(&argc, &argv);
                switch (*flag) {
                case 'w': target = TARGET_WEB;   break;
                case 'l': target = TARGET_LINUX; break;
                default:
                    nob_log(NOB_INFO, "unrecognized flag %c", *flag);
                    log_usage();
                    return 1;
                }
            } else {
                log_usage();
            }
        }
    }

    Nob_Cmd cmd = {0};

    switch (target) {
    case TARGET_LINUX:
        if (!build_raylib_linux()) return 1;
        if (!build_exec_linux())   return 1;

        /* run after building*/
        nob_cmd_append(&cmd, "./main");
        if (!nob_cmd_run_sync(cmd)) return 1;
        break;
    case TARGET_WEB:
    default:
        if (!build_raylib_web()) return 1;
        if (!build_exec_web())   return 1;
        else return 0;
    } 

    nob_cmd_free(cmd);
    return 0;
}
