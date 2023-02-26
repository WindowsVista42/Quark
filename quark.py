#!/usr/sys/env python3

import sys
import os
import glob
import shutil
import atexit
import re
import time

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

def pop_arg(help_msg: str) -> str:
    if len(arg_stack) == 0:
        print(help_msg)
        exit(-1)
    
    return arg_stack.pop()

# print a help menu with the right formatting
def print_help(help_msg: str, opts):
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

# Error handler for del_dir
# https://stackoverflow.com/questions/2656322/shutil-rmtree-fails-on-windows-with-access-is-denied
def onerror(func, path, exc_info):
    """
    Error handler for ``shutil.rmtree``.

    If the error is due to an access error (read only file)
    it attempts to add write permission and then retries.

    If the error is for another reason it re-raises the error.
    
    Usage : ``shutil.rmtree(path, onerror=onerror)``
    """
    import stat
    # Is the error an access error?
    if not os.access(path, os.W_OK):
        os.chmod(path, stat.S_IWUSR)
        func(path)
    else:
        raise

# delete the specified directory
def del_dir(dir):
    shutil.rmtree(dir, onerror=onerror)
    return

def try_clone_repo(url, dst_path):
    #print("git clone " + url + " " + dst_path)
    return os.system("git clone " + url + " " + dst_path) == 0

# BUILD

def build():
    pop_arg_and_run(BUILD_OPTS)

def build_setup(mode):
    print("- Initializing " + mode + " build compilation cache")
    os.system("cmake -B build/" + mode +" -GNinja -DCMAKE_BUILD_TYPE=" + mode.capitalize() + " -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .")
    CONFIG_VALUES["refresh_build_" + mode] = 0

def write_tracking(cache_path, curr_cpps):
    print("wrote")
    tracked_cache_f = open(cache_path, "w")

    lines = []
    for key in curr_cpps.keys():
        lines.append(key + "," + curr_cpps[key])
        print(key + "," + curr_cpps[key])

    tracked_cache_f.writelines(lines)
    tracked_cache_f.close()
    return True


def build_internal(mode):
    # CLEAN FILES
    print("- Cleaning previous build")
    clean_dir("build/current")

    start_t = time.time()

    should_reinit = False
    changed_set = set([])
    rebuild_set = set([])

    # COMPILE FILES
    plugin_dirs = glob.glob("plugins/*/")
    for i in range(0, len(plugin_dirs)):
        plugin_dirs[i] = plugin_dirs[i][len("plugins/"):][:-1]

    timestamps_curr_cpps = {}
    timestamps_prev_cpps = {}

    # build and check timestamp cache
    for p in plugin_dirs:
        print("-- Building plugin: ", p)

        glob_cpps = glob.glob("plugins/" + p + "/" + p +"/**/*.cpp", recursive=True)

        for cpp in glob_cpps:
            timestamps_curr_cpps[cpp] = str(os.path.getmtime(cpp)) + "\n"

        cache_path = ".quark/tracked/" + p + ".txt"
        if os.path.exists(cache_path):
            tracked_cache_f = open(cache_path, "r")

            lines = tracked_cache_f.readlines()
            for line in lines:
                (file, timestamp) = line.split(",")
                timestamps_prev_cpps[file] = timestamp

            tracked_cache_f.close()
        else:
            should_reinit = write_tracking(cache_path, timestamps_curr_cpps)

        if set(timestamps_curr_cpps.keys()) != set(timestamps_prev_cpps.keys()):
            should_reinit = write_tracking(cache_path, timestamps_curr_cpps)

        if set(timestamps_curr_cpps.values()) != set(timestamps_prev_cpps.values()):
            write_tracking(cache_path, timestamps_curr_cpps)
            changed_set.add(p)

        print("curr: ", timestamps_curr_cpps)
        print("prev: ", timestamps_prev_cpps)

    # if the files changed for any plugin, you need to rebuild the whole cmake project
    #
    # if the timestamps for a specific project changed, but no files were added or removed, then we can safely rebuild just that project
    # and anything that depends on it

    build_dir = "build/" + mode

    print("- Building " + mode + " build")
    if not os.path.exists(build_dir) or CONFIG_VALUES["refresh_build_" + mode] == 1 or should_reinit:
        build_setup(mode)

    plugin_deps = {}
    for p in plugin_dirs:
        deps_path = "plugins" + os.sep + p + os.sep + "deps.txt"
        if os.path.exists(deps_path):
            f = open(deps_path, "r")
            deps_list = f.readlines()
            f.close()

            plugin_deps[p] = ["quark"]
            for dep in deps_list:
                plugin_deps[p].append(dep.strip())
        else:
            plugin_deps[p] = ["quark"]

    print("changed list: ", changed_set)
    print("plugin deps: ", plugin_deps)

    inv_plugin_deps = {"quark": set([])}
    for p in plugin_dirs:
        inv_plugin_deps[p] = set([])

    for key in plugin_deps.keys():
        for p in plugin_deps[key]:
            inv_plugin_deps[p].add(key)

    print("inv: ", inv_plugin_deps)

    #changed_set.add("quark_engine")
    #changed_set.add("quark_loader")

    while len(changed_set) != 0:
        p = changed_set.pop()

        for p2 in inv_plugin_deps[p]:
            changed_set.add(p2)

        rebuild_set.add(p)

    print("rebuild: ", rebuild_set)

    rebuild_set.add("quark_loader")

    # rebuild in-order everything that needs to be rebuilt
    for p in rebuild_set:
        if os.system("cmake --build " + build_dir + " --target " + p) != 0:
            sys.exit("- Failed to build!")

    print("- Finished building " + mode + " build")
    end_t = time.time()
    print("Build took: ", end_t - start_t, "s")

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
    valid_prefix_list = ["build\\/" + mode + "\\/quark\\/quark\\/", "build\\/" + mode + "\\/plugins\\/", "build\\/" + mode +"\\/lib\\/"]
    for v in valid_prefix_list:
        found = re.findall(v + "([^\\/]*)", targets_s)
        for f in found:
            valid_libs.add(f)

    if "CMakeFiles" in valid_libs:
        valid_libs.remove("CMakeFiles")

    # COPY FILES
    print("- Copying " + mode + " build")

    # build_lib_dir = build_dir + "/lib"

    # print(glob.glob("*/*.txt", recursive=True))

    shared_lib_paths = glob.glob(build_dir + "/**/*.dll", recursive=True)
    print(shared_lib_paths)
    loader_path = build_dir + "/quark/quark/quark_loader/quark_loader.exe"

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

def build_init():
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

def plugin():
    pop_arg_and_run(PLUGIN_OPTS)

def plugin_add():
    fmt_msg = "Plugin add expects: 'User/Repo'"

    user_repo = pop_arg(fmt_msg)
    splt = user_repo.split("/")

    if len(splt) != 2:
        print(fmt_msg)
        return

    (_, repo) = splt

    if os.path.exists("plugins/" + repo):
        print("Plugin already exists, please call 'quark plugin remove PLUGIN_NAME'")
        return

    # http format: https://github.com/WindowsVista42/Quark.git
    http = "https://github.com/" + user_repo + ".git"

    # ssh format: git@github.com:WindowsVista42/Quark.git
    ssh = "git@github.com" + user_repo + ".git"

    if not try_clone_repo(http, "plugins" + os.sep + repo):
        if not try_clone_repo(ssh, "plugins" + os.sep + repo):
            print("Failed to git clone plugin: '" + user_repo + "'")
            return

    CONFIG_VALUES["refresh_build_debug"] = 1
    CONFIG_VALUES["refresh_build_release"] = 1
    print("Added plugin: '" + repo + "'")
    return

def plugin_create():
    # making folder
    plugin_name = pop_arg("Plugin create expects: 'plugin_name'")
    plugin_name_caps = plugin_name.upper()

    path = "plugins" + os.sep + plugin_name
    if not os.path.exists(path):
        os.mkdir(path)
    else:
        print(path)
        print("Plugin already exists, please call 'quark plugin remove PLUGIN_NAME' if you wish to erase the existing plugin")
        return

    os.mkdir(path + "/" + plugin_name)
    
    shutil.copy("defaults/default_plugin/README.md", path + "/README.md")
    shutil.copy("defaults/default_plugin/deps.txt", path + "/deps.txt")
    
    shutil.copy("defaults/default_plugin/default_plugin/api.hpp", path + "/" + plugin_name + "/api.hpp")
    shutil.copy("defaults/default_plugin/default_plugin/mod_main.cpp", path + "/" + plugin_name + "/mod_main.cpp")
    shutil.copy("defaults/default_plugin/default_plugin/CMakeLists.txt", path + "/" + plugin_name + "/CMakeLists.txt")
    
    shutil.copy("defaults/default_plugin/default_plugin/default_plugin.hpp", path + "/" + plugin_name + "/" + plugin_name + ".hpp")
    shutil.copy("defaults/default_plugin/default_plugin/default_plugin.cpp", path + "/" + plugin_name + "/" + plugin_name + ".cpp")

    for file in glob.glob(path + os.sep + "**/**.*", recursive=True):
        f = open(file, "r")
        string = f.read()
        f.close()
        
        string = string.replace("$default_plugin_caps", plugin_name_caps)
        string = string.replace("$default_plugin", plugin_name)

        print(string)

        f = open(file, "w")
        f.write(string)
        f.close()

    print("Created plugin: '" + plugin_name + "'")
    return

def plugin_remove():
    name = pop_arg("Plugin remove expects plugin name")

    plugin_dir = "./plugins/" + name
    if os.path.exists(plugin_dir):
        del_dir(plugin_dir)
    else:
        print("Plugin not found: '" + name + "'")
        return

    print("Removed plugin: '" + name + "'")
    return

def plugin_export():
    return

def plugin_update():
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
    "init":    (build_init,    "- Initialize build files"),
    "help":    (build_help,    "- Build help"),
    
    "1":       (custom_text,   "\nAliases:"),
    "d":       (build_debug,   "- Debug build"),
    "r":       (build_release, "- Release build"),
    "i":       (build_init, "- Initialize build files"),
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

    if not os.path.exists(".quark/config.txt"):
        cfg_f = open(".quark/config.txt", "w")
        for key in CONFIG_VALUES.keys():
            cfg_f.write(key + "=" + str(CONFIG_VALUES[key]) + "\n")
        cfg_f.close()

    if not os.path.exists(".quark/tracked"):
        os.mkdir(".quark/tracked")

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
