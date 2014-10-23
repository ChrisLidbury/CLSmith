dnl -*- mode: m4 -*-
dnl
dnl Copyright (c) 2008, 2009 The University of Utah
dnl All rights reserved.
dnl
dnl This file is part of `csmith', a random generator of C programs.
dnl
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions are met:
dnl
dnl   * Redistributions of source code must retain the above copyright notice,
dnl     this list of conditions and the following disclaimer.
dnl
dnl   * Redistributions in binary form must reproduce the above copyright
dnl     notice, this list of conditions and the following disclaimer in the
dnl     documentation and/or other materials provided with the distribution.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
dnl AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
dnl IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
dnl ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
dnl LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
dnl CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
dnl SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
dnl INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
dnl CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
dnl ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
dnl POSSIBILITY OF SUCH DAMAGE.
dnl
dnl For more information, see:
dnl https://www.securecoding.cert.org/confluence/display/cplusplus/04.+Integers+(INT)

#ifndef SAFE_MATH_H
#define SAFE_MATH_H

define(`safe_signed_math',`

#define safe_unary_minus_func_$1_s(_si) \
  ({ $1 si = (_si) ; \
   ((($1)(si))==($2))? \
    (($1)(si)): \
    (-(($1)(si))) \
  ;})

#define safe_add_func_$1_s_s(_si1,_si2) \
		({ $1 si1 = (_si1); $1 si2 = (_si2) ; \
                 ((((($1)(si1))>(($1)0)) && ((($1)(si2))>(($1)0)) && ((($1)(si1)) > (($3)-(($1)(si2))))) \
		  || (((($1)(si1))<(($1)0)) && ((($1)(si2))<(($1)0)) && ((($1)(si1)) < (($2)-(($1)(si2)))))) ? \
		 (($1)(si1)) :						\
		 ((($1)(si1)) + (($1)(si2)))				\
		;}) 

#define safe_sub_func_$1_s_s(_si1,_si2) \
		({ $1 si1 = (_si1); $1 si2 = (_si2) ; \
                ((((($1)(si1))^(($1)(si2))) \
		& ((((($1)(si1)) ^ (((($1)(si1))^(($1)(si2))) \
		& ((($1)1) << (sizeof($1)*CHAR_BIT-1))))-(($1)(si2)))^(($1)(si2)))) < (($1)0)) \
		? (($1)(si1)) \
		: ((($1)(si1)) - (($1)(si2))) \
		;})

#define safe_mul_func_$1_s_s(_si1,_si2) \
  ({ $1 si1 = (_si1); $1 si2 = (_si2) ; \
  ((((($1)(si1)) > (($1)0)) && ((($1)(si2)) > (($1)0)) && ((($1)(si1)) > (($3) / (($1)(si2))))) || \
  (((($1)(si1)) > (($1)0)) && ((($1)(si2)) <= (($1)0)) && ((($1)(si2)) < (($2) / (($1)(si1))))) || \
  (((($1)(si1)) <= (($1)0)) && ((($1)(si2)) > (($1)0)) && ((($1)(si1)) < (($2) / (($1)(si2))))) || \
  (((($1)(si1)) <= (($1)0)) && ((($1)(si2)) <= (($1)0)) && ((($1)(si1)) != (($1)0)) && ((($1)(si2)) < (($3) / (($1)(si1)))))) \
  ? (($1)(si1)) \
  : (($1)(si1)) * (($1)(si2));})

#define safe_mod_func_$1_s_s(_si1,_si2) \
  ({ $1 si1 = (_si1); $1 si2 = (_si2) ; \
  (((($1)(si2)) == (($1)0)) || (((($1)(si1)) == ($2)) && ((($1)(si2)) == (($1)-1)))) \
  ? (($1)(si1)) \
  : ((($1)(si1)) % (($1)(si2)));})

#define safe_div_func_$1_s_s(_si1,_si2) \
  ({ $1 si1 = (_si1); $1 si2 = (_si2) ; \
  (((($1)(si2)) == (($1)0)) || (((($1)(si1)) == ($2)) && ((($1)(si2)) == (($1)-1)))) \
  ? (($1)(si1)) \
  : ((($1)(si1)) / (($1)(si2)));})

#define safe_lshift_func_$1_s_s(_left,_right) \
  ({ $1 left = (_left); int right = (_right) ; \
   (((($1)(left)) < (($1)0)) \
  || (((int)(right)) < (($1)0)) \
  || (((int)(right)) >= sizeof($1)*CHAR_BIT) \
  || ((($1)(left)) > (($3) >> ((int)(right))))) \
  ? (($1)(left)) \
  : ((($1)(left)) << ((int)(right)));})

#define safe_lshift_func_$1_s_u(_left,_right) \
  ({ $1 left = (_left); unsigned int right = (_right) ; \
   (((($1)(left)) < (($1)0)) \
  || (((unsigned int)(right)) >= sizeof($1)*CHAR_BIT) \
  || ((($1)(left)) > (($3) >> ((unsigned int)(right))))) \
  ? (($1)(left)) \
  : ((($1)(left)) << ((unsigned int)(right)));})

#define safe_rshift_func_$1_s_s(_left,_right) \
	({ $1 left = (_left); int right = (_right) ; \
        (((($1)(left)) < (($1)0)) \
			 || (((int)(right)) < (($1)0)) \
			 || (((int)(right)) >= sizeof($1)*CHAR_BIT)) \
			? (($1)(left)) \
			: ((($1)(left)) >> ((int)(right)));})

#define safe_rshift_func_$1_s_u(_left,_right) \
  ({ $1 left = (_left); unsigned int right = (_right) ; \
   (((($1)(left)) < (($1)0)) \
			 || (((unsigned int)(right)) >= sizeof($1)*CHAR_BIT)) \
			? (($1)(left)) \
			: ((($1)(left)) >> ((unsigned int)(right)));})
')

safe_signed_math(int8_t,INT8_MIN,INT8_MAX)
safe_signed_math(int16_t,INT16_MIN,INT16_MAX)
safe_signed_math(int32_t,INT32_MIN,INT32_MAX)
safe_signed_math(int64_t,INT64_MIN,INT64_MAX)

define(`promote',`ifelse($1,uint64_t,unsigned long,unsigned int)')

define(`safe_unsigned_math',`

#define safe_unary_minus_func_$1_u(_ui) \
  ({ $1 ui = (_ui); -(($1)(ui));})

#define safe_add_func_$1_u_u(_ui1,_ui2) \
  ({ $1 ui1 = (_ui1); $1 ui2 = (_ui2) ; \
  (($1)(ui1)) + (($1)(ui2));})

#define safe_sub_func_$1_u_u(_ui1,_ui2) \
  ({ $1 ui1 = (_ui1); $1 ui2 = (_ui2) ; (($1)(ui1)) - (($1)(ui2));})

#define safe_mul_func_$1_u_u(_ui1,_ui2) \
  ({ $1 ui1 = (_ui1); $1 ui2 = (_ui2) ; ($1)(((promote($1))(ui1)) * ((promote($1))(ui2)));})

#define safe_mod_func_$1_u_u(_ui1,_ui2) \
	({ $1 ui1 = (_ui1); $1 ui2 = (_ui2) ; \
         ((($1)(ui2)) == (($1)0)) \
			? (($1)(ui1)) \
			: ((($1)(ui1)) % (($1)(ui2)));})

#define safe_div_func_$1_u_u(_ui1,_ui2) \
	        ({ $1 ui1 = (_ui1); $1 ui2 = (_ui2) ; \
                 ((($1)(ui2)) == (($1)0)) \
			? (($1)(ui1)) \
			: ((($1)(ui1)) / (($1)(ui2)));})

#define safe_lshift_func_$1_u_s(_left,_right) \
	({ $1 left = (_left); int right = (_right) ; \
          ((((int)(right)) < (($1)0)) \
			 || (((int)(right)) >= sizeof($1)*CHAR_BIT) \
			 || ((($1)(left)) > (($2) >> ((int)(right))))) \
			? (($1)(left)) \
			: ((($1)(left)) << ((int)(right)));})

#define safe_lshift_func_$1_u_u(_left,_right) \
	 ({ $1 left = (_left); unsigned int right = (_right) ; \
           ((((unsigned int)(right)) >= sizeof($1)*CHAR_BIT) \
			 || ((($1)(left)) > (($2) >> ((unsigned int)(right))))) \
			? (($1)(left)) \
			: ((($1)(left)) << ((unsigned int)(right)));})

#define safe_rshift_func_$1_u_s(_left,_right) \
	({ $1 left = (_left); int right = (_right) ; \
          ((((int)(right)) < (($1)0)) \
			 || (((int)(right)) >= sizeof($1)*CHAR_BIT)) \
			? (($1)(left)) \
			: ((($1)(left)) >> ((int)(right)));})

#define safe_rshift_func_$1_u_u(_left,_right) \
	({ $1 left = (_left); unsigned int right = (_right) ; \
                 (((unsigned int)(right)) >= sizeof($1)*CHAR_BIT) \
			 ? (($1)(left)) \
			 : ((($1)(left)) >> ((unsigned int)(right)));})

')

safe_unsigned_math(uint8_t,UINT8_MAX)
safe_unsigned_math(uint16_t,UINT16_MAX)
safe_unsigned_math(uint32_t,UINT32_MAX)
safe_unsigned_math(uint64_t,UINT64_MAX)

define(`logical_2',`

#define LOGICAL_AND_$1(_left,_right) \
        ({ $2 left = (_left); $2 right = (_right) ; \
          (($2)((left).s0 && (right).s0, (left).s1 && (right).s1));})
        
#define LOGICAL_OR_$1(_left,_right) \
        ({ $2 left = (_left); $2 right = (_right) ; \
          (($2)((left).s0 || (right).s0, (left).s1 || (right).s1));})
        
#define LOGICAL_NOT_$1(_left) \
        ({ $2 left = (_left) ; \
          (($2)(!(left).s0, !(left).s1));})

')

logical_2(int8_t2,char2)
logical_2(int16_t2,short2)
logical_2(int32_t2,int2)
logical_2(int64_t2,long2)
logical_2(uint8_t2,uchar2)
logical_2(uint16_t2,ushort2)
logical_2(uint32_t2,uint2)
logical_2(uint64_t2,ulong2)

define(`logical_4',`

#define LOGICAL_AND_$1(_left,_right) \
        ({ $2 left = (_left); $2 right = (_right) ; \
            (($2)((left).s0 && (right).s0, (left).s1 && (right).s1, \
            (left).s2 && (right).s2, (left).s3 && (right).s3));})
        
#define LOGICAL_OR_$1(_left,_right) \
        ({ $2 left = (_left); $2 right = (_right) ; \
            (($2)((left).s0 || (right).s0, (left).s1 || (right).s1, \
            (left).s2 || (right).s2, (left).s3 || (right).s3));})
        
#define LOGICAL_NOT_$1(_left) \
        ({ $2 left = (_left) ; \
              (($2)(!(left).s0, !(left).s1, !(left).s2, !(left).s3));})

')

logical_4(int8_t4,char4)
logical_4(int16_t4,short4)
logical_4(int32_t4,int4)
logical_4(int64_t4,long4)
logical_4(uint8_t4,uchar4)
logical_4(uint16_t4,ushort4)
logical_4(uint32_t4,uint4)
logical_4(uint64_t4,ulong4)

define(`logical_8',`

#define LOGICAL_AND_$1(_left,_right) \
        ({ $2 left = (_left); $2 right = (_right) ; \
            (($2)((left).s0 && (right).s0, (left).s1 && (right).s1, \
            (left).s2 && (right).s2, (left).s3 && (right).s3, \
            (left).s4 && (right).s4, (left).s5 && (right).s5, \
            (left).s6 && (right).s6, (left).s7 && (right).s7));})
        
#define LOGICAL_OR_$1(_left,_right) \
        ({ $2 left = (_left); $2 right = (_right) ; \
            (($2)((left).s0 || (right).s0, (left).s1 || (right).s1, \
            (left).s2 || (right).s2, (left).s3 || (right).s3, \
            (left).s4 || (right).s4, (left).s5 || (right).s5, \
            (left).s6 || (right).s6, (left).s7 || (right).s7));})
        
#define LOGICAL_NOT_$1(_left) \
        ({ $2 left = (_left) ; \
            (($2)(!(left).s0, !(left).s1, !(left).s2, !(left).s3, \
            !(left).s4, !(left).s5, !(left).s6, !(left).s7));})

')

logical_8(int8_t8,char8)
logical_8(int16_t8,short8)
logical_8(int32_t8,int8)
logical_8(int64_t8,long8)
logical_8(uint8_t8,uchar8)
logical_8(uint16_t8,ushort8)
logical_8(uint32_t8,uint8)
logical_8(uint64_t8,ulong8)

define(`logical_16',`

#define LOGICAL_AND_$1(_left,_right) \
        ({ $2 left = (_left); $2 right = (_right) ; \
            (($2)((left).s0 && (right).s0, (left).s1 && (right).s1, \
            (left).s2 && (right).s2, (left).s3 && (right).s3, \
            (left).s4 && (right).s4, (left).s5 && (right).s5, \
            (left).s6 && (right).s6, (left).s7 && (right).s7, \
            (left).s8 && (right).s8, (left).s9 && (right).s9, \
            (left).sa && (right).sa, (left).sb && (right).sb, \
            (left).sc && (right).sc, (left).sd && (right).sd, \
            (left).se && (right).se, (left).sf && (right).sf));})
        
#define LOGICAL_OR_$1(_left,_right) \
        ({ $2 left = (_left); $2 right = (_right) ; \
            (($2)((left).s0 || (right).s0, (left).s1 || (right).s1, \
            (left).s2 || (right).s2, (left).s3 || (right).s3, \
            (left).s4 || (right).s4, (left).s5 || (right).s5, \
            (left).s6 || (right).s6, (left).s7 || (right).s7, \
            (left).s8 || (right).s8, (left).s9 || (right).s9, \
            (left).sa || (right).sa, (left).sb || (right).sb, \
            (left).sc || (right).sc, (left).sd || (right).sd, \
            (left).se || (right).se, (left).sf || (right).sf));})
        
#define LOGICAL_NOT_$1(_left) \
        ({ $2 left = (_left) ; \
            (($2)(!(left).s0, !(left).s1, !(left).s2, !(left).s3, \
            !(left).s4, !(left).s5, !(left).s6, !(left).s7, \
            !(left).s8, !(left).s9, !(left).sa, !(left).sb, \
            !(left).sc, !(left).sd, !(left).se, !(left).sf));})

')

logical_16(int8_t16,char16)
logical_16(int16_t16,short16)
logical_16(int32_t16,int16)
logical_16(int64_t16,long16)
logical_16(uint8_t16,uchar16)
logical_16(uint16_t16,ushort16)
logical_16(uint32_t16,uint16)
logical_16(uint64_t16,ulong16)

#endif
