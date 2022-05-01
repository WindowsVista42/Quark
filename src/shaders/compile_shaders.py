import pathlib
import os
import glob
import shutil
import threading

OUTPUT_PATH = "assets/shaders/"

SHADER_DIRS = [
    "",
    "lib/Quark/src/shaders/",
]

FLAGS = [
    ["include"      , "default"],
    ["shader_type"  , "default"],
    ["push_constant", "default"],
]

HEADER = """#version 460

#define i32 int
#define u32 uint
#define f32 float
"""

def read_flag(input_text, flag, default):
    read_flag = default
    split = input_text.split(flag)
    if len(split) > 1:
        read_flag = split[1].lstrip().split()[0].strip()
    read_flag = read_flag.lower()
    return read_flag

def try_read(path, ext, flag):
    print(path)
    try:
        return open(path + ext, "r").read() + "\n"
    except:
        pass

    try:
        return open(path + ".glsl", "r").read() + "\n"
    except:
        print("For flag: '" + flag[0].upper() + "' could not find option '" + flag[1].upper() + "'")

def compile_ext_shader(path):
    text = open(path, "r").read()

    vert_top_text = ""
    frag_top_text = ""

    flags = FLAGS

    print("a")

    for flag in flags:
        flag[1] = read_flag(text, flag[0].upper() + ":", flag[1])
        if flag[1] == "ignore": continue
        flag_path = os.path.dirname(path) + "/" + flag[0] + "/" + flag[1]
        vert_top_text = vert_top_text + try_read(flag_path, ".vert", flag)
        frag_top_text = frag_top_text + try_read(flag_path, ".frag", flag)


    vertex_shader_text = HEADER + "\n" + text[:text.find("// SECTION: VERTEX")] + vert_top_text + text[text.find("// SECTION: VERTEX"):text.find("// SECTION: FRAGMENT")]
    fragment_shader_text = HEADER + "\n" + text[:text.find("// SECTION: VERTEX")]  + frag_top_text + text[text.find("// SECTION: FRAGMENT"):]

    real_path = path.split(".")[0] + ".vert"
    f = open(real_path, "w")
    f.seek(0)
    f.write(vertex_shader_text)
    f.truncate()
    f.close()

    real_path = path.split(".")[0] + ".frag"
    f = open(real_path, "w")
    f.seek(0)
    f.write(fragment_shader_text)
    f.truncate()
    f.close()

def compile_spv_shader(path):
    spv_path = path + ".spv"
    cmd = "glsLangValidator " + path + " -V -o " + spv_path
    os.system(cmd)

if __name__ == "__main__":
    ext_shader_paths = []
    for dir in SHADER_DIRS:
        ext_shader_paths.extend(glob.glob(dir + "*.shader.glsl"))

    for path in ext_shader_paths:
        compile_ext_shader(path)

    shader_paths = []
    for dir in SHADER_DIRS:
        shader_paths.extend(glob.glob(dir + "*.vert"))
        shader_paths.extend(glob.glob(dir + "*.frag"))
        shader_paths.extend(glob.glob(dir + "*.comp"))

    threads = []
    for path in shader_paths:
        if path.find(".ext") != -1: continue
        threads.append(threading.Thread(target=compile_spv_shader, args=(path,)))

    for thread in threads:
        thread.start()

    for thread in threads:
        thread.join()

    spv_paths = []
    for path in shader_paths:
        if path.find(".ext") != -1: continue
        full_path = path + ".spv"
        name = os.path.basename(full_path)
        shutil.move(full_path, OUTPUT_PATH + name)

    #for path in ext_shader_paths:
    #    os.remove(path.replace(".shader.glsl", ".vert"))
    #    os.remove(path.replace(".shader.glsl", ".frag"))
