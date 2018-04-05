#include "fp_funcs.h"
#include <fixedpoint/fixedpoint.h> // gemmlowp

#include <stdio.h>

using namespace gemmlowp;
typedef FixedPoint<Integer, 0> FP0;

#define DATA_COUNT 16

void test__MaskIfZero() {
  for (int i = 0; i < DATA_COUNT; i++) {
    Integer input = i - DATA_COUNT/2;
    Integer res_fp = MaskIfZero(input);
    Integer res_int = fp_mask_if_zero(input);
    printf("%d\t%d\t%d\t%s\n", input, res_fp, res_int, res_fp != res_int? "FAIL": "OK");
  }
}

void test__MaskIfNonZero() {
  for (int i = 0; i < DATA_COUNT; i++) {
    Integer input = i - DATA_COUNT/2;
    Integer res_fp = MaskIfNonZero(input);
    Integer res_int = fp_mask_if_non_zero(input);
    printf("%d\t%d\t%d\t%s\n", input, res_fp, res_int, res_fp != res_int? "FAIL": "OK");
  }
}

void test__SelectUsingMask() {
  for (int i = 0; i < DATA_COUNT; i++) {
    Integer input = i - DATA_COUNT/2;
    Integer res_fp = SelectUsingMask(input, 100, 200);
    Integer res_int = fp_select_using_mask(input, 100, 200);
    printf("%d\t%d\t%d\t%s\n", input, res_fp, res_int, res_fp != res_int? "FAIL": "OK");
  }
}

void test__RoundingDivideByPOT(int exponent) {
  for (int i = 0; i < DATA_COUNT; i++) {
    Integer step = 21000;
    Integer input = -step*5 + step*i;
    Integer res_fp = RoundingDivideByPOT(input, exponent);
    Integer res_int = fp_rounding_divide_by_POT(input, exponent);
    printf("%d\t%d\t%d\t%s\n", input, res_fp, res_int, res_fp != res_int? "FAIL": "OK");
  }
}

template <int exponent>
void test__SaturatingRoundingMultiplyByPOT() {
  for (int i = 0; i < DATA_COUNT; i++) {
    Integer step = 21000;
    Integer input = -step*5 + step*i;
    Integer res_fp = SaturatingRoundingMultiplyByPOT<exponent>(input);
    Integer res_int = fp_saturating_rounding_mult_by_POT(input, exponent);
    printf("%d\t%d\t%d\t%s\n", input, res_fp, res_int, res_fp != res_int? "FAIL": "OK");
  }
}

void test__RoundingHalfSum() {
  for (int i = 0; i < DATA_COUNT; i++) {
    Integer step = 21000000;
    Integer input1 = -step*5 + step*i;
    Integer input2 = step*6 - step*2*i;
    Integer res_fp = RoundingHalfSum(input1, input2);
    Integer res_int = fp_rounding_half_sum(input1, input2);
    printf("%d\t%d\t%d\t%d\t%s\n", input1, input2, res_fp, res_int, res_fp != res_int? "FAIL": "OK");
  }
}

void test__OperatorMult() {
  for (int i = 0; i < DATA_COUNT; i++) {
    Integer step = 21000000;
    Integer input1 = -step*5 + step*i;
    Integer input2 = step*6 - step*2*i;
    FP0 input1_fp = FP0::FromRaw(input1);
    FP0 input2_fp = FP0::FromRaw(input2);
    Integer res_fp = (input1_fp * input2_fp).raw();
    Integer res_int = fp_mult(input1, input2);

    /*int left_shift = 2;
    int quantized_multiplier = 1073741824;
    Integer res_fp = SaturatingRoundingDoublingHighMul(-i * (1 << left_shift), quantized_multiplier);
    Integer res_int = fp_mult(-i * (1 << left_shift), quantized_multiplier);*/
    
    printf("%d\t%d\t%d\t%d\t%s\n", input1, input2, res_fp, res_int, res_fp != res_int? "FAIL": "OK");
  }
}

void test_fp_exp_on_interval_between_negative_one_quarter_and_0_excl() {
  for (int i = 0; i < DATA_COUNT; i++) {
    Integer step = 10000000;
    Integer input = -step*i;
    Integer res_fp = exp_on_interval_between_negative_one_quarter_and_0_excl(FP0::FromRaw(input)).raw();
    Integer res_int = fp_exp_on_interval_between_negative_one_quarter_and_0_excl(input);
    printf("%d\t%d\t%d\t%s\n", input, res_fp, res_int, res_fp != res_int? "FAIL": "OK");
  }
}

template <int kIntegerBits>
void test__exp_on_negative_values() {
  for (int i = 0; i < DATA_COUNT; i++) {
    Integer input = -10000000*i;
    Integer res_fp = exp_on_negative_values(FixedPoint<Integer, kIntegerBits>::FromRaw(input)).raw();
    Integer res_int = fp_exp_on_negative_values(input, kIntegerBits);
    printf("%d\t%d\t%d\t%s\n", input, res_fp, res_int, res_fp != res_int? "FAIL": "OK");
  }
}

void test__one_over_one_plus_x_for_x_in_0_1() {
  for (int i = 0; i < DATA_COUNT; i++) {
    Integer input = 10000000*i;
    Integer res_fp = one_over_one_plus_x_for_x_in_0_1(FP0::FromRaw(input)).raw();
    Integer res_int = fp_one_over_one_plus_x_for_x_in_0_1(input);
    printf("%d\t%d\t%d\t%s\n", input, res_fp, res_int, res_fp != res_int? "FAIL": "OK");
  }
}

int main() {
  //test__MaskIfZero();
  //test__MaskIfNonZero();
  //test__SelectUsingMask();
  //test__RoundingDivideByPOT(0);
  //test__RoundingDivideByPOT(31);
  //test__RoundingDivideByPOT(12);
  //test__SaturatingRoundingMultiplyByPOT<2>();
  //test__SaturatingRoundingMultiplyByPOT<-2>();
  //test__RoundingHalfSum();
  //test__OperatorMult();
  //test_fp_exp_on_interval_between_negative_one_quarter_and_0_excl();
  //test__exp_on_negative_values<5>();
  //test__exp_on_negative_values<12>();
  test__one_over_one_plus_x_for_x_in_0_1();
  return 0;
}
