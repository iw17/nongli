#include <cstdint>

constexpr inline int64_t NIAN_MIN = +2024;
constexpr inline int64_t NIAN_MAX = +2029;
constexpr inline int64_t NIAN_NUM = 6;

constexpr inline int64_t SUI_MIN = +2024;
constexpr inline int64_t SUI_MAX = +2029;
constexpr inline int64_t SUI_NUM = 6;

constexpr inline int64_t YEAR_MIN = +2024;
constexpr inline int64_t YEAR_MAX = +2029;
constexpr inline int64_t YEAR_NUM = 6;

constexpr inline uint8_t NR_RUNS[] = {
    0x6d, 0xdd, 0xd5, 0x3d, 0xbd, 0xdd, 0xd6, 0x5d, 0xdd, 0xd2, 0xd7, 0x5d,
};

constexpr inline int64_t NY_BITS = 14;

constexpr inline int64_t NY_COEF[] = { 12, -24366, 6033, 7356 };

constexpr inline uint8_t NY_RESD[] = {
    0x6d, 0x6d, 0x6b, 0x6b, 0x5b, 0x5b, 0xdb, 0xda, 0xda, 0xd6, 0xd6, 0xb6,
};

constexpr inline int64_t YD_BITS = 19;

constexpr inline int64_t YD_COEF[] = { 29, 34, 278179, 372817 };

constexpr inline uint8_t YD_RESD_0[] = {
    0xaa, 0x99, 0xea, 0xaa, 0x66, 0xa6, 0xbb, 0x9b, 0x95, 0xea, 0xae, 0x56,
    0xa6, 0xbb, 0x6a, 0x9a, 0xaa, 0xaa, 0xa9, 0xaa, 0xaa, 0x6a, 0xaa, 0xaa,
};

constexpr inline const uint8_t *const YD_ARRD[] = {
    YD_RESD_0,
};

constexpr inline int64_t DY_BITS = 18;

constexpr inline int64_t DY_COEF[] = {   -1,  8877, 76207 };

constexpr inline int64_t YN_BITS = 17;

constexpr inline int64_t YN_COEF[] = { 1970, 10597, 66080 };

constexpr inline int64_t SS_BITS = 12;

constexpr inline int64_t SS_CLIN[] = { 31556894, -62168131570 };

constexpr inline int64_t SS_COEF[][7] = {
    { -403,  4368,   -289, -56352,  -65056, 421063,     6716 },
    { -253,  4159,  -4156, -52255,  -33880, 398783,  1281906 },
    { -105,  3792,  -7776, -45138,   -6378, 364436,  2563730 },
    {   46,  3249, -10969, -35255,   15657, 320093,  3854585 },
    {  203,  2504, -13526, -23011,   30643, 268511,  5155994 },
    {  368,  1536, -15227,  -9006,   37336, 213013,  6469394 },
    {  538,   350, -15853,   5951,   34965, 157318,  7794669 },
    {  701, -1005, -15240,  20851,   23375, 105299,  9131233 },
    {  839, -2435, -13316,  34548,    3120,  60707, 10477685 },
    {  927, -3797, -10138,  45866,  -24506,  26859, 11832156 },
    {  944, -4927,  -5914,  53747,  -57530,   6334, 13191320 },
    {  872, -5666,   -991,  57399,  -93457,    726, 14552784 },
    {  707, -5896,   4187,  56425, -129511,  10479, 15912606 },
    {  463, -5569,   9137,  50894, -162913,  34839, 17268010 },
    {  164, -4721,  13409,  41334, -191162,  71930, 18616080 },
    { -152, -3463,  16649,  28656, -212266, 118943, 19954449 },
    { -444, -1960,  18637,  14014, -224895, 172404, 21281740 },
    { -680,  -393,  19301,  -1351, -228434, 228483, 22597083 },
    { -837,  1070,  18705, -16266, -222962, 283307, 23900703 },
    { -906,  2309,  17010, -29731, -209153, 333234, 25193102 },
    { -894,  3258,  14433, -40985, -188158, 375073, 26476337 },
    { -817,  3904,  11208, -49508, -161470, 406245, 27752210 },
    { -697,  4275,   7556, -54989, -130811, 424887, 29023533 },
    { -554,  4415,   3669, -57280,  -98043, 429911, 30293078 },
};

constexpr inline uint8_t SS_RESS_0[] = {
    0x11, 0x07, 0x65, 0x13, 0x05, 0x62, 0xac, 0x05, 0x67, 0x53, 0x0d, 0x69,
    0x03, 0x0e, 0x66, 0x90, 0x0e, 0x68, 0xb6, 0x0e, 0x6c, 0xf6, 0x07, 0x6c,
    0x44, 0x07, 0x65, 0xab, 0x0e, 0x5f, 0x85, 0x06, 0x57, 0x5a, 0x06, 0x53,
    0xee, 0x07, 0x4e, 0xd4, 0x05, 0x4b, 0x9c, 0x0f, 0x55, 0xbf, 0x07, 0x60,
    0xcf, 0x07, 0x62, 0x32, 0x0b, 0x63, 0x7d, 0x0f, 0x61, 0x48, 0x0f, 0x56,
    0x16, 0x07, 0x48, 0xa9, 0x0f, 0x43, 0x86, 0x0f, 0x43, 0xf4, 0x03, 0x4c,
    0x49, 0x05, 0x51, 0xb7, 0x07, 0x53, 0xad, 0x05, 0x5f, 0xb0, 0x05, 0x6a,
    0x88, 0x0e, 0x6d, 0xc5, 0x07, 0x71, 0x28, 0x07, 0x71, 0x49, 0x0f, 0x62,
    0x58, 0x05, 0x4c, 0x0e, 0x07, 0x41, 0x15, 0x0b, 0x3f, 0x98, 0x0b, 0x46,
    0xba, 0x0f, 0x46, 0xa0, 0x0b, 0x3f, 0xd7, 0x0f, 0x3e, 0x47, 0x0b, 0x3f,
    0x2c, 0x07, 0x41, 0x34, 0x0d, 0x49, 0xc7, 0x0f, 0x53, 0x24, 0x0e, 0x53,
    0x13, 0x07, 0x48, 0xed, 0x06, 0x3e, 0x7d, 0x0f, 0x35, 0x6f, 0x0c, 0x36,
    0x4b, 0x0d, 0x34, 0x3a, 0x06, 0x2e, 0x97, 0x0e, 0x2f, 0x16, 0x0f, 0x33,
    0xf2, 0x05, 0x3c, 0xff, 0x0f, 0x51, 0x89, 0x0f, 0x67, 0x78, 0x0f, 0x6b,
    0xb9, 0x07, 0x5c, 0xca, 0x07, 0x48, 0x85, 0x03, 0x34, 0xd1, 0x03, 0x2d,
    0x78, 0x0f, 0x2b, 0xbc, 0x09, 0x2e, 0xd5, 0x0a, 0x3d, 0x19, 0x03, 0x4c,
    0x61, 0x04, 0x57, 0x7e, 0x05, 0x68, 0x26, 0x0f, 0x77, 0x83, 0x0f, 0x75,
    0x51, 0x0e, 0x61, 0xe2, 0x0d, 0x4b, 0xe0, 0x0b, 0x35, 0x27, 0x0f, 0x2b,
    0x9f, 0x07, 0x28, 0xa8, 0x0f, 0x27, 0x74, 0x03, 0x30, 0x5b, 0x0f, 0x34,
};

constexpr inline const uint8_t *const SS_ARRS[] = {
    SS_RESS_0,
};

constexpr inline int64_t SJ_BITS = 19;

constexpr inline int64_t SJ_COEF[] = { 1, 209053, 138479 };

