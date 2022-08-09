#!/usr/sys/env python3

import sys
import os
import glob
import shutil
import atexit
import re

arg_stack = []

# UTILS

# specify that we want custom text output
def custom_text():
    return

# run args in arg stack
def pop_arg_and_run(opts):
    if len(arg_stack) == 0:
        opts["help"][0]()
        return

    arg = arg_stack.pop()
    if (arg not in opts) or (opts[arg][0] == custom_text):
        print("Argument \"" + arg + "\" not valid!")
        print()
        opts["help"][0]()
        exit(-1)
    opts[arg][0]()

def pop_arg(help_msg) -> str:
    if len(arg_stack) == 0:
        print(help_msg)
        exit(-1)
    
    return arg_stack.pop()

# print a help menu with the right formatting
def print_help(help_msg, opts):
    print(help_msg)
    for key in opts.keys():
        comment = opts[key][1];
        if opts[key][0] == custom_text:
            for line in comment.splitlines():
                print("  {}".format(line))
        else:
            lines = comment.splitlines()
            print("    {:<12} {}".format(key, lines[0]))
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

# copy all files from src to dst
def copy_dir(src, dst):
    shutil.copytree(src, dst, ignore=shutil.ignore_patterns(".git"))
    return

# delete all files in a specified directory
def clean_dir(dir):
    fulldir = dir
    if dir[-1] != '/':
        fulldir += "/*"
    else:
        fulldir += "*"

    paths = glob.glob(fulldir)
    for p in paths:
        if os.path.isdir(p):
            shutil.rmtree(p)
        else:
            os.remove(p)
    return

# delete the specified directory
def del_dir(dir):
    os.remove(dir)
    return

# BUILD

def build():
    pop_arg_and_run(BUILD_OPTS)

def build_setup(mode):
    print("- Initializing " + mode + " build compilation cache")
    os.system("cmake -B build/" + mode +" -GNinja -DCMAKE_BUILD_TYPE=" + mode.capitalize() + " -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .")
    CONFIG_VALUES["refresh_build_" + mode] = 0

def build_internal(mode):
    # CLEAN FILES
    print("- Cleaning previous build")
    clean_dir("build/current")

    # COMPILE FILES
    build_dir = "build/" + mode

    print("- Building " + mode + " build")
    if not os.path.exists(build_dir) or CONFIG_VALUES["refresh_build_" + mode] == 1:
        build_setup(mode)

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

    # CLEAN OLD FILES
    print("- Cleaning old shared libraries")

    targets_f = open(build_dir + "/CMakeFiles/TargetDirectories.txt", "r")
    targets_s = targets_f.read()
    targets_f.close()

    valid_libs = {"glfw3", "quark_engine", "quark_platform"}

    # (build\/debug\/quark\/src\/)([^\/]*)
    # (build\/debug\/plugins\/)([^\/]*)
    # (build\/debug\/lib\/)([^\/]*)
    # our valid directory prefixes
    valid_prefix_list = ["build\\/" + mode + "\\/quark\\/src\\/", "build\\/" + mode + "\\/plugins\\/", "build\\/" + mode +"\\/lib\\/"]
    print(valid_prefix_list)
    for v in valid_prefix_list:
        found = re.findall(v + "([^\\/]*)", targets_s)
        for f in found:
            valid_libs.add(f)

    if "CMakeFiles" in valid_libs:
        valid_libs.remove("CMakeFiles")

    # COPY FILES
    print("- Copying " + mode + " build")

    build_lib_dir = build_dir + "/lib"

    shared_lib_paths = glob.glob(build_lib_dir + "/*.dll")
    loader_path = build_dir + "/quark/src/quark_loader/quark_loader.exe"

    copy_dir("quark", "build/current/quark")
    print("-- Copied \"quark\" to \"build/current/quark\"")
    for dir in plugin_dirs:
        copy_dir("plugins/" + dir, "build/current/" + dir)
        print("-- Copied \"" + "plugins/" + dir + "\" to \"build/current/" + dir + "\"")

    copy_file(loader_path, "build/current")
    print("-- Copied \"" + loader_path + "\"")
    for path in shared_lib_paths:
        name = os.path.basename(path)
        name = name[0:name.find('.')]
        if name.find("_Debug") != -1:
            name = name[0:name.find("_Debug")]
        if name in valid_libs:
            copy_file(path, "build/current")
            print("-- Copied \"" + path + "\"")

    print("- Finished copying " + mode + " build")

    return

def build_initialize():
    build_setup("debug")
    build_setup("release")
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

def run_program():
    os.chdir("build" + os.sep + "current")
    os.system("." + os.sep + "quark_loader")

def attach_run_on_exit():
    atexit.register(run_program)
    return

def run():
    pop_arg_and_run(RUN_OPTS)

def run_debug():
    build_debug()
    attach_run_on_exit()
    return

def run_release():
    build_release()
    attach_run_on_exit()
    return

def run_help():
    print_help("Run Help:", RUN_OPTS)

# PLUGIN

def add_git_submodule(url, dst):
    return

def rem_git_submodule():
    return

def plugin():
    pop_arg_and_run(PLUGIN_OPTS)

def plugin_add():
    user_repo = "WindowsVista42/simple_plugin"
    (_, repo) = user_repo.split("/")
    url = "https://github.com/" + user_repo
    cmd = "git clone " + url + " plugins" + os.sep + repo
    print(cmd)
    os.system(cmd)
    #print(url)
    CONFIG_VALUES["refresh_build_debug"] = 1
    CONFIG_VALUES["refresh_build_release"] = 1
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
    "initialize": (build_initialize, "- Initialize build files"),
    "help":    (build_help,    "- Build help"),
    
    "1":       (custom_text,   "\nAliases:"),
    "d":       (build_debug,   "- Debug build"),
    "r":       (build_release, "- Release build"),
    "i":       (build_initialize, "- Initialize build files"),
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

CONFIG_VALUES = {
    "refresh_build_debug": 1,
    "refresh_build_release": 1,
}

# MAIN
if __name__ == "__main__":
    if not os.path.exists(".quark"):
        os.mkdir(".quark")
        cfg_f = open(".quark/config.txt", "w")
        for key in CONFIG_VALUES.keys():
            cfg_f.write(key + "=" + str(CONFIG_VALUES[key]) + "\n")
        cfg_f.close()

    cfg_f = open(".quark/config.txt", "r")
    cfg_s = cfg_f.read()

    for line in cfg_s.splitlines():
        key_value = line.split("=")
        CONFIG_VALUES[key_value[0]] = int(key_value[1])

    cfg_f.close()

    for arg in sys.argv:
        if arg != "":
            arg_stack.append(arg)

    arg_stack.reverse()
    arg_stack.pop()

    pop_arg_and_run(GLOBAL_OPTS)

    cfg_f = open(".quark/config.txt", "w")
    for key in CONFIG_VALUES.keys():
        cfg_f.write(key + "=" + str(CONFIG_VALUES[key]) + "\n")
    cfg_f.close()
