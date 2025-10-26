/*
 * @file main.c
 * @brief Comprehensive Multi-Driver Test for TM4C123GH6PM LaunchPad
 * @details This program demonstrates integration of multiple drivers:
 *          - UART: Bidirectional communication with PC terminal
 *          - Timer: Periodic interrupts for automatic LED cycling
 *          - GPIO: Pin control for LEDs and buttons
 *          - LED: High-level LED control abstraction
 *          - Button: Debounced button input with event detection
 *
 * Features:
 *          - Auto-cycling LEDs every 2 seconds (Timer2A interrupt)
 *          - Manual LED control via UART commands
 *          - Button control: SW1 (mode toggle), SW2 (next color)
 *          - Real-time status reporting via UART
 *          - Uptime counter and statistics
 *          - Button press counter
 *
 * Hardware Setup:
 * - UART0: PA0 (RX), PA1 (TX) - Connected to USB debug port
 * - Red LED: PF1 (active-high)
 * - Blue LED: PF2 (active-high)
 * - Green LED: PF3 (active-high)
 * - SW1 Button: PF4 (active-low, internal pull-up)
 * - SW2 Button: PF0 (active-low, internal pull-up)
 *
 * UART Configuration:
 * - Baud Rate: 115200 bps
 * - Data: 8 bits, No parity, 1 stop bit (8N1)
 * - FIFO enabled
 *
 * Timer Configuration:
 * - Timer2A: 2 seconds periodic (32,000,000 ticks @ 16MHz)
 * - Interrupt-driven automatic color cycling
 *
 * Button Functions:
 * - SW1 (PF4): Toggle AUTO/MANUAL mode
 * - SW2 (PF0): Next color (in manual mode) / Pause auto-cycle
 *
 * Terminal Commands:
 * - '0'-'7' = Set LED color (OFF, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE)
 * - 'a' = Enable auto-cycling (Timer)
 * - 's' = Stop auto-cycling (Manual mode)
 * - 't' = Show uptime and statistics
 * - 'h' = Show help menu
 *
 * @author Mohamed Yasser
 * @date Oct 26, 2025
 * @version 3.0.0
 */

#include "../../MCAL/GPIO/Gpio.h"
#include "../../MCAL/UART/Uart.h"
#include "../../MCAL/TIMER/Timer.h"
#include "../../ECUAL/LED/Led.h"
#include "../../ECUAL/Button/Button.h"

/* ===================[NVIC Register]=================== */
#define NVIC_EN0_R              (*((volatile uint32*)0xE000E100UL))

/* ===================[External Configurations]=================== */
extern const Led_ConfigType Led_Red;
extern const Led_ConfigType Led_Blue;
extern const Led_ConfigType Led_Green;
extern const Button_ConfigType Button_SW1;
extern const Button_ConfigType Button_SW2;

/* ===================[Global Variables]=================== */
static volatile uint8 currentColor = 0;      /* Current LED color (0-7) */
static volatile boolean autoMode = FALSE;     /* Auto-cycling disabled by default */
static volatile uint32 cycleCount = 0;        /* Number of color cycles */
static volatile uint32 uptimeSeconds = 0;     /* Uptime in seconds */
static volatile uint32 sw1PressCount = 0;     /* SW1 button press counter */
static volatile uint32 sw2PressCount = 0;     /* SW2 button press counter */
static volatile boolean pauseAuto = FALSE;    /* Pause auto-cycling temporarily */

/* ===================[Helper Functions]=================== */

/**
 * @brief Set LED color based on command
 * @param color Color code (0-7)
 */
void SetLedColor(uint8 color) {
    switch(color) {
        case 0:  /* OFF */
            Led_SetState(&Led_Red, LED_OFF);
            Led_SetState(&Led_Green, LED_OFF);
            Led_SetState(&Led_Blue, LED_OFF);
            break;
        case 1:  /* RED */
            Led_SetState(&Led_Red, LED_ON);
            Led_SetState(&Led_Green, LED_OFF);
            Led_SetState(&Led_Blue, LED_OFF);
            break;
        case 2:  /* GREEN */
            Led_SetState(&Led_Red, LED_OFF);
            Led_SetState(&Led_Green, LED_ON);
            Led_SetState(&Led_Blue, LED_OFF);
            break;
        case 3:  /* BLUE */
            Led_SetState(&Led_Red, LED_OFF);
            Led_SetState(&Led_Green, LED_OFF);
            Led_SetState(&Led_Blue, LED_ON);
            break;
        case 4:  /* YELLOW */
            Led_SetState(&Led_Red, LED_ON);
            Led_SetState(&Led_Green, LED_ON);
            Led_SetState(&Led_Blue, LED_OFF);
            break;
        case 5:  /* MAGENTA */
            Led_SetState(&Led_Red, LED_ON);
            Led_SetState(&Led_Green, LED_OFF);
            Led_SetState(&Led_Blue, LED_ON);
            break;
        case 6:  /* CYAN */
            Led_SetState(&Led_Red, LED_OFF);
            Led_SetState(&Led_Green, LED_ON);
            Led_SetState(&Led_Blue, LED_ON);
            break;
        case 7:  /* WHITE */
            Led_SetState(&Led_Red, LED_ON);
            Led_SetState(&Led_Green, LED_ON);
            Led_SetState(&Led_Blue, LED_ON);
            break;
        default:
            break;
    }
}

/**
 * @brief Get color name string
 */
const uint8* GetColorName(uint8 color) {
    switch(color) {
        case 0: return (const uint8*)"OFF";
        case 1: return (const uint8*)"RED";
        case 2: return (const uint8*)"GREEN";
        case 3: return (const uint8*)"BLUE";
        case 4: return (const uint8*)"YELLOW";
        case 5: return (const uint8*)"MAGENTA";
        case 6: return (const uint8*)"CYAN";
        case 7: return (const uint8*)"WHITE";
        default: return (const uint8*)"UNKNOWN";
    }
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
    
    /* Reverse */
    uint8 j;
    for (j = 0; j < i; j++) {
        buffer[j] = temp[i - j - 1];
    }
    buffer[j] = '\0';
}

/**
 * @brief Display help menu via UART
 */
void ShowMenu(void) {
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"================================================\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  TM4C123 Multi-Driver Integration Test\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  UART + Timer + GPIO + LED + Button\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"================================================\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"UART Commands:\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  0-7 : Set LED color (OFF/R/G/B/Y/M/C/W)\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  a   : Enable AUTO mode (Timer cycling)\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  s   : STOP auto mode (Manual control)\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  t   : Show uptime & statistics\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  h   : Show this menu\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nButton Controls:\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  SW1 (PF4): Toggle AUTO/MANUAL mode\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  SW2 (PF0): Next color / Pause-Resume\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"================================================\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"Status: ");
    Uart_SendString(UART_MODULE_0, autoMode ? (const uint8*)"AUTO" : (const uint8*)"MANUAL");
    if (pauseAuto && autoMode) {
        Uart_SendString(UART_MODULE_0, (const uint8*)" [PAUSED]");
    }
    Uart_SendString(UART_MODULE_0, (const uint8*)" | Color: ");
    Uart_SendString(UART_MODULE_0, GetColorName(currentColor));
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nReady > ");
}

/**
 * @brief Show system statistics
 */
void ShowStatistics(void) {
    uint8 buffer[12];
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n========== System Statistics ==========\r\n");
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"Uptime: ");
    Uint32ToString(uptimeSeconds, buffer);
    Uart_SendString(UART_MODULE_0, buffer);
    Uart_SendString(UART_MODULE_0, (const uint8*)" seconds\r\n");
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"Color Cycles: ");
    Uint32ToString(cycleCount, buffer);
    Uart_SendString(UART_MODULE_0, buffer);
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"SW1 Presses: ");
    Uint32ToString(sw1PressCount, buffer);
    Uart_SendString(UART_MODULE_0, buffer);
    Uart_SendString(UART_MODULE_0, (const uint8*)" (Mode Toggle)\r\n");
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"SW2 Presses: ");
    Uint32ToString(sw2PressCount, buffer);
    Uart_SendString(UART_MODULE_0, buffer);
    Uart_SendString(UART_MODULE_0, (const uint8*)" (Next Color)\r\n");
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nCurrent Mode: ");
    Uart_SendString(UART_MODULE_0, autoMode ? (const uint8*)"AUTO" : (const uint8*)"MANUAL");
    if (pauseAuto && autoMode) {
        Uart_SendString(UART_MODULE_0, (const uint8*)" [PAUSED]");
    }
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"Current Color: ");
    Uart_SendString(UART_MODULE_0, GetColorName(currentColor));
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nSystem Configuration:\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  System Clock: 16 MHz\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  UART Baud: 115200 bps\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  Timer Period: 2000 ms\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  Button Debounce: 20 ms\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"=======================================\r\n");
}

/**
 * @brief Process received command
 * @param cmd Command character
 */
void ProcessCommand(uint8 cmd) {
    /* Echo the received character */
    Uart_SendByte(UART_MODULE_0, cmd);
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    
    if (cmd >= '0' && cmd <= '7') {
        /* LED color command - switch to manual mode */
        autoMode = FALSE;
        currentColor = cmd - '0';
        SetLedColor(currentColor);
        
        /* Send confirmation */
        Uart_SendString(UART_MODULE_0, (const uint8*)"[MANUAL] LED set to: ");
        Uart_SendString(UART_MODULE_0, GetColorName(currentColor));
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n");
    }
    else if (cmd == 'a' || cmd == 'A') {
        /* Enable auto mode */
        autoMode = TRUE;
        Uart_SendString(UART_MODULE_0, (const uint8*)"[AUTO MODE ENABLED] Timer will cycle colors every 2s\r\n");
    }
    else if (cmd == 's' || cmd == 'S') {
        /* Stop auto mode */
        autoMode = FALSE;
        Uart_SendString(UART_MODULE_0, (const uint8*)"[AUTO MODE STOPPED] Use 0-7 for manual control\r\n");
    }
    else if (cmd == 't' || cmd == 'T') {
        /* Show statistics */
        ShowStatistics();
    }
    else if (cmd == 'h' || cmd == 'H') {
        /* Show help menu */
        ShowMenu();
    }
    else {
        /* Unknown command */
        Uart_SendString(UART_MODULE_0, (const uint8*)"Unknown command. Press 'h' for help.\r\n");
    }
    
    Uart_SendString(UART_MODULE_0, (const uint8*)"Ready > ");
}

/* ===================[Timer Interrupt Handler]=================== */
/**
 * @brief Timer2A ISR - Auto-cycles LED colors every 2 seconds
 */
void Timer2A_Handler(void) {
    /* Clear interrupt flag */
    Timer_ClearInterrupt(TIMER_MODULE_2, TIMER_BLOCK_A, TIMER_INT_TIMEOUT);
    
    /* Update uptime */
    uptimeSeconds += 2;
    
    /* Only cycle if in auto mode and not paused */
    if ((autoMode == TRUE) && (pauseAuto == FALSE)) {
        /* Cycle to next color */
        currentColor = (currentColor + 1) % 8;
        cycleCount++;
        
        /* Set new color */
        SetLedColor(currentColor);
        
        /* Optional: Send notification via UART */
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n[AUTO] ");
        Uart_SendString(UART_MODULE_0, GetColorName(currentColor));
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nReady > ");
    }
}

/* ===================[Button Handler Functions]=================== */
/**
 * @brief Handle SW1 button press - Toggle AUTO/MANUAL mode
 */
void HandleSW1Press(void) {
    sw1PressCount++;
    
    /* Toggle mode */
    autoMode = !autoMode;
    pauseAuto = FALSE;  /* Clear pause flag when switching modes */
    
    /* Send notification */
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n[SW1] Mode: ");
    Uart_SendString(UART_MODULE_0, autoMode ? (const uint8*)"AUTO" : (const uint8*)"MANUAL");
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nReady > ");
}

/**
 * @brief Handle SW2 button press - Next color or Pause/Resume
 */
void HandleSW2Press(void) {
    sw2PressCount++;
    
    if (autoMode == TRUE) {
        /* In AUTO mode: Toggle pause */
        pauseAuto = !pauseAuto;
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n[SW2] Auto-cycle: ");
        Uart_SendString(UART_MODULE_0, pauseAuto ? (const uint8*)"PAUSED" : (const uint8*)"RESUMED");
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nReady > ");
    } else {
        /* In MANUAL mode: Next color */
        currentColor = (currentColor + 1) % 8;
        SetLedColor(currentColor);
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n[SW2] Next: ");
        Uart_SendString(UART_MODULE_0, GetColorName(currentColor));
        Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nReady > ");
    }
}

/* ===================[Main Function]=================== */
int main(void) {
    uint8 receivedByte;
    Button_StateType sw1State, sw2State;
    
    /* Step 1: Initialize GPIO driver */
    Gpio_Init(&Gpio_Configuration);

    /* Step 2: Initialize LED driver */
    Led_Init(&Led_Red);
    Led_Init(&Led_Blue);
    Led_Init(&Led_Green);

    /* Turn off all LEDs initially */
    Led_SetState(&Led_Red, LED_OFF);
    Led_SetState(&Led_Blue, LED_OFF);
    Led_SetState(&Led_Green, LED_OFF);

    /* Step 3: Initialize Button driver */
    Button_Init(&Button_SW1);
    Button_Init(&Button_SW2);

    /* Step 4: Configure UART0 - 115200 baud, 8N1 */
    const Uart_ConfigType Uart0_Config = {
        .Module = UART_MODULE_0,
        .BaudRate = UART_BAUD_115200,
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

    /* Step 5: Initialize UART */
    Uart_Init(&Uart0_Config);

    /* Step 6: Configure Timer2A - 2 seconds periodic for auto-cycling */
    const Timer_ConfigType Timer2A_Config = {
        .Module = TIMER_MODULE_2,
        .Block = TIMER_BLOCK_A,
        .ConfigMode = TIMER_CONFIG_32BIT,
        .OperationMode = TIMER_MODE_PERIODIC,
        .CountDirection = TIMER_COUNT_DOWN,
        .PwmMode = TIMER_PWM_DISABLED,
        .LoadValue = 32000000u,             /* 2 seconds @ 16MHz */
        .MatchValue = 0u,
        .Prescaler = 0u,
        .InterruptEnable = TRUE,
        .MatchInterruptEnable = FALSE,
        .TimeoutCallback = NULL_PTR,
        .MatchCallback = NULL_PTR
    };

    /* Step 7: Initialize Timer */
    Timer_Init(&Timer2A_Config);

    /* Step 8: Enable Timer2A interrupt in NVIC (IRQ 23) */
    NVIC_EN0_R |= (1 << 23);

    /* Step 9: Start Timer */
    Timer_Start(TIMER_MODULE_2, TIMER_BLOCK_A);

    /* Step 10: Send welcome message */
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\n\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"****************************************************\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"*   TM4C123GH6PM Multi-Driver Integration Test    *\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"****************************************************\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"Drivers Active:\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  [x] GPIO Driver\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  [x] LED Driver\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  [x] Button Driver (SW1, SW2 with debounce)\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  [x] UART Driver (115200 bps, 8N1)\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  [x] Timer Driver (2s periodic interrupt)\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nSystem Configuration:\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  System Clock: 16 MHz\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  Mode: MANUAL (LEDs OFF)\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  Waiting for your command...\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"\r\nQuick Start:\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  - Press SW1 or type 'a' for AUTO mode\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  - Press SW2 or type '1-7' for LED colors\r\n");
    Uart_SendString(UART_MODULE_0, (const uint8*)"  - Type 'h' for full help menu\r\n");
    
    /* Show menu */
    ShowMenu();

    /* Step 11: Main loop - Poll buttons and UART */
    while(1) {
        /* Poll SW1 button - Mode toggle */
        if (Button_HasStateChanged(&Button_SW1, &sw1State) == TRUE) {
            if (sw1State == BUTTON_PRESSED) {
                HandleSW1Press();
            }
        }
        
        /* Poll SW2 button - Next color / Pause */
        if (Button_HasStateChanged(&Button_SW2, &sw2State) == TRUE) {
            if (sw2State == BUTTON_PRESSED) {
                HandleSW2Press();
            }
        }
        
        /* Poll UART for commands */
        if (Uart_IsRxDataAvailable(UART_MODULE_0) == TRUE) {
            /* Receive byte */
            if (Uart_ReceiveByte(UART_MODULE_0, &receivedByte) == E_OK) {
                /* Process the command */
                ProcessCommand(receivedByte);
            }
        }
        
        /* Background: Timer ISR handles auto-cycling */
        /* Optional: Add low-power mode */
        /* __asm("    WFI"); */  /* Wait For Interrupt */
    }

    /* In embedded systems, main() should never return */
}
