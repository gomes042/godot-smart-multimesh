#!/usr/bin/env python

import os
import platform as host_platform
from SCons.Script import (
    Environment, Variables, BoolVariable, Help,
    SConscript, Default, Glob
)
from SCons.Errors import UserError

# ========== CONFIG ==========
libname = "smm"

# ========== HELPER FUNCTIONS ==========

def normalize_path(val, env):
    return val if os.path.isabs(val) else os.path.join(env.Dir("#").abspath, val)

def validate_parent_dir(key, val, env):
    if not os.path.isdir(os.path.dirname(normalize_path(val, env))):
        raise UserError("'%s' is not a directory: %s" % (key, os.path.dirname(val)))

def recursive_glob(base_dir, patterns):
    matches = []
    for root, _, filenames in os.walk(base_dir):
        for pattern in patterns:
            matches.extend(
                os.path.join(root, filename)
                for filename in filenames if filename.endswith(pattern)
            )
    return matches

def get_unique_dirs(files):
    return list(set(os.path.dirname(str(f)) for f in files))

# ========== VARIABLE PARSING ==========

opts = Variables()
opts.Add(BoolVariable("compiledb", "Generate compile_commands.json", True))

localEnv = Environment(tools=["default"], PLATFORM="")
opts.Update(localEnv)
Help(opts.GenerateHelpText(localEnv))

# Determine target/debug early
target = localEnv.get("target", "")
is_debug = target == "template_debug"

env = localEnv.Clone()
env["compiledb"] = True if is_debug else localEnv.get("compiledb", False)

# ========== ENABLE SCONS CACHE ==========
if "SCONS_CACHE_DIR" in os.environ:
    CacheDir(os.environ["SCONS_CACHE_DIR"]) # type: ignore

# ========== LOAD GODOT ENVIRONMENT ==========
env = SConscript("include/godot-cpp/SConstruct", {"env": env})

# ========== FLAGS & DEFINES ==========
if host_platform.system().lower() == "linux":
    env.Tool("mingw")
    env.Append(CCFLAGS=[
        "-std=c++17",
        "-fexceptions",
        "-fPIC",
    ])

env.Append(CPPDEFINES="SMM_DEBUG" if is_debug else "SMM_RELEASE")

# ========== SOURCE COLLECTION ==========
source_files = recursive_glob("src", [".cpp"])
header_dirs = get_unique_dirs(recursive_glob("src", [".h"]))
env.Append(CPPPATH=["src"] + header_dirs)

# ========== OUTPUT SETUP ==========
platform_name = env["platform"]
architecture = env["arch"]
build_type = "debug" if is_debug else "release"

output_name = f"lib{libname}.{platform_name}.{build_type}.{architecture}"

output_path = os.path.join(
    "build", platform_name, architecture, build_type,
    f"{output_name}{env['SHLIBSUFFIX']}"
)

# ========== BUILD SHARED LIBRARY ==========
library = env.SharedLibrary(target=output_path, source=source_files)

# ========== COMPILATION DATABASE ==========
default_args = [library]

if env["compiledb"]:
    env.Tool("compilation_db")
    compilation_db = env.CompilationDatabase(
        target="compile_commands.json",
        source=source_files
    )
    env.Alias("compiledb", compilation_db)

    default_args.append(compilation_db)
    env.Requires(compilation_db, library)

Default(*default_args)
