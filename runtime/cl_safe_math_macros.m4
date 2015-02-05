dnl Safe math macros for OpenCL built-in integer functions.
dnl Each macro usually has a variant for each of the 4 basic signed types, when
dnl further combined with 4 vector sizes, this would lead to 20 different macros
dnl for each function. To prevent this, we bundle a runtime type in as a
dnl parameter for the macro.

define(`safe_signed_math',`

dnl The check here is probably not necessary, we do not output it but keep it
dnl here for now.
#define safe_mul_hi_func_$1_s_s(_t1,_si1,_si2) \
  ({ _t1 si1 = (_si1) ; _t1 si2 = (_si2) ; \
  ((((si1) > (($1)0)) && ((si2) > (($1)0)) && ((si1) > (($3) / (si2)))) || \
  (((si1) > (($1)0)) && ((si2) <= (($1)0)) && ((si2) < (($2) / (si1)))) || \
  (((si1) <= (($1)0)) && ((si2) > (($1)0)) && ((si1) < (($2) / (si2)))) || \
  (((si1) <= (($1)0)) && ((si2) <= (($1)0)) && (si1) != (($1)0)) && ((si2) < (($3) / (si1))))) \
  ? (si1) \
  : mul_hi((si1), (si2));})

dnl As before the first check for the mul_hi part is not needed.
#define safe_mad_hi_func_$1_s_s_s(_t1,_si1,_si2,_si3) \
  ({ _t1 si1 = (_si1) ; _t1 si2 = (_si2) ; _t1 si3 = (_si3) ; \
dnl  ((((_si1) > (($1)0)) && ((_si2) > (($1)0)) && ((_si1) > (($3) / (_si2)))) || \
dnl  (((_si1) > (($1)0)) && ((_si2) <= (($1)0)) && ((_si2) < (($2) / (_si1)))) || \
dnl  (((_si1) <= (($1)0)) && ((_si2) > (($1)0)) && ((_si1) < (($2) / (_si2)))) || \
dnl  (((_si1) <= (($1)0)) && ((_si2) <= (($1)0)) && ((_si1) != (($1)0)) && ((_si2) < (($3) / (_si1))))) \
dnl  ? (_si1) \
dnl  : \
    ({ _t1 tmp = mul_hi((si1), (si2)) ; \
    ((((tmp) > (($1)0)) && ((si3) > (($1)0)) && ((tmp) > ((($1)($3))-(si3)))) || \
    (((tmp) < (($1)0)) && ((si3) < (($1)0)) && ((tmp) < ((($1)($2))-(si3))))) \
    ? (si1) \
    : mad_hi((si1), (si2), (si3));}) \
  ;})

')

safe_signed_math(int8_t,INT8_MIN,INT8_MAX)
safe_signed_math(int16_t,INT16_MIN,INT16_MAX)
safe_signed_math(int32_t,INT32_MIN,INT32_MAX)
safe_signed_math(int64_t,INT64_MIN,INT64_MAX)

define(`safe_signed_24bit_math',`

#define safe_mul24_func_$1_s_s(_t1,_si1,_si2) \
  ({ _t1 si1 = (_si1) ; _t1 si2 = (_si2) ; \
  ((((si1) < ($4)) || ((si1) > ($5)) || ((si2) < ($4)) || ((si2) > ($5))) || \
  (((si1) > (($1)0)) && ((si2) > (($1)0)) && ((si1) > (($3) / (si2)))) || \
  (((si1) > (($1)0)) && ((si2) <= (($1)0)) && ((si2) < (($2) / (si1)))) || \
  (((si1) <= (($1)0)) && ((si2) > (($1)0)) && ((si1) < (($2) / (si2)))) || \
  (((si1) <= (($1)0)) && ((si2) <= (($1)0)) && ((si1) != (($1)0)) && ((si2) < (($3) / (si1))))) \
  ? (si1) \
  : mul24((si1), (si2));})

#define safe_mad24_func_$1_s_s_s(_t1,_si1,_si2,_si3) \
  ({ _t1 si1 = (_si1) ; _t1 si2 = (_si2) ; _t1 si3 = (_si3) ; \
  ((((si1) < ($4)) || ((si1) > ($5)) || ((si2) < ($4)) || ((si2) > ($5))) || \
  (((si1) > (($1)0)) && ((si2) > (($1)0)) && ((si1) > (($3) / (si2)))) || \
  (((si1) > (($1)0)) && ((si2) <= (($1)0)) && ((si2) < (($2) / (si1)))) || \
  (((si1) <= (($1)0)) && ((si2) > (($1)0)) && ((si1) < (($2) / (si2)))) || \
  (((si1) <= (($1)0)) && ((si2) <= (($1)0)) && ((si1) != (($1)0)) && ((si2) < (($3) / (si1))))) \
  ? (si1) \
  : ({ _t1 tmp = mul24((si1), (si2)) ; \
    ((((tmp) > (($1)0)) && ((si3) > (($1)0)) && ((tmp) > (($3)-(si3)))) || \
    (((tmp) < (($1)0)) && ((si3) < (($1)0)) && ((tmp) < (($2)-(si3))))) \
    ? (si1) \
    : mad24((si1), (si2), (si3));});})

')

safe_signed_24bit_math(int32_t,INT32_MIN,INT32_MAX,-(1<<23),(1<<23)-1)

define(`safe_unsigned_24bit_math',`

#define safe_mul24_func_$1_u_u(_t1,_ui1,_ui2) \
  ({ _t1 ui1 = (_ui1) ; _t1 ui2 = (_ui2) ; \
  (((ui1) < ($2)) || ((ui1) > ($3)) || ((ui2) < ($2)) || ((ui2) > ($3))) \
  ? (ui1) \
  : mul24((ui1), (ui2));})

#define safe_mad24_func_$1_u_u_u(_t1,_ui1,_ui2,_ui3) \
  ({ _t1 ui1 = (_ui1) ; _t1 ui2 = (_ui2) ; _t1 ui3 = (_ui3) ; \
  (((ui1) < ($2)) || ((ui1) > ($3)) || ((ui2) < ($2)) || ((ui2) > ($3))) \
  ? (ui1) \
  : mad24((ui1), (ui2), (ui3));})

')

safe_unsigned_24bit_math(uint32_t,0,1<<24)

define(`safe_math',`

#define safe_clamp_func(_t1,_t2,_x,_y,_z) \
  ({ _t1 x = (_x) ; _t2 y = (_y) ; _t2 z = (_z) ; \
  ((y) > (z)) \
  ? (x) \
  : clamp((x), (y), (z));})

')

safe_math()
