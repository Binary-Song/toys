# 本文件使用 cmake_format 格式化： 安装步骤：(1) pip install cmake_format (2) VSCODE：安装拓展： cheshirekow.cmake-format 完成后按 ctrl+shift+f 即可格式化

# =============== 添加custom target必读 ===============
# https://cmake.org/cmake/help/latest/command/add_custom_command.html#example-generating-files-for-multiple-targets
# https://samthursfield.wordpress.com/2015/11/21/cmake-dependencies-between-targets-and-files-and-custom-commands/
# (这篇文章的图示中，虚线表示command、实线椭圆表示文件、方框表示target )
#
# 关于 add_dependency: https://stackoverflow.com/questions/75641864/cmake-target-does-not-build-when-its-dependency-target-builds
#
# Q: 为啥既要添加target-level dependency也要添加file-level dependency？ A: target-level 仅能确保 target
# 生成的先后顺序。不会自动认为一个target的输出文件就是另一个target的输入。所以 添加target-level依赖（把command包起来）是为了防止同时执行一个command导致竞争和浪费资源。
#
# file-level 依赖是为了确保所依赖的文件更新时，另一个target能自动执行。如果一个target没有任何文件依赖，则它仅会在自己的输出文件不存在时执行。 这样等于说这个文件只会生成一次，之后永远被认为最新而不再生成。
#
# 允许的源文件和头文件扩展名
set(LLAMA_SOURCE_EXTENSIONS
    "*.cpp" "*.c"
    CACHE INTERNAL "source file extensions that are allowed within the llama project")
set(LLAMA_HEADER_EXTENSIONS
    "*.hpp" "*.h"
    CACHE INTERNAL "header file extensions that are allowed within the llama project")
unset(llama_doc_sources_sp CACHE)
unset(llama_doc_sources_ls CACHE)

# 自动创建 llama 模块。“模块”包含多个自动创建的目标。
#
# module包括如下目标： (1) {name}-object 库，由src和include里的文件生成。 (2) ${name} 主库 链接 ${name}-object ， 它的源文件只有 dummy.cpp。 (3) ${name}-test
# 测试库，链接 ${name}-object ， 由test里的文件生成。

# 其中，主库 ${name} 根据 type 的值可以是静态库、动态库或者可执行程序。
#
# 【注意】各种属性（include dir、compile flag）设置在 ${name}-object 或 ${name}-if 上，并根据需要选择 PUBLIC 或 PRIVATE —— 设置为 PUBLIC 的影响 object 库和主库，设置为 PRIVATE 的只影响
# object 库自己。
# 如果不仅要设置 ${name}-object 和 ${name} 的属性，还要这个属性传递到下游的库，将属性设置在 ${name}-if 上。
#
# llama项目的组织原则是：用固定的文件树约定代替繁琐重复的cmake配置。所有目标都必须具有相同的目录结构： src - 目标的私有源文件和头文件目录：存放源文件和不让下游包含的头文件。 include - 目标的公有头文件。存放下游需要的头文件。
# test - 测试目标。作为主目标的一个下游目标，链接到主目标。必须存在。
#
# 在创建模块前，先用scripts/update_filelist.py生成源文件列表sources.cmake。 该文件应当和源码一起检入git。 必要参数： name - 模块的名称。模块对应的目标名以它为前缀。 type - 目标类型。目前支持的目标有
# HEADER_ONLY 、SHARED 和 EXECUTABLE
function(llama_module name type)
    cmake_parse_arguments(
        "ARG" # prefix
        "" # option
        "" # one-val
        "" # multi-val
        ${ARGN})
    
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    # 定义平台宏
    if(WIN32)
        list(APPEND PLAT_DEF "LLAMA_WIN")
    else()
        if(UNIX)
            list(APPEND PLAT_DEF "LLAMA_UNIX")
        endif()
        if(LINUX)
            list(APPEND PLAT_DEF "LLAMA_LINUX")
        endif()
    endif()

    # 验证项目结构的正确性，包括目录结构是不是存在等 p.s. CMAKE_CURRENT_LIST_DIR 是调用者的位置，不是本文件的位置
    llama_internal_verify_target("${name}" "${type}")

    # include 文件 source.cmake
    llama_internal_include_source_list()

    # llama_doc_sources_sp 和 llama_doc_sources_ls 都是 文档的输入文件列表，不过前者用空格切分，后者用分号
    foreach(src ${SOURCE_LIST})
        set(llama_doc_sources_sp
            "${CMAKE_CURRENT_LIST_DIR}/${src} $CACHE{llama_doc_sources_sp}"
            CACHE INTERNAL "space-separated list of source files that need to be documented")
        set(llama_doc_sources_ls
            "${CMAKE_CURRENT_LIST_DIR}/${src};$CACHE{llama_doc_sources_ls}"
            CACHE INTERNAL "semi-column-separated list of source files that need to be documented")
    endforeach()

    # 因为 cmake 要求 target至少要有一个源文件。所以弄一个凑数的。
    set(DUMMY_PATH "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/dummy.cpp")

    # 对所有type都创建 ${name}-object / ${name}-if (if = interface) 库
    add_library("${name}-object" OBJECT "${SOURCE_LIST}")
    add_library("${name}-if" INTERFACE)
    
    # 各个 type 不一样的逻辑写这里，记得只改 ${name}-object 的属性。其他target利用 PUBLIC 传递 预期后续不应该有判断 type 的逻辑。
    if(type STREQUAL SHARED)
        add_library("${name}" SHARED "${DUMMY_PATH}")
        target_compile_definitions("${name}-object" PUBLIC "LLAMA_${name}_EXPORT=1")
    elseif(type STREQUAL EXECUTABLE)
        add_executable("${name}" "${DUMMY_PATH}")
    elseif(type STREQUAL STATIC)
        add_library("${name}" STATIC "${DUMMY_PATH}")
    else()
        message(FATAL_ERROR "unknown type: ${type}")
    endif()

    # 关键target之间的连接
    target_link_libraries("${name}-object" PUBLIC "${name}-if")
    target_link_libraries("${name}" PUBLIC "${name}-if")
    target_link_libraries("${name}" PRIVATE "${name}-object")

    # 设置 库各种属性
    target_include_directories("${name}-if" INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
    target_include_directories("${name}-object" PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src")
    target_compile_definitions("${name}-object" PUBLIC "${PLAT_DEF}")
    if(MSVC)
        target_compile_options(${name}-object PRIVATE /W4)
    else()
        target_compile_options(${name}-object PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    target_link_libraries("${name}-if" INTERFACE fmt::fmt)

    # 设置 test 库各种属性
    add_executable("${name}-test" "${TEST_SOURCE_LIST}")
    target_link_libraries("${name}-test" PRIVATE "${name}" GTest::gtest GTest::gtest_main)
    add_test(NAME "${name}-test" COMMAND "${name}-test")

    # 处理 protobuf。创建自定义 target： ${name}-proto
    if(PROTO_LIST OR TEST_PROTO_LIST)
        llama_internal_add_protobuf_targets()

        add_dependencies("${name}-object" "${name}-proto")
        target_include_directories("${name}-if" INTERFACE "${proto_output_dir}")
        target_sources("${name}-object" PRIVATE "${proto_source_output_path}" "${proto_header_output_path}")
        target_link_libraries("${name}-object" PUBLIC protobuf::libprotobuf protobuf::libprotoc)
    endif()

endfunction()

# llama_link(<module> <PUBLIC|PRIVATE> [dependency1]...) 
# module: 由 llama_module 创建的模块名称
# dependency: target 名称，可以是三方库、模块等
function(llama_link module visibility)
    
    foreach(dependency ${ARGN})
        if(NOT module STREQUAL dependency) # 不许自己连接自己
            if(visibility STREQUAL PUBLIC)
                llama_internal_register_linkage(${module}-if INTERFACE ${dependency})
            elseif(visibility STREQUAL PRIVATE)
                llama_internal_register_linkage(${module}-object PUBLIC ${dependency})
            else()
                message(FATAL_ERROR "Unknown visibility")
            endif()
        endif()
    endforeach()
endfunction()

# 应用由 llama_link 指定的依赖关系。
function(llama_finalize)
    foreach(link_name ${LLAMA_LINKS})
        target_link_libraries(${LLAMA_LINK_MOD1_${link_name}} ${LLAMA_LINK_VIS_${link_name}} ${LLAMA_LINK_MOD2_${link_name}})
    endforeach()
endfunction()

function(llama_internal_register_linkage mod1 vis mod2)
    # e.g. link_name = foo-bar
    set(link_name "${mod1}-${mod2}")
    # e.g. LLAMA_LINKS += foo-bar
    set(LLAMA_LINKS "$CACHE{LLAMA_LINKS};${link_name}" CACHE INTERNAL "")
    # e.g. LLAMA_LINK_MOD1_foo-bar = foo
    set(LLAMA_LINK_MOD1_${link_name} "${mod1}" CACHE INTERNAL "")
    # e.g. LLAMA_LINK_VIS_foo-bar = PUBLIC
    set(LLAMA_LINK_VIS_${link_name}  "${vis}" CACHE INTERNAL "")
    # e.g. LLAMA_LINK_MOD2_foo-bar = bar
    set(LLAMA_LINK_MOD2_${link_name} "${mod2}" CACHE INTERNAL "")
endfunction()

function(llama_docs)
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs") # Doxygen 输出目录，文档输出在这里
    set(DOXYFILE_INPUT_PATH "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../docs/llama.doxyfile") # 输入的 doxyfile 模板
    set(DOXYFILE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/docs/Doxyfile") # 输出的 Doxyfile 文件，文件里的路径被替换了
    option(LLAMA_DOC_USE_STYLESHEET "Whether you want to use cool stylesheets for doc" ON)
    if(LLAMA_DOC_USE_STYLESHEET)
        set(DOXYGEN_STYLE_SHEET_PATH1 "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../docs/doc-css/doxygen-awesome.css")
        file(TO_CMAKE_PATH "${DOXYGEN_STYLE_SHEET_PATH1}" DOXYGEN_STYLE_SHEET_PATH1)
        set(DOXYGEN_STYLE_SHEET_PATH2 "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../docs/doc-css/doxygen-awesome-sidebar-only.css")
        file(TO_CMAKE_PATH "${DOXYGEN_STYLE_SHEET_PATH2}" DOXYGEN_STYLE_SHEET_PATH2)
        set(DOXYGEN_STYLE_SHEET_PATH "${DOXYGEN_STYLE_SHEET_PATH1} ${DOXYGEN_STYLE_SHEET_PATH2}")
    endif()
    string(REPLACE ";" " " "INPUT_FILE_PATTERNS" "${LLAMA_SOURCE_EXTENSIONS};${LLAMA_HEADER_EXTENSIONS}")
    file(MAKE_DIRECTORY "${OUTPUT_DIR}")

    option(LLAMA_BUILD_DOCS "Build documentation" OFF)

    if(LLAMA_BUILD_DOCS)
        message("Building docs")
        # 无法找到Doxygen。如果不希望生成文档，请在cmake命令行增加 -DLLAMA_BUILD_DOCS=OFF
        find_package(Doxygen REQUIRED)

        # 配置Doxyfile
        add_custom_command(
            OUTPUT "${DOXYFILE_OUTPUT_PATH}"
            COMMAND
                "${CMAKE_COMMAND}" "-DDOXYFILE_INPUT_PATH=${DOXYFILE_INPUT_PATH}" "-DDOXYFILE_OUTPUT_PATH=${DOXYFILE_OUTPUT_PATH}"
                "-DINPUT=$CACHE{llama_doc_sources_sp}" "-DOUTPUT_DIR=${OUTPUT_DIR}"
                "-DCLANG_DATABASE_PATH=${CMAKE_BINARY_DIR}/compile_commands.json"
                "-DDOXYGEN_STYLE_SHEET_PATH=${DOXYGEN_STYLE_SHEET_PATH}" "-P"
                "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/llama_configure_doxygen.cmake"
            WORKING_DIRECTORY "${OUTPUT_DIR}"
            DEPENDS "${DOXYFILE_INPUT_PATH}"
            COMMENT "Configuring doxyfile"
            VERBATIM)
        add_custom_target(docs-configure DEPENDS "${DOXYFILE_OUTPUT_PATH}")

        # DEPENDS 为指令的输入文件和目标，OUTPUT为输出文件
        add_custom_command(
            OUTPUT "${OUTPUT_DIR}/docs.stamp"
            COMMAND "${DOXYGEN_EXECUTABLE}"
            COMMAND echo "delete me to trigger a rebuild of the docs" > "${OUTPUT_DIR}/docs.stamp"
            COMMAND "${CMAKE_COMMAND}" -E create_symlink "${OUTPUT_DIR}/html/index.html" "${OUTPUT_DIR}/llama-documentation.html"
            WORKING_DIRECTORY "${OUTPUT_DIR}"
            DEPENDS "${DOXYFILE_OUTPUT_PATH}" "docs-configure" ${llama_doc_sources_ls} # add_custom_command 的 depend 可以是文件或者
                                                                                       # target
            COMMENT "Generating docs"
            VERBATIM)
        add_custom_target(docs ALL DEPENDS "${OUTPUT_DIR}/docs.stamp") # add_custom_target 的 depend 仅限文件且必须是同一个 CMakeLists.txt
        

    endif()
endfunction()

function(llama_auto_housekeep)
    if(LLAMA_AUTO_HOUSEKEEP)
        execute_process(COMMAND python "${PROJECT_SOURCE_DIR}/scripts/housekeep.py")
    endif()
endfunction()

macro(llama_internal_include_source_list)
    include("${CMAKE_CURRENT_LIST_DIR}/sources.cmake")
endmacro()

function(llama_internal_verify_dir)
    cmake_parse_arguments(
        "ARG" # prefix
        "" # options
        "PATH" # one-vals
        "MUST_CONTAIN" # multi-vals
        ${ARGN})
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
            message(
                FATAL_ERROR
                    "check on target '${name}' failed: directory ${path} does not contain files matching these patterns: ${ARG_MUST_CONTAIN}"
            )
        endif()
    endif()
endfunction()

function(llama_internal_verify_target name type)
    option(LLAMA_IGNORE_TARGET_VERIFICATION OFF)

    if(NOT CACHE{LLAMA_IGNORE_TARGET_VERIFICATION})
        if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/sources.cmake")
            message(
                FATAL_ERROR "${CMAKE_CURRENT_LIST_DIR}/sources.cmake does not exist\nuse scripts/housekeep.py to generate one.")
        endif()
        llama_internal_verify_dir(PATH "${CMAKE_CURRENT_LIST_DIR}/include")
        llama_internal_verify_dir(PATH "${CMAKE_CURRENT_LIST_DIR}/include/${name}" MUST_CONTAIN ${LLAMA_HEADER_EXTENSIONS})
        llama_internal_verify_dir(PATH "${CMAKE_CURRENT_LIST_DIR}/test" MUST_CONTAIN ${LLAMA_SOURCE_EXTENSIONS})

        if(type STREQUAL HEADER_ONLY)

        else()
            llama_internal_verify_dir(PATH "${CMAKE_CURRENT_LIST_DIR}/src" MUST_CONTAIN ${LLAMA_SOURCE_EXTENSIONS})
        endif()

        # 要求 file list 必须同步
        llama_internal_include_source_list()
        set(src "${SOURCE_LIST};${TEST_SOURCE_LIST}")
        foreach(ext ${LLAMA_SOURCE_EXTENSIONS} ${LLAMA_HEADER_EXTENSIONS})
            file(
                GLOB_RECURSE g FOLLOW_SYMLINKS
                RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
                "src/${ext}")
            list(APPEND actual_src "${g}")
            file(
                GLOB_RECURSE g FOLLOW_SYMLINKS
                RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
                "include/${ext}")
            list(APPEND actual_src "${g}")
            file(
                GLOB_RECURSE g FOLLOW_SYMLINKS
                RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
                "test/${ext}")
            list(APPEND actual_src "${g}")
        endforeach()

        llama_internal_normalize_list(src)
        llama_internal_normalize_list(actual_src)

        if(NOT src STREQUAL actual_src)
            message(
                WARNING
                    "sources.cmake for target '${name}' may not be up to date because it differs from the source tree.\nthe file: ${src}\nthe source tree: ${actual_src}\nuse scripts/housekeep.py to update it."
            )
        endif()

    endif()

endfunction()

function(llama_internal_normalize_list list)
    list(REMOVE_DUPLICATES "${list}")
    list(FILTER "${list}" EXCLUDE REGEX "^$")
    list(SORT "${list}")
    set("${list}"
        "${${list}}"
        PARENT_SCOPE)
endfunction()

macro(llama_internal_add_protobuf_targets)
    unset(proto_output_list)
    foreach(src ${TEST_PROTO_LIST} ${PROTO_LIST})
        set(proto_input_path "${CMAKE_CURRENT_LIST_DIR}/${src}")
        get_filename_component(proto_input_dir "${proto_input_path}" DIRECTORY)

        set(proto_output_dir "${CMAKE_CURRENT_BINARY_DIR}/proto/${src}/..")
        file(MAKE_DIRECTORY "${proto_output_dir}")

        get_filename_component(proto_input_fname_no_ext "${CMAKE_CURRENT_LIST_DIR}/${src}" NAME_WLE)
        set(proto_header_output_path "${proto_output_dir}/${proto_input_fname_no_ext}.pb.h")
        set(proto_source_output_path "${proto_output_dir}/${proto_input_fname_no_ext}.pb.cc")
        list(APPEND proto_output_list "${proto_header_output_path}" "${proto_source_output_path}")
        add_custom_command(
            OUTPUT "${proto_source_output_path}" "${proto_header_output_path}"
            COMMAND "${Protobuf_PROTOC_EXECUTABLE}" "-I=${proto_input_dir}" "--cpp_out=${proto_output_dir}" "${proto_input_path}"
            DEPENDS "${proto_input_path}"
            COMMENT "Running protoc"
            VERBATIM)
    endforeach()
    add_custom_target("${name}-proto" ALL DEPENDS "${proto_output_list}")
endmacro()
