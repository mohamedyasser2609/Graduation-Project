/**
 * @file main_gps_production.c
 * @brief Production GPS NEO-M8N Test Application
 * @details Comprehensive test and demonstration of production GPS driver features
 *
 * Hardware Connections:
 * - GPS TX → TM4C RX (UART1: PB0)
 * - GPS RX → TM4C TX (UART1: PB1)
 * - GPS VCC → 3.3V
 * - GPS GND → GND
 *
 * Debug Output:
 * - UART0 (PA0/PA1) → USB Serial to PC terminal at 115200 baud
 *
 * Features Demonstrated:
 * - Multi-GNSS configuration (GPS + GLONASS + Galileo)
 * - UBX ACK/NAK verification
 * - SBAS system selection
 * - Power management modes
 * - Real-time constellation tracking
 * - Enhanced diagnostics with DOP values
 * - Fix type detection (2D/3D/DGPS)
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 2.0.0 Production
 */

#if 0  /* Disabled - moved to testing folder */


/* ===================[Includes]=================== */
#include "CONFIG/Det.h"
#include "MCAL/GPIO/Gpio.h"
#include "MCAL/UART/Uart.h"
#include "MCAL/MCU/Mcu.h"
#include "ECUAL/GPS/GPS.h"
#include "CONFIG/Std_Types.h"

/* ===================[Definitions]=================== */
#define DEBUG_UART_MODULE       UART_MODULE_0
#define SYSCTL_RCGCWD_R         (*((volatile uint32 *)0x400FE600))
#define UPDATE_INTERVAL         80000    /* ~1 second at 80MHz */
#define STATS_INTERVAL          10       /* Statistics every 10 updates */

/* ===================[External Configurations]=================== */
extern const Mcu_ConfigType Mcu_Config_80MHz;
extern const Gpio_ConfigType Gpio_Configuration;
extern const Uart_ConfigType Uart0_Config_115200;
extern const Uart_ConfigType Uart1_Config_9600;
extern const GPS_ConfigType GPS_Config_Automotive;

/* ===================[Global Variables]=================== */
static uint32 updateCounter = 0;
static uint32 updateNumber = 0;
static uint32 fixStartTime = 0;
static uint32 totalHdop = 0;
static uint32 totalPdop = 0;
static uint32 totalSats = 0;
static uint32 statsCount = 0;

/* ===================[Interrupt Handlers]=================== */
void Timer2A_Handler(void) {
    /* Not used */
}

/* ===================[Helper Functions]=================== */

/**
 * @brief Send string to debug UART
 */
static void Print(const char* str) {
    volatile uint32 delay;
    Uart_SendString(DEBUG_UART_MODULE, (const uint8*)str);
    for (delay = 0; delay < 50; delay++);
}

/**
 * @brief Print integer
 */
static void PrintInt(sint32 value) {
    char buffer[12];
    uint8 i = 0, j, temp;
    sint32 num = value;
    
    if (num < 0) {
        buffer[i++] = '-';
        num = -num;
    }
    
    if (num == 0) {
        buffer[i++] = '0';
    } else {
        j = i;
        while (num > 0) {
            buffer[i++] = '0' + (num % 10);
            num /= 10;
        }
        buffer[i] = '\0';
        i--;
        while (j < i) {
            temp = buffer[j];
            buffer[j] = buffer[i];
            buffer[i] = temp;
            j++;
            i--;
        }
    }
    buffer[i + 1] = '\0';
    Print(buffer);
}

/**
 * @brief Print float with specified decimals
 */
static void PrintFloat(float32 value, uint8 decimals) {
    char buffer[20];
    sint32 intPart, fracPart, multiplier = 1;
    uint8 i, len = 0, fracLen = 0;
    
    for (i = 0; i < decimals; i++) multiplier *= 10;
    
    if (value < 0) {
        buffer[len++] = '-';
        value = -value;
    }
    
    intPart = (sint32)value;
    fracPart = (sint32)((value - (float32)intPart) * (float32)multiplier);
    if (fracPart < 0) fracPart = -fracPart;
    
    /* Integer part */
    if (intPart == 0) {
        buffer[len++] = '0';
    } else {
        uint8 start = len;
        while (intPart > 0) {
            buffer[len++] = '0' + (intPart % 10);
            intPart /= 10;
        }
        /* Reverse */
        for (i = 0; i < (len - start) / 2; i++) {
            char temp = buffer[start + i];
            buffer[start + i] = buffer[len - 1 - i];
            buffer[len - 1 - i] = temp;
        }
    }
    
    buffer[len++] = '.';
    
    /* Fractional part */
    if (fracPart == 0) {
        for (i = 0; i < decimals; i++) buffer[len++] = '0';
    } else {
        uint8 fracStart = len;
        while (fracPart > 0) {
            buffer[len++] = '0' + (fracPart % 10);
            fracPart /= 10;
            fracLen++;
        }
        /* Add leading zeros */
        while (fracLen < decimals) {
            buffer[len++] = '0';
            fracLen++;
        }
        /* Reverse fractional part */
        for (i = 0; i < fracLen / 2; i++) {
            char temp = buffer[fracStart + i];
            buffer[fracStart + i] = buffer[len - 1 - i];
            buffer[len - 1 - i] = temp;
        }
    }
    
    buffer[len] = '\0';
    Print(buffer);
}

/**
 * @brief Print configuration ACK/NAK result
 */
static void PrintConfigResult(const char* name, Std_ReturnType result) {
    Print("  -> ");
    Print(name);
    Print(": ");
    if (result == E_OK) {
        Print("ACK \xFB\r\n");  /* ✓ */
    } else {
        Print("NAK \xFE\r\n");  /* ✗ */
    }
}

/**
 * @brief Get fix type string
 */
static const char* GetFixTypeString(uint8 fixQuality) {
    switch (fixQuality) {
        case 0: return "NO FIX";
        case 1: return "GPS";
        case 2: return "DGPS";
        case 3: return "PPS";
        case 4: return "RTK";
        case 5: return "RTK FLOAT";
        case 6: return "ESTIMATED";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Display startup banner
 */
static void DisplayBanner(void) {
    Print("\r\n\r\n");
    Print("===========================================\r\n");
    Print(" NEO-M8N GPS Production Test\r\n");
    Print(" AUTOSAR ECUAL Driver Demo\r\n");
    Print(" (C) Mohamed Yasser - Nov 2025\r\n");
    Print("===========================================\r\n");
    Print("System: TM4C123GH6PM @ 80MHz\r\n");
    Print("Debug:  UART0 @ 115200 baud\r\n");
    Print("GPS:    UART1 @ 9600 baud (NEO-M8N)\r\n");
    Print("===========================================\r\n\r\n");
}

/**
 * @brief Configure GPS and verify with ACK/NAK
 */
static void ConfigureGPS(void) {
    volatile uint32 delay;
    Std_ReturnType result;
    
    Print("Configuring NEO-M8N...\r\n");
    
    /* Multi-GNSS Configuration (already done in GPS_Init, just verify) */
    Print("  -> Multi-GNSS configured in GPS_Init()\r\n");
    Print("     GPS: Enabled | GLONASS: Enabled | Galileo: Enabled\r\n");
    Print("     (Check constellation breakdown in status output)\r\n\r\n");
    
    /* SBAS Configuration */
    Print("  -> Configuring SBAS...\r\n");
    result = GPS_ConfigureSBAS(GPS_SBAS_AUTO);
    for (delay = 0; delay < 100000; delay++);
    PrintConfigResult("SBAS (Auto-select)", result);
    
    /* Power Mode */
    Print("  -> Configuring Power Mode...\r\n");
    result = GPS_SetPowerMode(GPS_POWER_MODE_FULL);
    for (delay = 0; delay < 100000; delay++);
    PrintConfigResult("Power Mode (Full)", result);
    
    /* Dynamic Model */
    Print("  -> Configuring Dynamic Model...\r\n");
    result = GPS_SetDynamicModel(GPS_DYNAMIC_MODEL_AUTOMOTIVE);
    for (delay = 0; delay < 100000; delay++);
    PrintConfigResult("Dynamic Model (Automotive)", result);
    
    /* Update Rate */
    Print("  -> Configuring Update Rate...\r\n");
    result = GPS_SetUpdateRate(1);
    for (delay = 0; delay < 100000; delay++);
    PrintConfigResult("Update Rate (1Hz)", result);
    
    Print("\r\nConfiguration complete!\r\n");
    Print("Waiting for GPS fix (30-60s for cold start)...\r\n\r\n");
}

/**
 * @brief Display enhanced GPS status
 */
static void DisplayGPSStatus(void) {
    GPS_DataType gpsData;
    GPS_ConstellationInfoType constellations;
    
    if (GPS_GetData(&gpsData) != E_OK) {
        Print("ERROR: Cannot read GPS data\r\n");
        return;
    }
    
    GPS_GetConstellationInfo(&constellations);
    
    /* Header */
    Print("\r\n");
    Print("===========================================\r\n");
    
    /* Fix Status */
    Print("GPS STATUS: ");
    Print(GetFixTypeString(gpsData.fixQuality));
    if (gpsData.validFix) {
        Print(" | HDOP: ");
        PrintFloat(gpsData.dop.hdop, 2);
        Print(" | Sats: ");
        PrintInt(gpsData.satellitesUsed);
        Print("/");
        PrintInt(gpsData.satellitesInView);
        Print("\r\n");
        
        /* Position */
        Print("LAT: ");
        PrintFloat(gpsData.position.latitude, 6);
        Print(" | LON: ");
        PrintFloat(gpsData.position.longitude, 6);
        Print("\r\nALT: ");
        PrintFloat(gpsData.position.altitude, 2);
        Print(" m | SPD: ");
        PrintFloat(gpsData.velocity.speedKmh, 2);
        Print(" km/h\r\n");
        
        /* Constellations */
        Print("GPS: ");
        PrintInt(constellations.gpsUsed);
        Print(" | GLONASS: ");
        PrintInt(constellations.glonassUsed);
        Print(" | GALILEO: ");
        PrintInt(constellations.galileoUsed);
        Print(" | BEIDOU: ");
        PrintInt(constellations.beidouUsed);
        Print("\r\n");
        
        /* Satellite PRNs (for debugging) */
        {
            uint8 i;
            Print("PRNs: ");
            for (i = 0; i < gpsData.satellitesUsed && i < 12; i++) {
                if (gpsData.satellites[i].prn > 0) {
                    PrintInt(gpsData.satellites[i].prn);
                    if (i < gpsData.satellitesUsed - 1) Print(",");
                }
            }
            Print("\r\n");
        }
        
        /* DOP Values */
        Print("HDOP: ");
        PrintFloat(gpsData.dop.hdop, 2);
        Print(" | PDOP: ");
        PrintFloat(gpsData.dop.pdop, 2);
        Print(" | VDOP: ");
        PrintFloat(gpsData.dop.vdop, 2);
        Print("\r\n");
        
        /* Time */
        Print("UTC: ");
        if (gpsData.time.hour < 10) Print("0");
        PrintInt(gpsData.time.hour);
        Print(":");
        if (gpsData.time.minute < 10) Print("0");
        PrintInt(gpsData.time.minute);
        Print(":");
        if (gpsData.time.second < 10) Print("0");
        PrintInt(gpsData.time.second);
        Print("\r\n");
        
        /* Accumulate statistics */
        totalHdop += (uint32)(gpsData.dop.hdop * 100);
        totalPdop += (uint32)(gpsData.dop.pdop * 100);
        totalSats += gpsData.satellitesUsed;
        statsCount++;
        
        if (fixStartTime == 0) {
            fixStartTime = updateNumber;
        }
    } else {
        Print(" | Searching... Sats: ");
        PrintInt(gpsData.satellitesInView);
        Print("\r\n");
        Print("Waiting for fix...\r\n");
    }
    
    Print("===========================================\r\n");
    
    /* Statistics every 10 updates */
    if (statsCount > 0 && (updateNumber % STATS_INTERVAL) == 0) {
        Print("\r\n--- 10-Second Statistics ---\r\n");
        Print("Avg HDOP: ");
        PrintFloat((float32)totalHdop / (float32)statsCount / 100.0f, 2);
        Print(" | Avg PDOP: ");
        PrintFloat((float32)totalPdop / (float32)statsCount / 100.0f, 2);
        Print("\r\nAvg Sats: ");
        PrintInt(totalSats / statsCount);
        if (fixStartTime > 0) {
            Print(" | Fix Uptime: ");
            PrintInt(updateNumber - fixStartTime);
            Print("s");
        }
        Print("\r\n----------------------------\r\n");
        
        /* Reset statistics */
        totalHdop = 0;
        totalPdop = 0;
        totalSats = 0;
        statsCount = 0;
    }
}

/* ===================[Main Function]=================== */

int main(void) {
    volatile uint32 delay;
    
    /* Disable watchdog */
    SYSCTL_RCGCWD_R = 0;
    
    /* Initialize DET */
    Det_Init();
    
    /* Initialize MCU @ 80MHz */
    Mcu_Init(&Mcu_Config_80MHz);
    Mcu_InitClock(MCU_CLOCK_80MHZ);
    for (delay = 0; delay < 50000; delay++);
    Mcu_DistributePllClock();
    for (delay = 0; delay < 50000; delay++);
    
    /* Initialize GPIO */
    Gpio_Init(&Gpio_Configuration);
    for (delay = 0; delay < 10000; delay++);
    
    /* Initialize UARTs */
    Uart_Init(&Uart0_Config_115200);
    for (delay = 0; delay < 50000; delay++);
    Uart_Init(&Uart1_Config_9600);
    for (delay = 0; delay < 50000; delay++);
    
    /* Display banner */
    DisplayBanner();
    
    /* Wait for GPS module to boot (CRITICAL!) */
    Print("Waiting for GPS module to boot...\r\n");
    for (delay = 0; delay < 800000; delay++);  /* ~1 second delay */
    Print("GPS module ready\r\n\r\n");
    
    /* Initialize GPS driver */
    Print("Initializing GPS driver...\r\n");
    if (GPS_Init(&GPS_Config_Automotive) != E_OK) {
        Print("ERROR: GPS initialization failed!\r\n");
        Print("Possible causes:\r\n");
        Print("  1. GPS module not powered\r\n");
        Print("  2. UART connections incorrect\r\n");
        Print("  3. GPS module not responding to UBX\r\n");
        Print("  4. Need to power cycle GPS module\r\n");
        while(1);
    }
    Print("GPS driver initialized successfully!\r\n\r\n");
    
    /* Configure GPS with ACK verification */
    ConfigureGPS();
    
    /* Main loop */
    Print("Entering main loop...\r\n");
    
    while (1) {
        /* Process GPS data */
        GPS_ProcessData();
        
        /* Display status periodically */
        updateCounter++;
        if (updateCounter >= UPDATE_INTERVAL) {
            updateNumber++;
            DisplayGPSStatus();
            updateCounter = 0;
        }
        
        /* Small delay */
        for (delay = 0; delay < 100; delay++);
    }
}

#endif  /* <<<< END OF COMMENTED OUT IMU TEST CODE >>>> */

