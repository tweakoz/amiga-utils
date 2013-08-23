#ifndef AMIGATYPES_H_INCLUDED
#define AMIGATYPES_H_INCLUDED

#include <sys/types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
* 64-bit unsigned int
*/
typedef u_int64_t u64;
/*!
* 32-bit unsigned int
*/
typedef u_int32_t u32;
/*!
* 16-bit unsigned int
*/
typedef u_int16_t u16;
/*!
* 8-bit unsigned int
*/
typedef u_int8_t u8;
/*!
* 64-bit signed int
*/
typedef int64_t s64;
/*!
* 32-bit signed int
*/
typedef int32_t s32;
/*!
* 16-bit signed int
*/
typedef int16_t s16;
/*!
* 64-bit signed int
*/
typedef int8_t s8;

/*!
*	A 512 byte buffer for sector operations
*/
typedef u8 Block[512];

#ifdef __cplusplus
}
#endif

#endif // AMIGATYPES_H_INCLUDED
