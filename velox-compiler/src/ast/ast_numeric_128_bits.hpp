#pragma once

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>

class Float128 final
{
public:
  llvm::APFloat val;

  Float128()
    : val(llvm::APFloatBase::IEEEquad(), llvm::StringRef("0.0"))
  {
  }

  Float128(const llvm::APFloat& value)
    : val(value)
  {
  }

  Float128(double value)
    : val(value)
  {
  }

  Float128(const std::string& s)
    : val(llvm::APFloatBase::IEEEquad(), llvm::StringRef(s))
  {
  }

  void string_to_f128(const std::string& s)
  {
    llvm::StringRef sr(s);
    llvm::APFloat   tmp(llvm::APFloatBase::IEEEquad(), sr);
    val = tmp;
  }

  std::string float128_to_string(int precision = 36) const
  {
    llvm::SmallVector<char, 128> buf;
    val.toString(buf, precision);
    return std::string(buf.begin(), buf.end());
  }
};

class Int128 final
{
public:
  llvm::APInt val;

  Int128()
    : val(128, 0, true)
  {
  }

  Int128(const llvm::APInt& value)
    : val(value)
  {
  }

  explicit Int128(long long value)
    : val(128, value)
  {
  }

  explicit Int128(const std::string& s, uint8_t radix = 10)
    : val(128, llvm::StringRef(s), radix)
  {
  }

  void string_to_i128(const std::string& s, int base = 10)
  {
    llvm::APInt tmp(128, 0,
                    true); // 128 bits signed
    bool        ok = false;

    try {
      tmp = llvm::APInt(128, llvm::StringRef(s),
                        base); // base 10 or 16
      ok  = true;
    } catch (...) {
      ok = false;
    }

    if (!ok) {
      throw std::invalid_argument("String contains invalid characters for int128");
    }

    val = tmp;
  }

  std::string i128_to_string(int base = 10) const
  {
    if (base != 10 && base != 16) {
      throw std::invalid_argument("Base must be 10 or 16");
    }
    llvm::SmallVector<char, 128> buf;
    val.toString(buf, base, true);
    return std::string(buf.begin(), buf.end());
  }
};