/**
 * @file App_ThermalTask.c
 * @brief Thermal Management Task Implementation
 * @details Manages active cooling system based on temperature readings
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.0.0
 */

#include "../../CONFIG/Std_Types.h"
#include "../../SERVICES/THERMAL/ThermalMgmt.h"

/* ===================[External Configurations]=================== */
extern const ThermalMgmt_ConfigType ThermalMgmt_Config;

/* ===================[Private Variables]=================== */
static boolean App_ThermalInitialized = FALSE;

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize thermal task
 */
void App_ThermalTask_Init(void)
{
    /* ThermalMgmt should be initialized in System_Init */
    App_ThermalInitialized = TRUE;
}

/**
 * @brief Thermal task main function (called by FreeRTOS task)
 */
void App_ThermalTask_Run(void)
{
    if (!App_ThermalInitialized)
    {
        App_ThermalTask_Init();
    }
    
    /* Run thermal management main function */
    ThermalMgmt_MainFunction();
}
