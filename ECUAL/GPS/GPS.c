/**
 * @file GPS.c
 * @brief GPS Driver Implementation for NEO-M8M
 * @details Complete implementation with NMEA parsing and UBX protocol
 *
 * @author Mohamed Yasser
 * @date Nov 7, 2025
 * @version 1.0.0
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

/* ===================[Public Function Implementations]=================== */

/**
 * @brief Initialize GPS driver
 */
Std_ReturnType GPS_Init(const GPS_ConfigType* ConfigPtr) {
    if (ConfigPtr == NULL_PTR) {
        return E_NOT_OK;
    }
    
    /* Store configuration */
    GPS_Config = *ConfigPtr;
    
    /* Initialize GPS data structure */
    memset(&GPS_CurrentData, 0, sizeof(GPS_DataType));
    
    /* Reset buffer */
    GPS_BufferIndex = 0;
    
    /* Configure GPS module */
    if (GPS_Config.EnableSBAS) {
        GPS_EnableSBAS();
    }
    
    if (GPS_Config.EnableGLONASS) {
        GPS_EnableGLONASS();
    }
    
    if (GPS_Config.UpdateRate != GPS_DEFAULT_UPDATE_RATE) {
        GPS_SetUpdateRate(GPS_Config.UpdateRate);
    }
    
    GPS_SetDynamicModel(GPS_Config.DynamicModel);
    
    GPS_Initialized = TRUE;
    
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
 * @brief Process incoming GPS data
 */
Std_ReturnType GPS_ProcessData(void) {
    uint8 receivedByte;
    
    if (!GPS_Initialized) {
        return E_NOT_OK;
    }
    
    /* Read available data from UART */
    while (Uart_ReceiveByte(GPS_Config.UartModule, &receivedByte) == E_OK) {
        /* Check for start of NMEA sentence */
        if (receivedByte == '$') {
            GPS_BufferIndex = 0;
        }
        
        /* Store byte in buffer */
        if (GPS_BufferIndex < GPS_NMEA_BUFFER_SIZE - 1) {
            GPS_NmeaBuffer[GPS_BufferIndex++] = receivedByte;
            
            /* Check for end of sentence */
            if (receivedByte == '\n') {
                GPS_NmeaBuffer[GPS_BufferIndex] = '\0';
                
                /* Parse the sentence */
                GPS_ParseNmeaSentence(GPS_NmeaBuffer);
                
                GPS_BufferIndex = 0;
            }
        } else {
            /* Buffer overflow, reset */
            GPS_BufferIndex = 0;
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
    
    /* Parse time */
    if (fields[1][0] != '\0') {
        uint32 time = GPS_ParseInt(fields[1]);
        GPS_CurrentData.time.hour = (time / 10000) % 100;
        GPS_CurrentData.time.minute = (time / 100) % 100;
        GPS_CurrentData.time.second = time % 100;
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
    
    /* Parse time */
    if (fields[1][0] != '\0') {
        uint32 time = GPS_ParseInt(fields[1]);
        GPS_CurrentData.time.hour = (time / 10000) % 100;
        GPS_CurrentData.time.minute = (time / 100) % 100;
        GPS_CurrentData.time.second = time % 100;
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
    if (fieldCount < 18) return;
    
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
    uint8 totalSats, satIndex, i;
    
    if (fieldCount < 4) return;
    
    /* Parse total satellites in view */
    totalSats = GPS_ParseInt(fields[3]);
    GPS_CurrentData.satellitesInView = totalSats;
    
    /* Parse satellite info (up to 4 per sentence) */
    for (i = 0; i < 4 && (4 + i * 4) < fieldCount; i++) {
        satIndex = GPS_ParseInt(fields[2]) * 4 + i;
        if (satIndex < GPS_MAX_SATELLITES) {
            GPS_CurrentData.satellites[satIndex].prn = GPS_ParseInt(fields[4 + i * 4]);
            GPS_CurrentData.satellites[satIndex].elevation = GPS_ParseInt(fields[5 + i * 4]);
            GPS_CurrentData.satellites[satIndex].azimuth = GPS_ParseInt(fields[6 + i * 4]);
            GPS_CurrentData.satellites[satIndex].snr = GPS_ParseInt(fields[7 + i * 4]);
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
        GPS_ParseGGA(fields, fieldCount);
    } else if (GPS_StringCompare(sentenceType, (uint8*)GPS_NMEA_RMC)) {
        GPS_ParseRMC(fields, fieldCount);
    } else if (GPS_StringCompare(sentenceType, (uint8*)GPS_NMEA_GSA)) {
        GPS_ParseGSA(fields, fieldCount);
    } else if (GPS_StringCompare(sentenceType, (uint8*)GPS_NMEA_GSV)) {
        GPS_ParseGSV(fields, fieldCount);
    }
}
