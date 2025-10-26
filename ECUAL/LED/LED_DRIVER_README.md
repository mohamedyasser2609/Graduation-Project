# LED Driver - Complete Documentation

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

The LED driver provides a high-level interface for controlling LEDs connected to GPIO pins. It supports both active-high and active-low LED configurations with comprehensive error handling and documentation.

### Key Highlights

- ✅ **Active-High/Active-Low Support:** Configurable LED polarity
- ✅ **Error Detection:** DET integration for parameter validation
- ✅ **Version Information:** API for retrieving driver version
- ✅ **Thread-Safe:** Uses atomic GPIO operations
- ✅ **Memory Efficient:** Minimal RAM footprint
- ✅ **AUTOSAR-Inspired:** Structured API with detailed documentation

### Files Structure

```
ECUAL/LED/
├── Led.h              (114 lines) - API and type definitions
├── Led.c              (166 lines) - Implementation
├── Led_Cfg.h          (67 lines)  - Configuration parameters
├── Led_PBCfg.c        (45 lines)  - Example configurations
└── LED_DRIVER_README.md (this file)
```

---

## Features

### Hardware Support
- **All GPIO Ports:** A, B, C, D, E, F (48 pins total)
- **Pin Modes:** Digital I/O (assumes GPIO configured as output)
- **LED Configurations:** Active-high, Active-low
- **State Control:** ON, OFF, Toggle operations
- **State Reading:** Get current LED state

### Software Features
- ✅ **Parameter Validation:** NULL pointer checks
- ✅ **Error Reporting:** DET integration
- ✅ **Version Information:** Module version API
- ✅ **Thread Safety:** Atomic GPIO operations
- ✅ **Documentation:** Comprehensive Doxygen comments

### AUTOSAR Compliance
- ✅ Standardized API structure
- ✅ DET integration
- ✅ Version information API
- ✅ Error code definitions
- ✅ Configuration management

---

## API Reference

### `Led_Init()`
```c
void Led_Init(const Led_ConfigType* ConfigPtr);
```
Initialize LED by setting initial state to OFF.

**Parameters:**
- `ConfigPtr` - Pointer to LED configuration structure

**Preconditions:**
- GPIO module must be initialized
- LED pin must be configured as output in GPIO driver

**Postconditions:**
- LED is set to OFF state

**Example:**
```c
Led_Init(&Led_Red);
```

---

### `Led_SetState()`
```c
void Led_SetState(const Led_ConfigType* Led, Led_StateType State);
```
Set LED to ON or OFF state.

**Parameters:**
- `Led` - Pointer to LED configuration
- `State` - `LED_ON` or `LED_OFF`

**Example:**
```c
Led_SetState(&Led_Red, LED_ON);   // Turn on red LED
Led_SetState(&Led_Red, LED_OFF);  // Turn off red LED
```

---

### `Led_Toggle()`
```c
void Led_Toggle(const Led_ConfigType* Led);
```
Toggle LED state (ON ↔ OFF).

**Example:**
```c
Led_Toggle(&Led_Blue);  // Toggle blue LED
```

---

### `Led_GetState()`
```c
Led_StateType Led_GetState(const Led_ConfigType* Led);
```
Get current LED state.

**Returns:** `LED_ON` or `LED_OFF`

**Example:**
```c
Led_StateType state = Led_GetState(&Led_Green);
if (state == LED_ON) {
    // LED is on
}
```

---

### `Led_GetVersionInfo()`
```c
void Led_GetVersionInfo(Std_VersionInfoType* VersionInfo);
```
Get driver version information.

---

## Configuration

### Type Definitions

#### `Led_StateType`
```c
typedef enum {
    LED_OFF = 0u,    /**< LED is turned off */
    LED_ON = 1u      /**< LED is turned on */
} Led_StateType;
```

#### `Led_ConfigType`
```c
typedef struct {
    Gpio_PortType   Port;        /**< GPIO port (GPIO_PORT_A to GPIO_PORT_F) */
    Gpio_PinType    Pin;         /**< GPIO pin (GPIO_PIN_0 to GPIO_PIN_7) */
    boolean         ActiveHigh;  /**< TRUE for active-high, FALSE for active-low */
} Led_ConfigType;
```

### Configuration Switches (Led_Cfg.h)

```c
#define LED_DEV_ERROR_DETECT              STD_ON   /* Enable DET */
#define LED_VERSION_INFO_API              STD_ON   /* Enable version info API */
```

### Example Configurations

#### Active-High LED (Typical)
```c
const Led_ConfigType Led_Red = {
    .Port = GPIO_PORT_F,
    .Pin = GPIO_PIN_1,
    .ActiveHigh = TRUE  /* ON = HIGH, OFF = LOW */
};
```

#### Active-Low LED
```c
const Led_ConfigType Led_External = {
    .Port = GPIO_PORT_A,
    .Pin = GPIO_PIN_0,
    .ActiveHigh = FALSE  /* ON = LOW, OFF = HIGH */
};
```

---

## Usage Examples

### Example 1: Basic LED Control

```c
#include "Led.h"

int main(void) {
    /* Initialize GPIO first */
    Gpio_Init(&Gpio_Configuration);

    /* Initialize LED */
    Led_Init(&Led_Red);

    while(1) {
        /* Toggle LED every 500ms */
        Led_Toggle(&Led_Red);
        delay_ms(500);
    }
}
```

### Example 2: Multiple LEDs

```c
int main(void) {
    /* Initialize GPIO */
    Gpio_Init(&Gpio_Configuration);

    /* Initialize LEDs */
    Led_Init(&Led_Red);
    Led_Init(&Led_Blue);
    Led_Init(&Led_Green);

    while(1) {
        /* Traffic light pattern */
        Led_SetState(&Led_Red, LED_ON);
        delay_ms(1000);
        Led_SetState(&Led_Red, LED_OFF);

        Led_SetState(&Led_Green, LED_ON);
        delay_ms(1000);
        Led_SetState(&Led_Green, LED_OFF);
    }
}
```

### Example 3: LED State Reading

```c
void CheckLedStates(void) {
    Led_StateType redState = Led_GetState(&Led_Red);
    Led_StateType blueState = Led_GetState(&Led_Blue);

    if (redState == LED_ON && blueState == LED_OFF) {
        /* Red is on, blue is off */
    }
}
```

---

## Testing

### Test Case 1: Basic Functionality
```c
void Test_LedBasic(void) {
    /* Initialize */
    Gpio_Init(&Gpio_Configuration);
    Led_Init(&Led_Red);

    /* Test states */
    Led_SetState(&Led_Red, LED_ON);
    assert(Led_GetState(&Led_Red) == LED_ON);

    Led_SetState(&Led_Red, LED_OFF);
    assert(Led_GetState(&Led_Red) == LED_OFF);

    /* Test toggle */
    Led_Toggle(&Led_Red);
    assert(Led_GetState(&Led_Red) == LED_ON);
}
```

### Test Case 2: Active-Low Configuration
```c
void Test_LedActiveLow(void) {
    const Led_ConfigType Led_Test = {
        .Port = GPIO_PORT_A,
        .Pin = GPIO_PIN_0,
        .ActiveHigh = FALSE  /* Active-low */
    };

    Led_Init(&Led_Test);

    /* Test ON (should be LOW) */
    Led_SetState(&Led_Test, LED_ON);
    Gpio_LevelType level = Gpio_ReadChannel(GPIO_CHANNEL(GPIO_PORT_A, GPIO_PIN_0));
    assert(level == GPIO_LEVEL_LOW);
}
```

### Test Case 3: Error Handling
```c
void Test_LedErrorHandling(void) {
    /* Test NULL pointer */
    Led_SetState(NULL_PTR, LED_ON);  /* Should handle gracefully */
    Led_GetState(NULL_PTR);          /* Should return LED_OFF */
}
```

---

## Error Handling

### Error Codes

| Error Code | Name | Description |
|------------|------|-------------|
| 0x01 | LED_E_PARAM_POINTER | NULL pointer passed to function |

### Error Detection Configuration

Enable/disable error detection in `Led_Cfg.h`:

```c
#define LED_DEV_ERROR_DETECT              STD_ON   /* Enable error detection */
#define LED_VERSION_INFO_API              STD_ON   /* Enable version info */
```

When `LED_DEV_ERROR_DETECT` is `STD_OFF`:
- Parameter validation is disabled
- NULL pointer checks are removed
- Smaller code size, faster execution

---

## Integration with GPIO Driver

### Required GPIO Configuration

The LED driver requires the GPIO driver to be properly configured:

```c
/* GPIO Configuration for LED pins */
const Gpio_PinConfigType GpioLedConfig[] = {
    {
        .Port = GPIO_PORT_F,
        .Pin = GPIO_PIN_1,
        .Direction = GPIO_PIN_OUT,
        .InitialLevel = GPIO_LEVEL_LOW,
        .Mode = GPIO_MODE_DIO,
        .InternalResistor = GPIO_RESISTOR_OFF,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 0,
        .DirectionChangeable = FALSE
    }
};

Gpio_Init(&GpioConfig);  /* Initialize GPIO first */
Led_Init(&Led_Red);      /* Then initialize LED */
```

---

## Memory Footprint

### Code Size
- **Led.h:** ~0.5 KB
- **Led.c:** ~1.0 KB
- **Led_Cfg.h:** ~0.3 KB
- **Total:** ~1.8 KB

### RAM Usage
- **Static variables:** 0 bytes (no static state)
- **Stack usage:** ~20-40 bytes per function call
- **Total:** Negligible

---

## Performance

### Execution Times (at 80 MHz)
- **Led_SetState():** ~50-100 ns (4-8 cycles)
- **Led_Toggle():** ~75-150 ns (6-12 cycles)
- **Led_GetState():** ~50-100 ns (4-8 cycles)
- **Led_Init():** ~100-200 ns (8-16 cycles)

### Thread Safety
- ✅ All functions use atomic GPIO operations
- ✅ Safe for use in interrupt handlers
- ✅ Safe for use in multi-threaded environments

---

## Best Practices

### ✅ DO:
- Configure GPIO pin as output before initializing LED
- Use consistent naming for LED configurations
- Handle return values and error conditions
- Test both ON and OFF states

### ❌ DON'T:
- Initialize LED before GPIO driver
- Pass NULL pointers to API functions
- Assume LED state without reading it
- Mix active-high and active-low without clear documentation

---

## Version History

### v1.0.0 (October 26, 2025)
- ✅ Complete LED driver implementation
- ✅ Active-high/active-low support
- ✅ DET integration and error handling
- ✅ Version information API
- ✅ Comprehensive documentation
- ✅ Example configurations for LaunchPad

---

## Support & Contact

For issues, questions, or contributions:
1. Review this documentation
2. Check example configurations in Led_PBCfg.c
3. Consult TM4C123GH6PM datasheet for hardware details
4. Contact: Mohamed Yasser

---

**End of Documentation**
