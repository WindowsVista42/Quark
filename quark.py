#!/usr/sys/env python3

import sys
import os
import glob
import shutil

arg_stack = []

# UTILS

# specify that we want custom text output
def custom_text():
    return

# run args in arg stack
def pop_arg_and_run(opts):
    if len(arg_stack) == 0:
        opts["help"][0]()
        exit(0)

    arg = arg_stack.pop()
    if (arg not in opts) or (opts[arg][0] == custom_text):
        print("Argument \"" + arg + "\" not valid!")
        print()
        opts["help"][0]()
        exit(-1)
    opts[arg][0]()

# print a help menu with the right formatting
def print_help(help_msg, opts):
    print("Plugin Help:")
    for key in opts.keys():
        comment = opts[key][1];
        if opts[key][0] == custom_text:
            for line in comment.splitlines():
                print("  {}".format(line))
        else:
            lines = comment.splitlines()
            print("    {:<8} {}".format(key, lines[0]))
            for line in lines[1:]:
                print("  {}".format(line))

# copy from src to dst
# derives name of file from src, so dst only needs to be an output dir
# ie: copy_file("special_files/special.txt", "my_things/special_things")
# will copy special_files/special.txt to my_things/special_things/special.txt
def copy_file(src, dst):
    dst_fullpath = dst + os.sep + os.path.basename(src)
    shutil.copyfile(src, dst_fullpath)
    return

# BUILD

def build():
    pop_arg_and_run(BUILD_OPTS)

def build_internal(mode):
    build_dir = "build/" + mode

    # COMPILE FILES
    print("- Building " + mode + " build")

    if not os.path.exists(build_dir):
        print("- Initializing " + mode + " build compilation cache")
        os.system("cmake -B build/" + mode +" -GNinja -DCMAKE_BUILD_TYPE=" + mode.capitalize() + " -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .")

    plugin_dirs = glob.glob("plugins/*/")
    for i in range(0, len(plugin_dirs)):
        plugin_dirs[i] = plugin_dirs[i][8:][:-1]

    print("-- Building: quark_loader")
    if os.system("cmake --build " + build_dir + " --target quark_loader") != 0:
        sys.exit("Failed to build!")

    for p in plugin_dirs:
        print("-- Building plugin: ", p)
        if os.system("cmake --build " + build_dir + " --target " + p) != 0:
            sys.exit("- Failed to build!")

    print("- Finished building " + mode + " build")

    # COPY FILES
    print("- Copying " + mode + " build")
    build_lib_dir = build_dir + "/lib"

    shared_lib_paths = glob.glob(build_lib_dir + "/*.dll")
    loader_path = build_dir + "/quark/src/quark_loader/quark_loader.exe"

    copy_file(loader_path, "build/current")
    print("-- Copied \"" + loader_path + "\"")
    for path in shared_lib_paths:
        copy_file(path, "build/current")
        print("-- Copied \"" + path + "\"")

    print("- Finished copying " + mode + " build")

    return

def build_debug():
    build_internal("debug")
    return

def build_release():
    build_internal("release")
    return

def build_help():
    print_help("Build Help:", BUILD_OPTS)

# RUN

def run():
    pop_arg_and_run(RUN_OPTS)

def run_debug():
    return

def run_release():
    return

def run_help():
    print_help("Run Help:", RUN_OPTS)

# PLUGIN

def plugin():
    pop_arg_and_run(PLUGIN_OPTS)

def plugin_add():
    return

def plugin_create():
    return

def plugin_remove():
    return

def plugin_export():
    return

def plugin_help():
    print_help("Plugin Help:", PLUGIN_OPTS)

# HELP

def help():
    print_help("Help:", GLOBAL_OPTS)

# OPTS

GLOBAL_OPTS = {
    "0":       (custom_text,   "Options:"),
    "build":   (build,         "- Build"),
    "run":     (run,           "- Run"),
    "plugin":  (plugin,        "- Plugin Utilities"),
    "help":    (help,          "- Help"),

    "1":       (custom_text,   "\nAliases:"),
    "b":       (build,         "- Build"),
    "r":       (run,           "- Run"),
    "p":       (plugin,        "- Plugin Utilities"),
    "h":       (help,          "- Help"),
}

BUILD_OPTS = {
    "0":       (custom_text,   "Options:"),
    "debug":   (build_debug,   "- Debug build"),
    "release": (build_release, "- Release build"),
    "help":    (build_help,    "- Build help"),
    
    "1":       (custom_text,   "\nAliases:"),
    "d":       (build_debug,   "- Debug build"),
    "r":       (build_release, "- Release build"),
    "h":       (build_help,    "- Build help"),
}

RUN_OPTS = {
    "0":       (custom_text,   "Options:"),
    "debug":   (run_debug,     "- Run debug build"),
    "release": (run_release,   "- Run release build"),
    "help":    (run_help,      "- Run help"),

    "1":       (custom_text,   "\nAliases:"),
    "d":       (run_debug,     "- Run debug build"),
    "r":       (run_release,   "- Run release build"),
    "h":       (run_help,      "- Run help"),
}

PLUGIN_OPTS = {
    "0":       (custom_text,   "Options:"),
    "add":     (plugin_add,    "- Add a plugin"),
    "remove":  (plugin_remove, "- Remove a plugin"),
    "create":  (plugin_create, "- Create a plugin"),
    "export":  (plugin_export, "- Export a plugin"),
    "help":    (plugin_help,   "- Plugin help"),

    "1":       (custom_text,   "\nAliases:"),
    "a":       (plugin_add,    "- Add a plugin"),
    "r":       (plugin_remove, "- Remove a plugin"),
    "c":       (plugin_create, "- Create a plugin"),
    "e":       (plugin_export, "- Export a plugin"),
    "h":       (plugin_help,   "- Plugin help"),
}

SHADER_OPTS = {
}

# MAIN
if __name__ == "__main__":
    for arg in sys.argv:
        if arg != "":
            arg_stack.append(arg)

    arg_stack.reverse()
    arg_stack.pop()

    pop_arg_and_run(GLOBAL_OPTS)
