import sys
import os

def publish():
    print("publish")

def build():
    print("here")
    pop_arg_and_run(BUILD_OPTS)

def build_debug():
    print("debug")

def build_release():
    print("release")

def build_help():
    print("build help")

def help():
    print("help")

GLOBAL_OPTS = {
    "build": build,
    "publish": publish,
    "help": help,
}

BUILD_OPTS = {
    "debug": build_debug,
    "release": build_release,
    "help": build_help,
}

argument_stack = []

def pop_arg_and_run(arg_opts):
    if len(argument_stack) == 0:
        arg_opts["help"]()
        exit(0)

    arg = argument_stack.pop()
    if(arg not in arg_opts):
        print("Argument \"" + arg + "\" not valid!")
        exit(-1)
    arg_opts[arg]()

if __name__ == "__main__":
    for arg in sys.argv:
        if arg != "":
            argument_stack.append(arg)

    argument_stack.reverse()
    argument_stack.pop()

    pop_arg_and_run(GLOBAL_OPTS)
