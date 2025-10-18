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

/* Code Sections - Default sections for TI ARM compiler */
#define MEMMAP_CODE_START_SEC        /* Default .text section */
#define MEMMAP_CODE_STOP_SEC         /* End of section */

#define MEMMAP_CODE_FAST_START_SEC   /* Fast code section */
#define MEMMAP_CODE_FAST_STOP_SEC    /* End of fast section */

#define MEMMAP_CODE_SLOW_START_SEC   /* Slow code section */
#define MEMMAP_CODE_SLOW_STOP_SEC    /* End of slow section */

/* ISR Sections */
#define MEMMAP_CODE_ISR_START_SEC    /* ISR code section */
#define MEMMAP_CODE_ISR_STOP_SEC     /* End of ISR section */

/* Data Sections */
#define MEMMAP_DATA_START_SEC        /* Default .data section */
#define MEMMAP_DATA_STOP_SEC         /* End of data section */

#define MEMMAP_DATA_FAST_START_SEC   /* Fast data section */
#define MEMMAP_DATA_FAST_STOP_SEC    /* End of fast data section */

#define MEMMAP_DATA_SLOW_START_SEC   /* Slow data section */
#define MEMMAP_DATA_SLOW_STOP_SEC    /* End of slow data section */

/* BSS Sections */
#define MEMMAP_BSS_START_SEC         /* Default .bss section */
#define MEMMAP_BSS_STOP_SEC          /* End of BSS section */

#define MEMMAP_BSS_FAST_START_SEC    /* Fast BSS section */
#define MEMMAP_BSS_FAST_STOP_SEC     /* End of fast BSS section */

#define MEMMAP_BSS_SLOW_START_SEC    /* Slow BSS section */
#define MEMMAP_BSS_SLOW_STOP_SEC     /* End of slow BSS section */

/* Const Sections */
#define MEMMAP_CONST_START_SEC       /* Default .const section */
#define MEMMAP_CONST_STOP_SEC        /* End of const section */

#define MEMMAP_CONST_FAST_START_SEC  /* Fast const section */
#define MEMMAP_CONST_FAST_STOP_SEC   /* End of fast const section */

#define MEMMAP_CONST_SLOW_START_SEC  /* Slow const section */
#define MEMMAP_CONST_SLOW_STOP_SEC   /* End of slow const section */

/* ===================[Application-Specific Sections]=================== */

/* Application Code Sections */
#define MEMMAP_APP_CODE_START_SEC    /* Application code section */
#define MEMMAP_APP_CODE_STOP_SEC     /* End of application code section */

/* Application Data Sections */
#define MEMMAP_APP_DATA_START_SEC    /* Application data section */
#define MEMMAP_APP_DATA_STOP_SEC     /* End of application data section */

/* Application BSS Sections */
#define MEMMAP_APP_BSS_START_SEC     /* Application BSS section */
#define MEMMAP_APP_BSS_STOP_SEC      /* End of application BSS section */

/* ===================[MCAL Sections]=================== */

/* MCAL Code Sections */
#define MEMMAP_MCAL_CODE_START_SEC   /* MCAL code section */
#define MEMMAP_MCAL_CODE_STOP_SEC    /* End of MCAL code section */

/* MCAL Data Sections */
#define MEMMAP_MCAL_DATA_START_SEC   /* MCAL data section */
#define MEMMAP_MCAL_DATA_STOP_SEC    /* End of MCAL data section */

/* MCAL BSS Sections */
#define MEMMAP_MCAL_BSS_START_SEC    /* MCAL BSS section */
#define MEMMAP_MCAL_BSS_STOP_SEC     /* End of MCAL BSS section */

/* ===================[ECUAL Sections]=================== */

/* ECUAL Code Sections */
#define MEMMAP_ECUAL_CODE_START_SEC  /* ECUAL code section */
#define MEMMAP_ECUAL_CODE_STOP_SEC   /* End of ECUAL code section */

/* ECUAL Data Sections */
#define MEMMAP_ECUAL_DATA_START_SEC  /* ECUAL data section */
#define MEMMAP_ECUAL_DATA_STOP_SEC   /* End of ECUAL data section */

/* ECUAL BSS Sections */
#define MEMMAP_ECUAL_BSS_START_SEC   /* ECUAL BSS section */
#define MEMMAP_ECUAL_BSS_STOP_SEC    /* End of ECUAL BSS section */

/* ===================[Services Sections]=================== */

/* Services Code Sections */
#define MEMMAP_SERVICES_CODE_START_SEC /* Services code section */
#define MEMMAP_SERVICES_CODE_STOP_SEC  /* End of services code section */

/* Services Data Sections */
#define MEMMAP_SERVICES_DATA_START_SEC /* Services data section */
#define MEMMAP_SERVICES_DATA_STOP_SEC  /* End of services data section */

/* Services BSS Sections */
#define MEMMAP_SERVICES_BSS_START_SEC  /* Services BSS section */
#define MEMMAP_SERVICES_BSS_STOP_SEC   /* End of services BSS section */

/* ===================[FreeRTOS Sections]=================== */

/* FreeRTOS Code Sections */
#define MEMMAP_FREERTOS_CODE_START_SEC /* FreeRTOS code section */
#define MEMMAP_FREERTOS_CODE_STOP_SEC  /* End of FreeRTOS code section */

/* FreeRTOS Data Sections */
#define MEMMAP_FREERTOS_DATA_START_SEC /* FreeRTOS data section */
#define MEMMAP_FREERTOS_DATA_STOP_SEC  /* End of FreeRTOS data section */

/* FreeRTOS BSS Sections */
#define MEMMAP_FREERTOS_BSS_START_SEC  /* FreeRTOS BSS section */
#define MEMMAP_FREERTOS_BSS_STOP_SEC   /* End of FreeRTOS BSS section */

/* ===================[Stack and Heap Sections]=================== */

/* Stack Sections */
#define MEMMAP_STACK_START_SEC       /* Stack section */
#define MEMMAP_STACK_STOP_SEC        /* End of stack section */

/* Heap Sections */
#define MEMMAP_HEAP_START_SEC        /* Heap section */
#define MEMMAP_HEAP_STOP_SEC         /* End of heap section */

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

