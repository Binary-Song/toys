# To future devs:	
# 在随意添加自定义命令/目标前，
# 请务必阅读并理解
# https://cmake.org/cmake/help/latest/command/add_custom_command.html#example-generating-files-for-multiple-targets
# 理解command和target的区别后，再进行添加。
# command: 是“类”，可以产生多个实例，他们直接毫不相干
# target: 是“对象”。所以依赖同一个command的两个不同实例的结果仍然允许并发编译。
#
# 关于 add_dependency: https://stackoverflow.com/questions/75641864/cmake-target-does-not-build-when-its-dependency-target-builds
# 另： https://samthursfield.wordpress.com/2015/11/21/cmake-dependencies-between-targets-and-files-and-custom-commands/

# source file 分为 generated 和 non-generated。
# generated: 添加的时候要求同时指定dependent target。然后还会检查这些target会不会真的生成这个generated source。
#           然后还要确保支持传递性搜索。问题来了，传递性搜索如何实现？因为每个target都是我创建的？，所以说传递性搜索
#           对于custom target，
# non-gen: 

# 允许的源文件和头文件扩展名
set(LLAMA_SOURCE_EXTENSIONS "*.cpp" "*.c" CACHE INTERNAL "source file extensions that are allowed within the llama project")
set(LLAMA_HEADER_EXTENSIONS "*.hpp" "*.h" CACHE INTERNAL "header file extensions that are allowed within the llama project")

# 自动创建 llama 目标。
# 在创建目标前，先用scripts\update_filelist.py生成或者更新源文件列表sources.cmake。
# 该文件应当和源码一起检入git。
# 参数：
# name - 目标的名称。
# type - 目标类型。目前支持的目标有 HEADER_ONLY 、SHARED 和 EXECUTABLE
function(llama_target name type)
	# p.s. CMAKE_CURRENT_LIST_DIR 是调用者的位置，不是本文件的位置
	llama_internal_include_source_list(SOURCE_LIST TEST_SOURCE_LIST)
	if(type STREQUAL HEADER_ONLY)
		add_library("${name}" INTERFACE)
		target_include_directories("${name}" INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
	else()
		if(type STREQUAL SHARED)
			add_library("${name}-obj" OBJECT "${SOURCE_LIST}")
			add_library("${name}" SHARED "$<TARGET-objECTS:${name}-obj>")
		elseif(type STREQUAL EXECUTABLE)
			add_library("${name}-obj" OBJECT)
			add_executable("${name}" "$<TARGET-objECTS:${name}-obj>")
		else()
			message(FATAL_ERROR "unknown type: ${type}")
		endif()		
		target_include_directories("${name}-obj" PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
		message(DEBUG  "added include for ${name}-obj : ${CMAKE_CURRENT_LIST_DIR}/include")
	endif()
	add_executable("${name}-test" "${TEST_SOURCE_LIST}")
	target_link_libraries("${name}-test" PUBLIC "${name}" GTest::gtest GTest::gtest_main)
	add_test(NAME "${name}-test" COMMAND "${name}-test")
	llama_internal_verify_target("${name}" "${type}")
endfunction()

function(llama_docs)
	set(INPUT_DIR "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../") # Doxygen 输入目录，也就是要文档化的源码路径
	set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/docs") # Doxygen 输出目录，文档输出在这里
	set(DOXYFILE_INPUT_PATH "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../docs/llama.doxyfile") # 输入的 doxyfile 模板
	set(DOXYFILE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile") # 输出的 Doxyfile 文件，文件里的路径被替换了
	string(REPLACE ";" " " "INPUT_FILE_PATTERNS" "${LLAMA_SOURCE_EXTENSIONS};${LLAMA_HEADER_EXTENSIONS}")
	file(MAKE_DIRECTORY "${OUTPUT_DIR}")

	option(LLAMA_BUILD_DOCS "Build documentation" OFF)
	if(LLAMA_BUILD_DOCS)
		# 无法找到Doxygen。如果不希望生成文档，请在cmake命令行增加 -DLLAMA_BUILD_DOCS=OFF
		find_package(Doxygen REQUIRED)

		# 配置Doxyfile
		add_custom_command(
			OUTPUT "${DOXYFILE_OUTPUT_PATH}"
			COMMAND 
				"${CMAKE_COMMAND}"
				"-DDOXYFILE_INPUT_PATH=${DOXYFILE_INPUT_PATH}"
				"-DDOXYFILE_OUTPUT_PATH=${DOXYFILE_OUTPUT_PATH}"
				"-DINPUT_DIR=${INPUT_DIR}"
				"-DOUTPUT_DIR=${OUTPUT_DIR}"
				"-DINPUT_FILE_PATTERNS=${INPUT_FILE_PATTERNS}"
				"-P"
				"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/llama_configure.cmake"
			WORKING_DIRECTORY "${OUTPUT_DIR}"
			DEPENDS "${DOXYFILE_INPUT_PATH}"
			COMMENT "Configuring doxyfile"
			VERBATIM)
		add_custom_target(
			docs-configure
			DEPENDS "${DOXYFILE_OUTPUT_PATH}"
		)

		# DEPENDS 为指令的输入文件和目标，OUTPUT为输出文件
		add_custom_command(
			OUTPUT  "${OUTPUT_DIR}/docs.stamp"
			COMMAND "${DOXYGEN_EXECUTABLE}"
			COMMAND echo "delete me to trigger a rebuild of the docs" > "${OUTPUT_DIR}/docs.stamp"
			COMMAND "${CMAKE_COMMAND}" -E create_symlink "${OUTPUT_DIR}/html/index.html" "${OUTPUT_DIR}/llama-documentation.html"
			WORKING_DIRECTORY "${OUTPUT_DIR}"
			DEPENDS "${DOXYFILE_OUTPUT_PATH}" "docs-configure" # add_custom_command 的 depend 可以是文件或者 target
			COMMENT "Generating docs"
			VERBATIM)
		add_custom_target(
			docs
			DEPENDS "${OUTPUT_DIR}/docs.stamp" # add_custom_target 的 depend 仅限文件且必须是同一个 CMakeLists.txt
		)
	endif()
endfunction()

function(llama_internal_include_source_list src_out test_src_out)

	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_FUNCTION_LIST_DIR}")
	unset(SOURCE_LIST)
	unset(TEST_SOURCE_LIST)
	include("sources.cmake")
	set(${src_out} "${SOURCE_LIST}" PARENT_SCOPE)
	set(${test_src_out} "${TEST_SOURCE_LIST}" PARENT_SCOPE)

endfunction()

function(llama_internal_verify_dir)
	cmake_parse_arguments(
		"ARG" # prefix
		"" # options
		"PATH" # one-vals
		"MUST_CONTAIN" # multi-vals
		${ARGN}
	)
	set(path "${ARG_PATH}")

	if(NOT EXISTS "${path}")
		message(FATAL_ERROR "check on target '${name}' failed: directory '${path}' is required, but it does not exist")
	endif()
	
	if(NOT IS_DIRECTORY "${path}")
		message(FATAL_ERROR "check on target '${name}' failed: '${path}' should be a directory, but it is not.")
	endif()
	unset(files_found)
	if(ARG_MUST_CONTAIN)
		foreach(patt ${ARG_MUST_CONTAIN})
			file(GLOB_RECURSE files FOLLOW_SYMLINKS "${path}/${patt}")
			list(APPEND files_found "${files}")
		endforeach()
		if(NOT files_found)
			message(FATAL_ERROR "check on target '${name}' failed: directory ${path} does not contain files matching these patterns: ${ARG_MUST_CONTAIN}")
		endif()
	endif()
endfunction()

function(llama_internal_verify_target name type)
	option(LLAMA_IGNORE_TARGET_VERIFICATION OFF)

	if(NOT CACHE{LLAMA_IGNORE_TARGET_VERIFICATION})
		llama_internal_verify_dir(PATH "${CMAKE_CURRENT_LIST_DIR}/include")
		llama_internal_verify_dir(PATH "${CMAKE_CURRENT_LIST_DIR}/include/${name}" MUST_CONTAIN ${LLAMA_HEADER_EXTENSIONS})
		llama_internal_verify_dir(PATH "${CMAKE_CURRENT_LIST_DIR}/test" MUST_CONTAIN ${LLAMA_SOURCE_EXTENSIONS})
		
		if(type STREQUAL HEADER_ONLY)
		else()
			llama_internal_verify_dir(PATH "${CMAKE_CURRENT_LIST_DIR}/src" MUST_CONTAIN ${LLAMA_SOURCE_EXTENSIONS})
		endif()

		# 要求 file list 必须同步
		llama_internal_include_source_list(src test_src)
		set(src "${src};${test_src}")
		foreach(ext ${LLAMA_SOURCE_EXTENSIONS} ${LLAMA_HEADER_EXTENSIONS})
			file(GLOB_RECURSE g FOLLOW_SYMLINKS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "src/${ext}" )
			list(APPEND actual_src "${g}")
			file(GLOB_RECURSE g FOLLOW_SYMLINKS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "include/${ext}" )
			list(APPEND actual_src "${g}")
			file(GLOB_RECURSE g FOLLOW_SYMLINKS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "test/${ext}" )
			list(APPEND actual_src "${g}")
		endforeach()
		
		llama_internal_normalize_list(src)
		llama_internal_normalize_list(actual_src)

		if(NOT src STREQUAL actual_src)
			message(WARNING "sources.cmake for target '${name}' may not be up to date because it differs from the source tree.\nthe file: ${src}\nthe source tree: ${actual_src}\n")
		endif()

	endif()

endfunction()

function(llama_internal_normalize_list list)
	list(REMOVE_DUPLICATES "${list}")
	list(FILTER "${list}" EXCLUDE REGEX "^$")
	list(SORT "${list}")
	set("${list}" "${${list}}" PARENT_SCOPE)
endfunction()