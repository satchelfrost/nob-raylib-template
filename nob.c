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
    if (!nob_mkdir_if_not_exists("build/raylib-linux")) nob_return_defer(false);
    Nob_Procs procs = {0};
    for (size_t i = 0; i < NOB_ARRAY_LEN(modules); i++) {
        const char *in  = nob_temp_sprintf("./src/ext/raylib-5.0/src/%s.c", modules[i]);
        const char *out = nob_temp_sprintf("./build/raylib-linux/%s.o", modules[i]);
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
    const char *libraylib_path = "./build/raylib-linux/libraylib.a";
    if (nob_needs_rebuild(libraylib_path, obj_files.items, obj_files.count)) {
        cmd.count = 0;
        nob_cmd_append(&cmd, "ar", "-crs", libraylib_path);
        for (size_t i = 0; i < NOB_ARRAY_LEN(modules); i++) {
            const char *in = nob_temp_sprintf("./build/raylib-linux/%s.o", modules[i]);
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
    const char *libraylib_path = "./build/raylib-web/libraylib.a";
    if (nob_needs_rebuild(libraylib_path, obj_files.items, obj_files.count)) {
        cmd.count = 0;
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


bool build_raylib_win()
{
    bool result = true;
    Nob_Cmd cmd = {0};
    Nob_File_Paths obj_files = {0};

    /* build raylib */
    if (!nob_mkdir_if_not_exists("build"))        nob_return_defer(false);
    if (!nob_mkdir_if_not_exists("build/raylib-windows")) nob_return_defer(false);
    Nob_Procs procs = {0};
    for (size_t i = 0; i < NOB_ARRAY_LEN(modules); i++) {
        const char *in  = nob_temp_sprintf("./src/ext/raylib-5.0/src/%s.c", modules[i]);
        const char *out = nob_temp_sprintf("./build/raylib-windows/%s.o", modules[i]);
        nob_da_append(&obj_files, out);
        if (nob_needs_rebuild(out, &in, 1)) {
            cmd.count = 0;
            nob_cmd_append(&cmd, "x86_64-w64-mingw32-gcc");
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
    const char *libraylib_path = "./libraylib.dll";
    if (nob_needs_rebuild(libraylib_path, obj_files.items, obj_files.count)) {
        nob_cmd_append(&cmd, "x86_64-w64-mingw32-gcc", "-static-libgcc", "-shared", "-o", libraylib_path);
        for (size_t i = 0; i < NOB_ARRAY_LEN(modules); i++) {
            const char *in = nob_temp_sprintf("./build/raylib-windows/%s.o", modules[i]);
            nob_cmd_append(&cmd, in);
        }
        nob_cmd_append(&cmd, "-lwinmm", "-lgdi32");
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
    nob_cmd_append(&cmd, "-L./build/raylib-linux", "-l:libraylib.a");
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

bool build_exec_win()
{
    bool result = true;

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "x86_64-w64-mingw32-gcc");
    nob_cmd_append(&cmd, "-I./src/ext/raylib-5.0/src");
    nob_cmd_append(&cmd, "-o", "main", "src/main.c");
    nob_cmd_append(&cmd, "-L./build/raylib-windows", "-l:libraylib.dll");
    nob_cmd_append(&cmd, "-lwinmm", "-lgdi32");
    if (!nob_cmd_run_sync(cmd)) nob_return_defer(false);

defer:
    nob_cmd_free(cmd);
    return result;
}

void log_usage()
{
    nob_log(NOB_ERROR, "usage: ./nob <flags>");
    nob_log(NOB_ERROR, "    -t <target> ([w]eb, [l]inux)");
}

typedef enum {
    TARGET_LINUX,
    TARGET_WEB,
    TARGET_WINDOWS,
    TARGET_MAC,
} Target;
Target target = {0};

const char *target_names[] = {
    [TARGET_LINUX] = "linux",
    [TARGET_WINDOWS] = "windows",
    [TARGET_MAC] = "mac (not supported)",
    [TARGET_WEB] = "web",
};

typedef enum {
    HOST_LINUX,
    HOST_WINDOWS, // not supported
    HOST_MAC,     // not supported
} Host;

#ifdef _WIN32
    Host host = HOST_WINDOWS;
#else
#   if defined (__APPLE__) || defined (__MACH__)
        Host host = HOST_MAC;
#   else
        Host host = HOST_LINUX;
#   endif
#endif

const char *host_names[] = {
    [HOST_LINUX] = "host linux",
    [HOST_WINDOWS] = "host windows",
    [HOST_MAC] = "host mac",
};

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (host != HOST_LINUX) {
        nob_log(NOB_ERROR, "%s not supported, but the target might be", host_names[host]);
        return 1;
    }

    /* process cmd args */
    char *program = nob_shift_args(&argc, &argv);
    bool target_found = false;
    while (argc > 0) {
        char *arg = nob_shift_args(&argc, &argv);
        if (arg[0] - '-' == 0 && argc > 0) {
            if (strcmp(arg, "-t") == 0) {
                char *target_arg = nob_shift_args(&argc, &argv);
                for (size_t i = 0; i < NOB_ARRAY_LEN(target_names); i++) {
                    if (strcmp(target_arg, target_names[i]) == 0) {
                        target = i;
                        target_found = true;
                    }
                }
                if (!target_found) {
                    nob_log(NOB_ERROR, "target %s does not exist, or is not supported", target_arg);
                    nob_log(NOB_ERROR, "listing available targets:");
                    for (size_t i = 0; i < NOB_ARRAY_LEN(target_names); i++) {
                        nob_log(NOB_ERROR, "    %s", target_names[i]);
                    }
                    log_usage();
                    return 1;
                }
            } else {
                log_usage();
                return 1;
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
        if (!build_raylib_web()) return 1;
        if (!build_exec_web())   return 1;
        break;
    case TARGET_WINDOWS:
        if (!build_raylib_win()) return 1;
        if (!build_exec_win())   return 1;

        /* run after building*/
        if (host != HOST_LINUX) {
            nob_log(NOB_ERROR, "target windows expects host linux for now");
            return 1;
        } else {
            nob_log(NOB_INFO, "all good son");
        }
        nob_cmd_append(&cmd, "wine", "./main.exe");
        if (!nob_cmd_run_sync(cmd)) return 1;
        break;
    default:
        nob_log(NOB_ERROR, "target %s not supported", target_names[target]);
        log_usage();
        return 1;
    } 

    nob_cmd_free(cmd);
    return 0;
}
