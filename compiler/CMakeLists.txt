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
    ./src/compiler/CLI11_type_serialization.hpp
    ./src/compiler/parser_command.cpp
    ./src/compiler/io.hpp
    ./src/compiler/compilation_unit.cpp
    ./src/compiler/parser_command.hpp
    ./src/compiler/compiler.cpp
    ./src/misc/error_output.cpp
    ./src/misc/notification/notification.hpp
    ./src/misc/notification/win.cpp
    ./src/misc/notification/notification.cpp
    ./src/misc/error_output.hpp
    ./src/misc/ast_printer.cpp
    ./src/misc/ast_printer.hpp
    ./src/parser/parser_expression.cpp
    ./src/parser/parser_operation.hpp
    ./src/parser/parser_base.cpp
    ./src/parser/parser_base.hpp
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
    ./src/codegen/static_evaluation.hpp
    ./src/codegen/codegen_type.hpp
    ./src/codegen/static_evaluation.cpp
    ./src/codegen/codegen.hpp
    ./src/codegen/codegen_tools.cpp
    ./src/codegen/codegen_tools.hpp
    ./src/codegen/llvm_forward.hpp
    ./src/codegen/codegen.cpp
    ./src/nexus/inference.hpp
    ./src/nexus/type/data.hpp
    ./src/nexus/type/definition.hpp
    ./src/nexus/type/rule.cpp
    ./src/nexus/type/type.hpp
    ./src/nexus/type/rule.hpp
    ./src/nexus/type/type.cpp
    ./src/nexus/type/forward.hpp
    ./src/nexus/scope.hpp
    ./src/nexus/definition.hpp
    ./src/nexus/lexer/token_viewer.cpp
    ./src/nexus/lexer/token_viewer.hpp
    ./src/nexus/lexer/token.cpp
    ./src/nexus/lexer/token.hpp
    ./src/nexus/lexer/stream_tracker.hpp
    ./src/nexus/lexer/lexer.hpp
    ./src/nexus/lexer/lexer.cpp
    ./src/nexus/forward.cpp
    ./src/nexus/inference.cpp
    ./src/nexus/module.hpp
    ./src/nexus/ids.cpp
    ./src/nexus/definition.cpp
    ./src/nexus/ids.hpp
    ./src/nexus/resolved.cpp
    ./src/nexus/ast/ast.hpp
    ./src/nexus/ast/data.cpp
    ./src/nexus/ast/data.hpp
    ./src/nexus/ast/definition.hpp
    ./src/nexus/ast/ast.cpp
    ./src/nexus/ast/forward.hpp
    ./src/nexus/forward.hpp
    ./src/nexus/module.cpp
    ./src/nexus/scope.cpp
    ./src/nexus/metacode/metacode.hpp
    ./src/nexus/metacode/preprocessor.hpp
    ./src/nexus/metacode/definition.hpp
    ./src/nexus/metacode/definition.cpp
    ./src/nexus/metacode/metacode.cpp
    ./src/nexus/metacode/token_generator.hpp
    ./src/nexus/metacode/preprocessor.cpp
    ./src/nexus/metacode/token_generator.cpp
    ./src/nexus/unresolved.hpp
    ./src/nexus/extension.hpp
    ./src/nexus/extension.cpp
    ./src/nexus/resolved.hpp
    ./src/pipeline/pipeline.cpp
    ./src/pipeline/linker.hpp
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
    ./src/resolver/resolver_base.cpp
    ./src/resolver/resolver_base.hpp
    ./src/resolver/resolver_inference.cpp
    ./src/resolver/resolver_symbol.hpp
    ./src/resolver/resolver_semantic.cpp
    ./src/resolver/resolver_symbol.cpp
    ./src/resolver/resolver_dump.cpp
    ./src/resolver/resolver_semantic.hpp
    ./src/resolver/resolver_inference.hpp
    ./src/resolver/resolver_dump.hpp
    ./src/ast/ast_numeric_128_bits.cpp
    ./src/ast/ast_literal.hpp
    ./src/ast/ast_expression.hpp
    ./src/ast/ast_statement.hpp
    ./src/ast/ast_declaration_local.hpp
    ./src/ast/ast_operation.hpp
    ./src/ast/ast_declaration_extension.hpp
    ./src/ast/ast_declaration_global.hpp
    ./src/ast/ast_declaration_sfm.hpp
    ./src/ast/ast_memory.hpp
    ./src/ast/ast_numeric_128_bits.hpp
    ./src/ast/ast_base.hpp
    ./src/ast/ast_generic.hpp
    ./src/binder/binder_ffi.cpp
    ./src/binder/ffi_json_reader.cpp
    ./src/binder/ffi_c_reader.cpp
    ./src/binder/binder_ffi.hpp
    ./src/binder/ffi_json_reader.hpp
    ./src/binder/ffi_c_reader.hpp

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
