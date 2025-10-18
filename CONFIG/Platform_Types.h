/*
 * @file Platform_Types.h
 * @brief Platform-specific type definitions for ARM Cortex M4F
 * @details This file contains platform-specific type definitions optimized for
 *          memory efficiency and easy debugging. Compliant with AUTOSAR standards.
 *
 * @author Mohamed Yasser
 * @date Oct 12, 2025
 * @version 1.0.0
 */

#ifndef CONFIG_PLATFORM_TYPES_H_
#define CONFIG_PLATFORM_TYPES_H_

/* Include standard types */
#include "std_types.h"

/* ===================[Version Information]=================== */
#define PLATFORM_VENDOR_ID                    (0x1234u)  /**< @brief Platform vendor ID */
#define PLATFORM_SW_MAJOR_VERSION             (1u)       /**< @brief Platform software major version */
#define PLATFORM_SW_MINOR_VERSION             (0u)       /**< @brief Platform software minor version */
#define PLATFORM_SW_PATCH_VERSION              (0u)      /**< @brief Platform software patch version */
#define PLATFORM_AR_RELEASE_MAJOR_VERSION      (4u)      /**< @brief AUTOSAR release major version */
#define PLATFORM_AR_RELEASE_MINOR_VERSION      (4u)      /**< @brief AUTOSAR release minor version */
#define PLATFORM_AR_RELEASE_REVISION_VERSION   (0u)      /**< @brief AUTOSAR release revision version */

/* ===================[CPU Type Definitions]=================== */
#define CPU_TYPE_8       8
#define CPU_TYPE_16      16
#define CPU_TYPE_32      32
#define CPU_TYPE_64      64

#define CPU_TYPE         CPU_TYPE_32

/* ===================[AUTOSAR Endianness]=================== */
#define CPU_BIT_ORDER_LSB_FIRST               (0u)       /**< @brief LSB first bit order */
#define CPU_BIT_ORDER_MSB_FIRST               (1u)       /**< @brief MSB first bit order */
#define CPU_BIT_ORDER                        CPU_BIT_ORDER_LSB_FIRST

#define CPU_BYTE_ORDER_LITTLE_ENDIAN          (0u)       /**< @brief Little endian byte order */
#define CPU_BYTE_ORDER_BIG_ENDIAN             (1u)      /**< @brief Big endian byte order */
#define CPU_BYTE_ORDER                        CPU_BYTE_ORDER_LITTLE_ENDIAN

/* ===================[Platform Type Definitions]=================== */
typedef boolean          Platform_BooleanType;           /**< @brief Platform boolean type */
typedef char            Platform_CharType;               /**< @brief Platform character type */

/* ===================[Integer Platform Types]=================== */
typedef uint8           Platform_UInt8Type;              /**< @brief Platform unsigned 8-bit integer */
typedef sint8           Platform_SInt8Type;              /**< @brief Platform signed 8-bit integer */
typedef uint16          Platform_UInt16Type;             /**< @brief Platform unsigned 16-bit integer */
typedef sint16          Platform_SInt16Type;             /**< @brief Platform signed 16-bit integer */
typedef uint32          Platform_UInt32Type;             /**< @brief Platform unsigned 32-bit integer */
typedef sint32          Platform_SInt32Type;             /**< @brief Platform signed 32-bit integer */

/* ===================[Floating Point Platform Types]=================== */
typedef float32         Platform_Float32Type;            /**< @brief Platform 32-bit floating point */
typedef float64         Platform_Float64Type;            /**< @brief Platform 64-bit floating point */

/* ===================[Pointer Platform Types]=================== */
typedef void*           Platform_PointerType;            /**< @brief Platform pointer type */
typedef void (*Platform_FunctionType)(void);             /**< @brief Platform function pointer type */

/* ===================[AUTOSAR Standard Types]=================== */
typedef unsigned char   Std_BitType;                     /**< @brief Standard bit type for bit operations */

#endif /* CONFIG_PLATFORM_TYPES_H_ */
