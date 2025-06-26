#include "test.hpp"

namespace im = iw17::math;
using namespace im::literal;

constexpr uint64_t N = 4;

bool fix_close(im::fix64 a, im::fix64 b) noexcept {
    constexpr int64_t TOL = 2;
    int64_t diff = im::pour_int64(a - b);
    return diff >= -TOL && diff <= TOL;
}

bool dbl_close(double a, double b) noexcept {
    constexpr double TOL = 0x1.0p-33;
    double diff = a - b;
    return diff >= -TOL && diff <= TOL;
}

// fixed-point arithmetics

constexpr double dbls[N] = {
    +0x00'0000'1a42.60p-32,
    -0x02'18c3'e2f7.80p-32,
    +0x07'b1d2'c469.b0p-32,
    -0x00'5314'ac37.00p-32,
};
constexpr im::fix64 fixs[N] = {
    im::fill_fix64(+0x00'0000'1a42_i64),
    im::fill_fix64(-0x02'18c3'e2f7_i64),
    im::fill_fix64(+0x07'b1d2'c46a_i64),
    im::fill_fix64(-0x00'5314'ac37_i64),
};
constexpr int64_t fs_ints[N] = {
    0_i64, -3_i64, +7_i64, -1_i64,
};
constexpr int64_t sf_ints[N] = {
    0_i64, -2_i64, +8_i64, 0_i64,
};
constexpr im::fix64 rhss[N] = {
    im::fill_fix64(+0x01'82ac'2316_i64),
    im::fill_fix64(+0x0d'4f8c'7125_i64),
    im::fill_fix64(-0x00'0281'a93e_i64),
    im::fill_fix64(-0x0b'1847'1b64_i64),
};
constexpr im::fix64 fs_muls[N] = {
    im::fill_fix64(+0x0000'27a9_i32),
    im::fill_fix64(-0xe8bd'71f2_i32),
    im::fill_fix64(-0x1349'5700_i32),
    im::fill_fix64(-0x663b'95df_i32),
};
constexpr im::fix64 tn_muls[N] = {
    im::fill_fix64(+0x0000'27a9_i32),
    im::fill_fix64(-0xe8bd'71f1_i32),
    im::fill_fix64(-0x1349'56ff_i32),
    im::fill_fix64(-0x663b'95de_i32),
};
constexpr im::fix64 fr_muls[N] = {
    im::fill_fix64(+0x00'0000'27a9_i64),
    im::fill_fix64(-0x1b'e8bd'71f2_i64),
    im::fill_fix64(-0x00'1349'5700_i64),
    im::fill_fix64(+0x03'99c4'6a21_i64),
};
constexpr im::fix64 sf_muls[N] = {
    im::fill_fix64(+0x00'0000'27a9_i64),
    im::fill_fix64(-0x1b'e8bd'71f1_i64),
    im::fill_fix64(-0x00'1349'56ff_i64),
    im::fill_fix64(+0x03'99c4'6a22_i64),
};

// fixed-point by integer divisions

constexpr int64_t fnums[N] = {
    +0x0000'0000'1ac3'479d_i64,
    -0x2ab2'6883'ac4f'50c4_i64,
    +0x1147'cfe1'0366'987b_i64,
    -0x0000'01de'74b8'2a70_i64,
};
constexpr int64_t fdens[N] = {
    +0x081c'4ab7_i64, +0x726e'fd39_i64,
    -0x46b7'ecfd_i64, -0x598a'0321_i64,
};
constexpr im::fix64 fs_divs[N] = {
    im::fill_fix64(+0x03'4cbd'8ae5_i64),
    im::fill_fix64(+0x00'bb39'4b0e_i64),
    im::fill_fix64(-0x00'0c4f'8bc7_i64),
    im::fill_fix64(+0x01'4db5'f546_i64),
};
constexpr im::fix64 fr_divs[N] = {
    im::fill_fix64(+0x0000'0003'4cbd'8ae5_i64),
    im::fill_fix64(-0x5f84'7bbf'8def'1477_i64),
    im::fill_fix64(-0x3e8e'25df'5ca8'4cd5_i64),
    im::fill_fix64(+0x0000'0557'f252'e4e4_i64),
};

// fixed-point trigonometrics

constexpr im::fix64 vals[N] = {
    im::fill_fix64(0x00'0000'0000_i64),
    im::fill_fix64(0x01'bb2c'a134_i64),
    im::fill_fix64(0x04'6dd9'81a1_i64),
    im::fill_fix64(0x0e'b0ff'6f30_i64),
};
constexpr im::fix64 coss[N] = { 
    1_fix,
    im::fill_fix64(-0xe982'558a_i64),
    im::fill_fix64(+0xc804'27e1_i64),
    im::fill_fix64(-0x774b'0596_i64),
};
constexpr im::fix64 sins[N] = {
    0_fix,
    im::fill_fix64(+0x68ed'1699_i64),
    im::fill_fix64(+0x9fc7'90f2_i64),
    im::fill_fix64(-0xe281'a60a_i64),
};

int main() {
    iw17::test_suite suite;
    for (uint64_t i = 0; i < N; i++) {
        suite.test("make_fix64",
            fixs[i], im::make_fix64, dbls[i]
        );
        suite.test("show_double", dbl_close,
            dbls[i], im::show_double, fixs[i]
        );
        suite.test("fast_int",
            fs_ints[i], im::fast_int, fixs[i]
        );
        suite.test("safe_int",
            sf_ints[i], im::safe_int, fixs[i]
        );
        suite.test("fast_mul",
            fs_muls[i], im::fast_mul, fixs[i], rhss[i]
        );
        suite.test("tiny_mul",
            tn_muls[i], im::tiny_mul, fixs[i], rhss[i]
        );
        suite.test("fair_mul",
            fr_muls[i], im::fair_mul, fixs[i], rhss[i]
        );
        suite.test("safe_mul",
            sf_muls[i], im::safe_mul, fixs[i], rhss[i]
        );
        suite.test("fast_div",
            fs_divs[i], im::fast_div, fnums[i], fdens[i]
        );
        suite.test("fair_div",
            fr_divs[i], im::fair_div, fnums[i], fdens[i]
        );
        suite.test("cosq", fix_close,
            coss[i], im::cosq, vals[i]
        );
        suite.test("sinq", fix_close,
            sins[i], im::sinq, vals[i]
        );
    }
    return suite.complete() != 0;
}