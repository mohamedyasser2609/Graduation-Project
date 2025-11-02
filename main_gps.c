/*
 * ============================================================================
 * THIS FILE IS CURRENTLY COMMENTED OUT - ROS 2 VERSION IS ACTIVE (main_ros2.c)
 * To use this GPS reader version, uncomment this file and comment main_ros2.c
 * ============================================================================
 */

#if 0  /* <<<< ENTIRE GPS MAIN COMMENTED OUT - REMOVE THIS LINE TO ACTIVATE >>>> */

/**
 * @file main_gps.c
 * @brief GPS NEO-M8M Data Reader for TM4C123GH6PM
 * @details Reads NMEA sentences from GPS module and displays on serial monitor
 *
 * Hardware Connections:
 * - GPS TX → TM4C RX (UART1: PB0 or UART2: PD6)
 * - GPS RX → TM4C TX (UART1: PB1 or UART2: PD7)
 * - GPS VCC → 3.3V
 * - GPS GND → GND
 *
 * Debug Output:
 * - UART0 (PA0/PA1) → USB Serial to PC terminal at 115200 baud
 *
 * Features:
 * - Reads raw NMEA sentences from GPS
 * - Parses GGA (position), RMC (recommended minimum), GSA (satellites)
 * - Displays formatted GPS data on terminal
 * - Shows satellite count, fix status, coordinates, speed, time
 *
 * @author Mohamed Yasser
 * @date Oct 28, 2025
 * @version 1.0.0
 */

/* ===================[Includes]=================== */
#include "MCAL/GPIO/Gpio.h"
#include "MCAL/UART/Uart.h"
#include "CONFIG/Std_Types.h"

/* ===================[Type Definitions]=================== */
typedef signed long int32;

/* ===================[Definitions]=================== */
#define GPS_UART_MODULE         UART_MODULE_1   /* GPS connected to UART1 (PB0/PB1) */
#define DEBUG_UART_MODULE       UART_MODULE_0   /* Debug output to PC via USB */
#define GPS_BAUD_RATE           UART_BAUD_9600  /* NEO-M8M default baud rate */
#define DEBUG_BAUD_RATE         UART_BAUD_115200

#define NMEA_BUFFER_SIZE        128
#define MAX_NMEA_FIELDS         20

/* ===================[External Configurations]=================== */
extern const Gpio_ConfigType Gpio_Configuration;

/* ===================[Global Variables]=================== */
static uint8 nmeaBuffer[NMEA_BUFFER_SIZE];
static uint8 bufferIndex = 0;
static boolean newSentenceReady = FALSE;

/* GPS Data Structure */
typedef struct {
    uint8 fixQuality;           /* 0=No fix, 1=GPS fix, 2=DGPS fix */
    uint8 satelliteCount;       /* Number of satellites in use */
    uint8 satellitesInView;     /* Number of satellites visible */
    float latitude;             /* Latitude in decimal degrees */
    float longitude;            /* Longitude in decimal degrees */
    float altitude;             /* Altitude in meters */
    float speed;                /* Speed in knots */
    uint8 hour;                 /* UTC hour */
    uint8 minute;               /* UTC minute */
    uint8 second;               /* UTC second */
    boolean validData;          /* Data validity flag */
} GpsData_t;

static GpsData_t gpsData = {0};

/* ===================[Interrupt Handlers]=================== */

/**
 * @brief Timer2A interrupt handler (dummy - not used in GPS application)
 * @note Required by startup file even if not used
 */
void Timer2A_Handler(void) {
    /* Not used in GPS application */
    /* If you need timer functionality, implement here */
}

/* ===================[Helper Functions]=================== */

/**
 * @brief Send string to debug UART
 */
void Debug_Print(const uint8* str) {
    Uart_SendString(DEBUG_UART_MODULE, str);
}

/**
 * @brief Enable SBAS (WAAS/EGNOS) for improved accuracy
 * @note This can improve accuracy from 3-5m to 1-2m
 */
void Gps_EnableSBAS(void) {
    /* UBX-CFG-SBAS command to enable SBAS */
    const uint8 enableSBAS[] = {
        0xB5, 0x62,       /* UBX header */
        0x06, 0x16,       /* CFG-SBAS */
        0x08, 0x00,       /* Length = 8 bytes */
        0x01,             /* Enable SBAS */
        0x07,             /* Use for ranging, correction, integrity */
        0x03,             /* Max 3 SBAS channels */
        0x00,             /* Scanmode2 */
        0x00, 0x00, 0x00, 0x00  /* Scanmode (auto) */
    };
    
    /* Calculate checksum */
    uint8 CK_A = 0, CK_B = 0;
    uint8 i;
    for (i = 2; i < sizeof(enableSBAS); i++) {
        CK_A += enableSBAS[i];
        CK_B += CK_A;
    }
    
    /* Send command */
    Uart_SendBuffer(GPS_UART_MODULE, enableSBAS, sizeof(enableSBAS));
    Uart_SendByte(GPS_UART_MODULE, CK_A);
    Uart_SendByte(GPS_UART_MODULE, CK_B);
    
    Debug_Print((const uint8*)"SBAS/WAAS enabled for improved accuracy\r\n");
}

/**
 * @brief Enable GLONASS constellation for more satellites
 * @note NEO-M8M supports GPS + GLONASS simultaneously
 */
void Gps_EnableGLONASS(void) {
    /* UBX-CFG-GNSS command to enable GPS + GLONASS */
    const uint8 enableGLONASS[] = {
        0xB5, 0x62,       /* UBX header */
        0x06, 0x3E,       /* CFG-GNSS */
        0x3C, 0x00,       /* Length = 60 bytes */
        0x00,             /* msgVer = 0 */
        0x00,             /* numTrkChHw (read-only) */
        0xFF,             /* numTrkChUse = 255 (use all) */
        0x05,             /* numConfigBlocks = 5 */
        
        /* GPS Block */
        0x00,             /* gnssId = GPS */
        0x08,             /* resTrkCh = 8 channels min */
        0x10,             /* maxTrkCh = 16 channels max */
        0x00,             /* reserved1 */
        0x01, 0x00, 0x01, 0x01,  /* flags: enable + L1C/A */
        
        /* SBAS Block */
        0x01,             /* gnssId = SBAS */
        0x01,             /* resTrkCh = 1 */
        0x03,             /* maxTrkCh = 3 */
        0x00,             /* reserved1 */
        0x01, 0x00, 0x01, 0x01,  /* flags: enable */
        
        /* Galileo Block (disable - not supported by M8M) */
        0x02,             /* gnssId = Galileo */
        0x00,             /* resTrkCh = 0 */
        0x00,             /* maxTrkCh = 0 */
        0x00,             /* reserved1 */
        0x00, 0x00, 0x01, 0x01,  /* flags: disable */
        
        /* BeiDou Block (disable - not supported by M8M) */
        0x03,             /* gnssId = BeiDou */
        0x00,             /* resTrkCh = 0 */
        0x00,             /* maxTrkCh = 0 */
        0x00,             /* reserved1 */
        0x00, 0x00, 0x01, 0x01,  /* flags: disable */
        
        /* GLONASS Block - ENABLE THIS! */
        0x06,             /* gnssId = GLONASS */
        0x08,             /* resTrkCh = 8 channels min */
        0x0E,             /* maxTrkCh = 14 channels max */
        0x00,             /* reserved1 */
        0x01, 0x00, 0x01, 0x01   /* flags: enable + L1OF */
    };
    
    /* Calculate checksum */
    uint8 CK_A = 0, CK_B = 0;
    uint8 i;
    for (i = 2; i < sizeof(enableGLONASS); i++) {
        CK_A += enableGLONASS[i];
        CK_B += CK_A;
    }
    
    /* Send command */
    Uart_SendBuffer(GPS_UART_MODULE, enableGLONASS, sizeof(enableGLONASS));
    Uart_SendByte(GPS_UART_MODULE, CK_A);
    Uart_SendByte(GPS_UART_MODULE, CK_B);
    
    Debug_Print((const uint8*)"GLONASS enabled (GPS + GLONASS dual constellation)\r\n");
}

/**
 * @brief Convert uint32 to string
 */
void Uint32ToString(uint32 value, uint8* buffer) {
    uint8 i = 0;
    uint8 temp[12];
    
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    while (value > 0) {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }
    
    uint8 j;
    for (j = 0; j < i; j++) {
        buffer[j] = temp[i - j - 1];
    }
    buffer[j] = '\0';
}

/**
 * @brief Convert float to string (simple version)
 */
void FloatToString(float value, uint8* buffer, uint8 decimals) {
    int32 intPart = (int32)value;
    float fracPart = value - intPart;
    uint8 i;
    
    if (fracPart < 0) fracPart = -fracPart;
    
    uint8 temp[20];
    Uint32ToString((uint32)(intPart < 0 ? -intPart : intPart), temp);
    
    uint8 idx = 0;
    if (intPart < 0) buffer[idx++] = '-';
    
    i = 0;
    while (temp[i] != '\0') {
        buffer[idx++] = temp[i++];
    }
    
    buffer[idx++] = '.';
    
    for (i = 0; i < decimals; i++) {
        fracPart *= 10;
        buffer[idx++] = '0' + (uint8)fracPart;
        fracPart -= (uint8)fracPart;
    }
    
    buffer[idx] = '\0';
}

/**
 * @brief Compare two strings
 */
boolean StringCompare(const uint8* str1, const uint8* str2) {
    while (*str1 != '\0' && *str2 != '\0') {
        if (*str1 != *str2) return FALSE;
        str1++;
        str2++;
    }
    return (*str1 == *str2);
}

/**
 * @brief Split NMEA sentence into fields
 */
uint8 SplitNmeaSentence(uint8* sentence, uint8* fields[], uint8 maxFields) {
    uint8 fieldCount = 0;
    uint8* ptr = sentence;
    
    fields[fieldCount++] = ptr;
    
    while (*ptr != '\0' && fieldCount < maxFields) {
        if (*ptr == ',' || *ptr == '*') {
            *ptr = '\0';
            fields[fieldCount++] = ptr + 1;
        }
        ptr++;
    }
    
    return fieldCount;
}

/**
 * @brief Convert NMEA coordinate to decimal degrees
 * @param nmeaCoord NMEA format: DDMM.MMMM or DDDMM.MMMM
 * @param direction N/S/E/W
 */
float NmeaToDecimal(uint8* nmeaCoord, uint8 direction) {
    float coord = 0.0f;
    uint8 degrees = 0;
    float minutes = 0.0f;
    uint8* dotPtr;
    uint8 degDigits;
    uint8 minStart;
    uint8 i;
    float divisor;
    
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
    minStart = degDigits;
    i = minStart;
    while (nmeaCoord[i] != '\0') {
        if (nmeaCoord[i] >= '0' && nmeaCoord[i] <= '9') {
            minutes = minutes * 10 + (nmeaCoord[i] - '0');
        } else if (nmeaCoord[i] == '.') {
            i++;
            divisor = 10.0f;
            while (nmeaCoord[i] >= '0' && nmeaCoord[i] <= '9') {
                minutes += (nmeaCoord[i] - '0') / divisor;
                divisor *= 10.0f;
                i++;
            }
            break;
        }
        i++;
    }
    
    coord = degrees + (minutes / 60.0f);
    
    /* Apply direction */
    if (direction == 'S' || direction == 'W') {
        coord = -coord;
    }
    
    return coord;
}

/**
 * @brief Parse NMEA GGA sentence (Global Positioning System Fix Data)
 * Format: $GPGGA,time,lat,N/S,lon,E/W,quality,numSV,HDOP,alt,M,...
 */
void ParseGGA(uint8* fields[], uint8 fieldCount) {
    if (fieldCount < 10) return;
    
    /* Parse time (hhmmss.sss) */
    if (fields[1][0] != '\0') {
        gpsData.hour = (fields[1][0] - '0') * 10 + (fields[1][1] - '0');
        gpsData.minute = (fields[1][2] - '0') * 10 + (fields[1][3] - '0');
        gpsData.second = (fields[1][4] - '0') * 10 + (fields[1][5] - '0');
    }
    
    /* Parse latitude */
    gpsData.latitude = NmeaToDecimal(fields[2], fields[3][0]);
    
    /* Parse longitude */
    gpsData.longitude = NmeaToDecimal(fields[4], fields[5][0]);
    
    /* Parse fix quality */
    if (fields[6][0] != '\0') {
        gpsData.fixQuality = fields[6][0] - '0';
    }
    
    /* Parse satellite count */
    if (fields[7][0] != '\0') {
        gpsData.satelliteCount = (fields[7][0] - '0') * 10 + (fields[7][1] - '0');
    }
    
    /* Parse altitude */
    if (fields[9][0] != '\0') {
        gpsData.altitude = 0.0f;
        uint8 i = 0;
        while (fields[9][i] >= '0' && fields[9][i] <= '9') {
            gpsData.altitude = gpsData.altitude * 10 + (fields[9][i] - '0');
            i++;
        }
    }
    
    gpsData.validData = (gpsData.fixQuality > 0);
}

/**
 * @brief Parse NMEA RMC sentence (Recommended Minimum)
 * Format: $GPRMC,time,status,lat,N/S,lon,E/W,speed,course,date,...
 */
void ParseRMC(uint8* fields[], uint8 fieldCount) {
    if (fieldCount < 8) return;
    
    /* Parse speed in knots */
    if (fields[7][0] != '\0') {
        gpsData.speed = 0.0f;
        uint8 i = 0;
        while (fields[7][i] >= '0' && fields[7][i] <= '9') {
            gpsData.speed = gpsData.speed * 10 + (fields[7][i] - '0');
            i++;
        }
        if (fields[7][i] == '.') {
            i++;
            float divisor = 10.0f;
            while (fields[7][i] >= '0' && fields[7][i] <= '9') {
                gpsData.speed += (fields[7][i] - '0') / divisor;
                divisor *= 10.0f;
                i++;
            }
        }
    }
}

/**
 * @brief Parse NMEA GSV sentence (Satellites in View)
 * Format: $GPGSV,numMsg,msgNum,numSV,...
 */
void ParseGSV(uint8* fields[], uint8 fieldCount) {
    if (fieldCount < 4) return;
    
    /* Parse number of satellites in view (field 3) */
    if (fields[3][0] != '\0') {
        gpsData.satellitesInView = 0;
        uint8 i = 0;
        while (fields[3][i] >= '0' && fields[3][i] <= '9') {
            gpsData.satellitesInView = gpsData.satellitesInView * 10 + (fields[3][i] - '0');
            i++;
        }
    }
}

/**
 * @brief Process complete NMEA sentence
 */
void ProcessNmeaSentence(uint8* sentence) {
    uint8* fields[MAX_NMEA_FIELDS];
    uint8 fieldCount;
    
    /* Check for valid NMEA sentence */
    if (sentence[0] != '$') return;
    
    /* Split into fields */
    fieldCount = SplitNmeaSentence(sentence, fields, MAX_NMEA_FIELDS);
    
    /* Parse based on sentence type */
    /* Support GP (GPS), GL (GLONASS), GN (combined) prefixes */
    if (StringCompare(fields[0], (const uint8*)"$GPGGA") || 
        StringCompare(fields[0], (const uint8*)"$GLGGA") ||
        StringCompare(fields[0], (const uint8*)"$GNGGA")) {
        ParseGGA(fields, fieldCount);
    }
    else if (StringCompare(fields[0], (const uint8*)"$GPRMC") || 
             StringCompare(fields[0], (const uint8*)"$GLRMC") ||
             StringCompare(fields[0], (const uint8*)"$GNRMC")) {
        ParseRMC(fields, fieldCount);
    }
    else if (StringCompare(fields[0], (const uint8*)"$GPGSV") || 
             StringCompare(fields[0], (const uint8*)"$GLGSV") ||
             StringCompare(fields[0], (const uint8*)"$GNGSV")) {
        ParseGSV(fields, fieldCount);
    }
}

/**
 * @brief Display GPS data on terminal
 */
void DisplayGpsData(void) {
    uint8 buffer[32];
    
    Debug_Print((const uint8*)"\r\n========================================\r\n");
    Debug_Print((const uint8*)"GPS NEO-M8M Status\r\n");
    Debug_Print((const uint8*)"========================================\r\n");
    
    /* Fix Status */
    Debug_Print((const uint8*)"Fix Status: ");
    if (gpsData.fixQuality == 0) {
        Debug_Print((const uint8*)"NO FIX\r\n");
    } else if (gpsData.fixQuality == 1) {
        Debug_Print((const uint8*)"GPS FIX\r\n");
    } else if (gpsData.fixQuality == 2) {
        Debug_Print((const uint8*)"DGPS FIX\r\n");
    }
    
    /* Satellites */
    Debug_Print((const uint8*)"Satellites in use: ");
    Uint32ToString(gpsData.satelliteCount, buffer);
    Debug_Print(buffer);
    Debug_Print((const uint8*)" | In view: ");
    Uint32ToString(gpsData.satellitesInView, buffer);
    Debug_Print(buffer);
    Debug_Print((const uint8*)"\r\n");
    
    /* Time */
    Debug_Print((const uint8*)"UTC Time: ");
    Uint32ToString(gpsData.hour, buffer);
    if (gpsData.hour < 10) Debug_Print((const uint8*)"0");
    Debug_Print(buffer);
    Debug_Print((const uint8*)":");
    Uint32ToString(gpsData.minute, buffer);
    if (gpsData.minute < 10) Debug_Print((const uint8*)"0");
    Debug_Print(buffer);
    Debug_Print((const uint8*)":");
    Uint32ToString(gpsData.second, buffer);
    if (gpsData.second < 10) Debug_Print((const uint8*)"0");
    Debug_Print(buffer);
    Debug_Print((const uint8*)"\r\n");
    
    if (gpsData.validData) {
        /* Latitude */
        Debug_Print((const uint8*)"Latitude:  ");
        FloatToString(gpsData.latitude, buffer, 6);
        Debug_Print(buffer);
        Debug_Print((const uint8*)"°\r\n");
        
        /* Longitude */
        Debug_Print((const uint8*)"Longitude: ");
        FloatToString(gpsData.longitude, buffer, 6);
        Debug_Print(buffer);
        Debug_Print((const uint8*)"°\r\n");
        
        /* Altitude */
        Debug_Print((const uint8*)"Altitude:  ");
        FloatToString(gpsData.altitude, buffer, 1);
        Debug_Print(buffer);
        Debug_Print((const uint8*)" m\r\n");
        
        /* Speed */
        Debug_Print((const uint8*)"Speed:     ");
        FloatToString(gpsData.speed, buffer, 1);
        Debug_Print(buffer);
        Debug_Print((const uint8*)" knots\r\n");
    } else {
        Debug_Print((const uint8*)"\r\nWaiting for valid GPS fix...\r\n");
    }
    
    Debug_Print((const uint8*)"========================================\r\n");
}

/**
 * @brief Read GPS data byte by byte and build NMEA sentence
 */
void ReadGpsData(void) {
    uint8 receivedByte;
    Std_ReturnType status;
    
    while (Uart_IsRxDataAvailable(GPS_UART_MODULE)) {
        status = Uart_ReceiveByte(GPS_UART_MODULE, &receivedByte);
        if (status != E_OK) continue;
        
        /* Echo raw data to debug (optional - comment out for cleaner output) */
        // Uart_SendByte(DEBUG_UART_MODULE, receivedByte);
        
        if (receivedByte == '$') {
            /* Start of new sentence */
            bufferIndex = 0;
            nmeaBuffer[bufferIndex++] = receivedByte;
        }
        else if (receivedByte == '\n') {
            /* End of sentence */
            nmeaBuffer[bufferIndex] = '\0';
            newSentenceReady = TRUE;
        }
        else if (bufferIndex < NMEA_BUFFER_SIZE - 1) {
            /* Add to buffer */
            nmeaBuffer[bufferIndex++] = receivedByte;
        }
    }
}

/* ===================[Main Function]=================== */
int main(void) {
    uint32 updateCounter = 0;
    
    /* Step 1: Initialize GPIO */
    Gpio_Init(&Gpio_Configuration);
    
    /* Step 2: Configure Debug UART (UART0 to PC) */
    const Uart_ConfigType DebugUart_Config = {
        .Module = DEBUG_UART_MODULE,
        .BaudRate = DEBUG_BAUD_RATE,
        .DataBits = UART_DATA_BITS_8,
        .Parity = UART_PARITY_NONE,
        .StopBits = UART_STOP_BITS_1,
        .FlowControl = UART_FLOW_CONTROL_NONE,
        .FifoEnable = TRUE,
        .RxInterruptEnable = FALSE,
        .TxInterruptEnable = FALSE,
        .RxCallback = NULL_PTR,
        .TxCallback = NULL_PTR
    };
    Uart_Init(&DebugUart_Config);
    
    /* Step 3: Configure GPS UART (UART1 to GPS module) */
    const Uart_ConfigType GpsUart_Config = {
        .Module = GPS_UART_MODULE,
        .BaudRate = GPS_BAUD_RATE,
        .DataBits = UART_DATA_BITS_8,
        .Parity = UART_PARITY_NONE,
        .StopBits = UART_STOP_BITS_1,
        .FlowControl = UART_FLOW_CONTROL_NONE,
        .FifoEnable = TRUE,
        .RxInterruptEnable = FALSE,
        .TxInterruptEnable = FALSE,
        .RxCallback = NULL_PTR,
        .TxCallback = NULL_PTR
    };
    Uart_Init(&GpsUart_Config);
    
    /* Small delay for UART stability */
    {
        volatile uint32 delay;
        for (delay = 0; delay < 10000; delay++) { }
    }
    
    /* Enable SBAS for improved accuracy (1-2m instead of 3-5m) */
    Gps_EnableSBAS();
    
    /* Small delay after configuration */
    {
        volatile uint32 delay;
        for (delay = 0; delay < 10000; delay++) { }
    }
    
    /* Enable GLONASS for more satellites and better accuracy */
    Gps_EnableGLONASS();
    
    /* Small delay after configuration */
    {
        volatile uint32 delay;
        for (delay = 0; delay < 10000; delay++) { }
    }
    
    /* Send welcome message */
    Debug_Print((const uint8*)"\r\n\r\n");
    Debug_Print((const uint8*)"========================================\r\n");
    Debug_Print((const uint8*)"  GPS NEO-M8M Reader for TM4C123GH6PM  \r\n");
    Debug_Print((const uint8*)"========================================\r\n");
    Debug_Print((const uint8*)"GPS UART: UART1 (PB0/PB1) @ 9600 baud\r\n");
    Debug_Print((const uint8*)"Debug UART: UART0 (PA0/PA1) @ 115200 baud\r\n");
    Debug_Print((const uint8*)"\r\nWaiting for GPS data...\r\n\r\n");
    
    /* Main loop */
    while (1) {
        /* Read GPS data */
        ReadGpsData();
        
        /* Process complete sentence */
        if (newSentenceReady) {
            ProcessNmeaSentence(nmeaBuffer);
            newSentenceReady = FALSE;
            
            /* Display GPS data every 10 sentences (~1 second) */
            updateCounter++;
            if (updateCounter >= 10) {
                DisplayGpsData();
                updateCounter = 0;
            }
        }
        
        /* Small delay to prevent CPU hogging */
        {
            volatile uint32 delay;
            for (delay = 0; delay < 1000; delay++) { }
        }
    }
    
    /* Never reached - infinite loop above */
}

#endif  /* <<<< END OF COMMENTED OUT GPS READER CODE >>>> */
