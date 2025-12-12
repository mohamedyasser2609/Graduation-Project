/*
 * @file Compiler.h
 * @brief Compiler abstraction layer for ARM Cortex M4F
 * @details This file provides compiler abstraction macros and definitions
 *          optimized for Code Composer Studio and ARM Cortex M4F platform.
 *          Ensures portability across different compilers.
 *
 * @author Mohamed Yasser
 * @date Oct 12, 2025
 * @version 1.0.0
 */

#ifndef CONFIG_COMPILER_H_
#define CONFIG_COMPILER_H_

/* Include standard types */
#include <Std_types.h>

/* Include offsetof for TI ARM compiler */
#ifdef COMPILER_TI_CCS
    #include <stddef.h>
#endif

/* ===================[Compiler Detection]=================== */
#ifdef __TI_COMPILER_VERSION__
    #define COMPILER_TI_CCS
#elif defined(__GNUC__)
    #define COMPILER_GCC
#elif defined(__ARMCC_VERSION)
    #define COMPILER_ARMCC
#elif defined(__IAR_SYSTEMS_ICC__)
    #define COMPILER_IAR
#else
    #define COMPILER_UNKNOWN
#endif

/* ===================[Memory Alignment]=================== */
/* ARM Cortex M4F requires 4-byte alignment for optimal performance */
#define COMPILER_ALIGN_1BYTE     __attribute__((aligned(1)))
#define COMPILER_ALIGN_2BYTE     __attribute__((aligned(2)))
#define COMPILER_ALIGN_4BYTE     __attribute__((aligned(4)))
#define COMPILER_ALIGN_8BYTE     __attribute__((aligned(8)))

/* Default alignment for ARM Cortex M4F */
#define COMPILER_DEFAULT_ALIGN   COMPILER_ALIGN_4BYTE

/* ===================[Function Attributes]=================== */
/* Function optimization attributes */
#define COMPILER_FUNC_INLINE     __attribute__((always_inline))
#define COMPILER_FUNC_NOINLINE   __attribute__((noinline))
#define COMPILER_FUNC_PURE       __attribute__((pure))
#define COMPILER_FUNC_CONST      __attribute__((const))

/* Function visibility */
#define COMPILER_FUNC_WEAK       __attribute__((weak))
#define COMPILER_FUNC_STRONG     __attribute__((strong))

/* ===================[Variable Attributes]=================== */
/* Variable storage attributes */
#define COMPILER_VAR_VOLATILE    volatile
#define COMPILER_VAR_CONST      const
#define COMPILER_VAR_STATIC      static
#define COMPILER_VAR_EXTERN      extern

/* Variable optimization */
#define COMPILER_VAR_UNUSED      __attribute__((unused))
#define COMPILER_VAR_USED        __attribute__((used))

/* ===================[Section Placement]=================== */
/* Code sections */
#define COMPILER_SECTION_CODE    __attribute__((section(".text")))
#define COMPILER_SECTION_ISR     __attribute__((section(".text:isr")))
#define COMPILER_SECTION_INIT    __attribute__((section(".text:init")))

/* Data sections */
#define COMPILER_SECTION_DATA    __attribute__((section(".data")))
#define COMPILER_SECTION_BSS     __attribute__((section(".bss")))
#define COMPILER_SECTION_RODATA  __attribute__((section(".rodata")))

/* ===================[Interrupt Handling]=================== */
/* Interrupt service routine attributes - TI ARM compiler compatible */
#ifdef COMPILER_TI_CCS
    #define COMPILER_ISR         __attribute__((interrupt))
    #define COMPILER_ISR_IRQ     __attribute__((interrupt("IRQ")))
#else
    #define COMPILER_ISR         __attribute__((interrupt))
    #define COMPILER_ISR_IRQ     __attribute__((interrupt("IRQ")))
#endif

/* ===================[Optimization Control]=================== */
/* Optimization levels - TI ARM compiler compatible */
#ifdef COMPILER_TI_CCS
    /* TI ARM compiler uses pragmas for optimization control */
    #define COMPILER_OPTIMIZE_O0     COMPILER_PRAGMA(OPT_LEVEL=0)
    #define COMPILER_OPTIMIZE_O1     COMPILER_PRAGMA(OPT_LEVEL=1)
    #define COMPILER_OPTIMIZE_O2     COMPILER_PRAGMA(OPT_LEVEL=2)
    #define COMPILER_OPTIMIZE_O3     COMPILER_PRAGMA(OPT_LEVEL=3)
    #define COMPILER_OPTIMIZE_OS     COMPILER_PRAGMA(OPT_LEVEL=2)
#else
    /* GCC-style optimization attributes */
    #define COMPILER_OPTIMIZE_O0     __attribute__((optimize("O0")))
    #define COMPILER_OPTIMIZE_O1     __attribute__((optimize("O1")))
    #define COMPILER_OPTIMIZE_O2     __attribute__((optimize("O2")))
    #define COMPILER_OPTIMIZE_O3     __attribute__((optimize("O3")))
    #define COMPILER_OPTIMIZE_OS     __attribute__((optimize("Os")))
#endif

/* ===================[Debugging Support]=================== */
/* Debug information - TI ARM compiler compatible */
#ifdef COMPILER_TI_CCS
    /* TI ARM compiler debug support */
    #define COMPILER_DEBUG_INFO      /* Debug info enabled by default in TI CCS */
    #define COMPILER_NO_DEBUG_INFO   COMPILER_PRAGMA(DIAG_SUPPRESS=225)
#else
    /* GCC-style debug attributes */
    #define COMPILER_DEBUG_INFO      __attribute__((debug_info))
    #define COMPILER_NO_DEBUG_INFO   __attribute__((no_debug_info))
#endif

/* ===================[Compiler-Specific Pragmas]=================== */
#ifdef COMPILER_TI_CCS
    /* TI Code Composer Studio specific pragmas */
    #define COMPILER_PRAGMA(x)   _Pragma(#x)
    #define COMPILER_DATA_SECTION(section) COMPILER_PRAGMA(DATA_SECTION(section))
    #define COMPILER_DIAG_SUPPRESS(x) COMPILER_PRAGMA(diag_suppress x)
    #define COMPILER_DIAG_ERROR(x)   COMPILER_PRAGMA(diag_error x)
    #define COMPILER_DIAG_WARNING(x) COMPILER_PRAGMA(diag_warning x)
#else
    /* Generic pragma support */
    #define COMPILER_PRAGMA(x)   _Pragma(#x)
    #define COMPILER_DATA_SECTION(section) COMPILER_PRAGMA(DATA_SECTION(section))
    #define COMPILER_DIAG_SUPPRESS(x)
    #define COMPILER_DIAG_ERROR(x)
    #define COMPILER_DIAG_WARNING(x)
#endif

/* ===================[Memory Barriers]=================== */
/* Memory barrier macros for ARM Cortex M4F */
#define COMPILER_MEMORY_BARRIER()    __asm__ __volatile__("" ::: "memory")
#define COMPILER_DATA_SYNC_BARRIER() __asm__ __volatile__("dsb" ::: "memory")
#define COMPILER_INSTRUCTION_SYNC_BARRIER() __asm__ __volatile__("isb" ::: "memory")

/* ===================[Atomic Operations]=================== */
/* Atomic operation support - TI ARM compiler compatible */
#ifdef COMPILER_TI_CCS
    /* TI ARM compiler atomic operations */
    #define COMPILER_ATOMIC_READ(x)      (x)
    #define COMPILER_ATOMIC_WRITE(x, v)  ((x) = (v))
#else
    /* GCC atomic operations */
    #define COMPILER_ATOMIC_READ(x)      __atomic_load_n(&(x), __ATOMIC_RELAXED)
    #define COMPILER_ATOMIC_WRITE(x, v)  __atomic_store_n(&(x), v, __ATOMIC_RELAXED)
#endif

/* ===================[Endianness Control]=================== */
/* Endianness conversion macros */
#define COMPILER_BE16(x)             __builtin_bswap16(x)
#define COMPILER_BE32(x)             __builtin_bswap32(x)
#define COMPILER_LE16(x)             (x)
#define COMPILER_LE32(x)             (x)

/* ===================[Built-in Functions]=================== */
/* Count leading zeros - useful for bit manipulation */
#ifdef COMPILER_TI_CCS
    /* TI ARM compiler built-ins */
    #define COMPILER_CLZ(x)              __clz(x)
    #define COMPILER_CLZLL(x)            __clzll(x)
    #define COMPILER_POPCOUNT(x)         __popcount(x)
    #define COMPILER_POPCOUNTLL(x)       __popcountll(x)
#else
    /* GCC built-ins */
    #define COMPILER_CLZ(x)              __builtin_clz(x)
    #define COMPILER_CLZLL(x)            __builtin_clzll(x)
    #define COMPILER_POPCOUNT(x)         __builtin_popcount(x)
    #define COMPILER_POPCOUNTLL(x)       __builtin_popcountll(x)
#endif

/* ===================[Error Handling]=================== */
/* Compiler error and warning suppression */
#define COMPILER_SUPPRESS_UNUSED_WARNING(x) ((void)(x))

/* ===================[Function Macros]=================== */
/* Function declaration macros */
#define COMPILER_FUNC_DECLARE        extern
#define COMPILER_FUNC_DEFINE         static inline

/* ===================[Size and Offset Macros]=================== */
/* Structure member offset - TI ARM compiler compatible */
#ifdef COMPILER_TI_CCS
    #define COMPILER_OFFSETOF(type, member) offsetof(type, member)
#else
    #define COMPILER_OFFSETOF(type, member) __builtin_offsetof(type, member)
#endif

/* Array size calculation */
#define COMPILER_ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))

#endif /* CONFIG_COMPILER_H_ */
