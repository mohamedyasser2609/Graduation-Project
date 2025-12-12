/**
 * @file GPS.c
 * @brief Production GPS Driver Implementation for NEO-M8N
 * @details Complete implementation with NMEA parsing, UBX protocol, and ACK/NAK handling
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 2.0.0
 * @compliance AUTOSAR 4.4.0
 */

#include "GPS.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ===================[Private Variables]=================== */

static GPS_ConfigType GPS_Config;
static GPS_DataType GPS_CurrentData;
static boolean GPS_Initialized = FALSE;

/* NMEA parsing buffers */
static uint8 GPS_NmeaBuffer[GPS_NMEA_BUFFER_SIZE];
static uint8 GPS_BufferIndex = 0;

/* UBX parsing buffers and state */
static uint8 GPS_UbxBuffer[GPS_UBX_BUFFER_SIZE];
static uint16 GPS_UbxBufferIndex = 0;
static boolean GPS_UbxReceiving = FALSE;
static uint16 GPS_UbxExpectedLength = 0;

/* UBX ACK/NAK tracking */
typedef struct {
    boolean ackReceived;
    boolean nakReceived;
    uint8 ackClass;
    uint8 ackId;
    uint32 timestamp;
} GPS_AckStatusType;

static GPS_AckStatusType GPS_AckStatus = {FALSE, FALSE, 0, 0, 0};
static uint32 GPS_SystemTicks = 0;  /* Simple tick counter */

/* Last NMEA sentence type */
static const char* GPS_LastNmeaType = "NONE";

/* ===================[Private Function Prototypes]=================== */

static Std_ReturnType GPS_SendUBXCommand(const uint8* cmd, uint16 length);
static void GPS_CalculateUBXChecksum(const uint8* data, uint16 length, uint8* ckA, uint8* ckB);
static uint8 GPS_SplitNmeaSentence(uint8* sentence, uint8* fields[], uint8 maxFields);
static float32 GPS_NmeaToDecimal(uint8* nmeaCoord, uint8 direction);
static float32 GPS_ParseFloat(uint8* str);
static uint32 GPS_ParseInt(uint8* str);
static boolean GPS_StringCompare(const uint8* str1, const uint8* str2);
static void GPS_ParseGGA(uint8* fields[], uint8 fieldCount);
static void GPS_ParseRMC(uint8* fields[], uint8 fieldCount);
static void GPS_ParseGSA(uint8* fields[], uint8 fieldCount);
static void GPS_ParseGSV(uint8* fields[], uint8 fieldCount);
static void GPS_ParseNmeaSentence(uint8* sentence);
static void GPS_ProcessUbxMessage(const uint8* msg, uint16 length);
static void GPS_HandleAckNak(const uint8* payload, uint16 length, boolean isAck);
static Std_ReturnType GPS_SendUBXCommandWithAck(const uint8* cmd, uint16 length, uint32 timeoutMs);

/* ===================[Public Function Implementations]=================== */

/**
 * @brief Initialize GPS driver
 */
Std_ReturnType GPS_Init(const GPS_ConfigType* ConfigPtr) {
    volatile uint32 i;
    
    if (ConfigPtr == NULL_PTR) {
        return E_NOT_OK;
    }
    
    /* Store configuration */
    GPS_Config = *ConfigPtr;
    
    /* Initialize GPS data structure */
    memset(&GPS_CurrentData, 0, sizeof(GPS_DataType));
    
    /* Clear buffer */
    GPS_BufferIndex = 0;
    
    /* Initialize GPS data structure */
    GPS_CurrentData.satellitesUsed = 0;
    GPS_CurrentData.satellitesInView = 0;
    GPS_CurrentData.validFix = FALSE;
    
    /* Set initialized flag BEFORE sending UBX commands */
    GPS_Initialized = TRUE;
    
    /* CRITICAL: Configure UART port to accept UBX protocol (CFG-PRT) */
    /* This MUST be done before sending any other UBX commands */
    {
        uint8 cfgPrtCmd[] = {
            GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,
            GPS_UBX_CLASS_CFG, GPS_UBX_CFG_PRT,
            0x14, 0x00,  /* Length = 20 bytes */
            0x01,        /* portID = 1 (UART) */
            0x00,        /* reserved */
            0x00, 0x00,  /* txReady (disabled) */
            0xD0, 0x08, 0x00, 0x00,  /* mode: 8N1 (8 bits, no parity, 1 stop) */
            0x80, 0x25, 0x00, 0x00,  /* baudRate: 9600 (0x2580 = 9600) */
            0x03, 0x00,  /* inProtoMask: UBX (0x01) + NMEA (0x02) = 0x03 */
            0x03, 0x00,  /* outProtoMask: UBX (0x01) + NMEA (0x02) = 0x03 */
            0x00, 0x00,  /* flags: no extended timeout */
            0x00, 0x00   /* reserved */
        };
        
        /* Send CFG-PRT and verify ACK - Try to configure but don't fail init */
        /* Some GPS modules may already have UBX enabled by default */
        GPS_SendUBXCommandWithAck(cfgPrtCmd, sizeof(cfgPrtCmd), 2000);
        
        /* Wait for port configuration to apply - NEO-M8N needs more time */
        for (i = 0; i < 400000; i++);  /* ~500ms at 80MHz */
    }
    
    /* Configure multi-GNSS constellations - Try to configure but continue if fails */
    /* GPS will still work with default constellation (GPS only) */
    /* NOTE: Some NEO-M8N modules reject CFG-GNSS - they use factory defaults */
    if (GPS_Config.EnableGLONASS || GPS_Config.EnableGalileo || GPS_Config.EnableBeiDou) {
        GPS_ConfigureGNSS(TRUE, GPS_Config.EnableGLONASS, 
                          GPS_Config.EnableGalileo, GPS_Config.EnableBeiDou);
        
        /* Wait longer for GNSS configuration to apply (200ms) */
        for (i = 0; i < 200000; i++);
    }
    
    /* Configure SBAS if enabled */
    if (GPS_Config.EnableSBAS) {
        GPS_ConfigureSBAS(GPS_Config.SbasSystem);
        for (i = 0; i < 50000; i++);
    }
    
    /* Set dynamic platform model */
    GPS_SetDynamicModel(GPS_Config.DynamicModel);
    for (i = 0; i < 50000; i++);
    
    /* Set update rate */
    if (GPS_Config.UpdateRate != GPS_DEFAULT_UPDATE_RATE) {
        GPS_SetUpdateRate(GPS_Config.UpdateRate);
        for (i = 0; i < 50000; i++);
    }
    
    /* Configure power mode */
    GPS_SetPowerMode(GPS_Config.PowerMode);
    for (i = 0; i < 50000; i++);
    
    /* Configure PPS if enabled */
    if (GPS_Config.EnablePPS) {
        GPS_ConfigurePPS(TRUE, 1);  /* 1 Hz default */
        for (i = 0; i < 50000; i++);
    }
    
    /* Save configuration to flash - IMPORTANT for persistence */
    if (GPS_SaveConfiguration() != E_OK) {
        /* Save failed - configuration won't persist across power cycles */
        /* Continue anyway as GPS is functional, just won't remember settings */
    }
    for (i = 0; i < 100000; i++);  /* Wait for save to complete */
    
    /* Enable RTCM if configured */
    if (GPS_Config.EnableRTCM) {
        GPS_EnableRTCM(TRUE);
        for (i = 0; i < 50000; i++);
    }
    
    return E_OK;
}

/**
 * @brief De-initialize GPS
 */
Std_ReturnType GPS_DeInit(void) {
    GPS_Initialized = FALSE;
    return E_OK;
}

/**
 * @brief Process incoming GPS data (NMEA and UBX)
 */
Std_ReturnType GPS_ProcessData(void) {
    uint8 receivedByte;
    
    if (!GPS_Initialized) {
        return E_NOT_OK;
    }
    
    /* Increment system tick counter */
    GPS_SystemTicks++;
    
    /* Read available data from UART (non-blocking) */
    while (Uart_IsRxDataAvailable(GPS_Config.UartModule)) {
        if (Uart_ReceiveByte(GPS_Config.UartModule, &receivedByte) == E_OK) {
            
            /* Check for UBX message start */
            if (!GPS_UbxReceiving && receivedByte == GPS_UBX_SYNC_CHAR_1) {
                GPS_UbxBuffer[0] = receivedByte;
                GPS_UbxBufferIndex = 1;
                GPS_UbxReceiving = FALSE;  /* Wait for second sync char */
                continue;
            }
            
            /* Check for second UBX sync character */
            if (GPS_UbxBufferIndex == 1 && receivedByte == GPS_UBX_SYNC_CHAR_2) {
                GPS_UbxBuffer[1] = receivedByte;
                GPS_UbxBufferIndex = 2;
                GPS_UbxReceiving = TRUE;
                GPS_UbxExpectedLength = 0;
                continue;
            }
            
            /* Handle UBX message reception */
            if (GPS_UbxReceiving) {
                if (GPS_UbxBufferIndex < GPS_UBX_BUFFER_SIZE) {
                    GPS_UbxBuffer[GPS_UbxBufferIndex++] = receivedByte;
                    
                    /* Get payload length after header */
                    if (GPS_UbxBufferIndex == 6) {
                        GPS_UbxExpectedLength = GPS_UbxBuffer[4] | (GPS_UbxBuffer[5] << 8);
                        GPS_UbxExpectedLength += 8;  /* Header(2) + Class(1) + ID(1) + Len(2) + Checksum(2) */
                    }
                    
                    /* Check if complete message received */
                    if (GPS_UbxExpectedLength > 0 && GPS_UbxBufferIndex >= GPS_UbxExpectedLength) {
                        GPS_ProcessUbxMessage(GPS_UbxBuffer, GPS_UbxBufferIndex);
                        GPS_UbxReceiving = FALSE;
                        GPS_UbxBufferIndex = 0;
                    }
                } else {
                    /* Buffer overflow */
                    GPS_UbxReceiving = FALSE;
                    GPS_UbxBufferIndex = 0;
                }
                continue;
            }
            
            /* Handle NMEA sentence */
            if (receivedByte == '$') {
                GPS_BufferIndex = 0;
            }
            
            if (GPS_BufferIndex < GPS_NMEA_BUFFER_SIZE - 1) {
                GPS_NmeaBuffer[GPS_BufferIndex++] = receivedByte;
                
                if (receivedByte == '\n') {
                    GPS_NmeaBuffer[GPS_BufferIndex] = '\0';
                    GPS_ParseNmeaSentence(GPS_NmeaBuffer);
                    GPS_BufferIndex = 0;
                }
            } else {
                GPS_BufferIndex = 0;
            }
        }
    }
    
    return E_OK;
}

/**
 * @brief Get current GPS data
 */
Std_ReturnType GPS_GetData(GPS_DataType* Data) {
    if (!GPS_Initialized || Data == NULL_PTR) {
        return E_NOT_OK;
    }
    
    *Data = GPS_CurrentData;
    GPS_CurrentData.dataUpdated = FALSE;
    
    return E_OK;
}

/**
 * @brief Get current position
 */
Std_ReturnType GPS_GetPosition(GPS_PositionType* Position) {
    if (!GPS_Initialized || Position == NULL_PTR) {
        return E_NOT_OK;
    }
    
    *Position = GPS_CurrentData.position;
    return E_OK;
}

/**
 * @brief Get current velocity
 */
Std_ReturnType GPS_GetVelocity(GPS_VelocityType* Velocity) {
    if (!GPS_Initialized || Velocity == NULL_PTR) {
        return E_NOT_OK;
    }
    
    *Velocity = GPS_CurrentData.velocity;
    return E_OK;
}

/**
 * @brief Get current time
 */
Std_ReturnType GPS_GetTime(GPS_TimeType* Time) {
    if (!GPS_Initialized || Time == NULL_PTR) {
        return E_NOT_OK;
    }
    
    *Time = GPS_CurrentData.time;
    return E_OK;
}

/**
 * @brief Get satellite information
 */
Std_ReturnType GPS_GetSatellites(GPS_SatelliteType* Satellites, uint8* Count) {
    uint8 i;
    
    if (!GPS_Initialized || Satellites == NULL_PTR || Count == NULL_PTR) {
        return E_NOT_OK;
    }
    
    *Count = GPS_CurrentData.satellitesInView;
    
    for (i = 0; i < GPS_CurrentData.satellitesInView && i < GPS_MAX_SATELLITES; i++) {
        Satellites[i] = GPS_CurrentData.satellites[i];
    }
    
    return E_OK;
}

/**
 * @brief Check if GPS has valid fix
 */
boolean GPS_HasValidFix(void) {
    return GPS_CurrentData.validFix;
}

/**
 * @brief Get GPS status
 */
GPS_StatusType GPS_GetStatus(void) {
    if (!GPS_Initialized) {
        return GPS_STATUS_NOT_INITIALIZED;
    }
    
    if (!GPS_CurrentData.validFix) {
        return GPS_STATUS_NO_FIX;
    }
    
    return GPS_STATUS_OK;
}

/**
 * @brief Enable SBAS
 */
Std_ReturnType GPS_EnableSBAS(void) {
    const uint8 enableSBAS[] = {
        GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,
        GPS_UBX_CLASS_CFG, GPS_UBX_CFG_SBAS,
        0x08, 0x00,  /* Length */
        0x01,        /* Enable SBAS */
        0x07,        /* Use for ranging, correction, integrity */
        0x03,        /* Max 3 SBAS channels */
        0x00,        /* Scanmode2 */
        0x00, 0x00, 0x00, 0x00  /* Scanmode (auto) */
    };
    
    return GPS_SendUBXCommand(enableSBAS, sizeof(enableSBAS));
}

/**
 * @brief Enable GLONASS
 */
Std_ReturnType GPS_EnableGLONASS(void) {
    const uint8 enableGLONASS[] = {
        GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,
        GPS_UBX_CLASS_CFG, GPS_UBX_CFG_GNSS,
        0x3C, 0x00,  /* Length = 60 */
        0x00, 0x00, 0xFF, 0x05,  /* Config */
        /* GPS Block */
        GPS_GNSS_GPS, 0x08, 0x10, 0x00,
        0x01, 0x00, 0x01, 0x01,
        /* SBAS Block */
        GPS_GNSS_SBAS, 0x01, 0x03, 0x00,
        0x01, 0x00, 0x01, 0x01,
        /* Galileo Block */
        GPS_GNSS_GALILEO, 0x04, 0x08, 0x00,
        0x00, 0x00, 0x01, 0x01,
        /* BeiDou Block */
        GPS_GNSS_BEIDOU, 0x02, 0x08, 0x00,
        0x00, 0x00, 0x01, 0x01,
        /* GLONASS Block */
        GPS_GNSS_GLONASS, 0x08, 0x0E, 0x00,
        0x01, 0x00, 0x01, 0x01
    };
    
    return GPS_SendUBXCommand(enableGLONASS, sizeof(enableGLONASS));
}

/**
 * @brief Set GPS update rate
 */
Std_ReturnType GPS_SetUpdateRate(uint8 RateHz) {
    uint16 measureRate;
    
    if (RateHz == 0 || RateHz > 10) {
        return E_NOT_OK;
    }
    
    measureRate = 1000 / RateHz;
    
    const uint8 setRate[] = {
        GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,
        GPS_UBX_CLASS_CFG, GPS_UBX_CFG_RATE,
        0x06, 0x00,  /* Length */
        (uint8)(measureRate & 0xFF), (uint8)(measureRate >> 8),  /* Measurement rate */
        0x01, 0x00,  /* Navigation rate */
        0x01, 0x00   /* Time reference: GPS */
    };
    
    return GPS_SendUBXCommand(setRate, sizeof(setRate));
}

/**
 * @brief Set dynamic platform model
 */
Std_ReturnType GPS_SetDynamicModel(uint8 Model) {
    const uint8 setModel[] = {
        GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,
        GPS_UBX_CLASS_CFG, GPS_UBX_CFG_NAV5,
        0x24, 0x00,  /* Length = 36 */
        0x01, 0x00,  /* Mask: only dynamic model */
        Model,       /* Dynamic model */
        0x03,        /* Fix mode: auto 2D/3D */
        0x00, 0x00, 0x00, 0x00,  /* Fixed altitude */
        0x10, 0x27, 0x00, 0x00,  /* Fixed altitude variance */
        0x05, 0x00,  /* Min elevation */
        0x00, 0x00,  /* Reserved */
        0xFA, 0x00,  /* Position DOP mask */
        0xFA, 0x00,  /* Time DOP mask */
        0x64, 0x00,  /* Position accuracy mask */
        0x2C, 0x01,  /* Time accuracy mask */
        0x00,        /* Static hold threshold */
        0x3C,        /* DGPS timeout */
        0x00, 0x00, 0x00, 0x00,  /* Reserved */
        0x00, 0x00, 0x00, 0x00   /* Reserved */
    };
    
    return GPS_SendUBXCommand(setModel, sizeof(setModel));
}

/**
 * @brief Reset GPS module
 */
Std_ReturnType GPS_Reset(void) {
    const uint8 resetCmd[] = {
        GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,
        GPS_UBX_CLASS_CFG, 0x04,  /* CFG-RST */
        0x04, 0x00,  /* Length */
        0xFF, 0xFF,  /* Clear all */
        0x00,        /* Hot start */
        0x00         /* Reserved */
    };
    
    return GPS_SendUBXCommand(resetCmd, sizeof(resetCmd));
}

/* ===================[Advanced Configuration Functions]=================== */

/**
 * @brief Configure multi-GNSS constellations
 * @details Sends UBX-CFG-GNSS command to enable/disable GNSS constellations
 *          Format: Header(2) + Class(1) + ID(1) + Length(2) + Payload(60) + Checksum(2)
 * @param enableGPS Enable GPS constellation (always recommended)
 * @param enableGLONASS Enable GLONASS constellation
 * @param enableGalileo Enable Galileo constellation
 * @param enableBeiDou Enable BeiDou constellation
 * @return E_OK if command sent successfully, E_NOT_OK otherwise
 */
Std_ReturnType GPS_ConfigureGNSS(boolean enableGPS, boolean enableGLONASS, 
                                  boolean enableGalileo, boolean enableBeiDou) {
    uint8 gnssConfig[] = {
        GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,  /* 0xB5 0x62 */
        GPS_UBX_CLASS_CFG, GPS_UBX_CFG_GNSS,       /* 0x06 0x3E */
        0x3C, 0x00,  /* Length = 60 bytes (5 blocks × 12 bytes) */
        
        /* Payload */
        0x00,        /* msgVer = 0 */
        0x00,        /* numTrkChHw = 0 (read-only, filled by receiver) */
        0xFF,        /* numTrkChUse = 255 (use maximum available) */
        0x05,        /* numConfigBlocks = 5 (GPS, SBAS, Galileo, BeiDou, GLONASS) */
        
        /* Block 1: GPS (gnssId=0) */
        GPS_GNSS_GPS,                              /* gnssId = 0 */
        0x08,                                       /* resTrkCh = 8 (min channels) */
        0x10,                                       /* maxTrkCh = 16 (max channels) */
        0x00,                                       /* reserved1 */
        (enableGPS ? 0x01 : 0x00), 0x01, 0x00, 0x00,  /* flags: enable, sigCfgMask */
        
        /* Block 2: SBAS (gnssId=1) - Always enable for corrections */
        GPS_GNSS_SBAS,                             /* gnssId = 1 */
        0x01,                                       /* resTrkCh = 1 */
        0x03,                                       /* maxTrkCh = 3 */
        0x00,                                       /* reserved1 */
        0x01, 0x01, 0x00, 0x00,                    /* flags: enable, sigCfgMask */
        
        /* Block 3: Galileo (gnssId=2) */
        GPS_GNSS_GALILEO,                          /* gnssId = 2 */
        0x04,                                       /* resTrkCh = 4 */
        0x08,                                       /* maxTrkCh = 8 */
        0x00,                                       /* reserved1 */
        (enableGalileo ? 0x01 : 0x00), 0x01, 0x00, 0x00,  /* flags: enable, sigCfgMask */
        
        /* Block 4: BeiDou (gnssId=3) */
        GPS_GNSS_BEIDOU,                           /* gnssId = 3 */
        0x02,                                       /* resTrkCh = 2 */
        0x08,                                       /* maxTrkCh = 8 */
        0x00,                                       /* reserved1 */
        (enableBeiDou ? 0x01 : 0x00), 0x01, 0x00, 0x00,  /* flags: enable, sigCfgMask */
        
        /* Block 5: GLONASS (gnssId=6) */
        GPS_GNSS_GLONASS,                          /* gnssId = 6 */
        0x08,                                       /* resTrkCh = 8 */
        0x0E,                                       /* maxTrkCh = 14 */
        0x00,                                       /* reserved1 */
        (enableGLONASS ? 0x01 : 0x00), 0x01, 0x00, 0x00  /* flags: enable, sigCfgMask */
        
        /* Checksum will be calculated and appended by GPS_SendUBXCommand */
    };
    
    /* Send with ACK verification (1500ms timeout for GNSS config) */
    return GPS_SendUBXCommandWithAck(gnssConfig, sizeof(gnssConfig), 1500);
}

/**
 * @brief Enable Galileo constellation
 */
Std_ReturnType GPS_EnableGalileo(void) {
    return GPS_ConfigureGNSS(TRUE, TRUE, TRUE, FALSE);
}

/**
 * @brief Enable BeiDou constellation
 */
Std_ReturnType GPS_EnableBeiDou(void) {
    return GPS_ConfigureGNSS(TRUE, TRUE, TRUE, TRUE);
}

/**
 * @brief Configure SBAS system
 */
Std_ReturnType GPS_ConfigureSBAS(uint8 sbasSystem) {
    uint8 sbasConfig[] = {
        GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,
        GPS_UBX_CLASS_CFG, GPS_UBX_CFG_SBAS,
        0x08, 0x00,  /* Length = 8 */
        0x01,        /* Mode: enabled */
        sbasSystem,  /* Usage: selected system */
        0x03,        /* Max 3 SBAS channels */
        0x00,        /* Scanmode2 */
        0x00, 0x00, 0x00, 0x00  /* Scanmode (auto if 0) */
    };
    
    return GPS_SendUBXCommand(sbasConfig, sizeof(sbasConfig));
}

/**
 * @brief Configure PPS/Timepulse output
 */
Std_ReturnType GPS_ConfigurePPS(boolean enable, uint32 freqHz) {
    uint8 ppsConfig[] = {
        GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,
        GPS_UBX_CLASS_CFG, GPS_UBX_CFG_TP5,
        0x20, 0x00,  /* Length = 32 */
        0x00,        /* tpIdx = 0 (TIMEPULSE) */
        0x01,        /* version = 1 */
        0x00, 0x00,  /* reserved */
        0x32, 0x00,  /* antCableDelay */
        0x00, 0x00,  /* rfGroupDelay */
        (uint8)(freqHz & 0xFF), (uint8)((freqHz >> 8) & 0xFF),
        (uint8)((freqHz >> 16) & 0xFF), (uint8)((freqHz >> 24) & 0xFF),  /* freqPeriod */
        (uint8)(freqHz & 0xFF), (uint8)((freqHz >> 8) & 0xFF),
        (uint8)((freqHz >> 16) & 0xFF), (uint8)((freqHz >> 24) & 0xFF),  /* freqPeriodLock */
        0x00, 0x00, 0x00, 0x00,  /* pulseLenRatio */
        0x00, 0x00, 0x00, 0x00,  /* pulseLenRatioLock */
        0x00, 0x00, 0x00, 0x00,  /* userConfigDelay */
        (enable ? 0x77 : 0x00), 0x00, 0x00, 0x00  /* flags */
    };
    
    return GPS_SendUBXCommand(ppsConfig, sizeof(ppsConfig));
}

/**
 * @brief Set power management mode
 */
Std_ReturnType GPS_SetPowerMode(uint8 mode) {
    uint8 powerConfig[] = {
        GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,
        GPS_UBX_CLASS_CFG, GPS_UBX_CFG_PM2,
        0x30, 0x00,  /* Length = 48 */
        0x01,        /* version */
        0x00, 0x00, 0x00,  /* reserved */
        mode,        /* maxStartupStateDur */
        0x00, 0x00, 0x00,  /* reserved */
        0x00, 0x00, 0x00, 0x00,  /* flags */
        0x10, 0x27, 0x00, 0x00,  /* updatePeriod */
        0x10, 0x27, 0x00, 0x00,  /* searchPeriod */
        0x00, 0x00, 0x00, 0x00,  /* gridOffset */
        0x00, 0x00,  /* onTime */
        0x00, 0x00,  /* minAcqTime */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00  /* reserved */
    };
    
    return GPS_SendUBXCommand(powerConfig, sizeof(powerConfig));
}

/**
 * @brief Configure geofence
 */
Std_ReturnType GPS_ConfigureGeofence(uint8 fenceId, const GPS_GeofenceType* fence) {
    sint32 lat, lon;
    
    if (fenceId >= GPS_MAX_GEOFENCES || fence == NULL_PTR) {
        return E_NOT_OK;
    }
    
    /* Convert to 1e-7 degrees */
    lat = (sint32)(fence->latitude * 10000000.0f);
    lon = (sint32)(fence->longitude * 10000000.0f);
    
    uint8 geofenceConfig[] = {
        GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,
        GPS_UBX_CLASS_CFG, GPS_UBX_CFG_GEOFENCE,
        0x18, 0x00,  /* Length = 24 */
        0x00,        /* version */
        0x01,        /* numFences */
        0x00,        /* confLvl */
        0x00,        /* reserved */
        0x01,        /* pioEnabled */
        0x00,        /* pinPolarity */
        0x00,        /* pin */
        0x00,        /* reserved */
        (uint8)(lat & 0xFF), (uint8)((lat >> 8) & 0xFF),
        (uint8)((lat >> 16) & 0xFF), (uint8)((lat >> 24) & 0xFF),
        (uint8)(lon & 0xFF), (uint8)((lon >> 8) & 0xFF),
        (uint8)((lon >> 16) & 0xFF), (uint8)((lon >> 24) & 0xFF),
        (uint8)(fence->radius & 0xFF), (uint8)((fence->radius >> 8) & 0xFF),
        (uint8)((fence->radius >> 16) & 0xFF), (uint8)((fence->radius >> 24) & 0xFF)
    };
    
    return GPS_SendUBXCommand(geofenceConfig, sizeof(geofenceConfig));
}

/**
 * @brief Enable RTCM input for D-GPS
 */
Std_ReturnType GPS_EnableRTCM(boolean enable) {
    /* Configure UART to accept RTCM messages */
    uint8 rtcmConfig[] = {
        GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,
        GPS_UBX_CLASS_CFG, GPS_UBX_CFG_PRT,
        0x14, 0x00,  /* Length = 20 */
        0x01,        /* portID = UART */
        0x00,        /* reserved */
        0x00, 0x00,  /* txReady */
        0xD0, 0x08, 0x00, 0x00,  /* mode: 8N1 */
        0x80, 0x25, 0x00, 0x00,  /* baudRate: 9600 */
        (enable ? 0x07 : 0x01), 0x00,  /* inProtoMask: UBX+NMEA(+RTCM) */
        0x01, 0x00,  /* outProtoMask: UBX */
        0x00, 0x00,  /* flags */
        0x00, 0x00   /* reserved */
    };
    
    return GPS_SendUBXCommand(rtcmConfig, sizeof(rtcmConfig));
}

/**
 * @brief Send AssistNow aiding data
 */
Std_ReturnType GPS_SendAssistNowData(uint8 mode, const uint8* data, uint16 length) {
    uint16 i;
    
    if (data == NULL_PTR || length == 0) {
        return E_NOT_OK;
    }
    
    /* Send aiding data directly to GPS */
    for (i = 0; i < length; i++) {
        Uart_SendByte(GPS_Config.UartModule, data[i]);
    }
    
    return E_OK;
}

/**
 * @brief Save current configuration to flash
 * @details Sends UBX-CFG-CFG command to save all settings to non-volatile memory.
 *          Waits for ACK to confirm save was successful.
 * @return E_OK if configuration saved successfully, E_NOT_OK otherwise
 */
Std_ReturnType GPS_SaveConfiguration(void) {
    const uint8 saveConfig[] = {
        GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,
        GPS_UBX_CLASS_CFG, GPS_UBX_CFG_CFG,
        0x0D, 0x00,  /* Length = 13 */
        0x00, 0x00, 0x00, 0x00,  /* clearMask */
        0xFF, 0xFF, 0x00, 0x00,  /* saveMask: all */
        0x00, 0x00, 0x00, 0x00,  /* loadMask */
        0x17  /* deviceMask: BBR, Flash, EEPROM, SPI Flash */
    };
    
    /* Send with ACK verification (2000ms timeout - save takes longer) */
    return GPS_SendUBXCommandWithAck(saveConfig, sizeof(saveConfig), 2000);
}

/**
 * @brief Load configuration from flash
 */
Std_ReturnType GPS_LoadConfiguration(void) {
    const uint8 loadConfig[] = {
        GPS_UBX_SYNC_CHAR_1, GPS_UBX_SYNC_CHAR_2,
        GPS_UBX_CLASS_CFG, GPS_UBX_CFG_CFG,
        0x0D, 0x00,  /* Length = 13 */
        0x00, 0x00, 0x00, 0x00,  /* clearMask */
        0x00, 0x00, 0x00, 0x00,  /* saveMask */
        0xFF, 0xFF, 0x00, 0x00,  /* loadMask: all */
        0x17  /* deviceMask: BBR, Flash, EEPROM, SPI Flash */
    };
    
    return GPS_SendUBXCommand(loadConfig, sizeof(loadConfig));
}

/**
 * @brief Get constellation usage information
 */
Std_ReturnType GPS_GetConstellationInfo(GPS_ConstellationInfoType* info) {
    uint8 i;
    
    if (!GPS_Initialized || info == NULL_PTR) {
        return E_NOT_OK;
    }
    
    /* Count satellites by constellation from satellite array */
    info->gpsUsed = 0;
    info->glonassUsed = 0;
    info->galileoUsed = 0;
    info->beidouUsed = 0;
    info->sbasUsed = 0;
    
    for (i = 0; i < GPS_CurrentData.satellitesUsed && i < GPS_MAX_SATELLITES; i++) {
        if (GPS_CurrentData.satellites[i].snr > 0) {  /* Only count satellites with signal */
            switch (GPS_CurrentData.satellites[i].gnssId) {
                case GPS_GNSS_GPS:
                    info->gpsUsed++;
                    break;
                case GPS_GNSS_GLONASS:
                    info->glonassUsed++;
                    break;
                case GPS_GNSS_GALILEO:
                    info->galileoUsed++;
                    break;
                case GPS_GNSS_BEIDOU:
                    info->beidouUsed++;
                    break;
                case GPS_GNSS_SBAS:
                    info->sbasUsed++;
                    break;
            }
        }
    }
    
    return E_OK;
}

/**
 * @brief Wait for UBX ACK/NAK response
 */
Std_ReturnType GPS_WaitForAck(uint8 msgClass, uint8 msgId, uint32 timeoutMs) {
    uint32 startTick = GPS_SystemTicks;
    uint32 timeoutTicks = timeoutMs;  /* Approximate */
    
    /* Reset ACK status */
    GPS_AckStatus.ackReceived = FALSE;
    GPS_AckStatus.nakReceived = FALSE;
    GPS_AckStatus.ackClass = msgClass;
    GPS_AckStatus.ackId = msgId;
    
    /* Wait for ACK or NAK */
    while ((GPS_SystemTicks - startTick) < timeoutTicks) {
        GPS_ProcessData();
        
        if (GPS_AckStatus.ackReceived) {
            return E_OK;
        }
        
        if (GPS_AckStatus.nakReceived) {
            return E_NOT_OK;
        }
    }
    
    return E_NOT_OK;  /* Timeout */
}

/**
 * @brief Get last NMEA sentence type processed
 */
const char* GPS_GetLastNmeaType(void) {
    return GPS_LastNmeaType;
}

/* ===================[Private Function Implementations]=================== */

/**
 * @brief Send UBX command
 */
static Std_ReturnType GPS_SendUBXCommand(const uint8* cmd, uint16 length) {
    uint8 ckA, ckB;
    uint16 i;
    
    if (!GPS_Initialized || cmd == NULL_PTR) {
        return E_NOT_OK;
    }
    
    /* Send command */
    for (i = 0; i < length; i++) {
        Uart_SendByte(GPS_Config.UartModule, cmd[i]);
    }
    
    /* Calculate and send checksum */
    GPS_CalculateUBXChecksum(cmd + 2, length - 2, &ckA, &ckB);
    Uart_SendByte(GPS_Config.UartModule, ckA);
    Uart_SendByte(GPS_Config.UartModule, ckB);
    
    return E_OK;
}

/**
 * @brief Calculate UBX checksum
 */
static void GPS_CalculateUBXChecksum(const uint8* data, uint16 length, uint8* ckA, uint8* ckB) {
    uint16 i;
    *ckA = 0;
    *ckB = 0;
    
    for (i = 0; i < length; i++) {
        *ckA += data[i];
        *ckB += *ckA;
    }
}

/**
 * @brief Split NMEA sentence into fields
 */
static uint8 GPS_SplitNmeaSentence(uint8* sentence, uint8* fields[], uint8 maxFields) {
    uint8 fieldCount = 0;
    uint8* ptr = sentence;
    
    fields[fieldCount++] = ptr;
    
    while (*ptr != '\0' && fieldCount < maxFields) {
        if (*ptr == ',' || *ptr == '*') {
            *ptr = '\0';
            if (fieldCount < maxFields) {
                fields[fieldCount++] = ptr + 1;
            }
        }
        ptr++;
    }
    
    return fieldCount;
}

/**
 * @brief Convert NMEA coordinate to decimal degrees
 */
static float32 GPS_NmeaToDecimal(uint8* nmeaCoord, uint8 direction) {
    float32 coord = 0.0f;
    uint8 degrees = 0;
    float32 minutes = 0.0f;
    uint8* dotPtr;
    uint8 degDigits;
    uint8 i;
    
    if (nmeaCoord[0] == '\0') return 0.0f;
    
    /* Find decimal point */
    dotPtr = nmeaCoord;
    while (*dotPtr != '.' && *dotPtr != '\0') dotPtr++;
    
    /* Calculate degrees (2 or 3 digits before minutes) */
    degDigits = (dotPtr - nmeaCoord) - 2;
    for (i = 0; i < degDigits; i++) {
        degrees = degrees * 10 + (nmeaCoord[i] - '0');
    }
    
    /* Parse minutes */
    minutes = GPS_ParseFloat(nmeaCoord + degDigits);
    coord = degrees + (minutes / 60.0f);
    
    /* Apply direction */
    if (direction == 'S' || direction == 'W') {
        coord = -coord;
    }
    
    return coord;
}

/**
 * @brief Parse float from string
 */
static float32 GPS_ParseFloat(uint8* str) {
    return (float32)atof((char*)str);
}

/**
 * @brief Parse integer from string
 */
static uint32 GPS_ParseInt(uint8* str) {
    return (uint32)atoi((char*)str);
}

/**
 * @brief Compare two strings
 */
static boolean GPS_StringCompare(const uint8* str1, const uint8* str2) {
    return (strcmp((char*)str1, (char*)str2) == 0);
}

/**
 * @brief Parse GGA sentence
 */
static void GPS_ParseGGA(uint8* fields[], uint8 fieldCount) {
    if (fieldCount < 10) return;
    
    /* Parse time (hhmmss.sss format) */
    if (fields[1][0] != '\0') {
        /* Parse as float to preserve decimal seconds, then extract components */
        float32 timeFloat = GPS_ParseFloat(fields[1]);
        uint32 timeInt = (uint32)timeFloat;
        GPS_CurrentData.time.hour = (timeInt / 10000) % 100;
        GPS_CurrentData.time.minute = (timeInt / 100) % 100;
        GPS_CurrentData.time.second = timeInt % 100;
    }
    
    /* Parse latitude */
    if (fields[2][0] != '\0' && fields[3][0] != '\0') {
        GPS_CurrentData.position.latitude = GPS_NmeaToDecimal(fields[2], fields[3][0]);
    }
    
    /* Parse longitude */
    if (fields[4][0] != '\0' && fields[5][0] != '\0') {
        GPS_CurrentData.position.longitude = GPS_NmeaToDecimal(fields[4], fields[5][0]);
    }
    
    /* Parse fix quality */
    GPS_CurrentData.fixQuality = GPS_ParseInt(fields[6]);
    GPS_CurrentData.validFix = (GPS_CurrentData.fixQuality > 0);
    
    /* Parse number of satellites */
    GPS_CurrentData.satellitesUsed = GPS_ParseInt(fields[7]);
    
    /* Parse HDOP */
    if (fields[8][0] != '\0') {
        GPS_CurrentData.dop.hdop = GPS_ParseFloat(fields[8]);
    }
    
    /* Parse altitude */
    if (fields[9][0] != '\0') {
        GPS_CurrentData.position.altitude = GPS_ParseFloat(fields[9]);
    }
    
    /* Parse geoid height */
    if (fieldCount > 11 && fields[11][0] != '\0') {
        GPS_CurrentData.position.geoidHeight = GPS_ParseFloat(fields[11]);
    }
    
    GPS_CurrentData.dataUpdated = TRUE;
}

/**
 * @brief Parse RMC sentence
 */
static void GPS_ParseRMC(uint8* fields[], uint8 fieldCount) {
    if (fieldCount < 10) return;
    
    /* Parse time (hhmmss.sss format) */
    if (fields[1][0] != '\0') {
        /* Parse as float to preserve decimal seconds, then extract components */
        float32 timeFloat = GPS_ParseFloat(fields[1]);
        uint32 timeInt = (uint32)timeFloat;
        GPS_CurrentData.time.hour = (timeInt / 10000) % 100;
        GPS_CurrentData.time.minute = (timeInt / 100) % 100;
        GPS_CurrentData.time.second = timeInt % 100;
    }
    
    /* Parse status */
    GPS_CurrentData.validFix = (fields[2][0] == 'A');
    
    /* Parse latitude */
    if (fields[3][0] != '\0' && fields[4][0] != '\0') {
        GPS_CurrentData.position.latitude = GPS_NmeaToDecimal(fields[3], fields[4][0]);
    }
    
    /* Parse longitude */
    if (fields[5][0] != '\0' && fields[6][0] != '\0') {
        GPS_CurrentData.position.longitude = GPS_NmeaToDecimal(fields[5], fields[6][0]);
    }
    
    /* Parse speed (knots) */
    if (fields[7][0] != '\0') {
        GPS_CurrentData.velocity.speedKnots = GPS_ParseFloat(fields[7]);
        GPS_CurrentData.velocity.speedKmh = GPS_CurrentData.velocity.speedKnots * 1.852f;
    }
    
    /* Parse course */
    if (fields[8][0] != '\0') {
        GPS_CurrentData.velocity.course = GPS_ParseFloat(fields[8]);
    }
    
    /* Parse date */
    if (fields[9][0] != '\0') {
        uint32 date = GPS_ParseInt(fields[9]);
        GPS_CurrentData.time.day = (date / 10000) % 100;
        GPS_CurrentData.time.month = (date / 100) % 100;
        GPS_CurrentData.time.year = 2000 + (date % 100);
    }
    
    GPS_CurrentData.dataUpdated = TRUE;
}

/**
 * @brief Parse GSA sentence
 */
static void GPS_ParseGSA(uint8* fields[], uint8 fieldCount) {
    uint8 i, prn, satCount = 0;
    
    if (fieldCount < 18) return;
    
    /* Clear satellite array before parsing */
    for (i = 0; i < GPS_MAX_SATELLITES; i++) {
        GPS_CurrentData.satellites[i].prn = 0;
        GPS_CurrentData.satellites[i].snr = 0;
        GPS_CurrentData.satellites[i].gnssId = 0;
    }
    
    /* Parse satellite PRNs used in solution (fields 3-14) */
    for (i = 3; i <= 14 && i < fieldCount; i++) {
        if (fields[i][0] != '\0' && fields[i][0] != '0') {
            prn = GPS_ParseInt(fields[i]);
            if (prn > 0 && satCount < GPS_MAX_SATELLITES) {
                /* Store PRN and determine constellation */
                GPS_CurrentData.satellites[satCount].prn = prn;
                GPS_CurrentData.satellites[satCount].snr = 1;  /* Mark as used */
                
                if (prn >= 1 && prn <= 32) {
                    GPS_CurrentData.satellites[satCount].gnssId = GPS_GNSS_GPS;
                } else if (prn >= 65 && prn <= 96) {
                    GPS_CurrentData.satellites[satCount].gnssId = GPS_GNSS_GLONASS;
                } else if (prn >= 301 && prn <= 336) {
                    GPS_CurrentData.satellites[satCount].gnssId = GPS_GNSS_GALILEO;
                } else if (prn >= 401 && prn <= 437) {
                    GPS_CurrentData.satellites[satCount].gnssId = GPS_GNSS_BEIDOU;
                } else if (prn >= 120 && prn <= 158) {
                    GPS_CurrentData.satellites[satCount].gnssId = GPS_GNSS_SBAS;
                }
                
                satCount++;
            }
        }
    }
    
    /* Fallback: If GSV not working, estimate in-view as used + 3-5 */
    if (GPS_CurrentData.satellitesInView == 0 && satCount > 0) {
        GPS_CurrentData.satellitesInView = satCount + 3;  /* Estimate */
    }
    
    /* Parse PDOP */
    if (fields[15][0] != '\0') {
        GPS_CurrentData.dop.pdop = GPS_ParseFloat(fields[15]);
    }
    
    /* Parse HDOP */
    if (fields[16][0] != '\0') {
        GPS_CurrentData.dop.hdop = GPS_ParseFloat(fields[16]);
    }
    
    /* Parse VDOP */
    if (fields[17][0] != '\0') {
        GPS_CurrentData.dop.vdop = GPS_ParseFloat(fields[17]);
    }
}

/**
 * @brief Parse GSV sentence
 */
static void GPS_ParseGSV(uint8* fields[], uint8 fieldCount) {
    uint8 totalSats, satIndex, i, prn;
    
    if (fieldCount < 4) return;
    
    /* Parse total satellites in view (field 3) */
    /* This field contains the same total count in all GSV messages */
    totalSats = GPS_ParseInt(fields[3]);
    if (totalSats > 0) {
        GPS_CurrentData.satellitesInView = totalSats;
    }
    
    /* Parse satellite info (up to 4 per sentence) */
    for (i = 0; i < 4 && (4 + i * 4) < fieldCount; i++) {
        /* Message number is 1-indexed, so subtract 1 */
        uint8 msgNum = GPS_ParseInt(fields[2]);
        if (msgNum > 0) {
            satIndex = (msgNum - 1) * 4 + i;
        } else {
            satIndex = i;
        }
        if (satIndex < GPS_MAX_SATELLITES) {
            prn = GPS_ParseInt(fields[4 + i * 4]);
            GPS_CurrentData.satellites[satIndex].prn = prn;
            GPS_CurrentData.satellites[satIndex].elevation = GPS_ParseInt(fields[5 + i * 4]);
            GPS_CurrentData.satellites[satIndex].azimuth = GPS_ParseInt(fields[6 + i * 4]);
            GPS_CurrentData.satellites[satIndex].snr = GPS_ParseInt(fields[7 + i * 4]);
            
            /* Determine constellation based on PRN range */
            if (prn >= 1 && prn <= 32) {
                GPS_CurrentData.satellites[satIndex].gnssId = GPS_GNSS_GPS;
            } else if (prn >= 65 && prn <= 96) {
                GPS_CurrentData.satellites[satIndex].gnssId = GPS_GNSS_GLONASS;
            } else if (prn >= 301 && prn <= 336) {
                GPS_CurrentData.satellites[satIndex].gnssId = GPS_GNSS_GALILEO;
            } else if (prn >= 401 && prn <= 437) {
                GPS_CurrentData.satellites[satIndex].gnssId = GPS_GNSS_BEIDOU;
            }
        }
    }
}

/**
 * @brief Parse NMEA sentence
 */
static void GPS_ParseNmeaSentence(uint8* sentence) {
    uint8* fields[GPS_MAX_NMEA_FIELDS];
    uint8 fieldCount;
    uint8* sentenceType;
    
    /* Skip $ and get sentence type */
    if (sentence[0] != '$') return;
    
    /* Split sentence into fields */
    fieldCount = GPS_SplitNmeaSentence(sentence + 1, fields, GPS_MAX_NMEA_FIELDS);
    
    if (fieldCount == 0) return;
    
    /* Get sentence type (skip talker ID) */
    sentenceType = fields[0] + 2;  /* Skip GP, GL, GN, etc. */
    
    /* Parse based on sentence type */
    if (GPS_StringCompare(sentenceType, (uint8*)GPS_NMEA_GGA)) {
        GPS_LastNmeaType = GPS_NMEA_GGA;
        GPS_ParseGGA(fields, fieldCount);
    } else if (GPS_StringCompare(sentenceType, (uint8*)GPS_NMEA_RMC)) {
        GPS_LastNmeaType = GPS_NMEA_RMC;
        GPS_ParseRMC(fields, fieldCount);
    } else if (GPS_StringCompare(sentenceType, (uint8*)GPS_NMEA_GSA)) {
        GPS_LastNmeaType = GPS_NMEA_GSA;
        GPS_ParseGSA(fields, fieldCount);
    } else if (GPS_StringCompare(sentenceType, (uint8*)GPS_NMEA_GSV)) {
        GPS_LastNmeaType = GPS_NMEA_GSV;
        GPS_ParseGSV(fields, fieldCount);
    }
}

/**
 * @brief Process UBX message
 */
static void GPS_ProcessUbxMessage(const uint8* msg, uint16 length) {
    uint8 msgClass, msgId;
    uint16 payloadLength;
    
    if (length < 8) return;  /* Minimum UBX message size */
    
    msgClass = msg[2];
    msgId = msg[3];
    payloadLength = msg[4] | (msg[5] << 8);
    
    /* Handle ACK/NAK messages */
    if (msgClass == GPS_UBX_CLASS_ACK) {
        if (msgId == GPS_UBX_ACK_ACK) {
            GPS_HandleAckNak(msg + 6, payloadLength, TRUE);
        } else if (msgId == GPS_UBX_ACK_NAK) {
            GPS_HandleAckNak(msg + 6, payloadLength, FALSE);
        }
    }
    
    /* Add more UBX message handlers here as needed */
}

/**
 * @brief Handle UBX ACK/NAK message
 */
static void GPS_HandleAckNak(const uint8* payload, uint16 length, boolean isAck) {
    if (length < 2) return;
    
    uint8 ackClass = payload[0];
    uint8 ackId = payload[1];
    
    /* Check if this ACK/NAK is for the message we're waiting for */
    if (ackClass == GPS_AckStatus.ackClass && ackId == GPS_AckStatus.ackId) {
        if (isAck) {
            GPS_AckStatus.ackReceived = TRUE;
        } else {
            GPS_AckStatus.nakReceived = TRUE;
        }
        GPS_AckStatus.timestamp = GPS_SystemTicks;
    }
}

/**
 * @brief Send UBX command and wait for ACK/NAK response
 * @details Sends UBX command with checksum and waits for ACK-ACK or ACK-NAK.
 *          Returns E_OK only if ACK-ACK received within timeout.
 * @param cmd UBX command buffer (including sync chars, class, ID, length, payload)
 * @param length Total command length
 * @param timeoutMs Timeout in milliseconds to wait for ACK
 * @return E_OK if ACK received, E_NOT_OK if NAK or timeout
 */
static Std_ReturnType GPS_SendUBXCommandWithAck(const uint8* cmd, uint16 length, uint32 timeoutMs) {
    uint8 msgClass, msgId;
    Std_ReturnType result;
    uint8 retryCount;
    const uint8 MAX_RETRIES = 1;  /* One retry on timeout */
    volatile uint32 i;
    
    retryCount = 0;
    
    if (cmd == NULL_PTR || length < 4) {
        return E_NOT_OK;
    }
    
    /* Extract message class and ID from command */
    msgClass = cmd[2];
    msgId = cmd[3];
    
    /* Attempt send with optional retry */
    do {
        /* Send command */
        if (GPS_SendUBXCommand(cmd, length) != E_OK) {
            return E_NOT_OK;
        }
        
        /* Wait for ACK/NAK */
        result = GPS_WaitForAck(msgClass, msgId, timeoutMs);
        
        if (result == E_OK) {
            return E_OK;  /* ACK received */
        }
        
        /* On timeout or NAK, retry once */
        retryCount++;
        if (retryCount <= MAX_RETRIES) {
            /* Small delay before retry */
            for (i = 0; i < 10000; i++);
        }
        
    } while (retryCount <= MAX_RETRIES && result != E_OK);
    
    return E_NOT_OK;  /* Failed after retries */
}
