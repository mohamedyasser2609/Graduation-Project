/******************************************************************************
*
* @file std_types.h
* @brief Essential types for ARM Cortex M4F - Optimized for memory efficiency
* @details This file contains essential data types and definitions for embedded
*          systems development on ARM Cortex M4F platform. Optimized for
*          memory efficiency and easy debugging.
*
* @author Mohamed Yasser
* @date Oct 12, 2025
* @version 1.0.0
*
*******************************************************************************/

#ifndef STD_TYPES_H_
#define STD_TYPES_H_

/* Include standard integer types for FreeRTOS compatibility */
#include <stdint.h>
#include <stddef.h>

/* ===================[Version Information]=================== */
#define STD_TYPES_SW_MAJOR_VERSION    (1u)
#define STD_TYPES_SW_MINOR_VERSION    (0u)
#define STD_TYPES_SW_PATCH_VERSION     (0u)

/* ===================[Boolean Values]=================== */
#ifndef FALSE
#define FALSE       (0u)
#endif
#ifndef TRUE
#define TRUE        (1u)
#endif

/* ===================[Logic Levels]=================== */
#define LOGIC_HIGH        (1u)
#define LOGIC_LOW         (0u)
#define STD_HIGH          (1u)
#define STD_LOW           (0u)

/* ===================[Null Pointer]=================== */
#define NULL_PTR    ((void*)0)

/* ===================[Essential Integer Types]=================== */
typedef uint8_t             uint8;          /**< @brief Unsigned 8-bit integer (0 .. 255) */
typedef int8_t              sint8;          /**< @brief Signed 8-bit integer (-128 .. +127) */
typedef uint16_t             uint16;         /**< @brief Unsigned 16-bit integer (0 .. 65535) */
typedef int16_t              sint16;        /**< @brief Signed 16-bit integer (-32768 .. +32767) */
typedef uint32_t             uint32;         /**< @brief Unsigned 32-bit integer (0 .. 4294967295) */
typedef int32_t              sint32;        /**< @brief Signed 32-bit integer (-2147483648 .. +2147483647) */

/* ===================[Floating Point Types]=================== */
typedef float                float32;        /**< @brief 32-bit floating point */
typedef double               float64;        /**< @brief 64-bit floating point */

/* ===================[Boolean Data Type]=================== */
typedef uint8                boolean;        /**< @brief Boolean data type */

/* ===================[Function Return Types]=================== */
typedef uint8                Std_ReturnType; /**< @brief Standard return type for functions */
#define E_OK                 (0x00u)         /**< @brief Function call successful */
#define E_NOT_OK             (0x01u)         /**< @brief Function call failed */
#define E_PENDING            (0x02u)         /**< @brief Function call pending */
#define E_BUSY               (0x03u)         /**< @brief Resource is busy */
#define E_TIMEOUT            (0x04u)         /**< @brief Operation timeout */
#define E_INVALID_PARAM      (0x05u)         /**< @brief Invalid parameter */
#define E_NOT_INITIALIZED    (0x06u)         /**< @brief Module not initialized */

/* ===================[Status Types]=================== */
typedef uint8                StatusType;     /**< @brief Status type for operations */
#define IDLE                 (0x00u)         /**< @brief Module is idle */
#define BUSY                 (0x01u)         /**< @brief Module is busy */
#define INITIALIZED          (0x02u)         /**< @brief Module is initialized */
#define UNINITIALIZED        (0x03u)         /**< @brief Module is not initialized */

#endif /* STD_TYPES_H_ */
