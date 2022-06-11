import os
import shutil
import sys
import atexit
from glob import glob

OPT_LEVELS = ["debug", "release", "release_with_debug_info"]
MODES = ["compile_run", "compile", "setup"]

mode = None
opt_level = None
bin_name = None
build_dir = None

def copy_file(dst_file, src_file):
  COMP_CMD = dst_file# "compile_commands.json"
  COMP_CMD_DIR = src_file#DEBUG_DIR + os.sep + COMP_CMD

  if not os.path.exists(COMP_CMD_DIR):
    print("Failed to locate " + COMP_CMD_DIR + " ignoring copy!")
    return

  if os.path.exists(COMP_CMD):
    os.remove(COMP_CMD)

  if shutil.copy2(COMP_CMD_DIR, COMP_CMD) != COMP_CMD:
    sys.exit("Failed to copy " + COMP_CMD + "!")

  print("Copied " + COMP_CMD_DIR + " to " + COMP_CMD)

def run_program():
  os.system("." + os.sep + build_dir + os.sep + "quark_loader")

if __name__ == "__main__":
  # Error handling
  if len(sys.argv) > 1:
    mode = sys.argv[1]
  
  if len(sys.argv) > 2:
    opt_level = sys.argv[2]
  
  if len(sys.argv) > 3:
    bin_name = sys.argv[3]

  if not (mode in MODES):
    sys.exit("mode not recognized: " + mode)
  
  # Compile if one of the compile modes
  if mode == "compile_run" or mode == "compile":
    if not (opt_level in OPT_LEVELS):
      sys.exit("opt_level not recognized: " + opt_level)

    build_dir = "build" + os.sep + opt_level

    print("Compiling " + build_dir + os.sep + bin_name)

    if os.system("cmake --build " + build_dir + " --target " + bin_name) != 0 or os.system("cmake --build " + build_dir + " --target quark_loader") != 0:
      sys.exit("Failed to build!")

    copy_file("compile_commands.json", build_dir + os.sep + "compile_commands.json")

    #TODO(sean): MAKE THIS OS INDEPENDANT
    dlls = glob(build_dir + os.sep + "lib" + os.sep + "*.dll")
    for dll in dlls:
      dll_name = os.path.basename(dll)
      copy_file(build_dir + os.sep + dll_name, build_dir + os.sep + "lib" + os.sep + dll_name)
      #copy_file(DEBUG_DIR + os.sep + "glfw3.dll", DEBUG_DIR + os.sep + "lib" + os.sep + "glfw3.dll")

    copy_file("mods" + os.sep + bin_name + ".dll", build_dir + os.sep + bin_name + ".dll")
  
    # Run if run mode
    if mode == "compile_run":
      atexit.register(run_program)
  
  # Setup if setup mode
  if mode == "setup":
    os.system("cmake -B build/debug -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .")
    os.system("cmake -B build/release -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .")
    os.system("cmake -B build/release_with_debug_info -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .")
    replace_compile_commands()

