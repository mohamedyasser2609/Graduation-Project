# PWM Driver Documentation

**Version:** 1.0.0  
**Date:** October 26, 2025  
**Target:** TM4C123GH6PM LaunchPad  
**Module:** MCAL - PWM Driver

---

## 📋 Overview

The PWM (Pulse Width Modulation) driver provides hardware-based PWM signal generation for the TM4C123GH6PM microcontroller. This AUTOSAR-compliant driver enables precise control of LED brightness, motor speed, servo position, and other PWM-controlled devices.

### **Key Features**

✅ **16 PWM Channels** - 2 modules × 4 generators × 2 outputs  
✅ **Configurable Frequency** - From Hz to MHz range  
✅ **Precise Duty Cycle Control** - 0-100% with high resolution  
✅ **Hardware Generation** - No CPU overhead during operation  
✅ **Independent Channels** - Each channel fully configurable  
✅ **Multiple Modes** - Variable period, fixed period, phase-shifted  
✅ **Helper Functions** - Percentage-based and frequency control  

---

## 🏗️ Architecture

```
┌─────────────────────────────────────┐
│      Application Layer              │
│  (LED brightness, motor control)    │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│         PWM Driver API              │
│  - Pwm_Init()                       │
│  - Pwm_SetDutyCycle()               │
│  - Pwm_SetDutyCyclePercent()        │
│  - Pwm_SetFrequency()               │
│  - Pwm_SetPeriodAndDuty()           │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│    Hardware Abstraction Layer       │
│  - PWM register access              │
│  - Clock configuration              │
│  - Generator setup                  │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│         TM4C123 Hardware            │
│  - PWM Module 0 (4 generators)     │
│  - PWM Module 1 (4 generators)     │
│  - GPIO alternate functions         │
└─────────────────────────────────────┘
```

---

## 📁 File Structure

| File | Lines | Description |
|------|-------|-------------|
| `PWM.h` | 308 | Driver interface and type definitions |
| `PWM.c` | 501 | Complete hardware implementation |
| `PWM_Cfg.h` | 84 | Compile-time configuration |
| `PWM_PBCfg.c` | 114 | Post-build configuration (RGB LEDs) |

**Total:** ~1000 lines of production-ready code

---

## 🎨 RGB LED PWM Mapping

### **TM4C123 LaunchPad RGB LED**

| LED Color | Pin | PWM Signal | Module | Generator | Output |
|-----------|-----|------------|--------|-----------|--------|
| **Red** | PF1 | M1PWM5 | PWM1 | Gen 2 | Output B |
| **Blue** | PF2 | M1PWM6 | PWM1 | Gen 3 | Output A |
| **Green** | PF3 | M1PWM7 | PWM1 | Gen 3 | Output B |

### **GPIO Configuration Required**

```c
/* PF1/PF2/PF3 must be configured as alternate function 5 */
.Mode = GPIO_MODE_ALT_FUNC
.AlternateFuncNum = 5u
```

---

## 🔧 Configuration

### **Compile-Time Configuration (PWM_Cfg.h)**

```c
#define PWM_DEV_ERROR_DETECT            FALSE
#define PWM_VERSION_INFO_API            TRUE
#define PWM_DE_INIT_API                 TRUE
#define PWM_SET_DUTY_CYCLE_API          TRUE
#define PWM_SET_PERIOD_AND_DUTY_API     TRUE
#define PWM_SET_OUTPUT_TO_IDLE_API      TRUE
#define PWM_GET_OUTPUT_STATE_API        TRUE
#define PWM_NOTIFICATION_SUPPORTED      FALSE

#define PWM_DEFAULT_FREQUENCY_HZ        1000u    /* 1 kHz */
#define PWM_CLOCK_DIVIDER               64u      /* System clock / 64 */
#define PWM_MAX_CHANNELS                8u
```

### **Clock Configuration**

```
System Clock: 16 MHz
PWM Clock Divider: 64
PWM Clock: 16 MHz / 64 = 250 kHz

For 1 kHz PWM:
Period = 250 kHz / 1 kHz = 250 ticks
```

### **Runtime Configuration Example**

```c
const Pwm_ConfigChannelType Pwm_RedLed = {
    .Channel = 0,                       /* Channel ID */
    .Module = PWM_MODULE_1,             /* PWM Module 1 */
    .Generator = PWM_GEN_2,             /* Generator 2 */
    .Output = PWM_OUT_B,                /* Output B */
    .ChannelClass = PWM_FIXED_PERIOD,   /* Fixed period */
    .Polarity = PWM_HIGH_POLARITY,      /* Active high */
    .IdleState = PWM_IDLE_LOW,          /* Idle = LOW */
    .DefaultPeriod = 250u,              /* 1 kHz */
    .DefaultDutyCycle = 0x0000,         /* 0% initially */
    .NotificationPtr = NULL_PTR
};
```

---

## 🚀 API Reference

### **Initialization**

#### `Pwm_Init()`
```c
void Pwm_Init(const Pwm_ConfigType* ConfigPtr);
```
**Description:** Initializes all configured PWM channels  
**Parameters:** ConfigPtr - Pointer to configuration structure  
**Returns:** void  
**Example:**
```c
Pwm_Init(&Pwm_Configuration);
```

---

### **Duty Cycle Control**

#### `Pwm_SetDutyCycle()`
```c
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, Pwm_DutyCycleType DutyCycle);
```
**Description:** Sets duty cycle using 16-bit value  
**Parameters:**  
- ChannelNumber - PWM channel (0-7)
- DutyCycle - 0x0000 = 0%, 0x8000 = 100%

**Returns:** void  
**Example:**
```c
Pwm_SetDutyCycle(PWM_CHANNEL_RED, 0x4000);  /* 50% */
```

#### `Pwm_SetDutyCyclePercent()` ⭐
```c
void Pwm_SetDutyCyclePercent(Pwm_ChannelType ChannelNumber, uint8 Percentage);
```
**Description:** Sets duty cycle using percentage (0-100)  
**Parameters:**  
- ChannelNumber - PWM channel
- Percentage - 0 to 100

**Returns:** void  
**Example:**
```c
Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, 75);  /* 75% brightness */
```

---

### **Frequency Control**

#### `Pwm_SetFrequency()` ⭐
```c
void Pwm_SetFrequency(Pwm_ChannelType ChannelNumber, uint16 FrequencyHz);
```
**Description:** Sets PWM frequency in Hz (variable period channels only)  
**Parameters:**  
- ChannelNumber - PWM channel
- FrequencyHz - Frequency in Hz

**Returns:** void  
**Example:**
```c
Pwm_SetFrequency(PWM_CHANNEL_RED, 2000);  /* 2 kHz */
```

#### `Pwm_SetPeriodAndDuty()`
```c
void Pwm_SetPeriodAndDuty(Pwm_ChannelType ChannelNumber, 
                          Pwm_PeriodType Period, 
                          Pwm_DutyCycleType DutyCycle);
```
**Description:** Sets both period and duty cycle  
**Parameters:**  
- ChannelNumber - PWM channel
- Period - Period in ticks
- DutyCycle - 0x0000 to 0x8000

**Returns:** void  

---

### **Control Functions**

#### `Pwm_SetOutputToIdle()`
```c
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber);
```
**Description:** Sets output to idle state (disables PWM)  
**Parameters:** ChannelNumber - PWM channel  
**Returns:** void  

#### `Pwm_GetOutputState()`
```c
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType ChannelNumber);
```
**Description:** Gets current output state  
**Parameters:** ChannelNumber - PWM channel  
**Returns:** PWM_LOW or PWM_HIGH  

---

## 💡 Usage Examples

### **Example 1: LED Brightness Control**

```c
#include "PWM.h"

#define PWM_CHANNEL_RED     0
#define PWM_CHANNEL_BLUE    1
#define PWM_CHANNEL_GREEN   2

int main(void) {
    /* Initialize GPIO for PWM */
    Gpio_Init(&Gpio_Configuration);
    
    /* Initialize PWM */
    Pwm_Init(&Pwm_Configuration);
    
    /* Set Red LED to 50% brightness */
    Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, 50);
    
    /* Set Blue LED to 75% brightness */
    Pwm_SetDutyCyclePercent(PWM_CHANNEL_BLUE, 75);
    
    /* Set Green LED to 25% brightness */
    Pwm_SetDutyCyclePercent(PWM_CHANNEL_GREEN, 25);
    
    while(1) {
        /* LEDs maintain their brightness */
    }
}
```

### **Example 2: Breathing LED Effect**

```c
void BreathingEffect(void) {
    uint8 brightness;
    
    /* Fade in */
    for (brightness = 0; brightness <= 100; brightness++) {
        Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, brightness);
        delay_ms(10);  /* 1 second fade in */
    }
    
    /* Fade out */
    for (brightness = 100; brightness > 0; brightness--) {
        Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, brightness);
        delay_ms(10);  /* 1 second fade out */
    }
}
```

### **Example 3: RGB Color Mixing**

```c
void SetRGBColor(uint8 red, uint8 green, uint8 blue) {
    /* Set each LED to specified brightness (0-100) */
    Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, red);
    Pwm_SetDutyCyclePercent(PWM_CHANNEL_GREEN, green);
    Pwm_SetDutyCyclePercent(PWM_CHANNEL_BLUE, blue);
}

/* Create custom colors */
SetRGBColor(100, 0, 0);      /* Pure Red */
SetRGBColor(0, 100, 0);      /* Pure Green */
SetRGBColor(0, 0, 100);      /* Pure Blue */
SetRGBColor(100, 100, 0);    /* Yellow */
SetRGBColor(100, 0, 100);    /* Magenta */
SetRGBColor(0, 100, 100);    /* Cyan */
SetRGBColor(100, 50, 0);     /* Orange */
SetRGBColor(50, 0, 100);     /* Purple */
```

### **Example 4: Smooth Color Transitions**

```c
void FadeToColor(uint8 targetR, uint8 targetG, uint8 targetB) {
    uint8 currentR = 0, currentG = 0, currentB = 0;
    uint8 steps = 50;
    uint8 i;
    
    for (i = 0; i <= steps; i++) {
        currentR = (targetR * i) / steps;
        currentG = (targetG * i) / steps;
        currentB = (targetB * i) / steps;
        
        SetRGBColor(currentR, currentG, currentB);
        delay_ms(20);  /* 1 second total transition */
    }
}
```

### **Example 5: UART-Controlled Brightness**

```c
void ProcessBrightnessCommand(uint8 command) {
    switch(command) {
        case '0': Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, 0); break;
        case '1': Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, 10); break;
        case '2': Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, 20); break;
        case '3': Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, 30); break;
        case '4': Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, 40); break;
        case '5': Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, 50); break;
        case '6': Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, 60); break;
        case '7': Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, 70); break;
        case '8': Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, 80); break;
        case '9': Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, 90); break;
        case 'f': Pwm_SetDutyCyclePercent(PWM_CHANNEL_RED, 100); break;
    }
}
```

---

## 🎯 PWM Channel Classes

### **PWM_VARIABLE_PERIOD**
- Both period and duty cycle can be changed
- Use for applications requiring frequency changes
- Example: Variable frequency motor control

### **PWM_FIXED_PERIOD**
- Only duty cycle can be changed
- Period is fixed at initialization
- Example: LED brightness control (most common)

### **PWM_FIXED_PERIOD_SHIFTED**
- Fixed period with phase shift capability
- Used for multi-phase applications
- Example: Stepper motor control

---

## ⚙️ Duty Cycle Formats

### **16-bit Format (0x0000 - 0x8000)**
```c
0x0000 = 0%
0x2000 = 25%
0x4000 = 50%
0x6000 = 75%
0x8000 = 100%
```

### **Percentage Format (0 - 100)**
```c
Pwm_SetDutyCyclePercent(channel, 0);    /* 0% */
Pwm_SetDutyCyclePercent(channel, 25);   /* 25% */
Pwm_SetDutyCyclePercent(channel, 50);   /* 50% */
Pwm_SetDutyCyclePercent(channel, 75);   /* 75% */
Pwm_SetDutyCyclePercent(channel, 100);  /* 100% */
```

---

## 📊 Frequency Ranges

**@ 16 MHz System Clock, Divider = 64:**

| PWM Clock | Min Frequency | Max Frequency | Typical Use |
|-----------|---------------|---------------|-------------|
| 250 kHz | 4 Hz | 125 kHz | LED dimming |
| 250 kHz | 100 Hz | 10 kHz | Motor control |
| 250 kHz | 50 Hz | 400 Hz | Servo control |
| 250 kHz | 1 kHz | 20 kHz | Audio PWM |

**Frequency Calculation:**
```
Frequency = PWM_Clock / Period
Period = PWM_Clock / Desired_Frequency
```

---

## 🐛 Troubleshooting

### **No PWM Output**
- ✅ Check GPIO configured as alternate function 5
- ✅ Verify PWM module clock is enabled
- ✅ Ensure duty cycle > 0%
- ✅ Check PWM output is enabled

### **Wrong Frequency**
- ✅ Verify PWM clock divider setting
- ✅ Check period calculation
- ✅ Ensure channel class is VARIABLE_PERIOD

### **Flickering LED**
- ✅ Increase PWM frequency (> 100 Hz)
- ✅ Check for interference
- ✅ Verify stable power supply

### **Uneven Brightness**
- ✅ Use higher PWM frequency
- ✅ Check duty cycle resolution
- ✅ Verify linear brightness mapping

---

## 📈 Performance

**@ 16 MHz System Clock:**

| Operation | Time | Notes |
|-----------|------|-------|
| Pwm_Init() | < 1 ms | One-time initialization |
| Pwm_SetDutyCycle() | < 10 μs | Register write only |
| Pwm_SetDutyCyclePercent() | < 15 μs | Includes calculation |
| Pwm_SetFrequency() | < 20 μs | Includes period calc |
| PWM Signal Generation | 0 CPU | Hardware-based |

---

## 🎓 Best Practices

1. **Initialize GPIO first** before PWM
2. **Use percentage functions** for easier control
3. **Choose appropriate frequency** for application
4. **Start with 0% duty cycle** to avoid glitches
5. **Use FIXED_PERIOD** for LED control
6. **Higher frequency** = smoother dimming
7. **Lower frequency** = better efficiency

---

## 🔗 Applications

### **LED Control**
- Brightness dimming
- RGB color mixing
- Breathing effects
- Status indicators

### **Motor Control**
- DC motor speed control
- Servo position control
- Stepper motor driving
- Fan speed control

### **Audio**
- Simple tone generation
- Buzzer control
- Audio PWM DAC

### **Power Control**
- Switching regulators
- Heater control
- Dimmer circuits

---

## 📚 Related Drivers

- **GPIO Driver** - Pin configuration for PWM
- **Timer Driver** - For PWM timing effects
- **ADC Driver** - Feedback control systems

---

## 📝 Revision History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | Oct 26, 2025 | Initial release with RGB LED support |

---

**End of PWM Driver Documentation**
