#include <cassert>
#include <ostream>

namespace mod {

template <unsigned Precision, unsigned BitWidth>
Integer<Precision, BitWidth> Integer<Precision, BitWidth>::add(
    const Integer<Precision, BitWidth> &other) const {
  std::bitset<kBitWidth> has_carry_viable, no_carry_viable;

  SelfTy result;

  const unsigned carry_bit_mask = kPrecisionStates;
  const uint64_t result_mask = kPrecisionStates - 1;

  for (unsigned bit_i = 0; bit_i < kBitWidth; bit_i++) {
    auto &viables = result._value[bit_i];
    for (uint64_t viable_lhs = get_first_viable(bit_i);
         viable_lhs != kPrecisionStates;
         viable_lhs = get_next_viable(bit_i, viable_lhs)) {
      bool found_one = false;
      for (uint64_t viable_rhs = other.get_first_viable(bit_i);
           viable_rhs != kPrecisionStates;
           viable_rhs = other.get_next_viable(bit_i, viable_rhs)) {
        found_one = true;
        if (bit_i < kPrecision || no_carry_viable.test(bit_i - kPrecision)) {
          uint64_t result = viable_lhs + viable_rhs;
          viables.set(result & result_mask);
          if (result & carry_bit_mask)
            has_carry_viable.set(bit_i);
          else
            no_carry_viable.set(bit_i);
        }
        if (bit_i >= kPrecision && has_carry_viable.test(bit_i - kPrecision)) {
          uint64_t result = viable_lhs + viable_rhs + 1;
          viables.set(result & result_mask);
          if (result & carry_bit_mask)
            has_carry_viable.set(bit_i);
          else
            no_carry_viable.set(bit_i);
        }
      }
      if (!found_one)
        continue;
    }
  }

  return result;
}

template <unsigned Precision, unsigned BitWidth>
Integer<Precision, BitWidth>
Integer<Precision, BitWidth>::left_shift(unsigned amount) const {
  assert(amount != 0 && amount < BitWidth && "Out of bounds shift!");

  SelfTy result;
  for (unsigned i = BitWidth; i > amount; i--)
    result._value[i - 1] = _value[i - 1 - amount];

  for (unsigned i = amount; i > 0; i--)
    result._value[i - 1].set(0);

  return result;
}

template <unsigned Precision, unsigned BitWidth>
void Integer<Precision, BitWidth>::coerce_bit(unsigned bit_idx, bool *known_one,
                                              bool *known_zero) const {
  if (known_zero)
    *known_zero = true;
  if (known_one)
    *known_one = true;

  if (known_one)
    for (unsigned i = 0; i < (kPrecisionStates >> 1); ++i)
      if (_value[bit_idx].test(i)) {
        *known_one = false;
        break;
      }

  if (known_zero)
    for (unsigned i = (kPrecisionStates >> 1); i < kPrecisionStates; ++i)
      if (_value[bit_idx].test(i)) {
        *known_zero = false;
        break;
      }
}

template <unsigned Precision, unsigned BitWidth>
bool Integer<Precision, BitWidth>::admits_u64(
    typename std::enable_if<kBitWidth == 64, uint64_t>::type element) const {
  for (unsigned bit_idx = 0; bit_idx < kBitWidth; bit_idx++) {
    uint64_t adjusted =
        (element << (kBitWidth - bit_idx - 1)) >> (kBitWidth - kPrecision);
    assert(adjusted < kPrecisionStates && "Bad bit math!");
    if (!_value[bit_idx].test(adjusted))
      return false;
  }
  return true;
}

template <unsigned Precision, unsigned BitWidth>
void Integer<Precision, BitWidth>::read(const std::string &val) {
  assert(val.length() <= kBitWidth && "Input too big!");

  std::bitset<kPrecisionStates> viability_mask;
  viability_mask.set(0);

  auto iterate_once = [&](unsigned bit_idx, bool is_one, bool is_zero) {
    for (unsigned i = 0; i < kPrecisionStates; ++i) {
      if (viability_mask.test((i >> 1) << 1) ||
          viability_mask.test(((i >> 1) << 1) | 1)) {
        if (is_one)
          _value[bit_idx].set((i >> 1) | (1 << (kPrecision - 1)));
        if (is_zero)
          _value[bit_idx].set(i >> 1);
      }
    }

    viability_mask = _value[bit_idx];
  };

  for (unsigned i = val.length(); i != 0; --i) {
    char c = val[i - 1];
    assert(c == '0' || c == '1' || c == 'u');
    iterate_once(val.length() - i, c == '1' || c == 'u', c == '0' || c == 'u');
  }

  for (unsigned i = val.length(); i < kBitWidth; i++)
    iterate_once(i, false, true);
}

template <unsigned Precision, unsigned BitWidth>
std::string
Integer<Precision, BitWidth>::write(bool trim_leading_zeroes) const {
  std::string result(kBitWidth, 'X');
  for (int bit_idx = 0; bit_idx < kBitWidth; ++bit_idx) {
    bool known_one, known_zero;
    coerce_bit(bit_idx, &known_one, &known_zero);
    result[kBitWidth - bit_idx - 1] = [&]() {
      if (known_one && known_zero)
        return 'T';
      else if (known_one)
        return '1';
      else if (known_zero)
        return '0';
      else
        return 'u';
    }();
  }

  if (trim_leading_zeroes) {
    std::size_t start_index = result.find_first_not_of('0');
    if (start_index != std::string::npos)
      result = result.substr(start_index);
  }

  return result;
}

template <unsigned Precision, unsigned BitWidth>
void Integer<Precision, BitWidth>::print_raw(std::ostream &out,
                                             bool trim_leading_zeroes,
                                             const char *header) const {
  if (header)
    out << header << "\n";
  unsigned length = kBitWidth;
  if (trim_leading_zeroes) {
    std::bitset<kBitWidth> only_zero;
    only_zero.set(0);
    while (length > 0 && _value[length - 1] == only_zero)
      length--;
    if (length != kBitWidth)
      length++;
  }

  for (unsigned i = 0; i < length; i++) {
    out << _value[i] << " : " << i  << "\n";
  }
}

template <unsigned A, unsigned B>
std::ostream &operator<<(std::ostream &os, const Integer<A, B> &val) {
  os << val.write();
  return os;
}
}
