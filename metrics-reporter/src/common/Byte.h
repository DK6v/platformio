#pragma once

#include <stdint.h>

/* Number of bits in basic data type
*/
#define BYTE_LEN    (8)
#define CHAR_LEN    (BYTE_LEN * sizeof(int))
#define WORD_LEN    (BYTE_LEN * sizeof(int))
#define DWORD_LEN   (BYTE_LEN * sizeof(int))

/* Macro VFUNC_NARG() returns the number of variadic arguments passed to it.
 * Example:
 *   VFUNC_NARG(a, b, c) => 3
 */

#define VFUNC_NARG(...) \
    VFUNC_NARG_(__VA_ARGS__, VFUNC_RSEQ_N_())

#define VFUNC_NARG_(...) \
    VFUNC_ARG_N_(__VA_ARGS__)

#define VFUNC_ARG_N_(                                 \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,          \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
    _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
    _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, \
    _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, \
    _61, _62, _63, N, ...) N

#define VFUNC_RSEQ_N_()                         \
    63, 62, 61, 60,                             \
        59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
        49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
        39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
        29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
        19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
        9, 8, 7, 6, 5, 4, 3, 2, 1, 0

/* Macro to get an argument by position number:
 *   VFUNC_PARG(_position, ...)
 * Example:
 *   VFUNC_PARG(3, A, B, C, D, E) => C
 */
#define VFUNC_PARG(_position, ...) \
    VFUNC_PARG_##_position(__VA_ARGS__)

#define VFUNC_PARG_1(N, ...) N
#define VFUNC_PARG_2(_1, N, ...) N
#define VFUNC_PARG_3(_1, _2, N, ...) N
#define VFUNC_PARG_4(_1, _2, _3, N, ...) N
#define VFUNC_PARG_5(_1, _2, _3, _4, N, ...) N
#define VFUNC_PARG_6(_1, _2, _3, _4, _5, N, ...) N
#define VFUNC_PARG_7(_1, _2, _3, _4, _5, _6, N, ...) N
#define VFUNC_PARG_8(_1, _2, _3, _4, _5, _6, _7, N, ...) N
#define VFUNC_PARG_9(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N

/* Macro FUNC(_func, ...) expands the function with a number of arguments.
 * Example:
 *   VFUNC(NAME, a, b, c) => NAME_3
 */
#define VFUNC_N(_func, _n) _func##_n
#define VFUNC_NAME(_func, _n) VFUNC_N(_func##_, _n)
#define VFUNC_EXPAND(...) __VA_ARGS__

#define VFUNC(_func, ...) \
    VFUNC_EXPAND(VFUNC_NAME(_func, VFUNC_NARG(__VA_ARGS__))(__VA_ARGS__))

/* A macro for extracting a single byte from a specified position:
 *   NBYTE(position, number).
 * Example:
 *   NBYTE(2, 0x04030201) => 0x03
 */
#define NBYTE(_position, _n)    (((_n) >> (8 * (_position))) & 0xFF)

/* Macros to extract the highest and lowest bytes of a 16-bit integer:
 *   LOBYTE(number), HIBYTE(number).
 * Example:
 *   LOBYTE(0x0201) => (0x01)
 *   HIBYTE(0x0201) => (0x02)
 */
#define LOBYTE(_n) NBYTE(0, (_n))
#define HIBYTE(_n) NBYTE(1, (_n))

/* A macro to extract the highest and lowest bytes of a 32-bit integer:
 *   LLBYTE(number), HLBYTE(number), LHBYTE(number), HHBYTE(number).
 * Example:
 *   LLBYTE(0x04030201) => (0x01)
 *   HLBYTE(0x04030201) => (0x02)
 *   LHBYTE(0x04030201) => (0x03)
 *   HHBYTE(0x04030201) => (0x04)
 */
#define LLBYTE(_n) NBYTE(0, (_n))
#define HLBYTE(_n) NBYTE(1, (_n))
#define LHBYTE(_n) NBYTE(2, (_n))
#define HHBYTE(_n) NBYTE(3, (_n))

/* A macro to set a single byte at a specific position:
 *   BYTE_SET(position, number, value).
 * Example:
 *   BYTE_SET(2, 0x04030201, 0xFF) => 0x04FF0201
 */
#define BYTE_SET(_position, _number, _value) \
    (((_number) & ~((0xFFUL) << (8 * (_position)))) | ((0x0UL | ((_value) & 0xFF)) << (8 * (_position))))


/* Macro to reverse the order of bits in a byte.
 *   BYTE_REVERSE(byte)
 * Example:
 *   BYTE_REVERSE(0b00000001) => 0b10000000
 */
#define BYTE_REVERSE(_b) (((((_b) & 1)   ? 128 : 0) | \
                           (((_b) & 2)   ? 64  : 0) | \
                           (((_b) & 4)   ? 32  : 0) | \
                           (((_b) & 8)   ? 16  : 0) | \
                           (((_b) & 16)  ? 8   : 0) | \
                           (((_b) & 32)  ? 4   : 0) | \
                           (((_b) & 64)  ? 2   : 0) | \
                           (((_b) & 128) ? 1   : 0)) & 0xFF)

/* Macros to caclulace bitwise XOR operation:
 *   BYTE_XOR(number, ...)
 * Example:
 *   BYTE_XOR(a, b, c) => (a ^ b ^ c)
 */
#define BYTE_XOR(...) VFUNC(BYTE_XOR, __VA_ARGS__)
#define BYTE_XOR_1(_n) ((_n) & 0xFF)
#define BYTE_XOR_2(_n, ...) ((_n) ^ BYTE_XOR_1(__VA_ARGS__))
#define BYTE_XOR_3(_n, ...) ((_n) ^ BYTE_XOR_2(__VA_ARGS__))
#define BYTE_XOR_4(_n, ...) ((_n) ^ BYTE_XOR_3(__VA_ARGS__))
#define BYTE_XOR_5(_n, ...) ((_n) ^ BYTE_XOR_4(__VA_ARGS__))
#define BYTE_XOR_6(_n, ...) ((_n) ^ BYTE_XOR_5(__VA_ARGS__))
#define BYTE_XOR_7(_n, ...) ((_n) ^ BYTE_XOR_6(__VA_ARGS__))
#define BYTE_XOR_8(_n, ...) ((_n) ^ BYTE_XOR_7(__VA_ARGS__))

/* Expand the integer to a comma-separated list of bytes:
 *   BYTE8(char), BYTE16(short), BYTE32(integer).
 * Example:
 *   BYTE8(0x01) => 0x01
 *   BYTE16(0x0201) => 0x01, 0x02
 *   BYTE32(0x04030201) => 0x01, 0x02, 0x03, 0x03
 */

#define BYTE8(_n)  BYTE_XOR(NBYTE(0, (_n)))
#define BYTE16(_n) BYTE_XOR(NBYTE(0, (_n)), NBYTE(1, (_n)))
#define BYTE32(_n) BYTE_XOR(NBYTE(0, (_n)), NBYTE(1, (_n)), NBYTE(2, (_n)), NBYTE(3, (_n)))

/* Macro to limit the value within a certain range:
 *   MIN(_number, _min)
 *   MAX(_number, _max)
 *   RANGE(_number, _min, _max)
 * Example:
 *   RANGE(1, 2, 4) => 2
 *   RANGE(3, 2, 4) => 3
 *   RANGE(5, 2, 4) => 4
 */
#define RANGE(value, min, max) \
    (((value) < (min)) ? (min) : (((value) > (max)) ? (max) : (value)))

/* Macro for raising a number to a power:
 *   POWER(_number, _degree)
 * Example:
 *   POWER(3, 2) => (3 * 3)
 */
#define POWER(_number, _degree) POWER_##_degree(_number)
#define POWER_1(_number) (_number)
#define POWER_2(_number) ((_number) * POWER_1(_number))
#define POWER_3(_number) ((_number) * POWER_2(_number))
#define POWER_4(_number) ((_number) * POWER_3(_number))
#define POWER_5(_number) ((_number) * POWER_4(_number))
#define POWER_6(_number) ((_number) * POWER_5(_number))
#define POWER_7(_number) ((_number) * POWER_6(_number))
#define POWER_8(_number) ((_number) * POWER_7(_number))
#define POWER_9(_number) ((_number) * POWER_8(_number))

/* Macro for raising the number 10 to a power:
 *   POWER10(_degree)
 * Example:
 *   POWER10(2) => 100
 */
#define POWER10(_degree) VFUNC_PARG(_degree, POWER10_SEQ_)

#define POWER10_SEQ_ 10, \
                     100, \
                     1000, \
                     10000, \
                     100000, \
                     1000000, \
                     10000000, \
                     100000000, \
                     1000000000

/* A macro for rounding a floating-point number to the closest number
 * with a specified precision:
 *   ROUNDF(_number, _presision)
 * Example:
 *   ROUNDF(123.456789, 2) => 123.46
 */
#define ROUNDF(_number, _precision) \
    (((int32_t)((_number) * (POWER10(_precision) * 1.0) + 0.5)) / ((POWER10(_precision) * 1.0)))

/* A macro to determine the endianness of the machine:
 * with a specified precision:
 *   IS_BIG_ENDIAN
 * Example:
 *   if (IS_BIG_ENDIAN) { ... }
 */
#define IS_BIG_ENDIAN (!(union { uint16_t u16; unsigned char c; }){ .u16 = 1 }.c)
