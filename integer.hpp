#ifndef INTEGER_HPP
#define INTEGER_HPP

#include <array>
#include <bitset>
#include <cassert>
#include <iterator>
#include <string>
#include <type_traits>

namespace mod {

template <std::size_t N>
class SetBitIterator
    : public std::iterator<std::forward_iterator_tag, uint64_t, uint64_t,
                           const uint64_t *, const uint64_t &> {
  const std::bitset<N> &_parent;
  uint64_t _current_value;

  uint64_t first_set_bit_after(uint64_t start) const {
    while (start < N && !_parent.test(start))
      start++;
    return start;
  }

public:
  SetBitIterator(const std::bitset<N> &parent, bool is_begin)
      : _parent(parent) {
    _current_value = is_begin ? first_set_bit_after(0) : N;
  }

  typedef SetBitIterator<N> SelfTy;

  SelfTy operator++() {
    SelfTy i = *this;
    _current_value = first_set_bit_after(_current_value + 1);
    return i;
  }

  SelfTy operator++(int pre_inc_baton) {
    _current_value = first_set_bit_after(_current_value + 1);
    return *this;
  }

  reference operator*() { return _current_value; }
  pointer operator->() { return &_current_value; }
  bool operator==(const SelfTy &rhs) {
    assert(&_parent == rhs._parent && "Bad iterator comparision!");
    return _current_value == rhs._current_value;
  }
  bool operator!=(const SelfTy &rhs) {
    assert(&_parent == rhs._parent && "Bad iterator comparision!");
    return _current_value != rhs._current_value;
  }
};

template <typename IteratorTy> struct Range {
  IteratorTy _begin, _end;

  IteratorTy begin() { return _begin; }
  IteratorTy end() { return _end; }
};

template <std::size_t N>
Range<SetBitIterator<N>> set_bits(const std::bitset<N> &bitset) {
  return Range<SetBitIterator<N>>(
      {SetBitIterator<N>(bitset, true), SetBitIterator<N>(bitset, false)});
}

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
