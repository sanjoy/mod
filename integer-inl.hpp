#include <cassert>
#include <iostream>

namespace mod {

template <unsigned LogPrecision, unsigned BitWidth>
Integer<LogPrecision, BitWidth> Integer<LogPrecision, BitWidth>::add(
    const Integer<LogPrecision, BitWidth> &other) const {
  std::bitset<kBitWidth + kPrecision> has_carry_viable, no_carry_viable;
  for (unsigned i = 0; i < kPrecision; i++)
    no_carry_viable.set(i);

  SelfTy result;

  const unsigned carry_bit_mask = 1 << (LogPrecision + 1);
  const uint64_t result_mask = kPrecision - 1;

  for (unsigned bit_i = 0; bit_i < kBitWidth; bit_i++) {
    uint64_t viable_lhs = 0, viable_rhs = 0;
    auto &viables = result._value[bit_i];
    for (uint64_t viable_lhs = get_first_viable(bit_i);
         viable_lhs != kPrecision;
         viable_lhs = get_next_viable(bit_i, viable_lhs)) {
      bool found_one = false;
      for (uint64_t viable_rhs = other.get_first_viable(bit_i);
           viable_rhs != kPrecision;
           viable_rhs = other.get_next_viable(bit_i, viable_rhs)) {
        found_one = true;
        if (no_carry_viable[bit_i]) {
          uint64_t result = viable_lhs + viable_rhs;
          viables.set(result & result_mask);
          if (result & carry_bit_mask)
            has_carry_viable.set(bit_i + kPrecision);
          else
            no_carry_viable.set(bit_i + kPrecision);
        }
        if (has_carry_viable[bit_i]) {
          uint64_t result = viable_lhs + viable_rhs + 1;
          viables.set(result & result_mask);
          if (result & carry_bit_mask)
            has_carry_viable.set(bit_i + kPrecision);
          else
            no_carry_viable.set(bit_i + kPrecision);
        }
      }
      if (!found_one)
        continue;
    }
  }

  return result;
}


template <unsigned LogPrecision, unsigned BitWidth>
Integer<LogPrecision, BitWidth>
Integer<LogPrecision, BitWidth>::left_shift(unsigned amount) const {
  assert(amount != 0 && amount < BitWidth && "Out of bounds shift!");

  SelfTy result;
  for (unsigned i = BitWidth; i > amount; i--)
    result._value[i - 1] = _value[i - 1 - amount];

  for (unsigned i = amount; i > 0; i--)
    result._value[i - 1].set(0);

  return result;
}

template <unsigned LogPrecision, unsigned BitWidth>
void Integer<LogPrecision, BitWidth>::coerce_bit(unsigned bit_idx,
                                                 bool *known_one,
                                                 bool *known_zero) const {
  if (known_zero)
    *known_zero = true;
  if (known_one)
    *known_one = true;

  if (known_one)
    for (unsigned i = 0; i < (kPrecision >> 1); ++i)
      if (_value[bit_idx].test(i)) {
        *known_one = false;
        break;
      }

  if (known_zero)
    for (unsigned i = (kPrecision >> 1); i < kPrecision; ++i)
      if (_value[bit_idx].test(i)) {
        *known_zero = false;
        break;
      }
}

template <unsigned LogPrecision, unsigned BitWidth>
Integer<LogPrecision, BitWidth> Integer<LogPrecision, BitWidth>::multiply(
    const Integer<LogPrecision, BitWidth> &other) const {
  SelfTy result(0);
  for (unsigned bit_idx = 0; bit_idx < kBitWidth; bit_idx++) {
    bool known_zero;
    other.coerce_bit(bit_idx, nullptr, &known_zero);
    if (!known_zero)
      result = result.add(bit_idx ? this->left_shift(bit_idx) : *this);
  }
  return result;
}

template <unsigned LogPrecision, unsigned BitWidth>
void Integer<LogPrecision, BitWidth>::read(const std::string &val) {
  assert(val.length() <= kBitWidth && "Input too big!");

  std::bitset<kPrecision> viability_mask;
  viability_mask.set(0);

  for (unsigned i = val.length(); i != 0; --i) {
    char c = val[i - 1];
    assert(c == '0' || c == '1' || c == 'u');
    unsigned bit_idx = val.length() - i;

    bool maybe_zero = c == '0' || c == 'u';
    bool maybe_one = c == '1' || c == 'u';

    unsigned trailing_mask = 1 << (kLogPrecision - bit_idx);

    for (unsigned i = 0; i < kPrecision; ++i) {
      if (viability_mask.test((i >> 1) << 1) ||
          viability_mask.test(((i >> 1) << 1) | 1)) {
        if (maybe_one)
          _value[bit_idx].set((i >> 1) | (1 << (kLogPrecision - 1)));
        if (maybe_zero)
          _value[bit_idx].set(i >> 1);
      }
    }

    viability_mask = _value[bit_idx];
  }

  for (unsigned i = val.length(); i < kBitWidth; i++)
    _value[i].set(0);
}


template <unsigned LogPrecision, unsigned BitWidth>
std::string Integer<LogPrecision, BitWidth>::write() const {
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

  return result;
}

template <unsigned A, unsigned B>
std::ostream &operator<<(std::ostream &os, const Integer<A, B> &val) {
  os << val.write();
  return os;
}
}
