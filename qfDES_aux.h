#if !defined(_qfDES_aux_h_)
#define _qfDES_aux_h_

#define B00 0x80000000
#define B01 0x40000000
#define B02 0x20000000
#define B03 0x10000000
#define B04 0x08000000
#define B05 0x04000000
#define B06 0x02000000
#define B07 0x01000000
#define B08 0x00800000
#define B09 0x00400000
#define B10 0x00200000
#define B11 0x00100000
#define B12 0x00080000
#define B13 0x00040000
#define B14 0x00020000
#define B15 0x00010000
#define B16 0x00008000
#define B17 0x00004000
#define B18 0x00002000
#define B19 0x00001000
#define B20 0x00000800
#define B21 0x00000400
#define B22 0x00000200
#define B23 0x00000100
#define B24 0x00000080
#define B25 0x00000040
#define B26 0x00000020
#define B27 0x00000010
#define B28 0x00000008
#define B29 0x00000004
#define B30 0x00000002
#define B31 0x00000001

#define INITIAL_PERMUTATION_AUX(_i0, _i1, _o0, _o1) \
{ \
_o0 = _o1 = 0; \
_o0 |= (_i1 & B25) << 25; /* 58 */ \
_o0 |= (_i1 & B17) << 16; /* 50 */ \
_o0 |= (_i1 & B09) <<  7; /* 42 */ \
_o0 |= (_i1 & B01) >>  2; /* 34 */ \
_o0 |= (_i0 & B25) << 21; /* 26 */ \
_o0 |= (_i0 & B17) << 12; /* 18 */ \
_o0 |= (_i0 & B09) <<  3; /* 10 */ \
_o0 |= (_i0 & B01) >>  6; /*  2 */ \
_o0 |= (_i1 & B27) << 19; /* 60 */ \
_o0 |= (_i1 & B19) << 10; /* 52 */ \
_o0 |= (_i1 & B11) <<  1; /* 44 */ \
_o0 |= (_i1 & B03) >>  8; /* 36 */ \
_o0 |= (_i0 & B27) << 15; /* 28 */ \
_o0 |= (_i0 & B19) <<  6; /* 20 */ \
_o0 |= (_i0 & B11) >>  3; /* 12 */ \
_o0 |= (_i0 & B03) >> 12; /*  4 */ \
_o0 |= (_i1 & B29) << 13; /* 62 */ \
_o0 |= (_i1 & B21) <<  4; /* 54 */ \
_o0 |= (_i1 & B13) >>  5; /* 46 */ \
_o0 |= (_i1 & B05) >> 14; /* 38 */ \
_o0 |= (_i0 & B29) <<  9; /* 30 */ \
_o0 |= (_i0 & B21)      ; /* 22 */ \
_o0 |= (_i0 & B13) >>  9; /* 14 */ \
_o0 |= (_i0 & B05) >> 18; /*  6 */ \
_o0 |= (_i1 & B31) <<  7; /* 64 */ \
_o0 |= (_i1 & B23) >>  2; /* 56 */ \
_o0 |= (_i1 & B15) >> 11; /* 48 */ \
_o0 |= (_i1 & B07) >> 20; /* 40 */ \
_o0 |= (_i0 & B31) <<  3; /* 32 */ \
_o0 |= (_i0 & B23) >>  6; /* 24 */ \
_o0 |= (_i0 & B15) >> 15; /* 16 */ \
_o0 |= (_i0 & B07) >> 24; /*  8 */ \
_o1 |= (_i1 & B24) << 24; /* 57 */ \
_o1 |= (_i1 & B16) << 15; /* 49 */ \
_o1 |= (_i1 & B08) <<  6; /* 41 */ \
_o1 |= (_i1 & B00) >>  3; /* 33 */ \
_o1 |= (_i0 & B24) << 20; /* 25 */ \
_o1 |= (_i0 & B16) << 11; /* 17 */ \
_o1 |= (_i0 & B08) <<  2; /*  9 */ \
_o1 |= (_i0 & B00) >>  7; /*  1 */ \
_o1 |= (_i1 & B26) << 18; /* 59 */ \
_o1 |= (_i1 & B18) <<  9; /* 51 */ \
_o1 |= (_i1 & B10)      ; /* 43 */ \
_o1 |= (_i1 & B02) >>  9; /* 35 */ \
_o1 |= (_i0 & B26) << 14; /* 27 */ \
_o1 |= (_i0 & B18) <<  5; /* 19 */ \
_o1 |= (_i0 & B10) >>  4; /* 11 */ \
_o1 |= (_i0 & B02) >> 13; /*  3 */ \
_o1 |= (_i1 & B28) << 12; /* 61 */ \
_o1 |= (_i1 & B20) <<  3; /* 53 */ \
_o1 |= (_i1 & B12) >>  6; /* 45 */ \
_o1 |= (_i1 & B04) >> 15; /* 37 */ \
_o1 |= (_i0 & B28) <<  8; /* 29 */ \
_o1 |= (_i0 & B20) >>  1; /* 21 */ \
_o1 |= (_i0 & B12) >> 10; /* 13 */ \
_o1 |= (_i0 & B04) >> 19; /*  5 */ \
_o1 |= (_i1 & B30) <<  6; /* 63 */ \
_o1 |= (_i1 & B22) >>  3; /* 55 */ \
_o1 |= (_i1 & B14) >> 12; /* 47 */ \
_o1 |= (_i1 & B06) >> 21; /* 39 */ \
_o1 |= (_i0 & B30) <<  2; /* 31 */ \
_o1 |= (_i0 & B22) >>  7; /* 23 */ \
_o1 |= (_i0 & B14) >> 16; /* 15 */ \
_o1 |= (_i0 & B06) >> 25; /*  7 */ \
}

#define FINAL_PERMUTATION_AUX(_i0, _i1, _o0, _o1) \
{ \
_o0 = _o1 = 0; \
_o0 |= (_i1 & B07) <<  7; /* 40 */ \
_o0 |= (_i0 & B07) <<  6; /*  8 */ \
_o0 |= (_i1 & B15) << 13; /* 48 */ \
_o0 |= (_i0 & B15) << 12; /* 16 */ \
_o0 |= (_i1 & B23) << 19; /* 56 */ \
_o0 |= (_i0 & B23) << 18; /* 24 */ \
_o0 |= (_i1 & B31) << 25; /* 64 */ \
_o0 |= (_i0 & B31) << 24; /* 32 */ \
_o0 |= (_i1 & B06) >>  2; /* 39 */ \
_o0 |= (_i0 & B06) >>  3; /*  7 */ \
_o0 |= (_i1 & B14) <<  4; /* 47 */ \
_o0 |= (_i0 & B14) <<  3; /* 15 */ \
_o0 |= (_i1 & B22) << 10; /* 55 */ \
_o0 |= (_i0 & B22) <<  9; /* 23 */ \
_o0 |= (_i1 & B30) << 16; /* 63 */ \
_o0 |= (_i0 & B30) << 15; /* 31 */ \
_o0 |= (_i1 & B05) >> 11; /* 38 */ \
_o0 |= (_i0 & B05) >> 12; /*  6 */ \
_o0 |= (_i1 & B13) >>  5; /* 46 */ \
_o0 |= (_i0 & B13) >>  6; /* 14 */ \
_o0 |= (_i1 & B21) <<  1; /* 54 */ \
_o0 |= (_i0 & B21)      ; /* 22 */ \
_o0 |= (_i1 & B29) <<  7; /* 62 */ \
_o0 |= (_i0 & B29) <<  6; /* 30 */ \
_o0 |= (_i1 & B04) >> 20; /* 37 */ \
_o0 |= (_i0 & B04) >> 21; /*  5 */ \
_o0 |= (_i1 & B12) >> 14; /* 45 */ \
_o0 |= (_i0 & B12) >> 15; /* 13 */ \
_o0 |= (_i1 & B20) >>  8; /* 53 */ \
_o0 |= (_i0 & B20) >>  9; /* 21 */ \
_o0 |= (_i1 & B28) >>  2; /* 61 */ \
_o0 |= (_i0 & B28) >>  3; /* 29 */ \
_o1 |= (_i1 & B03) <<  3; /* 36 */ \
_o1 |= (_i0 & B03) <<  2; /*  4 */ \
_o1 |= (_i1 & B11) <<  9; /* 44 */ \
_o1 |= (_i0 & B11) <<  8; /* 12 */ \
_o1 |= (_i1 & B19) << 15; /* 52 */ \
_o1 |= (_i0 & B19) << 14; /* 20 */ \
_o1 |= (_i1 & B27) << 21; /* 60 */ \
_o1 |= (_i0 & B27) << 20; /* 28 */ \
_o1 |= (_i1 & B02) >>  6; /* 35 */ \
_o1 |= (_i0 & B02) >>  7; /*  3 */ \
_o1 |= (_i1 & B10)      ; /* 43 */ \
_o1 |= (_i0 & B10) >>  1; /* 11 */ \
_o1 |= (_i1 & B18) <<  6; /* 51 */ \
_o1 |= (_i0 & B18) <<  5; /* 19 */ \
_o1 |= (_i1 & B26) << 12; /* 59 */ \
_o1 |= (_i0 & B26) << 11; /* 27 */ \
_o1 |= (_i1 & B01) >> 15; /* 34 */ \
_o1 |= (_i0 & B01) >> 16; /*  2 */ \
_o1 |= (_i1 & B09) >>  9; /* 42 */ \
_o1 |= (_i0 & B09) >> 10; /* 10 */ \
_o1 |= (_i1 & B17) >>  3; /* 50 */ \
_o1 |= (_i0 & B17) >>  4; /* 18 */ \
_o1 |= (_i1 & B25) <<  3; /* 58 */ \
_o1 |= (_i0 & B25) <<  2; /* 26 */ \
_o1 |= (_i1 & B00) >> 24; /* 33 */ \
_o1 |= (_i0 & B00) >> 25; /*  1 */ \
_o1 |= (_i1 & B08) >> 18; /* 41 */ \
_o1 |= (_i0 & B08) >> 19; /*  9 */ \
_o1 |= (_i1 & B16) >> 12; /* 49 */ \
_o1 |= (_i0 & B16) >> 13; /* 17 */ \
_o1 |= (_i1 & B24) >>  6; /* 57 */ \
_o1 |= (_i0 & B24) >>  7; /* 25 */ \
}

/* 64b -> 2x28b */
#define PC1_AUX(_i0, _i1, _o0, _o1) \
{ \
_o0 = _o1 = 0; \
_o0 |= (_i1 & B24) << 24; /* 57 */ \
_o0 |= (_i1 & B16) << 15; /* 49 */ \
_o0 |= (_i1 & B08) <<  6; /* 41 */ \
_o0 |= (_i1 & B00) >>  3; /* 33 */ \
_o0 |= (_i0 & B24) << 20; /* 25 */ \
_o0 |= (_i0 & B16) << 11; /* 17 */ \
_o0 |= (_i0 & B08) <<  2; /*  9 */ \
_o0 |= (_i0 & B00) >>  7; /*  1 */ \
_o0 |= (_i1 & B25) << 17; /* 58 */ \
_o0 |= (_i1 & B17) <<  8; /* 50 */ \
_o0 |= (_i1 & B09) >>  1; /* 42 */ \
_o0 |= (_i1 & B01) >> 10; /* 34 */ \
_o0 |= (_i0 & B25) << 13; /* 26 */ \
_o0 |= (_i0 & B17) <<  4; /* 18 */ \
_o0 |= (_i0 & B09) >>  5; /* 10 */ \
_o0 |= (_i0 & B01) >> 14; /*  2 */ \
_o0 |= (_i1 & B26) << 10; /* 59 */ \
_o0 |= (_i1 & B18) <<  1; /* 51 */ \
_o0 |= (_i1 & B10) >>  8; /* 43 */ \
_o0 |= (_i1 & B02) >> 17; /* 35 */ \
_o0 |= (_i0 & B26) <<  6; /* 27 */ \
_o0 |= (_i0 & B18) >>  3; /* 19 */ \
_o0 |= (_i0 & B10) >> 12; /* 11 */ \
_o0 |= (_i0 & B02) >> 21; /*  3 */ \
_o0 |= (_i1 & B27) <<  3; /* 60 */ \
_o0 |= (_i1 & B19) >>  6; /* 52 */ \
_o0 |= (_i1 & B11) >> 15; /* 44 */ \
_o0 |= (_i1 & B03) >> 24; /* 36 */ \
_o1 |= (_i1 & B30) << 30; /* 63 */ \
_o1 |= (_i1 & B22) << 21; /* 55 */ \
_o1 |= (_i1 & B14) << 12; /* 47 */ \
_o1 |= (_i1 & B06) <<  3; /* 39 */ \
_o1 |= (_i0 & B30) << 26; /* 31 */ \
_o1 |= (_i0 & B22) << 17; /* 23 */ \
_o1 |= (_i0 & B14) <<  8; /* 15 */ \
_o1 |= (_i0 & B06) >>  1; /*  7 */ \
_o1 |= (_i1 & B29) << 21; /* 62 */ \
_o1 |= (_i1 & B21) << 12; /* 54 */ \
_o1 |= (_i1 & B13) <<  3; /* 46 */ \
_o1 |= (_i1 & B05) >>  6; /* 38 */ \
_o1 |= (_i0 & B29) << 17; /* 30 */ \
_o1 |= (_i0 & B21) <<  8; /* 22 */ \
_o1 |= (_i0 & B13) >>  1; /* 14 */ \
_o1 |= (_i0 & B05) >> 10; /*  6 */ \
_o1 |= (_i1 & B28) << 12; /* 61 */ \
_o1 |= (_i1 & B20) <<  3; /* 53 */ \
_o1 |= (_i1 & B12) >>  6; /* 45 */ \
_o1 |= (_i1 & B04) >> 15; /* 37 */ \
_o1 |= (_i0 & B28) <<  8; /* 29 */ \
_o1 |= (_i0 & B20) >>  1; /* 21 */ \
_o1 |= (_i0 & B12) >> 10; /* 13 */ \
_o1 |= (_i0 & B04) >> 19; /*  5 */ \
_o1 |= (_i0 & B27) <<  3; /* 28 */ \
_o1 |= (_i0 & B19) >>  6; /* 20 */ \
_o1 |= (_i0 & B11) >> 15; /* 12 */ \
_o1 |= (_i0 & B03) >> 24; /*  4 */ \
}

/* 2x28b -> 8x6b */
#define PC2_AUX(_i0, _i1, _o0, _o1) \
{ \
_o0 = _o1 = 0; \
_o0 |= (_i0 & B13) << 11; /* 14 */ \
_o0 |= (_i0 & B16) << 13; /* 17 */ \
_o0 |= (_i0 & B10) <<  6; /* 11 */ \
_o0 |= (_i0 & B23) << 18; /* 24 */ \
_o0 |= (_i0 & B00) >>  6; /*  1 */ \
_o0 |= (_i0 & B04) >>  3; /*  5 */ \
_o0 |= (_i0 & B02) >>  8; /*  3 */ \
_o0 |= (_i0 & B27) << 16; /* 28 */ \
_o0 |= (_i0 & B14) <<  2; /* 15 */ \
_o0 |= (_i0 & B05) >>  8; /*  6 */ \
_o0 |= (_i0 & B20) <<  6; /* 21 */ \
_o0 |= (_i0 & B09) >>  6; /* 10 */ \
_o0 |= (_i0 & B22) <<  4; /* 23 */ \
_o0 |= (_i0 & B18) >>  1; /* 19 */ \
_o0 |= (_i0 & B11) >>  9; /* 12 */ \
_o0 |= (_i0 & B03) >> 18; /*  4 */ \
_o0 |= (_i0 & B25) <<  3; /* 26 */ \
_o0 |= (_i0 & B07) >> 16; /*  8 */ \
_o0 |= (_i0 & B15) >> 11; /* 16 */ \
_o0 |= (_i0 & B06) >> 21; /*  7 */ \
_o0 |= (_i0 & B26) >>  2; /* 27 */ \
_o0 |= (_i0 & B19) >> 10; /* 20 */ \
_o0 |= (_i0 & B12) >> 18; /* 13 */ \
_o0 |= (_i0 & B01) >> 30; /*  2 */ \
_o1 |= (_i1 & B12) << 10; /* 41 */ \
_o1 |= (_i1 & B23) << 20; /* 52 */ \
_o1 |= (_i1 & B02) >>  2; /* 31 */ \
_o1 |= (_i1 & B08) <<  3; /* 37 */ \
_o1 |= (_i1 & B18) << 12; /* 47 */ \
_o1 |= (_i1 & B26) << 19; /* 55 */ \
_o1 |= (_i1 & B01) >>  9; /* 30 */ \
_o1 |= (_i1 & B11)      ; /* 40 */ \
_o1 |= (_i1 & B22) << 10; /* 51 */ \
_o1 |= (_i1 & B16) <<  3; /* 45 */ \
_o1 |= (_i1 & B04) >> 10; /* 33 */ \
_o1 |= (_i1 & B19) <<  4; /* 48 */ \
_o1 |= (_i1 & B15) >>  3; /* 44 */ \
_o1 |= (_i1 & B20) <<  1; /* 49 */ \
_o1 |= (_i1 & B10) >> 10; /* 39 */ \
_o1 |= (_i1 & B27) <<  6; /* 56 */ \
_o1 |= (_i1 & B05) >> 17; /* 34 */ \
_o1 |= (_i1 & B24) <<  1; /* 53 */ \
_o1 |= (_i1 & B17) >>  9; /* 46 */ \
_o1 |= (_i1 & B13) >> 14; /* 42 */ \
_o1 |= (_i1 & B21) >>  7; /* 50 */ \
_o1 |= (_i1 & B07) >> 22; /* 36 */ \
_o1 |= (_i1 & B00) >> 30; /* 29 */ \
_o1 |= (_i1 & B03) >> 28; /* 32 */ \
}

static
Word s_p0[64] =
{ /* Combined S-Box1 and permutation P */
0x00808200, 0x00000000, 0x00008000, 0x00808202,
0x00808002, 0x00008202, 0x00000002, 0x00008000,
0x00000200, 0x00808200, 0x00808202, 0x00000200,
0x00800202, 0x00808002, 0x00800000, 0x00000002,
0x00000202, 0x00800200, 0x00800200, 0x00008200,
0x00008200, 0x00808000, 0x00808000, 0x00800202,
0x00008002, 0x00800002, 0x00800002, 0x00008002,
0x00000000, 0x00000202, 0x00008202, 0x00800000,
0x00008000, 0x00808202, 0x00000002, 0x00808000,
0x00808200, 0x00800000, 0x00800000, 0x00000200,
0x00808002, 0x00008000, 0x00008200, 0x00800002,
0x00000200, 0x00000002, 0x00800202, 0x00008202,
0x00808202, 0x00008002, 0x00808000, 0x00800202,
0x00800002, 0x00000202, 0x00008202, 0x00808200,
0x00000202, 0x00800200, 0x00800200, 0x00000000,
0x00008002, 0x00008200, 0x00000000, 0x00808002
};

static
Word s_p1[64] =
{ /* Combined S-Box2 and permutation P */
0x40084010, 0x40004000, 0x00004000, 0x00084010,
0x00080000, 0x00000010, 0x40080010, 0x40004010,
0x40000010, 0x40084010, 0x40084000, 0x40000000,
0x40004000, 0x00080000, 0x00000010, 0x40080010,
0x00084000, 0x00080010, 0x40004010, 0x00000000,
0x40000000, 0x00004000, 0x00084010, 0x40080000,
0x00080010, 0x40000010, 0x00000000, 0x00084000,
0x00004010, 0x40084000, 0x40080000, 0x00004010,
0x00000000, 0x00084010, 0x40080010, 0x00080000,
0x40004010, 0x40080000, 0x40084000, 0x00004000,
0x40080000, 0x40004000, 0x00000010, 0x40084010,
0x00084010, 0x00000010, 0x00004000, 0x40000000,
0x00004010, 0x40084000, 0x00080000, 0x40000010,
0x00080010, 0x40004010, 0x40000010, 0x00080010,
0x00084000, 0x00000000, 0x40004000, 0x00004010,
0x40000000, 0x40080010, 0x40084010, 0x00084000
};

static
Word s_p2[64] =
{ /* Combined S-Box3 and permutation P */
0x00000104, 0x04010100, 0x00000000, 0x04010004,
0x04000100, 0x00000000, 0x00010104, 0x04000100,
0x00010004, 0x04000004, 0x04000004, 0x00010000,
0x04010104, 0x00010004, 0x04010000, 0x00000104,
0x04000000, 0x00000004, 0x04010100, 0x00000100,
0x00010100, 0x04010000, 0x04010004, 0x00010104,
0x04000104, 0x00010100, 0x00010000, 0x04000104,
0x00000004, 0x04010104, 0x00000100, 0x04000000,
0x04010100, 0x04000000, 0x00010004, 0x00000104,
0x00010000, 0x04010100, 0x04000100, 0x00000000,
0x00000100, 0x00010004, 0x04010104, 0x04000100,
0x04000004, 0x00000100, 0x00000000, 0x04010004,
0x04000104, 0x00010000, 0x04000000, 0x04010104,
0x00000004, 0x00010104, 0x00010100, 0x04000004,
0x04010000, 0x04000104, 0x00000104, 0x04010000,
0x00010104, 0x00000004, 0x04010004, 0x00010100
};

static
Word s_p3[64] =
{ /* Combined S-Box4 and permutation P */
0x80401000, 0x80001040, 0x80001040, 0x00000040,
0x00401040, 0x80400040, 0x80400000, 0x80001000,
0x00000000, 0x00401000, 0x00401000, 0x80401040,
0x80000040, 0x00000000, 0x00400040, 0x80400000,
0x80000000, 0x00001000, 0x00400000, 0x80401000,
0x00000040, 0x00400000, 0x80001000, 0x00001040,
0x80400040, 0x80000000, 0x00001040, 0x00400040,
0x00001000, 0x00401040, 0x80401040, 0x80000040,
0x00400040, 0x80400000, 0x00401000, 0x80401040,
0x80000040, 0x00000000, 0x00000000, 0x00401000,
0x00001040, 0x00400040, 0x80400040, 0x80000000,
0x80401000, 0x80001040, 0x80001040, 0x00000040,
0x80401040, 0x80000040, 0x80000000, 0x00001000,
0x80400000, 0x80001000, 0x00401040, 0x80400040,
0x80001000, 0x00001040, 0x00400000, 0x80401000,
0x00000040, 0x00400000, 0x00001000, 0x00401040
};

static
Word s_p4[64] =
{ /* Combined S-Box5 and permutation P */
0x00000080, 0x01040080, 0x01040000, 0x21000080,
0x00040000, 0x00000080, 0x20000000, 0x01040000,
0x20040080, 0x00040000, 0x01000080, 0x20040080,
0x21000080, 0x21040000, 0x00040080, 0x20000000,
0x01000000, 0x20040000, 0x20040000, 0x00000000,
0x20000080, 0x21040080, 0x21040080, 0x01000080,
0x21040000, 0x20000080, 0x00000000, 0x21000000,
0x01040080, 0x01000000, 0x21000000, 0x00040080,
0x00040000, 0x21000080, 0x00000080, 0x01000000,
0x20000000, 0x01040000, 0x21000080, 0x20040080,
0x01000080, 0x20000000, 0x21040000, 0x01040080,
0x20040080, 0x00000080, 0x01000000, 0x21040000,
0x21040080, 0x00040080, 0x21000000, 0x21040080,
0x01040000, 0x00000000, 0x20040000, 0x21000000,
0x00040080, 0x01000080, 0x20000080, 0x00040000,
0x00000000, 0x20040000, 0x01040080, 0x20000080
};

static
Word s_p5[64] =
{ /* Combined S-Box6 and permutation P */
0x10000008, 0x10200000, 0x00002000, 0x10202008,
0x10200000, 0x00000008, 0x10202008, 0x00200000,
0x10002000, 0x00202008, 0x00200000, 0x10000008,
0x00200008, 0x10002000, 0x10000000, 0x00002008,
0x00000000, 0x00200008, 0x10002008, 0x00002000,
0x00202000, 0x10002008, 0x00000008, 0x10200008,
0x10200008, 0x00000000, 0x00202008, 0x10202000,
0x00002008, 0x00202000, 0x10202000, 0x10000000,
0x10002000, 0x00000008, 0x10200008, 0x00202000,
0x10202008, 0x00200000, 0x00002008, 0x10000008,
0x00200000, 0x10002000, 0x10000000, 0x00002008,
0x10000008, 0x10202008, 0x00202000, 0x10200000,
0x00202008, 0x10202000, 0x00000000, 0x10200008,
0x00000008, 0x00002000, 0x10200000, 0x00202008,
0x00002000, 0x00200008, 0x10002008, 0x00000000,
0x10202000, 0x10000000, 0x00200008, 0x10002008
};

static
Word s_p6[64] =
{ /* Combined S-Box7 and permutation P */
0x00100000, 0x02100001, 0x02000401, 0x00000000,
0x00000400, 0x02000401, 0x00100401, 0x02100400,
0x02100401, 0x00100000, 0x00000000, 0x02000001,
0x00000001, 0x02000000, 0x02100001, 0x00000401,
0x02000400, 0x00100401, 0x00100001, 0x02000400,
0x02000001, 0x02100000, 0x02100400, 0x00100001,
0x02100000, 0x00000400, 0x00000401, 0x02100401,
0x00100400, 0x00000001, 0x02000000, 0x00100400,
0x02000000, 0x00100400, 0x00100000, 0x02000401,
0x02000401, 0x02100001, 0x02100001, 0x00000001,
0x00100001, 0x02000000, 0x02000400, 0x00100000,
0x02100400, 0x00000401, 0x00100401, 0x02100400,
0x00000401, 0x02000001, 0x02100401, 0x02100000,
0x00100400, 0x00000000, 0x00000001, 0x02100401,
0x00000000, 0x00100401, 0x02100000, 0x00000400,
0x02000001, 0x02000400, 0x00000400, 0x00100001
};

static
Word s_p7[64] =
{ /* Combined S-Box8 and permutation P */
0x08000820, 0x00000800, 0x00020000, 0x08020820,
0x08000000, 0x08000820, 0x00000020, 0x08000000,
0x00020020, 0x08020000, 0x08020820, 0x00020800,
0x08020800, 0x00020820, 0x00000800, 0x00000020,
0x08020000, 0x08000020, 0x08000800, 0x00000820,
0x00020800, 0x00020020, 0x08020020, 0x08020800,
0x00000820, 0x00000000, 0x00000000, 0x08020020,
0x08000020, 0x08000800, 0x00020820, 0x00020000,
0x00020820, 0x00020000, 0x08020800, 0x00000800,
0x00000020, 0x08020020, 0x00000800, 0x00020820,
0x08000800, 0x00000020, 0x08000020, 0x08020000,
0x08020020, 0x08000000, 0x00020000, 0x08000820,
0x00000000, 0x08020820, 0x00020020, 0x08000020,
0x08020000, 0x08000800, 0x08000820, 0x00000000,
0x08020820, 0x00020800, 0x00020800, 0x00000820,
0x00000820, 0x00020020, 0x08000000, 0x08020800
};

#endif /* !_qfDES_aux_h_ */