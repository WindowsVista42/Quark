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

    should_reinit = False
    changed_set = set([])
    rebuild_set = set([])

    # COMPILE FILES
    plugin_dirs = glob.glob("plugins/*/")
    for i in range(0, len(plugin_dirs)):
        plugin_dirs[i] = plugin_dirs[i][len("plugins/"):][:-1]

    for p in plugin_dirs:
        print("-- Building plugin: ", p)

        glob_cpps = glob.glob("plugins/" + p + "/src/**/*.cpp", recursive=True)

        curr_cpps = {}
        for cpp in glob_cpps:
            curr_cpps[cpp] = str(os.path.getmtime(cpp)) + "\n"

        prev_cpps = {}

        cache_path = ".quark/tracked/" + p + ".txt"
        if os.path.exists(cache_path):
            tracked_cache_f = open(cache_path, "r")

            lines = tracked_cache_f.readlines()
            for line in lines:
                (file, timestamp) = line.split(",")
                prev_cpps[file] = timestamp

            tracked_cache_f.close()
        else:
            should_reinit = write_tracking(cache_path, curr_cpps)

        if set(curr_cpps.keys()) != set(prev_cpps.keys()):
            should_reinit = write_tracking(cache_path, curr_cpps)

        if set(curr_cpps.values()) != set(prev_cpps.values()):
            write_tracking(cache_path, curr_cpps)
            changed_set.add(p)

        print("curr: ", curr_cpps)
        print("prev: ", prev_cpps)

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
                plugin_deps[p].append(dep)
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

    using_decl = "USING_" + plugin_name_caps
    internal_decl = plugin_name_caps + "_INTERNAL"
    api_decl = plugin_name + "_api"
    var_decl = plugin_name + "_var"

    path = "plugins" + os.sep + plugin_name
    if not os.path.exists(path):
        os.mkdir(path)
    else:
        print(path)
        print("Plugin already exists, please call 'quark plugin remove PLUGIN_NAME' if you wish to erase the existing plugin")
        return

    # making src/api.hpp
    if True:
        src = """#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined($using_decl)
    #define $api_decl __declspec(dllimport)
    #define $var_decl extern __declspec(dllimport)
  #elif defined($internal_decl)
    #define $api_decl __declspec(dllexport)
    #define $var_decl extern __declspec(dllexport)
  #else
    #define $api_decl __declspec(dllexport)
    #define $var_decl extern __declspec(dllimport)
  #endif
#endif

#ifndef $api_decl
  #define $api_decl
  #define $var_decl extern
#endif"""

        src = src.replace("$using_decl", using_decl)
        src = src.replace("$internal_decl", internal_decl)
        src = src.replace("$api_decl", api_decl)
        src = src.replace("$var_decl", var_decl)

        src_path = path + os.sep + "src"
        os.mkdir(src_path)
        f = open(src_path + os.sep + "api.hpp", "w")
        f.write(src)
        f.close()

    # making src/CMakeLists.txt
    if True:
        src = """# automatically get mod name from mod directory name
get_filename_component(MOD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)
get_filename_component(MY_MOD_NAME ${MOD_DIR} NAME)
string(REPLACE " " "_" MY_MOD_NAME ${MY_MOD_NAME})
project(${MY_MOD_NAME} C CXX)

# add all source files in src/ directory
file(GLOB SRC *.cpp)
add_library(${MY_MOD_NAME} SHARED ${SRC})

# precomple headers and link to quark_engine
target_precompile_headers(${MY_MOD_NAME} PUBLIC ${QUARK_MODULE})
target_link_libraries(${MY_MOD_NAME} quark_engine)

FILE(STRINGS "../deps.txt" DEPS)

# precompile headers and link to all other dependencies
foreach(DEP ${DEPS})
	get_target_property(DIR ${DEP} SOURCE_DIR)
	target_precompile_headers(${MY_MOD_NAME} PUBLIC ${DIR}/module.hpp)
	message(${DIR}/module.hpp)
endforeach()

foreach(DEP ${DEPS})
	target_link_libraries(${MY_MOD_NAME} ${DEP})
endforeach()"""

        f = open(src_path + os.sep + "CMakeLists.txt", "w")
        f.write(src)
        f.close()

    # making src/$plugin_name.hpp
    if True:
        src = """#pragma once

#include "api.hpp"

namespace $plugin_name {
  $api_decl void print_hello();
}"""

        src = src.replace("$plugin_name", plugin_name)
        src = src.replace("$api_decl", api_decl)

        f = open(src_path + os.sep + plugin_name + ".hpp", "w")
        f.write(src)
        f.close()

    # making src/$plugin_name.cpp
    if True:
        src = """#define $internal_decl
#include "$plugin_name.hpp"
using namespace quark;

namespace $plugin_name {
  void print_hello() {
    static f32 t = 0.0f;
    t += DT;

    if (t > 0.5f) {
      t -= 0.5f;
      printf("Hello from $plugin_name!\\n");
    }
  }
}

mod_main() {
  system::list("update")
      .add(def($plugin_name::print_hello), -1);
}
"""

        src = src.replace("$plugin_name", plugin_name)
        src = src.replace("$api_decl", api_decl)

        f = open(src_path + os.sep + plugin_name + ".cpp", "w")
        f.write(src)
        f.close()

    # making src/module.hpp
    if True:
        src = """#pragma once

#define $using_decl
#include "$plugin_name.hpp\""""
        src = src.replace("$using_decl", using_decl)
        src = src.replace("$plugin_name", plugin_name)

        f = open(src_path + os.sep + "module.hpp", "w")
        f.write(src)
        f.close()

    # making deps.txt
    if True:
        f = open(src_path + os.sep + ".." + os.sep + "deps.txt", "w")
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
