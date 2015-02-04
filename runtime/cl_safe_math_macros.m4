dnl Safe math macros for OpenCL built-in integer functions.

define(`safe_signed_math',`

dnl The check here is probably not necessary, we do not output it but keep it
dnl here for now.
#define safe_mul_hi_func_$1_s_s(_si1,_si2) \
  ({ \
  ((((_si1) > (($1)0)) && ((_si2) > (($1)0)) && ((_si1) > (($3) / (_si2)))) || \
  (((_si1) > (($1)0)) && ((_si2) <= (($1)0)) && ((_si2) < (($2) / (_si1)))) || \
  (((_si1) <= (($1)0)) && ((_si2) > (($1)0)) && ((_si1) < (($2) / (_si2)))) || \
  (((_si1) <= (($1)0)) && ((_si2) <= (($1)0)) && ((_si1) != (($1)0)) && ((_si2) < (($3) / (_si1))))) \
  ? (_si1) \
  : mul_hi((_si1), (_si2));})

dnl As before the first check for the mul_hi part is not needed.
#define safe_mad_hi_func_$1_s_s_s(_si1,_si2,_si3,_tmp) \
dnl  ({ \
dnl  ((((_si1) > (($1)0)) && ((_si2) > (($1)0)) && ((_si1) > (($3) / (_si2)))) || \
dnl  (((_si1) > (($1)0)) && ((_si2) <= (($1)0)) && ((_si2) < (($2) / (_si1)))) || \
dnl  (((_si1) <= (($1)0)) && ((_si2) > (($1)0)) && ((_si1) < (($2) / (_si2)))) || \
dnl  (((_si1) <= (($1)0)) && ((_si2) <= (($1)0)) && ((_si1) != (($1)0)) && ((_si2) < (($3) / (_si1))))) \
dnl  ? (_si1) \
dnl  : \
    ({(_tmp) = mul_hi((_si1), (_si2)) ; \
    ((((_tmp) > (($1)0)) && ((_si3) > (($1)0)) && ((_tmp) > (($3)-(_si3)))) || \
    (((_tmp) < (($1)0)) && ((_si3) < (($1)0)) && ((_tmp) < (($2)-(_si3))))) \
    ? (_si1) \
    : mad_hi((_si1), (_si2), (_si3));})
dnl  ;})

')

safe_signed_math(int8_t,INT8_MIN,INT8_MAX)
safe_signed_math(int16_t,INT16_MIN,INT16_MAX)
safe_signed_math(int32_t,INT32_MIN,INT32_MAX)
safe_signed_math(int64_t,INT64_MIN,INT64_MAX)

define(`safe_signed_24bit_math',`

#define safe_mul24_func_$1_s_s(_si1,_si2) \
  ({ \
  ((((_si1) < ($4)) || ((_si1) > ($5)) || ((_si2) < ($4)) || ((_si2) > ($5))) || \
  (((_si1) > (($1)0)) && ((_si2) > (($1)0)) && ((_si1) > (($3) / (_si2)))) || \
  (((_si1) > (($1)0)) && ((_si2) <= (($1)0)) && ((_si2) < (($2) / (_si1)))) || \
  (((_si1) <= (($1)0)) && ((_si2) > (($1)0)) && ((_si1) < (($2) / (_si2)))) || \
  (((_si1) <= (($1)0)) && ((_si2) <= (($1)0)) && ((_si1) != (($1)0)) && ((_si2) < (($3) / (_si1))))) \
  ? (_si1) \
  : mul24((_si1), (_si2));})

#define safe_mad24_func_$1_s_s_s(_si1,_si2,_si3,_tmp) \
  ({ \
  ((((_si1) < ($4)) || ((_si1) > ($5)) || ((_si2) < ($4)) || ((_si2) > ($5))) || \
  (((_si1) > (($1)0)) && ((_si2) > (($1)0)) && ((_si1) > (($3) / (_si2)))) || \
  (((_si1) > (($1)0)) && ((_si2) <= (($1)0)) && ((_si2) < (($2) / (_si1)))) || \
  (((_si1) <= (($1)0)) && ((_si2) > (($1)0)) && ((_si1) < (($2) / (_si2)))) || \
  (((_si1) <= (($1)0)) && ((_si2) <= (($1)0)) && ((_si1) != (($1)0)) && ((_si2) < (($3) / (_si1))))) \
  ? (_si1) \
  : ({(_tmp) = mul24((_si1), (_si2)) ; \
    ((((_tmp) > (($1)0)) && ((_si3) > (($1)0)) && ((_tmp) > (($3)-(_si3)))) || \
    (((_tmp) < (($1)0)) && ((_si3) < (($1)0)) && ((_tmp) < (($2)-(_si3))))) \
    ? (_si1) \
    : mad24((_si1), (_si2), (_si3));});})

')

safe_signed_24bit_math(int32_t,INT32_MIN,INT32_MAX,-(1<<23),(1<<23)-1)

define(`safe_unsigned_24bit_math',`

#define safe_mul24_func_$1_u_u(_ui1,_ui2) \
  ((((_ui1) < ($2)) || ((_ui1) > ($3)) || ((_ui2) < ($2)) || ((_ui2) > ($3))) \
  ? (_ui1) \
  : mul24((_ui1), (_ui2)))

#define safe_mad24_func_$1_u_u_u(_si1,_si2,_si3) \
  ((((_ui1) < ($2)) || ((_ui1) > ($3)) || ((_ui2) < ($2)) || ((_ui2) > ($3))) \
  ? (_ui1) \
  : mad24((_ui1), (_ui2)))

')

safe_unsigned_24bit_math(uint32_t,0,1<<24)

define(`safe_math',`

#define safe_clamp_func(_x,_y,_z) \
  (((_y) > (_z)) \
  ? (_x) \
  : clamp((_x), (_y), (_z)))

')

safe_math()
