# JTAG Pin Protection Guide

## Overview

The TM4C123GH6PM GPIO driver includes **automatic protection** for JTAG/SWD pins (PC0-PC3) to prevent accidental loss of debug capability.

---

## Protected Pins

| Pin | Function | Description |
|-----|----------|-------------|
| **PC0** | TCK/SWCLK | JTAG Test Clock / Serial Wire Clock |
| **PC1** | TMS/SWDIO | JTAG Test Mode Select / Serial Wire Data I/O |
| **PC2** | TDI | JTAG Test Data In |
| **PC3** | TDO/SWO | JTAG Test Data Out / Serial Wire Output |

**WARNING:** Modifying these pins will disable debugging and programming capabilities!

---

## Protection Mechanisms

### 1. Unlock Function Protection ✅

The `Gpio_UnlockPin()` function **explicitly refuses** to unlock JTAG pins:

```c
static void Gpio_UnlockPin(Gpio_PortType Port, Gpio_PinType Pin)
{
    /* CRITICAL: Never unlock JTAG pins (PC0-PC3) - this would disable debugging! */
    if (Port == GPIO_PORT_C && Pin <= GPIO_PIN_3) {
        return;  /* JTAG pins must remain locked */
    }
    
    /* Only PF0 and PD7 can be unlocked */
    if ((Port == GPIO_PORT_F && Pin == GPIO_PIN_0) ||
        (Port == GPIO_PORT_D && Pin == GPIO_PIN_7)) {
        /* Unlock sequence... */
    }
}
```

### 2. Configuration Function Protection ✅

The `Gpio_ConfigurePin()` function **rejects** any attempt to configure JTAG pins:

```c
static void Gpio_ConfigurePin(const Gpio_PinConfigType* PinConfig)
{
    /* CRITICAL: Protect JTAG pins (PC0-PC3) from configuration */
    if (PinConfig->Port == GPIO_PORT_C && PinConfig->Pin <= GPIO_PIN_3) {
#if (GPIO_DEV_ERROR_DETECT == STD_ON)
        Det_ReportError(GPIO_MODULE_ID, GPIO_INSTANCE_ID, 
                       GPIO_INIT_SID, GPIO_E_JTAG_PIN_PROTECTED);
#endif
        return;  /* Skip JTAG pin configuration */
    }
    
    /* Continue with normal configuration... */
}
```

### 3. DET Error Reporting ✅

If JTAG pins are included in configuration, a DET error is reported:

- **Error Code:** `GPIO_E_JTAG_PIN_PROTECTED` (0x14)
- **Description:** Attempted to configure JTAG pins (PC0-PC3) which are protected
- **Action:** Configuration is skipped, pin remains in default JTAG state

---

## What Happens if You Try to Configure JTAG Pins?

### Example Configuration (WRONG - Will Be Rejected):

```c
const Gpio_PinConfigType Gpio_PinConfigs[] = {
    {
        .Port = GPIO_PORT_C,
        .Pin = GPIO_PIN_0,           // JTAG TCK pin
        .Direction = GPIO_PIN_OUT,
        .Mode = GPIO_MODE_DIO,
        // ... rest of config
    }
};
```

### Result:

1. ✅ `Gpio_Init()` is called
2. ✅ Driver attempts to configure PC0
3. ✅ `Gpio_ConfigurePin()` detects JTAG pin
4. ✅ DET error `GPIO_E_JTAG_PIN_PROTECTED` is reported
5. ✅ Configuration is **skipped**
6. ✅ PC0 remains in JTAG mode (debugging preserved)
7. ✅ Other pins are configured normally

---

## Safe Pins on Port C

Only **PC4-PC7** can be safely configured as GPIO:

```c
const Gpio_PinConfigType Gpio_PinConfigs[] = {
    // SAFE - PC4 is not a JTAG pin
    {
        .Port = GPIO_PORT_C,
        .Pin = GPIO_PIN_4,
        .Direction = GPIO_PIN_OUT,
        .Mode = GPIO_MODE_DIO,
        // ... rest of config
    },
    
    // SAFE - PC5 is not a JTAG pin
    {
        .Port = GPIO_PORT_C,
        .Pin = GPIO_PIN_5,
        .Direction = GPIO_PIN_IN,
        .Mode = GPIO_MODE_DIO,
        // ... rest of config
    }
};
```

---

## Why This Protection Exists

### Without Protection:
1. User accidentally configures PC0-PC3 as GPIO
2. JTAG/SWD functionality is lost
3. **Cannot debug or reprogram the device**
4. Device becomes "bricked" (requires recovery procedure)

### With Protection:
1. User accidentally includes PC0-PC3 in configuration
2. Driver detects and rejects the configuration
3. DET error is reported (if enabled)
4. **JTAG/SWD remains functional**
5. Debugging and programming continue to work

---

## Recovery Procedure (If JTAG is Disabled)

If JTAG pins were previously disabled (before this protection was added):

### Method 1: Mass Erase
1. Connect debugger
2. Hold RESET button
3. Perform mass erase in CCS
4. Release RESET
5. JTAG functionality restored

### Method 2: Boot from ROM
1. Connect BOOT0 pin to VCC
2. Power cycle device
3. Device boots from ROM bootloader
4. Use LM Flash Programmer to erase
5. JTAG functionality restored

---

## Advanced: Intentionally Using JTAG Pins as GPIO

**⚠️ WARNING: This will disable debugging!**

If you **absolutely must** use JTAG pins as GPIO (not recommended):

1. **Modify the driver** to remove protection (not recommended)
2. **Use a different microcontroller** with more GPIO pins
3. **Use an external GPIO expander** (recommended)

### To Remove Protection (NOT RECOMMENDED):

```c
// In Gpio.c - Gpio_ConfigurePin()
// Comment out or remove this block:
/*
if (PinConfig->Port == GPIO_PORT_C && PinConfig->Pin <= GPIO_PIN_3) {
    return;  // Skip JTAG pin configuration
}
*/
```

**CONSEQUENCE:** You will lose the ability to debug and reprogram via JTAG/SWD!

---

## Best Practices

### ✅ DO:
- Use PC4-PC7 for GPIO on Port C
- Leave PC0-PC3 alone (JTAG functionality)
- Use other ports (A, B, D, E, F) for GPIO
- Keep JTAG protection enabled

### ❌ DON'T:
- Try to configure PC0-PC3 as GPIO
- Remove JTAG protection unless absolutely necessary
- Disable JTAG without a recovery plan

---

## Pin Availability Summary

| Port | Total Pins | Available for GPIO | Protected Pins |
|------|-----------|-------------------|----------------|
| A | 8 | 8 | None |
| B | 8 | 8 | None |
| C | 8 | 4 (PC4-PC7) | **PC0-PC3 (JTAG)** |
| D | 8 | 8 | PD7 (locked, but can unlock) |
| E | 6 | 6 | None |
| F | 5 | 5 | PF0 (locked, but can unlock) |

**Total Available GPIO:** 43 pins (excluding JTAG)

---

## Testing JTAG Protection

### Test Case 1: Verify Protection

```c
const Gpio_PinConfigType TestConfig[] = {
    {
        .Port = GPIO_PORT_C,
        .Pin = GPIO_PIN_0,  // JTAG pin
        .Direction = GPIO_PIN_OUT,
        .Mode = GPIO_MODE_DIO,
        .InternalResistor = GPIO_RESISTOR_OFF,
        .DriveStrength = GPIO_DRIVE_2MA,
        .SlewRate = GPIO_SLEW_RATE_NORMAL,
        .IntTrigger = GPIO_INT_DISABLED,
        .AlternateFuncNum = 0,
        .DirectionChangeable = FALSE
    }
};

const Gpio_ConfigType TestGpioConfig = {
    .PinConfigs = TestConfig,
    .NumberOfPins = 1
};

// Initialize
Gpio_Init(&TestGpioConfig);

// Expected Result:
// - DET error GPIO_E_JTAG_PIN_PROTECTED is reported
// - PC0 remains in JTAG mode
// - Debugging still works
```

### Test Case 2: Verify Safe Pins Work

```c
const Gpio_PinConfigType TestConfig[] = {
    {
        .Port = GPIO_PORT_C,
        .Pin = GPIO_PIN_4,  // Safe pin
        .Direction = GPIO_PIN_OUT,
        .Mode = GPIO_MODE_DIO,
        // ... rest of config
    }
};

// Initialize
Gpio_Init(&TestGpioConfig);

// Expected Result:
// - No error
// - PC4 configured as GPIO
// - Can read/write PC4 normally
```

---

## Conclusion

The GPIO driver provides **automatic, fail-safe protection** for JTAG pins to prevent accidental loss of debug capability. This protection is:

- ✅ **Automatic** - No user action required
- ✅ **Transparent** - Silently skips JTAG pins
- ✅ **Reported** - DET error for debugging
- ✅ **Safe** - Preserves JTAG functionality
- ✅ **Tested** - Verified in driver implementation

**Your debugging capability is protected!** 🛡️

---

## References

1. **TM4C123GH6PM Datasheet** - Section 10.2.4 (Commit Control)
2. **Tiva C Series User Guide** - JTAG/SWD Interface
3. **GPIO Driver Implementation** - `Gpio.c` lines 124-157, 186-198

---

**Author:** Mohamed Yasser  
**Date:** October 26, 2025  
**Version:** 1.1.0
