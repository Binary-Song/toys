import os, sys, glob, json, numbers, tempfile, shutil, io

# 文件内容不变就不改动，防止触发重编
class LazilyModdedFile:
    def __init__(self, path) -> None:
        self.buff = io.StringIO()
        self.path = path

    def __enter__(self):
        return self.buff 

    def __exit__(self, type, value, trace):
        str_new = self.buff.getvalue()
        do_write = False
        if not os.path.exists(self.path):
            do_write = True
        else:
            with open(self.path, 'r', encoding='utf-8') as f:
                str_old = f.read()
            do_write = str_new != str_old
        if do_write:
            with open(self.path, 'w', encoding='utf-8') as f:
                f.write(str_new)
                print(self.path)

def is_in_dir(file_path: str, dir_path: str):
    if os.path.isabs(file_path) and os.path.isabs(dir_path):
        file_path = os.path.abspath(file_path) # just to normalize
        dir_path = os.path.abspath(dir_path)
        return file_path.startswith(dir_path)
    raise Exception("either of them is not an abs path")

def ends_with(path, suffixes):
    for s in suffixes:
        if path.endswith(s):
            return True
    return False

def is_c_or_cxx_header(path):
    return ends_with(path, ['.h', '.hpp'])

def is_c_or_cxx_source(path):
    return ends_with(path, ['.cpp', '.c'])

def is_c_or_cxx(path):
    return is_c_or_cxx_header(path) or is_c_or_cxx_source(path)

def is_proto_source(path):
    return ends_with(path, ['.proto'])

# file_type 是根据文件的后缀名区分的
def get_file_type(path):
    if is_c_or_cxx(path):
        return 'cxx'
    elif is_proto_source(path):
        return 'proto'
    return None

# file_role 是根据文件所在的位置区分的
def get_file_role(file_path, module_dir):
    if is_in_dir(file_path, os.path.join(module_dir, 'include')):
        return 'include'
    elif is_in_dir(file_path, os.path.join(module_dir, 'src')):
        return 'src'
    elif is_in_dir(file_path, os.path.join(module_dir, 'test')):
        return 'test'
    return None

def get_default_top_level_config(project_dir):
    return { 
        "include": glob.glob(os.path.join(project_dir, "*")),
        "exclude": [],
    }

def get_default_module_config(files, module_dir):
    return { 
        "include": files, 
        "exclude": [], 
        "umbrella_header": {
            "generate": True,
            "include": glob.glob(os.path.join(module_dir, "include", "**", "*"), recursive=True),
            "exclude": [],
            "force": False
        }
    }

def merge_configs(default_conf, loaded_conf, loaded_conf_path):
    
    for key in loaded_conf:
        
        if key not in default_conf:
            print(f"unknown key ignored: `{key}`, in file {loaded_conf_path}, json object: {loaded_conf}")
            continue

        if isinstance(loaded_conf[key], (numbers.Number, str, list)):
            default_conf[key] = loaded_conf[key]
            
        elif isinstance(loaded_conf[key], dict):
            default_conf[key] = merge_configs(default_conf[key], loaded_conf[key], loaded_conf_path) # Recursive!

    return default_conf

def include_exclude(includes, excludes, root, recursive=True):
    files = []
    for include in includes:
        path = os.path.join(root, include) # include 可以是绝对路径。此时 path.join 返回 include。
        path = os.path.abspath(path) # normalize
        files += glob.glob(path, recursive=recursive)
    
    xfiles = []
    for exclude in excludes:
        path = os.path.join(root, exclude)
        path = os.path.abspath(path)
        xfiles += glob.glob(path, recursive=recursive)
    
    files = set(files) - set(xfiles)
    files = list(files)
    files.sort()
    return files

def generate_umbrella_header(files, module_dir, conf):
    # module_dir: projects/llama/src/foundation/
    # umb_header: projects/llama/src/foundation/include/foundation/foundation.h
    module_name = os.path.basename(module_dir) 
    umb_header = os.path.join(module_dir, 'include', module_name, module_name + '.h')

    if conf['umbrella_header']['generate'] == False:
        return
    include_dir = os.path.join(module_dir, 'include')
    files = include_exclude(conf['umbrella_header']['include'], conf['umbrella_header']['exclude'], include_dir, recursive=True)

    # 如果不像是我生成的文件，就不要覆盖
    UMB_HEADER_WARNING = \
"""/* ---------------------------------------------------- *
 *  This is a generated umbrella header -- DO NOT EDIT  *
 * ---------------------------------------------------- */
"""
    if conf['umbrella_header']['force'] == False and os.path.isfile(umb_header):
        with open(umb_header, 'r', encoding='utf-8') as output_file:
            if not output_file.read().startswith(UMB_HEADER_WARNING):
                print(f"warning: umbrella header generation failed: {umb_header} already exists and does not look like I generated it")
                return

    with LazilyModdedFile(umb_header) as output_file:
        output_file.write(UMB_HEADER_WARNING)
        output_file.write("#pragma once\n")
        for file in files:
            if is_c_or_cxx(file) and get_file_role(file, module_dir) == 'include' and file != umb_header:
                relpath = os.path.relpath(file, os.path.join(module_dir, 'include'))
                relpath = relpath.replace('\\', '/')
                output_file.write(f'#include "{relpath}"\n')

def generate_source_list(module_dir, files):
    # projects/llama/src/foundation/sources.cmake
    output_path = os.path.join(module_dir, "sources.cmake")
    with LazilyModdedFile(output_path) as output_file:    
        for file_path in files:
            
            file_type = get_file_type(file_path)
            if file_type is None:
                continue
            
            file_role = get_file_role(file_path, module_dir)
            if file_role is None:
                continue
            
            if file_type == 'cxx':
                file_type = 'source'
            FILE_TYPE = file_type.upper()

            FILE_PATH = os.path.relpath(file_path, module_dir)
            FILE_PATH = FILE_PATH.replace('\\', '/')

            if file_role == 'test':
                FILE_ROLE_ = 'TEST_'
            else:
                FILE_ROLE_ = ''

            output_file.write(f'list(APPEND {FILE_ROLE_}{FILE_TYPE}_LIST "{FILE_PATH}")\n')

def load_config_file_or_empty(config_file_path):
    if os.path.isfile(config_file_path):
        with open(config_file_path, 'r', encoding='utf-8') as config_file:
            try:
                user_conf = json.load(config_file)
            except json.JSONDecodeError as e:
                raise Exception(f"json decode error in file: {config_file_path}, line: {e.lineno}, col: {e.colno}")
    else:
        user_conf = {}
    return user_conf

def load_config_and_merge_with_default(config_file_path, default_conf):
    user_conf = load_config_file_or_empty(config_file_path)
    conf = merge_configs(default_conf, user_conf, config_file_path)
    return conf

def housekeep_module(module_dir):
    # projects/llama/src/foundation
    if not os.path.isdir(module_dir):
        return
    
    src_files = glob.glob(os.path.join(module_dir, "src/**"), recursive=True)
    include_files = glob.glob(os.path.join(module_dir, "include/**"), recursive=True)
    test_files = glob.glob(os.path.join(module_dir, "test/**"), recursive=True)

    default_conf = get_default_module_config(src_files + include_files + test_files, module_dir)

    # projects/llama/src/foundation/housekeep.json
    config_file_path = os.path.join(module_dir, "housekeep.json")
    conf = load_config_and_merge_with_default(config_file_path, default_conf)

    # 处理 include exclude
    files = include_exclude(conf['include'], conf['exclude'], module_dir, recursive=True)
    generate_umbrella_header(files, module_dir, conf)
    generate_source_list(module_dir, files)

def housekeep(project_root):
    with LazilyModdedFile(os.path.join(project_root, "modules.cmake")) as modules_file:
        # 导入顶层 housekeep.json
        default_conf = get_default_top_level_config(project_root)
        conf_path = os.path.join(project_root, "housekeep-modules.json")
        conf = load_config_and_merge_with_default(conf_path, default_conf)
        
        # 处理 include exclude
        paths = include_exclude(conf['include'], conf['exclude'], project_root, recursive=False)

        # 写入 modules.cmake
        for path in paths:
            if os.path.isdir(path):
                module_name = os.path.relpath(path, project_root)
                modules_file.write(f'list(APPEND MODULE_LIST "{module_name}")\n')
                module_dir = os.path.join(project_root, module_name)
                housekeep_module(module_dir)

if __name__ == "__main__":
    # llama 里面有些是cpp模块，有些不是。把不是的排除掉。
    # projects/llama/src
    project_root = os.path.abspath(os.path.join(sys.argv[0], '..', '..', 'src'))
    housekeep(project_root)
