#pragma once


#include <cstdint>


namespace common::env
{
enum class EEnvironment : uint8_t;
enum class EArch : uint8_t;
enum class EVendor : uint8_t;
enum class EPlatform : uint8_t;
enum class EABI : uint8_t;
enum class ECallConvention : uint8_t;
enum class ELibC : uint8_t;
enum class ECStandard : uint8_t;
enum class FCSource : uint16_t;

struct TargetTriple;
struct Target;
struct LLVM;
struct Cffi;
struct Dir;
struct Option_Profile;
struct Log;
struct Warn;
struct Debug;
struct Preprocessor;
struct Diagnostic;
struct Dependency;
struct Manifest;
struct Profile;
} // namespace common::env


namespace common::compiler
{

enum class EMergeMode : uint8_t;
enum class ERelocModel : uint8_t;
enum class ECodeModel : uint8_t;
enum class EOptimization : uint8_t;
enum class EWarnLevel : uint8_t;
enum class FWarnMode : uint8_t;
enum class ELogLevel : uint8_t;
enum class FEmit : uint8_t;
enum class FPass : uint16_t;
enum class FDebugPrinter : uint8_t;
enum class FCPUFeature : uint16_t;
enum class EDiagnosticFormat : uint8_t;
enum class EErrorMode : uint8_t;

} // namespace common::compiler


namespace common::toolchain
{

struct Options;

}


namespace common
{

class Commander;

}