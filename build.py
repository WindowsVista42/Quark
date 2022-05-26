import os
import shutil
import sys

if __name__ == "__main__":
  mode = None
  opt_level = None
  binary = None
  
  if len(sys.argv) > 1:
    mode = sys.argv[1]
  
  if len(sys.argv) > 2:
    opt_level = sys.argv[2]
  
  if len(sys.argv) > 3:
    binary = sys.argv[3]
  
  if mode == "compile_run" or mode == "compile":
    print(opt_level)
    
    if opt_level != "debug" or opt_level != "release" or opt_level != "release_with_debug_info":
      sys.exit("opt_level not recognized!")
    
    os.system("ninja -C build" + os.sep + opt_level + " -f build.ninja")
    shutil.copyfile("build" + os.sep + "debug" + os.sep + "compile_commands.json", "compile_commands.json")
  
  if mode == "compile_run":
    os.system("." + os.sep + "build" + os.sep + opt_level + os.sep + binary)
  
  if mode == "setup":
    os.system("cmake -B build/debug -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .")
    os.system("cmake -B build/release -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .")
    os.system("cmake -B build/release_with_debug_info -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .")
    shutil.copyfile("build" + os.sep + "debug" + os.sep + "compile_commands.json", "compile_commands.json")
