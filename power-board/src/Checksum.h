#pragma once

#ifndef VFUNC

  #define VFUNC_NARG(...) \
      VFUNC_NARG_(__VA_ARGS__, VFUNC_RSEQ_N_())

  #define VFUNC_NARG_(...) \
      VFUNC_ARG_N_(__VA_ARGS__)
  
  #define VFUNC_ARG_N_( \
       _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
      _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
      _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
      _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
      _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
      _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
      _61,_62,_63,  N, ...) N
  
  #define VFUNC_RSEQ_N_() \
      63,62,61,60,                   \
      59,58,57,56,55,54,53,52,51,50, \
      49,48,47,46,45,44,43,42,41,40, \
      39,38,37,36,35,34,33,32,31,30, \
      29,28,27,26,25,24,23,22,21,20, \
      19,18,17,16,15,14,13,12,11,10, \
       9, 8, 7, 6, 5, 4, 3, 2, 1, 0
  
  #define VFUNC_EXPAND_(...)   __VA_ARGS__
  
  #define VFUNC_NAME_(_func, _n)   VFUNC_N_(_func ## _, _n)
  #define VFUNC_N_(_func, _n)  _func ## _n

  #define VFUNC(_func, ...) \
    VFUNC_EXPAND_(VFUNC_NAME_(_func, VFUNC_NARG(__VA_ARGS__)) (__VA_ARGS__))

#endif

#define CRC1(...)           VFUNC(CRC1, __VA_ARGS__)
#define CRC1_1(_n)          (_n)
#define CRC1_2(_n, ...)     ((_n) ^ CRC1_1(__VA_ARGS__))
#define CRC1_3(_n, ...)     ((_n) ^ CRC1_2(__VA_ARGS__))
#define CRC1_4(_n, ...)     ((_n) ^ CRC1_3(__VA_ARGS__))
#define CRC1_5(_n, ...)     ((_n) ^ CRC1_4(__VA_ARGS__))
#define CRC1_6(_n, ...)     ((_n) ^ CRC1_5(__VA_ARGS__))
#define CRC1_7(_n, ...)     ((_n) ^ CRC1_6(__VA_ARGS__))
#define CRC1_8(_n, ...)     ((_n) ^ CRC1_7(__VA_ARGS__))

#define NBYTE(n, x)         ((uint8_t)(((x) >> (8 * (n))) & 0xFF))
#define LOBYTE(x)           NBYTE(0, x)
#define HIBYTE(x)           NBYTE(1, x)

#define CRC(...)            CRC1(__VA_ARGS__)

#define CRC8(x)             CRC(NBYTE(0, x))
#define CRC16(x)            CRC(NBYTE(0, x), NBYTE(1, x))
#define CRC32(x)            CRC(NBYTE(0, x), NBYTE(1, x), NBYTE(2, x), NBYTE(3, x))
