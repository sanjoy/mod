#ifndef INTEGER_HPP
#define INTEGER_HPP

#include <array>
#include <bitset>
#include <cassert>
#include <string>
#include <vector>

namespace mod {
template <unsigned LogPrecision, unsigned BitWidth> class Integer {
public:
  static constexpr unsigned kPrecision = 1 << LogPrecision;
  static constexpr unsigned kLogPrecision = LogPrecision;
  static constexpr unsigned kBitWidth = BitWidth;
  static_assert(kPrecision < ((unsigned)-1), "Precision too large!");
  static_assert(BitWidth != 0, "Zero bit width not allowed!");
  static_assert(LogPrecision != 0, "Zero precision not allowed!");

private:
  std::array<std::bitset<kPrecision>, BitWidth> _value;

public:
  Integer() {}
  Integer(unsigned val) {
    assert((val == 0 || val == 1) && "Others not supported!");
    _value[0].set(val);
    for (unsigned bit_idx = 1; bit_idx < kBitWidth; bit_idx++)
      _value[bit_idx].set(0);
  }

  typedef Integer<LogPrecision, BitWidth> SelfTy;

  unsigned get_next_viable(unsigned bit_idx, unsigned previous) const {
    for (uint64_t i = (previous + 1), e = kPrecision; i < e; i++)
      if (_value[bit_idx].test(i))
        return i;

    return kPrecision;
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
  SelfTy multiply(const SelfTy &other) const;

  void coerce_bit(unsigned bit_idx, bool *known_one, bool *know_zero) const;

  void read(const std::string &val);
  std::string write() const;
};
}

#include "integer-inl.hpp"

#endif
