import pathlib
import os

def read_flag(input_text, flag, default):
    read_flag = default
    split = input_text.split(flag)
    if len(split) > 1:
        read_flag = split[1].lstrip().split()[0].strip()
    read_flag = read_flag.lower()
    return read_flag

def try_read(path, flag):
    try:
        f = open(flag_path, "r")
        out = f.read()
        f.close()
        return out
    except:
        print("For flag: '" + flag[0].upper() + "' could not find option '" + flag[1].upper() + "'")

        

if __name__ == "__main__":
    path = "test_0.vert"
    suffix = pathlib.Path(path).suffix
    text = open(path, "r").read()

    top_text = ""
    pre_main_text = ""

    flags = [
        ["include",       "include", "top"        ],
        ["shader_type",   "simple" , "top"        ],
        ["push_constant", "custom" , "before_main"],
    ]

    for flag in flags:
        flag[1] = read_flag(text, flag[0].upper() + ":", flag[1])
        flag_path = flag[0] + "/" + flag[1] + suffix
        if flag[2] == "top":
            top_text = top_text + try_read(flag_path, flag)
        elif flag[2] == "before_main":
            pre_main_text = pre_main_text + try_read(flag_path, flag)

    main_pos = text.find("void main()")
    result = top_text + text[:main_pos] + pre_main_text + "\n" + text[main_pos:]
    ext_path = path.split(".")[0] + ".ext" + suffix
    f = open(ext_path, "w")
    f.seek(0)
    f.write(result)
    f.truncate()
    f.close()

    stage = suffix[1:]
    spv_path = path + ".spv"
    cmd = "glsLangValidator " + ext_path + " -S " + stage + " -V -o " + spv_path
    os.system(cmd)
