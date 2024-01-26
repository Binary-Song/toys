import os, sys
import glob

def is_source_file(path):
	suffixes = ['.cpp', '.h', '.hpp', '.c']
	for s in suffixes:
		if path.endswith(s):
			return True
	return False

def traverse_files(path):
	output_path = os.path.join(path, "sources.cmake")
	output_file = open(output_path,"w")
	src = glob.glob(os.path.join(path, "src/**"), recursive=True)
	include = glob.glob(os.path.join(path, "include/**"), recursive=True) 
	test = glob.glob(os.path.join(path, "test/**"), recursive=True)
	for filename in src + include:
		if is_source_file(filename):
			filename = os.path.relpath(filename, path)
			filename = filename.replace("\\", "/")
			print(filename)
			output_file.write(f'list(APPEND SOURCE_LIST "{filename}")\n')
	for filename in test:
		if is_source_file(filename):
			filename = os.path.relpath(filename, path)
			filename = filename.replace("\\", "/")
			print(filename)
			output_file.write(f'list(APPEND TEST_SOURCE_LIST "{filename}")\n')

if __name__ == "__main__":
	if len(sys.argv) <= 2:
		input_dir = input("Type source directory of your module: ")
	else:
		input_dir = sys.argv[1]
	traverse_files(input_dir)
