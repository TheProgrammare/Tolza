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
    

./src/compiler/compilation_unit.hpp
./src/compiler/compiler.hpp
./src/compiler/main.cpp
./src/compiler/parser_command.cpp
./src/compiler/io.hpp
./src/compiler/compilation_unit.cpp
./src/compiler/parser_command.hpp
./src/compiler/compiler.cpp
./src/misc/error_output.cpp
./src/misc/notification/notification.hpp
./src/misc/notification/win.hpp
./src/misc/notification/notification.cpp
./src/misc/error_output.hpp
./src/misc/ast_printer.cpp
./src/misc/ast_printer.hpp
./src/type/data.hpp
./src/type/definition.hpp
./src/type/rule.cpp
./src/type/rule.hpp
./src/type/forward.hpp
./src/lexer/token_viewer.cpp
./src/lexer/definition.hpp
./src/lexer/token_viewer.hpp
./src/lexer/stream_tracker.hpp
./src/lexer/lexer.hpp
./src/lexer/lexer.cpp
./src/parser/parser_expression.cpp
./src/parser/parser_operation.hpp
./src/parser/parser_base.cpp
./src/parser/parser_base.hpp
./src/parser/parser_recover.hpp
./src/parser/parser_context.cpp
./src/parser/parser_declaration_extension.hpp
./src/parser/parser_declaration_global.hpp
./src/parser/parser_memory.cpp
./src/parser/parser_declaration_sfm.hpp
./src/parser/parser_context.hpp
./src/parser/parser_statement.hpp
./src/parser/parser_declaration_local.cpp
./src/parser/parser_literal.hpp
./src/parser/parser_declaration_local.hpp
./src/parser/parser_declaration_global.cpp
./src/parser/parser_statement.cpp
./src/parser/parser_memory.hpp
./src/parser/parser_recover.cpp
./src/parser/parser_declaration_extension.cpp
./src/parser/parser_operation.cpp
./src/parser/parser_type.cpp
./src/parser/parser_literal.cpp
./src/parser/parser_declaration_sfm.cpp
./src/parser/parser_expression.hpp
./src/parser/parser_type.hpp
./src/codegen/codegen_insurance.hpp
./src/codegen/codegen_insurance.cpp
./src/codegen/codegen_type.cpp
./src/codegen/debug_info.hpp
./src/codegen/static_evaluation.hpp
./src/codegen/codegen_type.hpp
./src/codegen/static_evaluation.cpp
./src/codegen/codegen.hpp
./src/codegen/codegen_tools.cpp
./src/codegen/codegen_tools.hpp
./src/codegen/debug_info.cpp
./src/codegen/llvm_forward.hpp
./src/codegen/codegen.cpp
./src/nexus/forward.cpp
./src/nexus/ids.cpp
./src/nexus/ids.hpp
./src/nexus/forward.hpp
./src/pipeline/configurator.cpp
./src/pipeline/pipeline.cpp
./src/pipeline/linker.hpp
./src/pipeline/configurator.hpp
./src/pipeline/module_resolver.cpp
./src/pipeline/pipeline.hpp
./src/pipeline/linker.cpp
./src/pipeline/codegen.hpp
./src/pipeline/module_resolver.hpp
./src/pipeline/resolver.hpp
./src/pipeline/preparer.cpp
./src/pipeline/preparer.hpp
./src/pipeline/codegen.cpp
./src/pipeline/resolver.cpp
./src/pool/metacode.hpp
./src/pool/link/evaluated.hpp
./src/pool/link/inference.hpp
./src/pool/link/definition.hpp
./src/pool/link/inference.cpp
./src/pool/link/definition.cpp
./src/pool/link/resolved.cpp
./src/pool/link/semantic_metadata.hpp
./src/pool/link/extension.hpp
./src/pool/link/extension.cpp
./src/pool/link/resolved.hpp
./src/pool/ast.hpp
./src/pool/scope.hpp
./src/pool/token.cpp
./src/pool/module.hpp
./src/pool/metacode.cpp
./src/pool/type.hpp
./src/pool/token.hpp
./src/pool/type.cpp
./src/pool/ast.cpp
./src/pool/module.cpp
./src/pool/scope.cpp
./src/pool/unresolved.hpp
./src/resolver/resolver_base.cpp
./src/resolver/resolver_eval.cpp
./src/resolver/resolver_base.hpp
./src/resolver/resolver_inference.cpp
./src/resolver/resolver_symbol.hpp
./src/resolver/resolver_semantic.cpp
./src/resolver/resolver_symbol.cpp
./src/resolver/resolver_dump.cpp
./src/resolver/resolver_semantic.hpp
./src/resolver/resolver_eval.hpp
./src/resolver/resolver_inference.hpp
./src/resolver/resolver_dump.hpp
./src/ast/definition/ast_numeric_128_bits.cpp
./src/ast/definition/ast_literal.hpp
./src/ast/definition/ast_expression.hpp
./src/ast/definition/ast_statement.hpp
./src/ast/definition/ast_declaration_local.hpp
./src/ast/definition/ast_operation.hpp
./src/ast/definition/ast_declaration_extension.hpp
./src/ast/definition/ast_declaration_global.hpp
./src/ast/definition/ast_declaration_sfm.hpp
./src/ast/definition/ast_memory.hpp
./src/ast/definition/ast_numeric_128_bits.hpp
./src/ast/definition/ast_base.hpp
./src/ast/definition/ast_generic.hpp
./src/ast/data.cpp
./src/ast/data.hpp
./src/ast/definition.hpp
./src/ast/forward.hpp
./src/binder/binder_ffi.cpp
./src/binder/ffi_json_reader.cpp
./src/binder/ffi_c_reader.cpp
./src/binder/binder_ffi.hpp
./src/binder/ffi_json_reader.hpp
./src/binder/ffi_c_reader.hpp
./src/metacode/preprocessor.hpp
./src/metacode/definition.hpp
./src/metacode/definition.cpp
./src/metacode/token_generator.hpp
./src/metacode/preprocessor.cpp
./src/metacode/token_generator.cpp


)

# add tolza-common scripts
# for the compiler data interface
add_subdirectory(../common tolza-common)

add_executable(tolza-compiler)

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
