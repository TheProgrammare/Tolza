#pragma once

#include "id/nodeid.hpp"
#include "nexus/forward.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace llvm
{
class Module;
}


namespace meta
{
struct Manager;
} // namespace meta


namespace cu
{

enum class EFileSource : uint8_t {
  src,    // user scripts
  vendor, // 3rd party scripts
  std,    // standard library
  pkg,    // package library
  bind,   // binding library
  self,   // relative script
};

struct FileInfo;

struct CU {
  CU();
  ~CU();

  // file cu
  explicit CU(cu::ID _parent_cuid, cu::ID _cuid, std::string_view _file_path, const std::string& _data,
              const std::vector<size_t>& _last_offset_line);
  // temp cu
  explicit CU(cu::ID _cuid);

  CU(const CU&)            = delete;
  CU& operator=(const CU&) = delete;

  [[nodiscard]] static CU make_root() noexcept;

  struct Status {
    bool root      = false;
    bool prepared  = false;
    bool analyzed  = false;
    bool codegened = false;
  };

  Status status;


  // base
  const ID  cuid;
  const ID  parent_cuid;
  FileInfo& file_info;

  // local pools
  metacode::Graph*   metacodes       = nullptr; // preprocessor pass
  ast::Arena*        ast             = nullptr; // parsing pass
  type::Arena*       types           = nullptr; // parsing pass
  scope::Graph*      scopes          = nullptr; // parsing pass
  definition::Arena* definitions     = nullptr; // parsing pass
  module::Graph*     modules         = nullptr; // parsing pass
  extension::Arena*  extensions      = nullptr; // parsing pass
  size_t             inference_count = 0;


  // dependencies
  std::unordered_map<ast::ID, module::ID, ast::ID::Hash> imports;
  std::unordered_map<ast::ID, module::ID, ast::ID::Hash> reexports;
  ast::ID                                                node_export;

  // extra
  llvm::Module* llvm_module = nullptr; // codegen pass
};

struct TEMP_CU : public CU {
  TEMP_CU(cu::ID _cuid, metacode::Graph* _metacode = nullptr, ast::Arena* _ast = nullptr, type::Arena* _type = nullptr,
          scope::Graph* _scope = nullptr, definition::Arena* _def = nullptr, module::Graph* _module = nullptr,
          extension::Arena* _ext = nullptr);

  size_t temp_cu_offset;
};


} // namespace cu
