cmake_minimum_required(VERSION 3.16)

project(velox-compiler
    VERSION 2026.1.1
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
set(SOFTWARE_NAME "velox-compiler")
string(TIMESTAMP SOFTWARE_VERSION "%Y-%m-%d")
set(SOFTWARE_VENDOR "CLOVIS")

# ----------------------------
# Sources
# ----------------------------
set(VELOX_SOURCES

src/ast/ast_base.hpp
src/ast/ast_declaration_extension.hpp
src/ast/ast_declaration_global.hpp
src/ast/ast_declaration_local.hpp
src/ast/ast_declaration_sfm.hpp
src/ast/ast_expression.hpp
src/ast/ast_generic.hpp
src/ast/ast_literal.hpp
src/ast/ast_memory.hpp
src/ast/ast_numeric_128_bits.cpp
src/ast/ast_numeric_128_bits.hpp
src/ast/ast_operation.hpp
src/ast/ast_statement.hpp
src/binder/binder_ffi.cpp
src/binder/binder_ffi.hpp
src/binder/ffi_c_file.hpp
src/binder/ffi_c_reader.cpp
src/binder/ffi_c_reader.hpp
src/binder/ffi_json_reader.cpp
src/binder/ffi_json_reader.hpp
src/codegen/codegen_tools.cpp
src/codegen/codegen_tools.hpp
src/codegen/llvm_forward.hpp
src/codegen/resolver_codegen.cpp
src/codegen/resolver_codegen.hpp
src/codegen/static_evaluation.cpp
src/codegen/static_evaluation.hpp
src/compiler/CLI11_type_seralization.hpp
src/compiler/compilation_unit.cpp
src/compiler/compilation_unit.hpp
src/compiler/compiler.cpp
src/compiler/compiler.hpp
src/compiler/main.cpp
src/compiler/parser_command.cpp
src/compiler/parser_command.hpp
src/misc/ast_printer.cpp
src/misc/ast_printer.hpp
src/misc/error_output.cpp
src/misc/error_output.hpp
src/misc/errors.hpp
src/nexus/ast/ast.cpp
src/nexus/ast/ast.hpp
src/nexus/ast/data.cpp
src/nexus/ast/data.hpp
src/nexus/ast/forward.hpp
src/nexus/forward.cpp
src/nexus/forward.hpp
src/nexus/ids.cpp
src/nexus/ids.hpp
src/nexus/implementation.hpp
src/nexus/inference.cpp
src/nexus/inference.hpp
src/nexus/lexer/lexer.cpp
src/nexus/lexer/lexer.hpp
src/nexus/lexer/stream_tracker.hpp
src/nexus/lexer/token.cpp
src/nexus/lexer/token.hpp
src/nexus/lexer/token_viewer.cpp
src/nexus/lexer/token_viewer.hpp
src/nexus/metacode/metacode.cpp
src/nexus/metacode/metacode.hpp
src/nexus/metacode/preprocessor.cpp
src/nexus/metacode/preprocessor.hpp
src/nexus/metacode/token_generator.cpp
src/nexus/metacode/token_generator.hpp
src/nexus/module.cpp
src/nexus/module.hpp
src/nexus/pipeline.cpp
src/nexus/pipeline.hpp
src/nexus/resolved.cpp
src/nexus/resolved.hpp
src/nexus/scope.cpp
src/nexus/scope.hpp
src/nexus/symbol.cpp
src/nexus/symbol.hpp
src/nexus/type/forward.hpp
src/nexus/type/type.cpp
src/nexus/type/type.hpp
src/nexus/unresolved.hpp
src/parser/parser_base.cpp
src/parser/parser_base.hpp
src/parser/parser_context.cpp
src/parser/parser_context.hpp
src/parser/parser_declaration_extension.cpp
src/parser/parser_declaration_extension.hpp
src/parser/parser_declaration_global.cpp
src/parser/parser_declaration_global.hpp
src/parser/parser_declaration_local.cpp
src/parser/parser_declaration_local.hpp
src/parser/parser_declaration_sfm.cpp
src/parser/parser_declaration_sfm.hpp
src/parser/parser_expression.cpp
src/parser/parser_expression.hpp
src/parser/parser_literal.cpp
src/parser/parser_literal.hpp
src/parser/parser_memory.cpp
src/parser/parser_memory.hpp
src/parser/parser_operation.cpp
src/parser/parser_operation.hpp
src/parser/parser_statement.cpp
src/parser/parser_statement.hpp
src/parser/parser_type.cpp
src/parser/parser_type.hpp
src/resolver/resolver_base.cpp
src/resolver/resolver_base.hpp
src/resolver/resolver_inference.cpp
src/resolver/resolver_inference.hpp
src/resolver/resolver_semantic.cpp
src/resolver/resolver_semantic.hpp
src/resolver/resolver_symbol.cpp
src/resolver/resolver_symbol.hpp
src/rules/rule_type.cpp
src/rules/rule_type.hpp

)

# add velox-common scripts
# for the compiler data interface
add_subdirectory(../velox-common velox-common)

add_executable(velox-compiler 
${VELOX_SOURCES}
)

target_include_directories(velox-compiler
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)


target_link_libraries(velox-compiler
    PRIVATE velox-common
)

# ----------------------------
# Vendor (header-only)
# ----------------------------
add_library(velox_vendor INTERFACE)

target_include_directories(velox_vendor
    INTERFACE
        SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/vendor"
)

target_link_libraries(velox-compiler
    PRIVATE velox_vendor
)

# ----------------------------
# LLVM / Clang
# ----------------------------
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

target_include_directories(velox-compiler
    PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
)

target_compile_definitions(velox-compiler
    PRIVATE
        ${LLVM_DEFINITIONS}
        $<$<CONFIG:Debug>:DEBUG>
)

target_compile_options(velox-compiler
    PRIVATE
        -Wno-unused-parameter
)

# ----------------------------
# LLVM linking 
# ----------------------------
find_package(LLVM REQUIRED CONFIG)

target_include_directories(velox-compiler PRIVATE
    ${LLVM_INCLUDE_DIRS}
)

target_compile_definitions(velox-compiler PRIVATE
    ${LLVM_DEFINITIONS}
)

target_link_libraries(velox-compiler PRIVATE
    LLVM-19
    clang-19
)

# define VELOX_CODE_VALID on debug mode, useful to detect a bad compiler behaviour
target_compile_definitions(velox-compiler PRIVATE
    $<$<CONFIG:Debug>:VELOX_CODE_VALID>
)


# ----------------------------
# Output naming (optionnel)
# ----------------------------
set_target_properties(velox-compiler PROPERTIES
    OUTPUT_NAME "${SOFTWARE_NAME}"
)
