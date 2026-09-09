#pragma once

// STL — high-frequency
#include <algorithm>
#include <array>
#include <cassert>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// Expensive + evidently widely used
#include <format>

// Project forward declarations
#include "ast/forward.hpp"
#include "codegen/llvm_forward.hpp"
#include "nexus/forward.hpp"
#include "type/forward.hpp"

// Fundamental IDs
#include "id/base.hpp"
#include "id/cuid.hpp"
#include "id/defid.hpp"
#include "id/id_query.hpp"
#include "id/metaid.hpp"
#include "id/modid.hpp"
#include "id/nodeid.hpp"
#include "id/scpid.hpp"
#include "id/semid.hpp"
#include "id/tokid.hpp"
#include "id/typeid.hpp"

// Expensive project-wide data
#include "ast/data.hpp"
#include "lexer/data.hpp"
#include "type/data.hpp"

// Stable storage
#include "pool/stable_storage.hpp"

// Common
#include <common/compiler_options.hpp>
#include <common/enum_lite.hpp>
#include <common/environment.hpp>
#include <common/forward.hpp>