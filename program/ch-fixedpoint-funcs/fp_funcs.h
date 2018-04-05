#ifndef __FP_FUNCS_H__
#define __FP_FUNCS_H__

#include <cstdint>
#include <cassert>
#include <limits>

typedef std::int32_t Integer;
typedef std::int32_t Int32;
typedef std::int64_t Int64;

inline Integer fp_mask_if_zero(Integer a) {
  return a ? 0 : ~0;
}

inline Integer fp_mask_if_non_zero(Integer a) {
  return a ? ~0 : 0;
}

// Each bit of the result is set to the corresponding bit of either then_val or
// else_val depending on whether the corresponding bit of if_mask is set.
// Equivalent to the VBSL instruction in ARM NEON.
inline Integer fp_select_using_mask(Integer if_mask, Integer then_val, Integer else_val) {
  return (if_mask & then_val) ^ (~if_mask & else_val);
}

// Correctly-rounded-to-nearest division by a power-of-two.
// Also known as a rounding arithmetic right shift.
// from library gemmlowp, fixedppoint.h, RoundingDivideByPOT().
inline Integer fp_rounding_divide_by_POT(Integer x, int exponent) {
  assert(exponent >= 0);
  assert(exponent <= 31);
  const Integer mask = (1 << exponent) - 1;
  const Integer threshold = (mask >> 1) + (x < 0? 1: 0);
  return (x >> exponent) + ((x & mask) > threshold ? 1: 0);
}

// Returns the product of a integer value by a power of two, with either a positive exponent
// (equivalent to an arithmetic left shift, saturating) or a negative exponent
// (equivalent to an arithmetic right shift, rounding to nearest).
// from library gemmlowp, fixedppoint.h, ImplSaturatingRoundingMultiplyByPOT.
inline Int32 fp_saturating_rounding_mult_by_POT(Int32 x, int exponent) {
  if (exponent < 0)
    return fp_rounding_divide_by_POT(x, -exponent);
    
  const Int32 min = std::numeric_limits<Int32>::min();
  const Int32 max = std::numeric_limits<Int32>::max();
  const Int32 threshold = ((1 << (31 - exponent)) - 1);
  const Int32 positive_mask = fp_mask_if_non_zero(x > threshold);
  const Int32 negative_mask = fp_mask_if_non_zero(x < -threshold);
  Int32 result = x << exponent;
  result = fp_select_using_mask(positive_mask, max, result);
  result = fp_select_using_mask(negative_mask, min, result);
  return result;
}

// Returns (a+b)/2, rounded to the nearest integer.
// from library gemmlowp, fixedppoint.h, RoundingHalfSum().
// Equivalent to VRHADD in the ARM NEON instruction set.
inline Int32 fp_rounding_half_sum(Int32 a, Int32 b) {
  Int64 a64 = a;
  Int64 b64 = b;
  Int64 sum = a64 + b64;
  Int64 sign = sum >= 0 ? 1 : -1;
  return static_cast<Int32>((sum + sign) / 2);
}

// Fixed point multiplication for underlying type Int32.
// Parameter 'a' has format Qa, parameter 'b' in format Qb, result has format Q(a+b).
// from library gemmlowp, fixedppoint.h, SaturatingRoundingDoublingHighMul().
// This function implements the same computation as the ARMv7 NEON VQRDMULH instruction.
inline Int32 fp_mult(Int32 a, Int32 b) {
  bool overflow = a == b && a == std::numeric_limits<Int32>::min();
  Int64 a_64(a);
  Int64 b_64(b);
  Int64 ab_64 = a_64 * b_64;
  Int32 nudge = ab_64 >= 0 ? (1 << 30) : (1 - (1 << 30));
  Int32 ab_x2_high32 = static_cast<Int32>((ab_64 + nudge) / (1ll << 31));
  return overflow ? std::numeric_limits<Int32>::max() : ab_x2_high32;
}

// Returns exp(x) for x in [-1/4, 0).
// Parameter 'a' and result have formats Q0
// from library gemmlowp, fixedppoint.h, exp_on_interval_between_negative_one_quarter_and_0_excl().
inline Integer fp_exp_on_interval_between_negative_one_quarter_and_0_excl(Integer a) {
  const int kFractionalBits = 8*sizeof(Integer) - 1;
  Integer constant_term = 1895147668;
  Integer constant_1_over_3 = 715827883;
  Integer x = a + (1 << (kFractionalBits - 3));
  Integer x2 = fp_mult(x, x);
  Integer x3 = fp_mult(x2, x);
  Integer x4 = fp_mult(x2, x2);
  Integer x4_over_4 = fp_rounding_divide_by_POT(x4, 2);
  Integer x4_over_24_plus_x3_over_6_plus_x2 = fp_mult((x4_over_4 + x3), constant_1_over_3) + x2;
  Integer x4_over_24_plus_x3_over_6_plus_x2_over_2 = fp_rounding_divide_by_POT(x4_over_24_plus_x3_over_6_plus_x2, 1);
  return constant_term + fp_mult(constant_term, x + x4_over_24_plus_x3_over_6_plus_x2_over_2);
}

// Returns exp(x) for x < 0.
// Parameter 'a' has format Q(kIntegerBits), result has format Q0
// from library gemmlowp, fixedppoint.h, exp_on_negative_values().
inline Integer fp_exp_on_negative_values(Integer a, int kIntegerBits) {

  const int kFractionalBits = 8*sizeof(Integer) - kIntegerBits - 1;

  Integer kOneQuarter = 1 << (kFractionalBits - 2);
  Integer mask = kOneQuarter - 1;
  Integer a_mod_quarter_minus_one_quarter = (a & mask) - kOneQuarter;
  Integer a_mod_quarter_minus_one_quarter_scaled = a_mod_quarter_minus_one_quarter << kIntegerBits;
  Integer result = fp_exp_on_interval_between_negative_one_quarter_and_0_excl(a_mod_quarter_minus_one_quarter_scaled);
  Integer remainder = a_mod_quarter_minus_one_quarter - a;

#define GEMMLOWP_EXP_BARREL_SHIFTER(Exponent, FixedPointMultiplier)        \
  if (kIntegerBits > Exponent) {                                            \
    const int kShiftAmount = kIntegerBits > Exponent ? kFractionalBits + Exponent : 0;           \
    result = fp_select_using_mask(                                               \
        fp_mask_if_non_zero(remainder & (1 << kShiftAmount)), \
        fp_mult(result, FixedPointMultiplier), result);                                      \
  }
  GEMMLOWP_EXP_BARREL_SHIFTER(-2, 1672461947);
  GEMMLOWP_EXP_BARREL_SHIFTER(-1, 1302514674);
  GEMMLOWP_EXP_BARREL_SHIFTER(+0, 790015084);
  GEMMLOWP_EXP_BARREL_SHIFTER(+1, 290630308);
  GEMMLOWP_EXP_BARREL_SHIFTER(+2, 39332535);
  GEMMLOWP_EXP_BARREL_SHIFTER(+3, 720401);
  GEMMLOWP_EXP_BARREL_SHIFTER(+4, 242);
#undef GEMMLOWP_EXP_BARREL_SHIFTER

  if (kIntegerBits > 5) {
    const Integer clamp = -(1 << (kFractionalBits + 5));
    result = fp_select_using_mask(fp_mask_if_non_zero(a < clamp), 0, result);
  }

  static const Integer one = std::numeric_limits<Integer>::max();
  return fp_select_using_mask(fp_mask_if_zero(a), one, result);
}

// Returns 1 / (1 + x) for x in (0, 1).
// Parameter 'a' and result have formats Q0
// from library gemmlowp, fixedppoint.h, one_over_one_plus_x_for_x_in_0_1().
inline Integer fp_one_over_one_plus_x_for_x_in_0_1(Integer a) {
  Integer Q0_one = std::numeric_limits<Integer>::max();
  Integer Q2_one = 1 << (8*sizeof(Integer) - 2 - 1);
  Integer half_denominator = fp_rounding_half_sum(a, Q0_one);
  Integer Q2_48_over_17 = 1515870810;
  Integer Q2_neg_32_over_17 = -1010580540;
  Integer x = Q2_48_over_17 + fp_mult(half_denominator, Q2_neg_32_over_17);
  for (int i = 0; i < 3; i++) {
    Integer half_denominator_times_x = fp_mult(half_denominator, x);
    Integer one_minus_half_denominator_times_x = Q2_one - half_denominator_times_x;
    Integer aaa = fp_mult(x, one_minus_half_denominator_times_x);
    x = x + fp_saturating_rounding_mult_by_POT(aaa, 2);
  }
  return fp_saturating_rounding_mult_by_POT(x, 1);
}

#endif // __FP_FUNCS_H__
