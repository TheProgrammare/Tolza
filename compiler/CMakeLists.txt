cmake_minimum_required(VERSION 3.28)

project(tolza-compiler
    VERSION 2026.8
    LANGUAGES C CXX
)

# ----------------------------
# Global options
# ----------------------------
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# ----------------------------
# Project metadata
# ----------------------------
set(SOFTWARE_NAME "tolza-compiler")
string(TIMESTAMP SOFTWARE_VERSION "%Y-%m-%d")
set(SOFTWARE_VENDOR "CLOVIS")

# ----------------------------
# Sources
# ----------------------------
set(TOLZA_SOURCES

./src/ast/data.hpp
./src/ast/dumper.cpp
./src/ast/dumper.hpp
./src/ast/forward.hpp
./src/ast/node/base.hpp
./src/ast/node/declaration_extension.hpp
./src/ast/node/declaration_global.hpp
./src/ast/node/declaration_local.hpp
./src/ast/node/declaration_sfm.hpp
./src/ast/node/expression.hpp
./src/ast/node/generic.hpp
./src/ast/node/literal.hpp
./src/ast/node/numeric_128_bits.cpp
./src/ast/node/numeric_128_bits.hpp
./src/ast/node/operation.hpp
./src/ast/node/statement.hpp
./src/ast/pool.cpp
./src/ast/pool.hpp
./src/ast/tool.cpp
./src/ast/tool.hpp
./src/codegen/codegen.cpp
./src/codegen/codegen.hpp
./src/codegen/debug_info.cpp
./src/codegen/debug_info.hpp
./src/codegen/eval.cpp
./src/codegen/eval.hpp
./src/codegen/insurance.cpp
./src/codegen/insurance.hpp
./src/codegen/llvm_forward.hpp
./src/codegen/tool.cpp
./src/codegen/tool.hpp
./src/codegen/type.cpp
./src/codegen/type.hpp
./src/compiler/compilation_unit.cpp
./src/compiler/compilation_unit.hpp
./src/compiler/compiler.cpp
./src/compiler/compiler.hpp
./src/compiler/file_info.cpp
./src/compiler/file_info.hpp
./src/compiler/io.cpp
./src/compiler/io.hpp
./src/compiler/main.cpp
./src/compiler/parser_command.cpp
./src/compiler/parser_command.hpp
./src/ffi/c_reader.cpp
./src/ffi/c_reader.hpp
./src/ffi/json_reader.cpp
./src/ffi/json_reader.hpp
./src/ffi/manager.cpp
./src/ffi/manager.hpp
./src/id/base.cpp
./src/id/base.hpp
./src/id/cuid.cpp
./src/id/cuid.hpp
./src/id/defid.cpp
./src/id/defid.hpp
./src/id/id_query.cpp
./src/id/id_query.hpp
./src/id/metaid.hpp
./src/id/modid.cpp
./src/id/modid.hpp
./src/id/nodeid.cpp
./src/id/nodeid.hpp
./src/id/scpid.cpp
./src/id/scpid.hpp
./src/id/semid.hpp
./src/id/tokid.cpp
./src/id/tokid.hpp
./src/id/typeid.cpp
./src/id/typeid.hpp
./src/lexer/data.hpp
./src/lexer/lexer.cpp
./src/lexer/lexer.hpp
./src/lexer/pool.cpp
./src/lexer/pool.hpp
./src/lexer/stream_tracker.hpp
./src/lexer/token_viewer.cpp
./src/lexer/token_viewer.hpp
./src/metacode/data.cpp
./src/metacode/data.hpp
./src/metacode/pool.cpp
./src/metacode/pool.hpp
./src/metacode/preprocessor.cpp
./src/metacode/preprocessor.hpp
./src/metacode/token_generator.cpp
./src/metacode/token_generator.hpp
./src/misc/error_output.cpp
./src/misc/error_output.hpp
./src/misc/notification/notification.cpp
./src/misc/notification/notification.hpp
./src/misc/notification/win.hpp
./src/module/module.cpp
./src/module/module.hpp
./src/module/pool.cpp
./src/module/pool.hpp
./src/module/tool.cpp
./src/module/tool.hpp
./src/nexus/forward.cpp
./src/nexus/forward.hpp
./src/parser/base.cpp
./src/parser/base.hpp
./src/parser/context.cpp
./src/parser/context.hpp
./src/parser/declaration_extension.cpp
./src/parser/declaration_extension.hpp
./src/parser/declaration_global.cpp
./src/parser/declaration_global.hpp
./src/parser/declaration_local.cpp
./src/parser/declaration_local.hpp
./src/parser/declaration_sfm.cpp
./src/parser/declaration_sfm.hpp
./src/parser/expression.cpp
./src/parser/expression.hpp
./src/parser/literal.cpp
./src/parser/literal.hpp
./src/parser/operation.cpp
./src/parser/operation.hpp
./src/parser/recovery.cpp
./src/parser/recovery.hpp
./src/parser/statement.cpp
./src/parser/statement.hpp
./src/parser/type.cpp
./src/parser/type.hpp
./src/pipeline/codegen.cpp
./src/pipeline/codegen.hpp
./src/pipeline/configurator.cpp
./src/pipeline/configurator.hpp
./src/pipeline/linker.cpp
./src/pipeline/linker.hpp
./src/pipeline/module_resolver.cpp
./src/pipeline/module_resolver.hpp
./src/pipeline/pipeline.cpp
./src/pipeline/pipeline.hpp
./src/pipeline/preparer.cpp
./src/pipeline/preparer.hpp
./src/pipeline/resolver.cpp
./src/pipeline/resolver.hpp
./src/pool/node_resolved.cpp
./src/pool/node_resolved.hpp
./src/pool/node_to_def.cpp
./src/pool/node_to_def.hpp
./src/pool/node_to_eval.hpp
./src/pool/node_to_ext.cpp
./src/pool/node_to_ext.hpp
./src/pool/node_to_inf.cpp
./src/pool/node_to_inf.hpp
./src/pool/node_to_metadata.hpp
./src/pool/node_unresolved.hpp
./src/pool/stable_storage.hpp
./src/resolver/base.cpp
./src/resolver/base.hpp
./src/resolver/dumper.cpp
./src/resolver/dumper.hpp
./src/resolver/eval.cpp
./src/resolver/eval.hpp
./src/resolver/evaluable.cpp
./src/resolver/evaluable.hpp
./src/resolver/inference.cpp
./src/resolver/inference.hpp
./src/resolver/semantic.cpp
./src/resolver/semantic.hpp
./src/resolver/symbol.cpp
./src/resolver/symbol.hpp
./src/scope/pool.cpp
./src/scope/pool.hpp
./src/scope/scope.cpp
./src/scope/scope.hpp
./src/type/data.hpp
./src/type/dumper.cpp
./src/type/dumper.hpp
./src/type/forward.hpp
./src/type/pool.cpp
./src/type/pool.hpp
./src/type/rule.cpp
./src/type/rule.hpp
./src/type/tool.cpp
./src/type/tool.hpp
./src/type/type.hpp


)

# add tolza-common scripts
# for the compiler data interface
add_subdirectory(../common tolza-common)

add_executable(tolza-compiler)

target_precompile_headers(tolza-compiler
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/pch.hpp"
)

# version
string(TIMESTAMP SOFTWARE_VERSION "%Y-%m-%d")
string(TIMESTAMP TOLZA_VERSION "%Y-%m")

target_compile_definitions(tolza-compiler
    PUBLIC
        SOFTWARE_NAME="tolza-toolchain"
        SOFTWARE_VERSION="${SOFTWARE_VERSION}"
        TOLZA_VERSION="${TOLZA_VERSION}"
)

target_sources(tolza-compiler
    PRIVATE
        ${TOLZA_SOURCES}
)

target_include_directories(tolza-compiler
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(tolza-compiler
    PRIVATE tolza-common
)



# ----------------------------
# Vendor (header-only)
# ----------------------------
add_library(tolza_vendor INTERFACE)

target_include_directories(tolza_vendor
    SYSTEM INTERFACE 
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor"
)

target_link_libraries(tolza-compiler
    PRIVATE tolza_vendor
)

# ----------------------------
# LLVM / Clang
# ----------------------------
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

target_include_directories(tolza-compiler
    PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
)

target_compile_definitions(tolza-compiler
    PRIVATE
        ${LLVM_DEFINITIONS}
        $<$<CONFIG:Debug>:NO_DEBUG>
)

target_compile_options(tolza-compiler
    PRIVATE
        -Wno-unused-parameter
)

# ----------------------------
# LLVM linking 
# ----------------------------
find_package(LLVM REQUIRED CONFIG)

target_include_directories(tolza-compiler PRIVATE
    ${LLVM_INCLUDE_DIRS}
)

target_compile_definitions(tolza-compiler PRIVATE
    ${LLVM_DEFINITIONS}
)

target_link_libraries(tolza-compiler PRIVATE
    LLVM-19
    clang-19
)

# define TOLZA_CODE_VALID on debug mode, useful to detect a bad compiler behaviour
target_compile_definitions(tolza-compiler PRIVATE
    $<$<CONFIG:Debug>:TOLZA_CODE_VALID>
)


# ----------------------------
# Output naming (optionnal)
# ----------------------------
set_target_properties(tolza-compiler PROPERTIES
    OUTPUT_NAME "${SOFTWARE_NAME}"
)

include(GNUInstallDirs)

install(
    TARGETS tolza-compiler
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install public headers, if any
# install(
#     DIRECTORY include/
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )
