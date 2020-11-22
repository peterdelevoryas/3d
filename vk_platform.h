//
// file: vk_platform.h
//
/*
** copyright (c) 2014-2020 the khronos group inc.
**
** SPDX-license-identifier: apache-2.0
*/

#ifndef VK_PLATFORM_H_
#define VK_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
***************************************************************************************************
*   platform-specific directives and type declarations
***************************************************************************************************
*/

/* platform-specific calling convention macros.
 *
 * platforms should define these so that vulkan clients call vulkan commands
 * with the same calling conventions that the vulkan implementation expects.
 *
 * VKAPI_ATTR - placed before the return type in function declarations.
 *              useful for C++11 and GCC/clang-style function attribute syntax.
 * VKAPI_CALL - placed after the return type in function declarations.
 *              useful for MSVC-style calling convention syntax.
 * VKAPI_PTR  - placed between the '(' and '*' in function pointer types.
 *
 * function declaration:  VKAPI_ATTR void VKAPI_CALL vk_command(void) __asm("vkCommand");
 * function pointer type: typedef void (VKAPI_PTR *pfn_vk_command)(void);
 */
#if defined(_WIN32)
// on windows, vulkan commands use the stdcall convention
#define VKAPI_ATTR
#define VKAPI_CALL __stdcall
#define VKAPI_PTR VKAPI_CALL
#elif defined(__ANDROID__) && defined(__ARM_ARCH) && __ARM_ARCH < 7
#error "Vulkan isn't supported for the 'armeabi' NDK ABI"
#elif defined(__ANDROID__) && defined(__ARM_ARCH) && __ARM_ARCH >= 7 && defined(__ARM_32BIT_STATE)
// on android 32-bit ARM targets, vulkan functions use the "hardfloat"
// calling convention, i.e. float parameters are passed in registers. this
// is true even if the rest of the application passes floats on the stack,
// as it does by default when compiling for the armeabi-v7a NDK ABI.
#define VKAPI_ATTR __attribute__((pcs("aapcs-vfp")))
#define VKAPI_CALL
#define VKAPI_PTR VKAPI_ATTR
#else
// on other platforms, use the default calling convention
#define VKAPI_ATTR
#define VKAPI_CALL
#define VKAPI_PTR
#endif

#include <stddef.h>

#if !defined(VK_NO_STDINT_H)
#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef signed __int8    int8_t;
typedef unsigned __int8  uint8_t;
typedef signed __int16   int16_t;
typedef unsigned __int16 uint16_t;
typedef signed __int32   int32_t;
typedef unsigned __int32 uint32_t;
typedef signed __int64   int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif
#endif // !defined(VK_NO_STDINT_H)

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif
