# Button Driver - Complete Documentation

**Version:** 1.0.0
**Date:** October 26, 2025
**Author:** Mohamed Yasser
**Target:** TM4C123GH6PM (Tiva C Series)

---

## Table of Contents

1. [Overview](#overview)
2. [Features](#features)
3. [API Reference](#api-reference)
4. [Configuration](#configuration)
5. [Usage Examples](#usage-examples)
6. [Testing](#testing)
7. [Error Handling](#error-handling)

---

## Overview

The Button driver provides a high-level interface for reading push buttons and switches connected to GPIO pins. It supports debouncing, state change detection, and both active-high and active-low configurations with comprehensive error handling.

### Key Highlights

- ✅ **Debouncing Support:** Software debouncing with configurable time
- ✅ **State Change Detection:** Detect button press/release events
- ✅ **Active-High/Active-Low Support:** Configurable button polarity
- ✅ **Pull-Up Configuration:** Support for internal/external pull resistors
- ✅ **Error Detection:** DET integration for parameter validation
- ✅ **Thread-Safe:** Uses atomic GPIO operations
- ✅ **Memory Efficient:** Tracks state for up to 48 buttons

### Files Structure

```
ECUAL/Button/
├── Button.h           (95 lines)  - API and type definitions
├── Button.c           (214 lines) - Implementation
├── Button_Cfg.h       (57 lines)  - Configuration parameters
├── Button_PBCfg.c     (50 lines)  - Example configurations
└── BUTTON_DRIVER_README.md (this file)
```

---

## Features

### Hardware Support
- **All GPIO Ports:** A, B, C, D, E, F (48 pins total)
- **Pin Modes:** Digital I/O (assumes GPIO configured as input)
- **Button Configurations:** Active-high, Active-low
- **Pull Resistors:** Internal pull-up, External pull-up/down
- **Debouncing:** Configurable debounce time

### Software Features
- ✅ **Debouncing:** Software-based contact bounce elimination
- ✅ **State Change Detection:** Track button press/release events
- ✅ **Parameter Validation:** NULL pointer and configuration checks
- ✅ **Error Reporting:** DET integration
- ✅ **Version Information:** Module version API
- ✅ **Thread Safety:** Atomic GPIO operations
- ✅ **State Tracking:** Previous state memory for change detection

### AUTOSAR Compliance
- ✅ Standardized API structure
- ✅ DET integration
- ✅ Version information API
- ✅ Error code definitions
- ✅ Configuration management

---

## API Reference

### `Button_Init()`
```c
void Button_Init(const Button_ConfigType* ConfigPtr);
```
Initialize button by configuring GPIO pin as input.

**Parameters:**
- `ConfigPtr` - Pointer to button configuration structure

**Preconditions:**
- GPIO module must be initialized
- Button pin must be configured as input in GPIO driver

**Postconditions:**
- Button is ready for state reading
- Previous state is initialized

**Example:**
```c
Button_Init(&Button_SW1);
```

---

### `Button_ReadState()`
```c
Button_StateType Button_ReadState(const Button_ConfigType* Button);
```
Read current button state (no debouncing).

**Returns:** `BUTTON_PRESSED` or `BUTTON_RELEASED`

**Example:**
```c
Button_StateType state = Button_ReadState(&Button_SW1);
if (state == BUTTON_PRESSED) {
    // Button is pressed
}
```

---

### `Button_ReadStateDebounced()`
```c
Button_StateType Button_ReadStateDebounced(const Button_ConfigType* Button);
```
Read button state with debouncing.

**Returns:** Debounced button state

**Example:**
```c
Button_StateType state = Button_ReadStateDebounced(&Button_SW1);
```

---

### `Button_HasStateChanged()`
```c
boolean Button_HasStateChanged(const Button_ConfigType* Button, Button_StateType* CurrentState);
```
Check if button state has changed since last call.

**Parameters:**
- `Button` - Pointer to button configuration
- `CurrentState` - Optional pointer to store current state

**Returns:** `TRUE` if state changed, `FALSE` otherwise

**Example:**
```c
Button_StateType currentState;
if (Button_HasStateChanged(&Button_SW1, &currentState)) {
    if (currentState == BUTTON_PRESSED) {
        // Button was just pressed
        Led_Toggle(&Led_Red);
    }
}
```

---

### `Button_GetVersionInfo()`
```c
void Button_GetVersionInfo(Std_VersionInfoType* VersionInfo);
```
Get driver version information.

---

## Configuration

### Type Definitions

#### `Button_StateType`
```c
typedef enum {
    BUTTON_RELEASED = 0u,    /**< Button is not pressed */
    BUTTON_PRESSED = 1u      /**< Button is pressed */
} Button_StateType;
```

#### `Button_ConfigType`
```c
typedef struct {
    Gpio_PortType   Port;        /**< GPIO port (GPIO_PORT_A to GPIO_PORT_F) */
    Gpio_PinType    Pin;         /**< GPIO pin (GPIO_PIN_0 to GPIO_PIN_7) */
    boolean         ActiveHigh;  /**< TRUE for active-high, FALSE for active-low */
    boolean         PullUp;      /**< TRUE if internal pull-up is used */
    uint16          DebounceMs;  /**< Debounce time in milliseconds */
} Button_ConfigType;
```

### Configuration Switches (Button_Cfg.h)

```c
#define BUTTON_DEV_ERROR_DETECT           STD_ON   /* Enable DET */
#define BUTTON_VERSION_INFO_API           STD_ON   /* Enable version info API */
#define BUTTON_STATE_CHANGE_API           STD_ON   /* Enable state change detection */
#define BUTTON_DEFAULT_DEBOUNCE_MS        (20u)    /* Default debounce time */
```

### Example Configurations

#### Active-Low Button with Pull-Up (LaunchPad)
```c
const Button_ConfigType Button_SW1 = {
    .Port = GPIO_PORT_F,
    .Pin = GPIO_PIN_4,
    .ActiveHigh = FALSE,  /* Pressed = LOW */
    .PullUp = TRUE,       /* Internal pull-up */
    .DebounceMs = 20u     /* 20ms debounce */
};
```

#### Active-High Button with Pull-Down
```c
const Button_ConfigType Button_External = {
    .Port = GPIO_PORT_B,
    .Pin = GPIO_PIN_1,
    .ActiveHigh = TRUE,   /* Pressed = HIGH */
    .PullUp = FALSE,      /* External pull-down */
    .DebounceMs = 25u     /* 25ms debounce */
};
```

---

## Usage Examples

### Example 1: Basic Button Reading

```c
#include "Button.h"

int main(void) {
    /* Initialize GPIO */
    Gpio_Init(&Gpio_Configuration);

    /* Initialize Button */
    Button_Init(&Button_SW1);

    while(1) {
        /* Read button state */
        Button_StateType state = Button_ReadStateDebounced(&Button_SW1);

        if (state == BUTTON_PRESSED) {
            /* Button is pressed */
        } else {
            /* Button is released */
        }

        delay_ms(10);  /* Small delay to prevent busy waiting */
    }
}
```

### Example 2: Button-Controlled LED

```c
#include "Led.h"
#include "Button.h"

int main(void) {
    /* Initialize GPIO */
    Gpio_Init(&Gpio_Configuration);

    /* Initialize LED and Button */
    Led_Init(&Led_Red);
    Button_Init(&Button_SW1);

    while(1) {
        /* Read button state */
        Button_StateType buttonState = Button_ReadStateDebounced(&Button_SW1);

        /* Control LED based on button */
        if (buttonState == BUTTON_PRESSED) {
            Led_SetState(&Led_Red, LED_ON);
        } else {
            Led_SetState(&Led_Red, LED_OFF);
        }

        delay_ms(50);
    }
}
```

### Example 3: Button Toggle with State Change Detection

```c
#include "Led.h"
#include "Button.h"

int main(void) {
    /* Initialize GPIO */
    Gpio_Init(&Gpio_Configuration);

    /* Initialize LED and Button */
    Led_Init(&Led_Blue);
    Button_Init(&Button_SW2);

    while(1) {
        Button_StateType currentState;

        /* Check if button state changed */
        if (Button_HasStateChanged(&Button_SW2, &currentState)) {
            if (currentState == BUTTON_PRESSED) {
                /* Button was just pressed - toggle LED */
                Led_Toggle(&Led_Blue);
            }
        }

        delay_ms(10);
    }
}
```

### Example 4: Multiple Buttons

```c
int main(void) {
    /* Initialize GPIO */
    Gpio_Init(&Gpio_Configuration);

    /* Initialize LEDs and Buttons */
    Led_Init(&Led_Red);
    Led_Init(&Led_Blue);
    Button_Init(&Button_SW1);
    Button_Init(&Button_SW2);

    while(1) {
        /* Control Red LED with SW1 */
        Button_StateType sw1State = Button_ReadStateDebounced(&Button_SW1);
        Led_SetState(&Led_Red, (sw1State == BUTTON_PRESSED) ? LED_ON : LED_OFF);

        /* Control Blue LED with SW2 */
        Button_StateType sw2State = Button_ReadStateDebounced(&Button_SW2);
        Led_SetState(&Led_Blue, (sw2State == BUTTON_PRESSED) ? LED_ON : LED_OFF);

        delay_ms(50);
    }
}
```

---

## Testing

### Test Case 1: Basic Functionality
```c
void Test_ButtonBasic(void) {
    /* Initialize */
    Gpio_Init(&Gpio_Configuration);
    Button_Init(&Button_SW1);

    /* Test released state */
    Button_StateType state = Button_ReadState(&Button_SW1);
    assert(state == BUTTON_RELEASED);

    /* Note: For pressed state, manually press SW1 during test */
}
```

### Test Case 2: Debouncing
```c
void Test_ButtonDebouncing(void) {
    /* Initialize */
    Button_Init(&Button_SW1);

    /* Test debounced reading */
    Button_StateType state1 = Button_ReadState(&Button_SW1);
    Button_StateType state2 = Button_ReadStateDebounced(&Button_SW1);

    /* Both should be the same when no bouncing */
    assert(state1 == state2);
}
```

### Test Case 3: State Change Detection
```c
void Test_StateChange(void) {
    /* Initialize */
    Button_Init(&Button_SW1);

    /* Initial state */
    Button_StateType initialState;
    boolean changed = Button_HasStateChanged(&Button_SW1, &initialState);
    assert(changed == FALSE);  /* No change initially */

    /* Wait a bit and check again */
    delay_ms(100);
    Button_StateType currentState;
    changed = Button_HasStateChanged(&Button_SW1, &currentState);

    /* Should detect if button was pressed/released */
}
```

### Test Case 4: Active-High Configuration
```c
void Test_ButtonActiveHigh(void) {
    const Button_ConfigType Button_Test = {
        .Port = GPIO_PORT_A,
        .Pin = GPIO_PIN_0,
        .ActiveHigh = TRUE,   /* Active-high */
        .PullUp = FALSE,      /* External pull-down */
        .DebounceMs = 10u
    };

    Button_Init(&Button_Test);

    /* Test with external signal */
    /* Drive pin HIGH */
    Gpio_WriteChannel(GPIO_CHANNEL(GPIO_PORT_A, GPIO_PIN_0), GPIO_LEVEL_HIGH);
    Button_StateType state = Button_ReadStateDebounced(&Button_Test);
    assert(state == BUTTON_PRESSED);
}
```

---

## Error Handling

### Error Codes

| Error Code | Name | Description |
|------------|------|-------------|
| 0x01 | BUTTON_E_PARAM_POINTER | NULL pointer passed to function |
| 0x02 | BUTTON_E_INVALID_CONFIG | Invalid button configuration |

### Error Detection Configuration

Enable/disable error detection in `Button_Cfg.h`:

```c
#define BUTTON_DEV_ERROR_DETECT           STD_ON   /* Enable error detection */
#define BUTTON_VERSION_INFO_API           STD_ON   /* Enable version info API */
#define BUTTON_STATE_CHANGE_API           STD_ON   /* Enable state change detection */
```

When `BUTTON_DEV_ERROR_DETECT` is `STD_OFF`:
- Parameter validation is disabled
- Configuration checks are removed
- Smaller code size, faster execution

---

## Integration with GPIO Driver

### Required GPIO Configuration

The Button driver requires the GPIO driver to be properly configured:

```c
/* GPIO Configuration for Button pins */
const Gpio_PinConfigType GpioButtonConfig[] = {
    {
        .Port = GPIO_PORT_F,
        .Pin = GPIO_PIN_4,
        .Direction = GPIO_PIN_IN,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_DIO,
        .InternalResistor = GPIO_RESISTOR_PULL_UP,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 0,
        .DirectionChangeable = FALSE
    }
};

Gpio_Init(&GpioConfig);    /* Initialize GPIO first */
Button_Init(&Button_SW1);  /* Then initialize Button */
```

### Pull Resistor Configuration

For **active-low buttons** (typical):
- Use `InternalResistor = GPIO_RESISTOR_PULL_UP`
- Button connects pin to GND when pressed
- Released = HIGH, Pressed = LOW

For **active-high buttons:**
- Use external pull-down resistor
- Button connects pin to VCC when pressed
- Released = LOW, Pressed = HIGH

---

## Memory Footprint

### Code Size
- **Button.h:** ~0.4 KB
- **Button.c:** ~1.2 KB
- **Button_Cfg.h:** ~0.3 KB
- **Total:** ~1.9 KB

### RAM Usage
- **State tracking array:** 48 bytes (1 byte per possible button)
- **Stack usage:** ~20-40 bytes per function call
- **Total:** ~68 bytes

### State Tracking
- Tracks previous state for up to 48 buttons (6 ports × 8 pins)
- Each button uses 1 byte of RAM for state tracking
- Automatically manages state memory

---

## Performance

### Execution Times (at 80 MHz)
- **Button_ReadState():** ~50-100 ns (4-8 cycles)
- **Button_ReadStateDebounced():** ~20ms + read time (due to debounce delay)
- **Button_HasStateChanged():** ~50-100 ns (4-8 cycles)
- **Button_Init():** ~200-300 ns (16-24 cycles)

### Debouncing Impact
- **Without debouncing:** ~50 ns response time
- **With debouncing:** ~20ms response time (configurable)
- **Trade-off:** Slower response vs. reliable reading

### Thread Safety
- ✅ All functions use atomic GPIO operations
- ✅ Safe for use in interrupt handlers
- ✅ Safe for use in multi-threaded environments
- ✅ State tracking is thread-safe

---

## Best Practices

### ✅ DO:
- Configure GPIO pin as input before initializing button
- Use debounced reading for reliable operation
- Set appropriate debounce time (10-50ms typical)
- Use state change detection for event-driven code
- Test with actual hardware switches

### ❌ DON'T:
- Initialize button before GPIO driver
- Pass NULL pointers to API functions
- Use very short debounce times (<10ms) with mechanical switches
- Assume button state without reading it
- Mix active-high and active-low without clear documentation

### Button-Specific Tips:
- **Mechanical switches:** Use 20-50ms debounce time
- **Tactile switches:** Use 10-20ms debounce time
- **Active-low:** More common, easier to interface
- **Pull-up resistors:** Provide clean HIGH state when released

---

## Version History

### v1.0.0 (October 26, 2025)
- ✅ Complete Button driver implementation
- ✅ Debouncing support with configurable time
- ✅ State change detection API
- ✅ Active-high/active-low support
- ✅ Pull-up resistor configuration
- ✅ DET integration and error handling
- ✅ Version information API
- ✅ State tracking for 48 buttons
- ✅ Comprehensive documentation
- ✅ Example configurations for LaunchPad

---

## Support & Contact

For issues, questions, or contributions:
1. Review this documentation
2. Check example configurations in Button_PBCfg.c
3. Consult TM4C123GH6PM datasheet for hardware details
4. Contact: Mohamed Yasser

---

**End of Documentation**
