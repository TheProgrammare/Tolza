#pragma once

#include <cstdint>
#include <string>

namespace llvm
{
class APFloat;
class APInt;
} // namespace llvm


class Float128 final
{
public:
  llvm::APFloat* val;

  Float128();
  ~Float128();
  explicit Float128(const llvm::APFloat& value);
  explicit Float128(double value);
  explicit Float128(const std::string& s);

  void        string_to_f128(const std::string& s);
  std::string float128_to_string(int precision = 36) const;
};

class Int128 final
{
public:
  llvm::APInt* val;

  Int128();
  ~Int128();
  explicit Int128(const llvm::APInt& value);
  explicit Int128(long long value);
  explicit Int128(const std::string& s, uint8_t radix = 10);

  void        string_to_i128(const std::string& s, int base = 10);
  std::string i128_to_string(int radix = 10) const;
};