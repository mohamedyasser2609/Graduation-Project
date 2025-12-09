/**
 * @file ENCODER_PBCfg.c
 * @brief Encoder Driver Post-Build Configuration for EMG49 Motor Encoder
 * @details Configuration for EMG49 encoder (12 PPR)
 *
 * @author Mohamed Yasser
 * @date Nov 5, 2025
 * @version 1.0.0
 */

#include "ENCODER.h"

/* ===================[Encoder Configuration]=================== */
/**
 * @brief EMG49 Encoder Configuration
 * @details EMG49 specifications:
 *          - ~245 PPR (pre-quadrature) = ~980 counts per revolution in quadrature
 *          - Quadrature mode: 980 counts per revolution
 *          - Velocity filtering enabled for smooth RPM readings
 * 
 * @note Velocity Timer Period Calculation:
 *       QEI VelocityTimerLoad = 160000 (from QEI_PBCfg.c)
 *       System Clock = 80 MHz
 *       Timer Period = VelocityTimerLoad / SystemClock
 *                    = 160000 / 80,000,000 = 0.002 seconds = 2000 microseconds
 */
const Encoder_ConfigType Encoder_Config = {
    .PulsesPerRevolution = 245u,              /* EMG49: ~245 PPR pre-quadrature */
    .QuadratureCountsPerRev = 980u,           /* ~980 counts per revolution in quadrature */
    .EnableVelocityFilter = TRUE,             /* Enable velocity filtering */
    .VelocityFilterAlpha = 200u,              /* Filter coefficient (200/256 = ~78% new, 22% old) */
    .VelocityTimerPeriodUs = 2000u           /* 2ms velocity measurement period (matches QEI config) */
};

