#ifndef RANDOM_RUNTIME_H
#define RANDOM_RUNTIME_H

#include "safe_math_macros.h"

inline void 
transparent_crc_ (uint64_t *crc64_context, uint64_t val, const __constant char* vname, int flag)
{
  *crc64_context += val;
}

#endif /* RANDOM_RUNTIME_H */
