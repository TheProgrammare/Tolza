/*
 * =============================================================================
 * The Tolza programming language (2026.1.1) - Apache License, Version 2.0
 * Copyright 2024-2026 Foz Florian
 * =============================================================================
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * =============================================================================
 */

#pragma once

#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <clang-c/Index.h>

#include "binder/binder_ffi.hpp"
#include <common/compiler_options.hpp>
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"


namespace ffi
{

struct C_Reader {
  C_Reader();

  [[nodiscard]] ast::ID c_nodecl_to_opaque_facet(CXCursor cur) noexcept;
  [[nodiscard]] ast::ID c_struct_to_facet(CXCursor cur) noexcept;
  [[nodiscard]] ast::ID c_function_to_func(CXCursor cur) noexcept;
  [[nodiscard]] ast::ID c_union_to_union(CXCursor cur) noexcept;
  [[nodiscard]] ast::ID c_enum_to_flag(CXCursor cur) noexcept;
  [[nodiscard]] ast::ID c_global_to_global(CXCursor cur) noexcept;

  [[nodiscard]] static CXType                   c_type_normalize(CXType t) noexcept;
  [[nodiscard]] static type::EPrimitiveTypeKind c_type_base_to_primitive(CXType t) noexcept;
  [[nodiscard]] static ast::EPassMode           c_type_to_pass_mode(CXType input) noexcept;

  [[nodiscard]] type::ID        c_type_to_type(CXType t) noexcept;
  [[nodiscard]] type::ID        c_type_resolve_ptr(CXType input) noexcept;
  [[nodiscard]] type::ID        c_type_resolve_atomic(CXType input) noexcept;
  [[nodiscard]] type::ID        c_type_resolve_array(CXType input, type::Qualifier& dec) noexcept;
  [[nodiscard]] type::ID        c_type_resolve_proto(CXType input, type::Qualifier& dec) noexcept;
  [[nodiscard]] static type::ID c_type_resolve_primitive(CXType input) noexcept;
  [[nodiscard]] type::ID        c_type_resolve_typedef(CXCursor decl, type::Qualifier& dec) noexcept;
  [[nodiscard]] type::ID        c_type_resolve_opaque(CXCursor decl, type::Qualifier& dec) noexcept;
  [[nodiscard]] type::ID        c_type_resolve_struct(CXCursor decl, type::Qualifier& dec) noexcept;
  [[nodiscard]] type::ID        c_type_resolve_union(CXCursor decl, type::Qualifier& dec) noexcept;
  [[nodiscard]] type::ID        c_type_resolve_enum(CXCursor decl, type::Qualifier& dec) noexcept;

  std::set<std::string> definitions_generated;

  static void generate_libc_wrappers() noexcept;

  /**
   * @param from can be a file or a directory
   * @param to can be a file or a directory
   * @param unique_alias only works on barrel when 'from' and 'to' parameters are directories
   *
   * 'from' and 'to' parameters must be both at the same time a file or a directory.
   * If 'from' and 'to' parameters are a file, the module name will be the 'to' file name
   */
  void generate_c_api_wrappers(std::string_view from, std::string_view to, std::string_view unique_alias = {}) noexcept;

  [[nodiscard]] static std::unique_ptr<ffi::AST>
  parse_c_compilation_unit(std::string_view p_file_path, const std::vector<std::string>& p_args = {}) noexcept;

private:
  std::unique_ptr<ffi::AST> current_ast = nullptr;
  cu::CU&                   current_cu;
};

CXChildVisitResult c_universal_visitor(CXCursor p_cursor, CXCursor p_parent, CXClientData p_client_data) noexcept;

struct BindManifest {
  bool is_valid = false;

  std::string binding_language;

  common::compiler::TargetTriple triple;

  common::env::EArch     target_arch;
  common::env::EPlatform target_os;
  common::env::EABI      target_abi;

  common::env::ELibC                  clang_libc;
  common::compiler::Cffi::LibCVersion clang_libc_version;

  std::string compiler_clang_version;

  bool   features_gnu_source       = false;
  bool   features_posix_c_source   = false;
  size_t features_file_offset_bits = 64;
  size_t features_time_bits        = 64;

  std::string sysroot_path;
  std::string sysroot_hash;

  [[nodiscard]] static BindManifest read_manifest(std::string_view path) noexcept;
  [[nodiscard]] bool                write_manifest(std::string_view path) const noexcept;

  [[nodiscard]] std::vector<std::string> to_clang_args() const noexcept;
};

constexpr std::initializer_list<std::string_view> HEADERS_C_ISO = {
    "stdio.h",  "stdlib.h", "string.h", "ctype.h",  "math.h",   "time.h",     "errno.h",
    "limits.h", "float.h",  "assert.h", "stddef.h", "stdint.h", "inttypes.h", "stdarg.h",
    "signal.h", "setjmp.h", "locale.h", "wchar.h",  "wctype.h"};


constexpr std::initializer_list<std::string_view> HEADERS_POSIX = {
    "unistd.h", "fcntl.h",      "sys/types.h",  "sys/stat.h",  "sys/time.h",  "sys/wait.h",
    "dirent.h", "poll.h",       "sys/mman.h",   "pthread.h",   "semaphore.h", "sched.h",
    "signal.h", "sys/socket.h", "netinet/in.h", "arpa/inet.h", "sys/select.h"};


constexpr std::initializer_list<std::string_view> HEADERS_LINUX = {"sys/ioctl.h", "sys/epoll.h",    "sys/eventfd.h",
                                                                   "sys/prctl.h", "linux/limits.h", "linux/futex.h"};


constexpr std::initializer_list<std::string_view> HEADERS_FREEBSD = {"sys/sysctl.h", "sys/event.h", "sys/param.h",
                                                                     "sys/uio.h"};

constexpr std::initializer_list<std::string_view> HEADERS_OPENBSD = {"sys/sysctl.h", "sys/event.h", "sys/param.h"};

constexpr std::initializer_list<std::string_view> HEADERS_NETBSD = {"sys/sysctl.h", "sys/event.h", "sys/param.h"};


constexpr std::initializer_list<std::string_view> HEADERS_APPLE_BSD = {"sys/event.h", "sys/sysctl.h"};

constexpr std::initializer_list<std::string_view> HEADERS_APPLE_POSIX = {"unistd.h",   "fcntl.h",  "sys/types.h",
                                                                         "sys/stat.h", "dirent.h", "sys/time.h"};

constexpr std::initializer_list<std::string_view> HEADERS_APPLE_MACH = {"mach/mach.h", "mach/task.h",
                                                                        "mach/thread_act.h"};

constexpr std::initializer_list<std::string_view> HEADERS_APPLE_FRAMEWORKS = {"CoreFoundation/CoreFoundation.h"};


constexpr std::initializer_list<std::string_view> HEADERS_DRAGONFLYBSD = {"sys/sysctl.h", "sys/event.h"};


constexpr std::initializer_list<std::string_view> HEADERS_WINDOWS_CORE = {
    "windows.h",  "winnt.h",     "processthreadsapi.h", "fileapi.h",     "handleapi.h",
    "synchapi.h", "memoryapi.h", "errhandlingapi.h",    "libloaderapi.h"};

constexpr std::initializer_list<std::string_view> HEADERS_WINDOWS_SOCKET = {"winsock2.h", "ws2tcpip.h"};

constexpr std::initializer_list<std::string_view> HEADERS_WINDOWS_ADVANCED = {"tlhelp32.h", "psapi.h"};


constexpr std::initializer_list<std::string_view> HEADERS_ANDROID = {
    // inherits Linux + Bionic specifics
    "android/api-level.h", "sys/rule_properties.h"};


constexpr std::initializer_list<std::string_view> HEADERS_SOLARIS = {"sys/types.h", "sys/stat.h", "unistd.h",
                                                                     "procfs.h"};


} // namespace ffi