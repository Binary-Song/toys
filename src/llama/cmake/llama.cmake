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

function(llama_stack_push stack elem)
	message(DEBUG "llama_stack_push: before: llama_stack_${stack} = $CACHE{llama_stack_${stack}}")
	
	set(llama_stack_${stack} "${llama_stack_${stack}}\t${elem}" CACHE INTERNAL "A stack internally used by llama")
	
	message(DEBUG "llama_stack_push: after: llama_stack_${stack} = $CACHE{llama_stack_${stack}}")
endfunction()

function(llama_stack_pop stack out_var)
	message(DEBUG "llama_stack_pop: before: llama_stack_${stack} = $CACHE{llama_stack_${stack}}")

	set(stack_content "$CACHE{llama_stack_${stack}}")
	string(FIND "${stack_content}" "\t" delim_index REVERSE)
	string(LENGTH "${stack_content}" len)
	math(EXPR last_elem_len "${len} - ${delim_index} - 1")
	math(EXPR delim_next_index "${delim_index} + 1")
	string(SUBSTRING "${stack_content}" ${delim_next_index} ${last_elem_len} last_elem)
	set("${out_var}" "${last_elem}" PARENT_SCOPE)
	message(DEBUG "llama_stack_pop: elem = ${last_elem}")
	string(SUBSTRING "${stack_content}" 0 ${delim_index} new_stack_content)
	set(llama_stack_${stack} "${new_stack_content}" CACHE INTERNAL "A stack internally used by llama")

	message(DEBUG "llama_stack_pop: after: llama_stack_${stack} = $CACHE{llama_stack_${stack}}")
endfunction()

function(llama_stack_top stack out_var)

	set(stack_content "$CACHE{llama_stack_${stack}}")
	string(FIND "${stack_content}" "\t" delim_index REVERSE)
	string(LENGTH "${stack_content}" len)
	math(EXPR last_elem_len "${len} - ${delim_index} - 1")
	math(EXPR delim_next_index "${delim_index} + 1")
	string(SUBSTRING "${stack_content}" ${delim_next_index} ${last_elem_len} last_elem)
	set("${out_var}" "${last_elem}" PARENT_SCOPE)

endfunction()

function(llama_target name type)
	llama_stack_push(target "${name}")
	llama_stack_push(target_type "${type}")

	if(type STREQUAL HEADER_ONLY)
		add_library("${name}" INTERFACE)
		target_include_directories("${name}" INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
	else()
		if(type STREQUAL SHARED)
			add_library("${name}_obj" OBJECT)
			add_library("${name}" SHARED "$<TARGET_OBJECTS:${name}_obj>")
		elseif(type STREQUAL EXECUTABLE)
			add_library("${name}_obj" OBJECT)
			add_executable("${name}" "$<TARGET_OBJECTS:${name}_obj>")
		else()
			message(FATAL_ERROR "unknown type: ${type}")
		endif()		
		target_include_directories("${name}_obj" PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
		message(DEBUG  "added include for ${name}_obj : ${CMAKE_CURRENT_LIST_DIR}/include")
	endif()
	add_executable("${name}_test")
	target_link_libraries("${name}_test" PUBLIC "${name}" GTest::gtest GTest::gtest_main)
	add_test(NAME "${name}_test" COMMAND "${name}_test")
endfunction()

function(llama_add_sources)
	llama_stack_top(target target)
	llama_stack_top(target_type target_type)
	foreach(ARG ${ARGN})
		target_sources("${target}_obj" PRIVATE "${CMAKE_CURRENT_LIST_DIR}/${ARG}")
		message(DEBUG "added include for ${target}_obj : ${CMAKE_CURRENT_LIST_DIR}/${ARG}")
	endforeach()
endfunction()

function(llama_add_test_sources)
	llama_stack_top(target target)
	llama_stack_top(target_type target_type)
	foreach(ARG ${ARGN})
		target_sources("${target}_test" PRIVATE "${CMAKE_CURRENT_LIST_DIR}/${ARG}")
		message(DEBUG  "added include for ${target}_test : ${CMAKE_CURRENT_LIST_DIR}/${ARG}")
	endforeach()
endfunction()

function(llama_add_link_libraries)
	llama_stack_top(target target)
	llama_stack_top(target_type target_type)
	if(target_type STREQUAL "HEADER_ONLY")
		foreach(ARG ${ARGN})
			target_link_libraries("${target}" INTERFACE "${ARG}")
			message(DEBUG "added link library for ${target} : ${ARG}")
		endforeach()
	else()
		foreach(ARG ${ARGN})
			target_link_libraries("${target}_obj" PUBLIC "${ARG}")
			message(DEBUG "added link library for ${target}_obj : ${ARG}")
		endforeach()
	endif()
endfunction()

function(llama_add_custom_deps)
	llama_stack_top(target target)
	llama_stack_top(target_type target_type)
	foreach(ARG ${ARGN})
		target_sources("${target}_obj" PRIVATE "${CMAKE_CURRENT_LIST_DIR}/${ARG}")
		message(DEBUG "added include for ${target}_obj : ${CMAKE_CURRENT_LIST_DIR}/${ARG}")
	endforeach()
endfunction()

function(llama_target_end)
	llama_stack_pop(target       _discard)
	llama_stack_pop(target_type  _discard)
endfunction()


# 语法：
#llama_custom_target(
#	OUTPUT path1...
#	INPUT 
#	COMMAND command1 [args1...] )
#	[COMMAND command2 [args1...]]
#	[WORKING_DIRECTORY dir]
#)
#llama_custom_target(
#	STAMP
#	INPUT 
#	COMMAND command1 [args1...] )
#	[COMMAND command2 [args1...]]
#	[WORKING_DIRECTORY dir]
#)
function(llama_custom_target)
	set(prefix           "ARG")
	set(options          "STAMP")
	set(one_value_args   "NAME" "WORKING_DIRECTORY")
	set(multi_value_args "OUTPUT")

	list(APPEND keywords "COMMAND;${options};${multi_value_args};${one_value_args}")
	foreach(ARG ${ARGN})
		if(ARG IN_LIST keywords)
			set(current_keyword "${ARG}")
		endif()

		if (current_keyword STREQUAL "COMMAND")
			list(APPEND command_args "${ARG}")
		else()
			list(APPEND other_args "${ARG}")
		endif()
	endforeach()

	if(NOT ARG_WORKING_DIRECTORY)
		set(ARG_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
	endif()

	message(DEBUG "command_args = ${command_args}")
	message(DEBUG "other_args = ${other_args}")

	cmake_parse_arguments(${prefix} "${options}" "${one_value_args}" "${multi_value_args}" ${other_args})
	
	if(ARG_STAMP)
		set(stamp_output_path "${CMAKE_CURRENT_BINARY_DIR}/${ARG_NAME}.stamp" )
		set(stamp_command     echo "edit me to trigger a rebuild of ${ARG_NAME}" > "${stamp_output_path}")
		list(APPEND ARG_OUTPUT "${stamp_output_path}")
	endif()

	add_custom_command(
		OUTPUT "${ARG_OUTPUT}"
		${command_args}
		${stamp_command}
		WORKING_DIRECTORY "${ARG_WORKING_DIRECTORY}"
		DEPENDS ALL
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM)
	
	add_custom_target(
		docs
		DEPENDS docs/docs.stamp 
	)
endfunction()


function(llama_docs)
	# Doxyfile.in的路径
	set(DOXYFILE_PATH "${PROJECT_SOURCE_DIR}/src/llama/docs/Doxyfile.in")
	
	# 源码目录，作为Doxygen的输入目录
	set(INPUT_DIR "${PROJECT_SOURCE_DIR}/src/llama" )
	
	# 输出目录
	set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/docs" )
	
	option(LLAMA_BUILD_DOCS "Build documentation" OFF)
	if(LLAMA_BUILD_DOCS)
		# 无法找到Doxygen。如果不希望生成文档，请在cmake命令行增加 -DLLAMA_BUILD_DOCS=OFF
		find_package(Doxygen REQUIRED)
		# DOXYFILE_PATH 文件中的 @INPUT_DIR@ @OUTPUT_DIR@ 会被替换成实际值
		# todo: 不要用这个鬼东西，，他会导致每次cmake都更新 Doxyfile，导致regen。
		configure_file("${DOXYFILE_PATH}" "${OUTPUT_DIR}/Doxyfile" @ONLY)
		add_custom_command(
			OUTPUT  "${OUTPUT_DIR}/docs.stamp"
			COMMAND "${DOXYGEN_EXECUTABLE}" "${OUTPUT_DIR}"
			COMMAND echo "edit me to trigger rebuild of the docs" > "${OUTPUT_DIR}/docs.stamp"
			WORKING_DIRECTORY "${OUTPUT_DIR}"
			DEPENDS ALL
			COMMENT "Generating API documentation with Doxygen"
			VERBATIM)
		add_custom_target(
			docs
			DEPENDS docs/docs.stamp 
		)
		set_property(CACHE CRYPTOBACKEND PROPERTY STRINGS
             "OpenSSL" "LibTomCrypt" "LibDES")
	endif()
endfunction()