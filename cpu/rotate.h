#ifndef INCLUDED_CPU_ROTATE_H
#define INCLUDED_CPU_ROTATE_H

#include <cstdint>
#include <type_traits>

#if defined __has_include
#if __has_include(<bit>)
#include <bit>
#endif
#endif

#ifdef _MSC_VER

#endif

// Based on https://gist.github.com/pabigot/7550454 
// (created ROTR version)

namespace door86::cpu {

template <typename T> 
[[nodiscard]] constexpr T rotl(T v, int b) {
  static_assert(std::is_integral<T>::value, "rotate of non-integral type");
  static_assert(!std::is_signed<T>::value, "rotate of signed type");
  constexpr unsigned int num_bits{std::numeric_limits<T>::digits};
  static_assert(0 == (num_bits & (num_bits - 1)), "rotate value bit length not power of two");
  constexpr unsigned int count_mask{num_bits - 1};
  const unsigned int mb{b & count_mask};
  using promoted_type = typename std::common_type<int, T>::type;
  using unsigned_promoted_type = typename std::make_unsigned<promoted_type>::type;
  return ((unsigned_promoted_type{v} << mb) | (unsigned_promoted_type{v} >> (-mb & count_mask)));
}

template <typename T> T rotr(T v, int b) {
  static_assert(std::is_integral<T>::value, "rotate of non-integral type");
  static_assert(!std::is_signed<T>::value, "rotate of signed type");
  constexpr unsigned int num_bits{std::numeric_limits<T>::digits};
  static_assert(0 == (num_bits & (num_bits - 1)), "rotate value bit length not power of two");
  constexpr unsigned int count_mask{num_bits - 1};
  const unsigned int mb{b & count_mask};
  using promoted_type = typename std::common_type<int, T>::type;
  using unsigned_promoted_type = typename std::make_unsigned<promoted_type>::type;
  return ((unsigned_promoted_type{v} >> mb) | (unsigned_promoted_type{v} << (-mb & count_mask)));
}

} // namespace door86::cpu

#endif
