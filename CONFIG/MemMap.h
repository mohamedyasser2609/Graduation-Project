/*
 * @file MemMap.h
 * @brief Memory mapping configuration for ARM Cortex M4F
 * @details This file maps code and data sections to compiler/linker-specific
 *          pragmas for AUTOSAR-compliant memory management. Optimized for
 *          TM4C123GH6PM and TI Code Composer Studio.
 *
 * @author Mohamed Yasser
 * @date Oct 12, 2025
 * @version 1.0.0
 */

#ifndef CONFIG_MEMMAP_H_
#define CONFIG_MEMMAP_H_

/* Include compiler abstraction */
#include "Compiler.h"

/* ===================[Memory Section Mapping]=================== */

/* Code Sections */
#define MEMMAP_CODE_START_SEC        COMPILER_PRAGMA(DATA_SECTION(".text"))
#define MEMMAP_CODE_STOP_SEC         COMPILER_PRAGMA(DATA_SECTION())

#define MEMMAP_CODE_FAST_START_SEC   COMPILER_PRAGMA(DATA_SECTION(".text:fast"))
#define MEMMAP_CODE_FAST_STOP_SEC    COMPILER_PRAGMA(DATA_SECTION())

#define MEMMAP_CODE_SLOW_START_SEC   COMPILER_PRAGMA(DATA_SECTION(".text:slow"))
#define MEMMAP_CODE_SLOW_STOP_SEC    COMPILER_PRAGMA(DATA_SECTION())

/* ISR Sections */
#define MEMMAP_CODE_ISR_START_SEC    COMPILER_PRAGMA(DATA_SECTION(".text:isr"))
#define MEMMAP_CODE_ISR_STOP_SEC     COMPILER_PRAGMA(DATA_SECTION())

/* Data Sections */
#define MEMMAP_DATA_START_SEC        COMPILER_PRAGMA(DATA_SECTION(".data"))
#define MEMMAP_DATA_STOP_SEC         COMPILER_PRAGMA(DATA_SECTION())

#define MEMMAP_DATA_FAST_START_SEC   COMPILER_PRAGMA(DATA_SECTION(".data:fast"))
#define MEMMAP_DATA_FAST_STOP_SEC    COMPILER_PRAGMA(DATA_SECTION())

#define MEMMAP_DATA_SLOW_START_SEC   COMPILER_PRAGMA(DATA_SECTION(".data:slow"))
#define MEMMAP_DATA_SLOW_STOP_SEC    COMPILER_PRAGMA(DATA_SECTION())

/* BSS Sections */
#define MEMMAP_BSS_START_SEC         COMPILER_PRAGMA(DATA_SECTION(".bss"))
#define MEMMAP_BSS_STOP_SEC          COMPILER_PRAGMA(DATA_SECTION())

#define MEMMAP_BSS_FAST_START_SEC    COMPILER_PRAGMA(DATA_SECTION(".bss:fast"))
#define MEMMAP_BSS_FAST_STOP_SEC     COMPILER_PRAGMA(DATA_SECTION())

#define MEMMAP_BSS_SLOW_START_SEC    COMPILER_PRAGMA(DATA_SECTION(".bss:slow"))
#define MEMMAP_BSS_SLOW_STOP_SEC     COMPILER_PRAGMA(DATA_SECTION())

/* Const Sections */
#define MEMMAP_CONST_START_SEC       COMPILER_PRAGMA(DATA_SECTION(".const"))
#define MEMMAP_CONST_STOP_SEC        COMPILER_PRAGMA(DATA_SECTION())

#define MEMMAP_CONST_FAST_START_SEC  COMPILER_PRAGMA(DATA_SECTION(".const:fast"))
#define MEMMAP_CONST_FAST_STOP_SEC   COMPILER_PRAGMA(DATA_SECTION())

#define MEMMAP_CONST_SLOW_START_SEC  COMPILER_PRAGMA(DATA_SECTION(".const:slow"))
#define MEMMAP_CONST_SLOW_STOP_SEC   COMPILER_PRAGMA(DATA_SECTION())

/* ===================[Application-Specific Sections]=================== */

/* Application Code Sections */
#define MEMMAP_APP_CODE_START_SEC    COMPILER_PRAGMA(DATA_SECTION(".text:app"))
#define MEMMAP_APP_CODE_STOP_SEC     COMPILER_PRAGMA(DATA_SECTION())

/* Application Data Sections */
#define MEMMAP_APP_DATA_START_SEC    COMPILER_PRAGMA(DATA_SECTION(".data:app"))
#define MEMMAP_APP_DATA_STOP_SEC     COMPILER_PRAGMA(DATA_SECTION())

/* Application BSS Sections */
#define MEMMAP_APP_BSS_START_SEC     COMPILER_PRAGMA(DATA_SECTION(".bss:app"))
#define MEMMAP_APP_BSS_STOP_SEC      COMPILER_PRAGMA(DATA_SECTION())

/* ===================[MCAL Sections]=================== */

/* MCAL Code Sections */
#define MEMMAP_MCAL_CODE_START_SEC   COMPILER_PRAGMA(DATA_SECTION(".text:mcal"))
#define MEMMAP_MCAL_CODE_STOP_SEC    COMPILER_PRAGMA(DATA_SECTION())

/* MCAL Data Sections */
#define MEMMAP_MCAL_DATA_START_SEC   COMPILER_PRAGMA(DATA_SECTION(".data:mcal"))
#define MEMMAP_MCAL_DATA_STOP_SEC    COMPILER_PRAGMA(DATA_SECTION())

/* MCAL BSS Sections */
#define MEMMAP_MCAL_BSS_START_SEC    COMPILER_PRAGMA(DATA_SECTION(".bss:mcal"))
#define MEMMAP_MCAL_BSS_STOP_SEC     COMPILER_PRAGMA(DATA_SECTION())

/* ===================[ECUAL Sections]=================== */

/* ECUAL Code Sections */
#define MEMMAP_ECUAL_CODE_START_SEC  COMPILER_PRAGMA(DATA_SECTION(".text:ecual"))
#define MEMMAP_ECUAL_CODE_STOP_SEC   COMPILER_PRAGMA(DATA_SECTION())

/* ECUAL Data Sections */
#define MEMMAP_ECUAL_DATA_START_SEC  COMPILER_PRAGMA(DATA_SECTION(".data:ecual"))
#define MEMMAP_ECUAL_DATA_STOP_SEC   COMPILER_PRAGMA(DATA_SECTION())

/* ECUAL BSS Sections */
#define MEMMAP_ECUAL_BSS_START_SEC   COMPILER_PRAGMA(DATA_SECTION(".bss:ecual"))
#define MEMMAP_ECUAL_BSS_STOP_SEC    COMPILER_PRAGMA(DATA_SECTION())

/* ===================[Services Sections]=================== */

/* Services Code Sections */
#define MEMMAP_SERVICES_CODE_START_SEC COMPILER_PRAGMA(DATA_SECTION(".text:services"))
#define MEMMAP_SERVICES_CODE_STOP_SEC  COMPILER_PRAGMA(DATA_SECTION())

/* Services Data Sections */
#define MEMMAP_SERVICES_DATA_START_SEC COMPILER_PRAGMA(DATA_SECTION(".data:services"))
#define MEMMAP_SERVICES_DATA_STOP_SEC  COMPILER_PRAGMA(DATA_SECTION())

/* Services BSS Sections */
#define MEMMAP_SERVICES_BSS_START_SEC  COMPILER_PRAGMA(DATA_SECTION(".bss:services"))
#define MEMMAP_SERVICES_BSS_STOP_SEC   COMPILER_PRAGMA(DATA_SECTION())

/* ===================[FreeRTOS Sections]=================== */

/* FreeRTOS Code Sections */
#define MEMMAP_FREERTOS_CODE_START_SEC COMPILER_PRAGMA(DATA_SECTION(".text:freertos"))
#define MEMMAP_FREERTOS_CODE_STOP_SEC  COMPILER_PRAGMA(DATA_SECTION())

/* FreeRTOS Data Sections */
#define MEMMAP_FREERTOS_DATA_START_SEC COMPILER_PRAGMA(DATA_SECTION(".data:freertos"))
#define MEMMAP_FREERTOS_DATA_STOP_SEC  COMPILER_PRAGMA(DATA_SECTION())

/* FreeRTOS BSS Sections */
#define MEMMAP_FREERTOS_BSS_START_SEC  COMPILER_PRAGMA(DATA_SECTION(".bss:freertos"))
#define MEMMAP_FREERTOS_BSS_STOP_SEC   COMPILER_PRAGMA(DATA_SECTION())

/* ===================[Stack and Heap Sections]=================== */

/* Stack Sections */
#define MEMMAP_STACK_START_SEC       COMPILER_PRAGMA(DATA_SECTION(".stack"))
#define MEMMAP_STACK_STOP_SEC        COMPILER_PRAGMA(DATA_SECTION())

/* Heap Sections */
#define MEMMAP_HEAP_START_SEC        COMPILER_PRAGMA(DATA_SECTION(".heap"))
#define MEMMAP_HEAP_STOP_SEC         COMPILER_PRAGMA(DATA_SECTION())

/* ===================[Default Section Mapping]=================== */

/* Default mappings for backward compatibility */
#define MEMMAP_DEFAULT_CODE_START_SEC    MEMMAP_CODE_START_SEC
#define MEMMAP_DEFAULT_CODE_STOP_SEC     MEMMAP_CODE_STOP_SEC

#define MEMMAP_DEFAULT_DATA_START_SEC    MEMMAP_DATA_START_SEC
#define MEMMAP_DEFAULT_DATA_STOP_SEC     MEMMAP_DATA_STOP_SEC

#define MEMMAP_DEFAULT_BSS_START_SEC     MEMMAP_BSS_START_SEC
#define MEMMAP_DEFAULT_BSS_STOP_SEC      MEMMAP_BSS_STOP_SEC

#define MEMMAP_DEFAULT_CONST_START_SEC   MEMMAP_CONST_START_SEC
#define MEMMAP_DEFAULT_CONST_STOP_SEC    MEMMAP_CONST_STOP_SEC

#endif /* CONFIG_MEMMAP_H_ */