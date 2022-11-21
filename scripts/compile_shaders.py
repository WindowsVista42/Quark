import pathlib
import os
import glob
import shutil
import threading
import re

OUTPUT_PATH = "quark/shaders/"

SHADER_DIRS = [
    "",
    "quark/quark/quark_engine/shaders/",
]

FLAGS = [
    ["include"      , "default"],
    ["shader_type"  , "default"],
    ["push_constant", "default"],
    ["world_data"   , "default"],
]

# SECTION_TYPES = [
#     "vertex",
#     "fragment",
#     "compute",
# ]

HEADER = """
#version 460

#define i32 int
#define u32 uint
#define f32 float
"""

VERT_HEADER = """
#define POSITION gl_Position
#define INSTANCE_ID gl_InstanceID
#define VERTEX_ID gl_VertexID
#define DRAW_ID gl_DrawID
#define BASE_VERTEX gl_BaseVertex
#define BASE_INSTANCE gl_BaseInstance
"""

FRAG_HEADER = """
#define FRAG_COORD gl_FragCoord
#define FRONT_FACING gl_FrontFacing
#define POINT_COORD gl_PointCoord
#define CLIP_DISTANCE gl_ClipDistance
#define PRIMITIVE_ID gl_PrimitiveID
#define FRAG_DEPTH gl_FragDepth
"""

def read_flag(input_text, flag, default):
    read_flag = default
    split = input_text.split(flag)
    if len(split) > 1:
        read_flag = split[1].lstrip().split()[0].strip()
    read_flag = read_flag.lower()
    return read_flag

def try_read(path, ext, flag):
    path0 = path + ext
    path1 = path + ".glsl"

    if(os.path.exists(path0)):
        return open(path0, "r").read() + "\n"
    elif(os.path.exists(path1)):
        return open(path1, "r").read() + "\n"
    else:
        print("For flag: '" + flag[0].upper() + "' could not find option '" + flag[1].upper() + "'")
        exit(-1)

    # try:
    #     return open(path + ext, "r").read() + "\n"
    # except:
    #     try:
    #         return open(path + ".glsl", "r").read() + "\n"
    #     except:
    #         print("For flag: '" + flag[0].upper() + "' could not find option '" + flag[1].upper() + "'")

def compile_ext_shader(path):
    PA = path
    text = open(path, "r").read()

    vert_top_text = ""
    frag_top_text = ""

    flags = FLAGS

    for flag in flags:
        flag[1] = read_flag(text, flag[0].upper() + ":", flag[1])
        if flag[1] == "ignore": continue
        flag_path = os.path.dirname(path) + "/" + flag[0] + "/" + flag[1]
        vert_top_text = vert_top_text + try_read(flag_path, ".vert", flag)
        frag_top_text = frag_top_text + try_read(flag_path, ".frag", flag)

    vertex_section_pos = text.find("// SECTION: VERTEX")
    fragment_section_pos = text.find("// SECTION: FRAGMENT")

    vertex_shader_text = HEADER + "\n" + VERT_HEADER + "\n" + text[:vertex_section_pos] + vert_top_text + text[vertex_section_pos:fragment_section_pos]
    fragment_shader_text = HEADER + "\n" + FRAG_HEADER + "\n" + text[:vertex_section_pos]  + frag_top_text + text[fragment_section_pos:]

    print(PA)
    i = 0;
    for line in vertex_shader_text.splitlines():
        print(str(i) + " " + line)
        i += 1

    i = 0;
    for line in fragment_shader_text.splitlines():
        print(str(i) + " " + line)
        i += 1

    # exit(0)

    # print(fragment_shader_text)

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
        # if path.find(".ext") != -1: continue
        threads.append(threading.Thread(target=compile_spv_shader, args=(path,)))
        #compile_spv_shader(path)

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

    for path in ext_shader_paths:
        os.remove(path.replace(".shader.glsl", ".vert"))
        os.remove(path.replace(".shader.glsl", ".frag"))
