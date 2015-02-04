#ifndef RANDOM_RUNTIME_H
#define RANDOM_RUNTIME_H

#include "cl_safe_math_macros.h"
#include "safe_math_macros.h"

inline __attribute__((always_inline)) void 
transparent_crc_no_string (uint64_t *crc64_context, uint64_t val)
{
  *crc64_context += val;
}

#define transparent_crc_(A, B, C, D) transparent_crc_no_string(A, B)

inline __attribute__((always_inline)) uint32_t
get_linear_group_id (void)
{
  return (get_group_id(2) * get_num_groups(1) + get_group_id(1)) * 
    get_num_groups(0) + get_group_id(0);
}

inline __attribute__((always_inline)) uint32_t
get_linear_global_id (void)
{
  return (get_global_id(2) * get_global_size(1) + get_global_id(1)) * 
    get_global_size(0) + get_global_id(0);
}

inline __attribute__((always_inline)) uint32_t
get_linear_local_id (void)
{
  return (get_local_id(2) * get_local_size(1) + get_local_id(1)) * 
    get_local_size(0) + get_local_id(0);
}

#endif /* RANDOM_RUNTIME_H */
