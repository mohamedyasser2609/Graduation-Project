/**
 * @file App_CommTask.c
 * @brief Communication Task Implementation
 * @details Handles ROS2 communication via UART with wheel speed parsing
 *
 * RX Protocol:
 *   "W,<LeftRadS>,<RightRadS>\n"  - Wheel speed command
 *   "S\n"                         - Emergency stop
 *   "R\n"                         - Resume
 *
 * TX Protocol:
 *   "F,<LTicks>,<RTicks>,<Yaw>,<Lat>,<Lon>\n" - Sensor feedback
 *
 * @author Mohamed Yasser
 * @date Jan 08, 2026
 * @version 1.1.0
 */

#include "../../CONFIG/Std_Types.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "queue.h"

/* Service includes */
#include "../../MCAL/UART/Uart.h"
#include "../../SERVICES/COMM/ComStack.h"
#include "../../SERVICES/RTOS/Tasks_Init.h"

/* Shared types */
#include "../Common/App_SharedTypes.h"

/* ===================[Private Variables]=================== */
static boolean App_CommInitialized = FALSE;

/* RX buffer for line-based parsing */
#define RX_BUFFER_SIZE  (64u)
static char App_RxBuffer[RX_BUFFER_SIZE];
static uint8 App_RxIndex = 0u;

/* Queue handle cache */
static QueueHandle_t App_WheelCmdQueue = NULL;
static QueueHandle_t App_FeedbackQueue = NULL;

/* ===================[Private Functions]=================== */

/**
 * @brief Parse a float from string
 * @param[in]  str    Input string
 * @param[out] endPtr Pointer to next character after number
 * @return Parsed float value
 */
static float32 App_ParseFloat(const char* str, char** endPtr)
{
    float32 result = 0.0f;
    float32 fraction = 0.0f;
    float32 divisor = 1.0f;
    boolean negative = FALSE;
    boolean inFraction = FALSE;
    const char* p = str;
    
    /* Skip whitespace */
    while (*p == ' ') p++;
    
    /* Handle sign */
    if (*p == '-')
    {
        negative = TRUE;
        p++;
    }
    else if (*p == '+')
    {
        p++;
    }
    
    /* Parse digits */
    while ((*p >= '0' && *p <= '9') || *p == '.')
    {
        if (*p == '.')
        {
            inFraction = TRUE;
        }
        else
        {
            if (inFraction)
            {
                divisor *= 10.0f;
                fraction += (float32)(*p - '0') / divisor;
            }
            else
            {
                result = result * 10.0f + (float32)(*p - '0');
            }
        }
        p++;
    }
    
    result += fraction;
    if (negative) result = -result;
    
    if (endPtr != NULL)
    {
        *endPtr = (char*)p;
    }
    
    return result;
}

/**
 * @brief Process a complete received line
 * @param[in] line The received line (null-terminated)
 */
static void App_ProcessReceivedLine(const char* line)
{
    WheelSpeedCmdType cmd;
    char* parsePtr;
    
    if (line[0] == PROTO_CMD_WHEEL && line[1] == PROTO_DELIMITER)
    {
        /* Parse wheel speed command: "W,<left>,<right>\n" */
        cmd.LeftRadPerSec = App_ParseFloat(&line[2], &parsePtr);
        
        if (*parsePtr == PROTO_DELIMITER)
        {
            parsePtr++;
            cmd.RightRadPerSec = App_ParseFloat(parsePtr, NULL);
            cmd.Timestamp = xTaskGetTickCount();
            cmd.Valid = TRUE;
            
            /* Send to Control Task via queue (non-blocking) */
            if (App_WheelCmdQueue != NULL)
            {
                (void)xQueueOverwrite(App_WheelCmdQueue, &cmd);
            }
        }
    }
    else if (line[0] == PROTO_CMD_STOP)
    {
        /* Emergency stop - send zero speed */
        cmd.LeftRadPerSec = 0.0f;
        cmd.RightRadPerSec = 0.0f;
        cmd.Timestamp = xTaskGetTickCount();
        cmd.Valid = TRUE;
        
        if (App_WheelCmdQueue != NULL)
        {
            (void)xQueueOverwrite(App_WheelCmdQueue, &cmd);
        }
        
        /* Also call direct stop for safety */
        extern void Robot_EmergencyStop(void);
        Robot_EmergencyStop();
    }
    else if (line[0] == PROTO_CMD_RESUME)
    {
        /* Resume from e-stop */
        extern Std_ReturnType Robot_Resume(void);
        (void)Robot_Resume();
    }
}

/**
 * @brief Process incoming UART bytes
 */
static void App_ProcessUartRx(void)
{
    uint8 rxByte;
    
    /* Poll for received bytes */
    while (Uart_IsRxDataAvailable(COMSTACK_DEFAULT_UART_MODULE))
    {
        if (Uart_ReceiveByte(COMSTACK_DEFAULT_UART_MODULE, &rxByte) == E_OK)
        {
            if (rxByte == PROTO_TERMINATOR)
            {
                /* End of line - process command */
                App_RxBuffer[App_RxIndex] = '\0';
                
                if (App_RxIndex > 0u)
                {
                    App_ProcessReceivedLine(App_RxBuffer);
                }
                
                App_RxIndex = 0u;
            }
            else if (rxByte == '\r')
            {
                /* Ignore carriage return */
            }
            else
            {
                /* Add to buffer */
                if (App_RxIndex < (RX_BUFFER_SIZE - 1u))
                {
                    App_RxBuffer[App_RxIndex] = (char)rxByte;
                    App_RxIndex++;
                }
                else
                {
                    /* Buffer overflow - reset */
                    App_RxIndex = 0u;
                }
            }
        }
    }
}

/**
 * @brief Format integer to string
 */
static uint8 App_IntToStr(sint32 value, char* buffer)
{
    char temp[12];
    uint8 i = 0u;
    uint8 len = 0u;
    boolean negative = FALSE;
    uint32 uvalue;
    
    if (value < 0)
    {
        negative = TRUE;
        uvalue = (uint32)(-value);
    }
    else
    {
        uvalue = (uint32)value;
    }
    
    if (uvalue == 0u)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1u;
    }
    
    while (uvalue > 0u)
    {
        temp[i++] = (char)('0' + (uvalue % 10u));
        uvalue /= 10u;
    }
    
    if (negative)
    {
        buffer[len++] = '-';
    }
    
    while (i > 0u)
    {
        i--;
        buffer[len++] = temp[i];
    }
    buffer[len] = '\0';
    
    return len;
}

/**
 * @brief Format float to string (2 decimal places)
 */
static uint8 App_FloatToStr(float32 value, char* buffer)
{
    sint32 intPart;
    sint32 fracPart;
    uint8 len;
    
    if (value < 0.0f)
    {
        buffer[0] = '-';
        value = -value;
        len = 1u;
    }
    else
    {
        len = 0u;
    }
    
    intPart = (sint32)value;
    fracPart = (sint32)((value - (float32)intPart) * 100.0f);
    
    len += App_IntToStr(intPart, &buffer[len]);
    buffer[len++] = '.';
    
    if (fracPart < 10)
    {
        buffer[len++] = '0';
    }
    len += App_IntToStr(fracPart, &buffer[len]);
    
    return len;
}

/**
 * @brief Transmit sensor feedback to ROS2
 */
static void App_TransmitSensorFeedback(void)
{
    SensorFeedbackType feedback;
    char txBuffer[128];
    uint8 txLen = 0u;
    
    /* Check if feedback data is available */
    if (App_FeedbackQueue != NULL)
    {
        if (xQueueReceive(App_FeedbackQueue, &feedback, 0) == pdTRUE)
        {
            if (feedback.Valid)
            {
                /* Format: "F,<LTicks>,<RTicks>,<Yaw>,<Lat>,<Lon>\n" */
                txBuffer[txLen++] = 'F';
                txBuffer[txLen++] = ',';
                
                /* Left encoder ticks */
                txLen += App_IntToStr(feedback.LeftEncoderTicks, &txBuffer[txLen]);
                txBuffer[txLen++] = ',';
                
                /* Right encoder ticks */
                txLen += App_IntToStr(feedback.RightEncoderTicks, &txBuffer[txLen]);
                txBuffer[txLen++] = ',';
                
                /* Yaw */
                txLen += App_FloatToStr(feedback.YawDegrees, &txBuffer[txLen]);
                txBuffer[txLen++] = ',';
                
                /* Latitude */
                txLen += App_FloatToStr(feedback.Latitude, &txBuffer[txLen]);
                txBuffer[txLen++] = ',';
                
                /* Longitude */
                txLen += App_FloatToStr(feedback.Longitude, &txBuffer[txLen]);
                txBuffer[txLen++] = '\n';
                txBuffer[txLen] = '\0';
                
                /* Transmit */
                {
                    uint8 txIdx;
                    for (txIdx = 0u; txIdx < txLen; txIdx++)
                    {
                        Uart_SendByte(COMSTACK_DEFAULT_UART_MODULE, (uint8)txBuffer[txIdx]);
                    }
                }
            }
        }
    }
}

/* ===================[Public Functions]=================== */

/**
 * @brief Initialize communication task
 */
void App_CommTask_Init(void)
{
    /* Get queue handles */
    App_WheelCmdQueue = Tasks_GetWheelSpeedCmdQueue();
    App_FeedbackQueue = Tasks_GetSensorFeedbackQueue();
    
    App_RxIndex = 0u;
    App_CommInitialized = TRUE;
}

/**
 * @brief Communication task main function (called by FreeRTOS task)
 */
void App_CommTask_Run(void)
{
    if (App_CommInitialized == FALSE)
    {
        App_CommTask_Init();
    }
    
    /* 1. Process incoming UART data */
    App_ProcessUartRx();
    
    /* 2. Transmit sensor feedback to ROS2 */
    App_TransmitSensorFeedback();
}
