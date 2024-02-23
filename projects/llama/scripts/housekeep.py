import os, sys
import glob

def ends_with(path, suffixes):
	for s in suffixes:
		if path.endswith(s):
			return True
	return False

def get_file_type(path):
	if ends_with(path, ['.cpp', '.h', '.hpp', '.c']):
		return 'SOURCE'
	elif ends_with(path, ['.proto']):
		return 'PROTO'
	return None

def traverse_files(path):
	output_path = os.path.join(path, "sources.cmake")
	output_file = open(output_path,"w")
	src = glob.glob(os.path.join(path, "src/**"), recursive=True)
	include = glob.glob(os.path.join(path, "include/**"), recursive=True) 
	test = glob.glob(os.path.join(path, "test/**"), recursive=True)
	
	for filename in src + include:
		file_type = get_file_type(filename)
		if file_type is None:
			continue
		filename = os.path.relpath(filename, path)
		filename = filename.replace("\\", "/")
		print(filename)
		output_file.write(f'list(APPEND {file_type}_LIST "{filename}")\n')

	for filename in test:
		file_type = get_file_type(filename)
		if file_type is None:
			continue
		filename = os.path.relpath(filename, path)
		filename = filename.replace("\\", "/")
		print(filename)
		output_file.write(f'list(APPEND TEST_{file_type}_LIST "{filename}")\n')

if __name__ == "__main__":
	# llama 里面有些是cpp模块，有些不是。把不是的排除掉。
	# projects/llama/modules
	project_root = os.path.abspath(os.path.join(sys.argv[0], '..', '..', 'src'))
	with open(os.path.join(project_root, "modules.cmake"), 'w', encoding='utf-8') as module_file:
		for dir_name in os.listdir(project_root):
			# e.g. projects/llama/cmake
			dir_path = os.path.join(project_root, dir_name)
			if os.path.isdir(dir_path):
				traverse_files(dir_path)
				module_file.write(f'list(APPEND MODULE_LIST "{dir_name}")\n')