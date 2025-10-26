/*
 * @file Compiler_Cfg.h
 * @brief Compiler configuration for ARM Cortex M4F
 * @details This file contains compiler-specific configuration settings
 *          optimized for Code Composer Studio and ARM Cortex M4F platform.
 *          Includes optimization settings, debugging options, and memory layout.
 *
 * @author Mohamed Yasser
 * @date Oct 12, 2025
 * @version 1.0.0
 */

#ifndef CONFIG_COMPILER_CFG_H_
#define CONFIG_COMPILER_CFG_H_

/* Include compiler abstraction */
#include "Compiler.h"

/* ===================[Compiler Version Information]=================== */
#define COMPILER_CFG_SW_MAJOR_VERSION         (1u)
#define COMPILER_CFG_SW_MINOR_VERSION          (0u)
#define COMPILER_CFG_SW_PATCH_VERSION          (0u)

/* ===================[Target Platform Configuration]=================== */
/* Target CPU: ARM Cortex M4F */
#define COMPILER_CFG_TARGET_CPU                "ARM Cortex M4F"
#define COMPILER_CFG_TARGET_ARCH               "ARMv7E-M"

/* Memory configuration */
#define COMPILER_CFG_FLASH_START_ADDR          (0x00000000u)
#define COMPILER_CFG_FLASH_SIZE                (0x00040000u)  /* 256KB */
#define COMPILER_CFG_RAM_START_ADDR             (0x20000000u)
#define COMPILER_CFG_RAM_SIZE                   (0x00008000u)  /* 32KB */

/* ===================[Optimization Configuration]=================== */
/* Default optimization level */
#define COMPILER_CFG_OPTIMIZATION_LEVEL       2  /* O2 optimization */

/* Size vs Speed optimization */
#define COMPILER_CFG_OPTIMIZE_FOR_SIZE         0  /* 0=Speed, 1=Size */

/* Function inlining */
#define COMPILER_CFG_INLINE_THRESHOLD          50  /* Inline functions smaller than 50 bytes */

/* ===================[Debug Configuration]=================== */
/* Debug information generation */
#define COMPILER_CFG_DEBUG_INFO_ENABLED        1  /* Enable debug info */

/* Debug optimization */
#define COMPILER_CFG_DEBUG_OPTIMIZATION        0  /* Disable optimization for debugging */

/* Stack usage analysis */
#define COMPILER_CFG_STACK_USAGE_ANALYSIS      1  /* Enable stack usage analysis */

/* ===================[Warning Configuration]=================== */
/* Warning levels */
#define COMPILER_CFG_WARNING_LEVEL             3  /* All warnings */

/* Specific warning controls */
#define COMPILER_CFG_WARN_UNUSED_VAR           1  /* Warn about unused variables */
#define COMPILER_CFG_WARN_UNUSED_PARAM         1  /* Warn about unused parameters */
#define COMPILER_CFG_WARN_MISSING_RETURN       1  /* Warn about missing return statements */

/* ===================[Memory Configuration]=================== */
/* Stack size configuration */
#define COMPILER_CFG_DEFAULT_STACK_SIZE        (1024u)   /* 1KB default stack */
#define COMPILER_CFG_MAIN_STACK_SIZE           (2048u)   /* 2KB main stack */
#define COMPILER_CFG_ISR_STACK_SIZE            (512u)    /* 512B ISR stack */

/* Heap size configuration */
#define COMPILER_CFG_HEAP_SIZE                 (4096u)   /* 4KB heap */

/* ===================[Code Generation Configuration]=================== */
/* Instruction set */
#define COMPILER_CFG_THUMB_MODE                1  /* Use Thumb-2 instruction set */

/* Floating point unit */
#define COMPILER_CFG_FPU_ENABLED               1  /* Enable FPU (Cortex M4F) */
#define COMPILER_CFG_FPU_TYPE                   "FPv4-SP"  /* Single precision FPU */

/* ===================[Interrupt Configuration]=================== */
/* Interrupt vector table */
#define COMPILER_CFG_VECTOR_TABLE_SIZE         (240u)  /* Number of interrupt vectors */

/* Interrupt priority levels */
#define COMPILER_CFG_MAX_INTERRUPT_PRIORITY     (7u)   /* 3-bit priority field */

/* ===================[Performance Configuration]=================== */
/* Cache configuration */
#define COMPILER_CFG_ICACHE_ENABLED            0  /* No instruction cache on M4F */
#define COMPILER_CFG_DCACHE_ENABLED            0  /* No data cache on M4F */

/* Branch prediction */
#define COMPILER_CFG_BRANCH_PREDICTION         1  /* Enable branch prediction */

/* ===================[Compiler-Specific Settings]=================== */
#ifdef COMPILER_TI_CCS
    /* TI Code Composer Studio specific settings */
    #define COMPILER_CFG_TI_VERSION             "20.2.7.LTS"
    #define COMPILER_CFG_TI_TARGET              "ti.targets.arm.elf.M4F"
    #define COMPILER_CFG_TI_PART                "PART_TM4C123GH6PM"
    
    /* TI-specific optimizations */
    #define COMPILER_CFG_TI_OPTIMIZE_FOR_SPEED 1
    #define COMPILER_CFG_TI_USE_VFP            1  /* Use VFP for floating point */
    #define COMPILER_CFG_TI_FLOAT_SUPPORT     "FPv4SPD16"  /* Single precision FPU */
    #define COMPILER_CFG_TI_CODE_STATE        16  /* Thumb-2 instruction set */
    #define COMPILER_CFG_TI_ABI               "eabi"  /* EABI calling convention */
    
    /* TI-specific pragmas */
    #define COMPILER_CFG_TI_PRAGMA(x)          _Pragma(#x)
    
    /* TI-specific diagnostic settings */
    #define COMPILER_CFG_TI_DIAG_WARNING_225   1  /* Suppress warning 225 */
    #define COMPILER_CFG_TI_DISPLAY_ERROR_NUM  1  /* Display error numbers */
    
#elif defined(COMPILER_GCC)
    /* GCC specific settings */
    #define COMPILER_CFG_GCC_VERSION           "10.3.1"
    #define COMPILER_CFG_GCC_TARGET            "arm-none-eabi"
    
    /* GCC-specific optimizations */
    #define COMPILER_CFG_GCC_OPTIMIZE_FOR_SPEED 1
    #define COMPILER_CFG_GCC_USE_FPU           1  /* Use FPU for floating point */
    
#endif

/* ===================[Runtime Configuration]=================== */
/* Runtime library */
#define COMPILER_CFG_RUNTIME_LIBRARY          "libc.a"

/* Math library */
#define COMPILER_CFG_MATH_LIBRARY             "libm.a"

/* ===================[Linker Configuration]=================== */
/* Linker script */
#define COMPILER_CFG_LINKER_SCRIPT            "tm4c123gh6pm.cmd"

/* Memory sections */
#define COMPILER_CFG_TEXT_SECTION            ".text"
#define COMPILER_CFG_DATA_SECTION             ".data"
#define COMPILER_CFG_BSS_SECTION              ".bss"
#define COMPILER_CFG_RODATA_SECTION           ".rodata"

/* ===================[Development Configuration]=================== */
/* Development mode settings */
#ifdef DEBUG
    #define COMPILER_CFG_DEBUG_MODE           1
    #define COMPILER_CFG_ASSERT_ENABLED       1
    #define COMPILER_CFG_TRACE_ENABLED        1
#else
    #define COMPILER_CFG_DEBUG_MODE           0
    #define COMPILER_CFG_ASSERT_ENABLED       0
    #define COMPILER_CFG_TRACE_ENABLED        0
#endif

/* ===================[Safety Configuration]=================== */
/* Safety-related settings */
#define COMPILER_CFG_SAFETY_ENABLED           1  /* Enable safety features */
#define COMPILER_CFG_STACK_CANARY             1  /* Enable stack canary */
#define COMPILER_CFG_BUFFER_OVERFLOW_CHECK    1  /* Enable buffer overflow check */

/* ===================[Performance Monitoring]=================== */
/* Performance counters */
#define COMPILER_CFG_PERF_COUNTERS_ENABLED    1  /* Enable performance counters */
#define COMPILER_CFG_CYCLE_COUNTER_ENABLED    1  /* Enable cycle counter */

#endif /* CONFIG_COMPILER_CFG_H_ */
