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

function(llama_target_end)
	llama_stack_pop(target       _discard)
	llama_stack_pop(target_type  _discard)
endfunction()

function(llama_docs DOXYFILE_PATH)
	option(LLAMA_BUILD_DOCS "Build documentation" OFF)
	if(LLAMA_BUILD_DOCS)
		
		find_package(Doxygen)
		if (DOXYGEN_FOUND)
			set(DOXYGEN_IN "${DOXYFILE_PATH}")
			set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

			# request to configure the file
			configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
			message("Doxygen build started")

			# note the option ALL which allows to build the docs together with the application
			add_custom_target( doc_doxygen ALL
				COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
				COMMENT "Generating API documentation with Doxygen"
				VERBATIM )
		else (DOXYGEN_FOUND)
		message("Doxygen need to be installed to generate the doxygen documentation")
		endif (DOXYGEN_FOUND)

	endif()
endfunction()