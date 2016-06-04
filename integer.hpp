#ifndef INTEGER_HPP
#define INTEGER_HPP

#include <array>
#include <bitset>
#include <cassert>
#include <string>
#include <type_traits>

namespace mod {

/// This is a class capable of reasoning about partially known integers.  For
/// every bit we have precise knowledge of \p Precision trailing bits, and we
/// use this information to propagate known-bits information through arithmetic.

template <unsigned Precision, unsigned BitWidth> class Integer {
public:
  static constexpr unsigned kPrecisionStates = 1 << Precision;
  static constexpr unsigned kPrecision = Precision;
  static constexpr unsigned kBitWidth = BitWidth;
  static_assert(kPrecisionStates < ((unsigned)-1), "Precision too large!");
  static_assert(BitWidth != 0, "Zero bit width not allowed!");
  static_assert(Precision != 0, "Zero precision not allowed!");
  static_assert(Precision <= BitWidth,
                "Precision > BitWidth does not make sense!");

private:
  std::array<std::bitset<kPrecisionStates>, BitWidth> _value;

public:
  Integer() {}

  Integer(const std::string &val) { read(val); }

  Integer(unsigned val) {
    assert((val == 0 || val == 1) && "Others not supported!");
    _value[0].set(val);
    for (unsigned bit_idx = 1; bit_idx < kBitWidth; bit_idx++)
      _value[bit_idx].set(0);
  }

  typedef Integer<Precision, BitWidth> SelfTy;

  bool operator==(const SelfTy &other) const { return _value == other._value; }
  bool operator!=(const SelfTy &other) const { return _value != other._value; }

  unsigned get_next_viable(unsigned bit_idx, unsigned previous) const {
    for (uint64_t i = (previous + 1), e = kPrecisionStates; i < e; i++)
      if (_value[bit_idx].test(i))
        return i;

    return kPrecisionStates;
  }

  uint64_t get_first_viable(unsigned bit_idx) const {
    return get_next_viable(bit_idx, (unsigned)-1);
  }

  void set_viable(unsigned bit_idx, unsigned value) {
    _value[bit_idx].set(value);
  }

  void clear_viable(unsigned bit_idx, uint64_t value) {
    _value[bit_idx].reset(value);
  }

  SelfTy add(const SelfTy &other) const;
  SelfTy left_shift(unsigned amount) const;

  void coerce_bit(unsigned bit_idx, bool *known_one, bool *know_zero) const;
  bool admits_u64(
      typename std::enable_if<kBitWidth == 64, uint64_t>::type) const;
  bool admits(
      typename std::enable_if<kBitWidth == 64, unsigned>::type val) const {
    return admits_u64(static_cast<uint64_t>(val));
  }

  void read(const std::string &val);
  std::string write(bool trim_leading_zeroes = true) const;
  void print_raw(std::ostream &out, bool trim_leading_zeroes = true,
                 const char *header = nullptr) const;
};
}

#include "integer-inl.hpp"

#endif
