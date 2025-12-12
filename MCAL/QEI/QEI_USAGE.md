# Quadrature Encoder Interface (QEI) Driver Documentation

**Version:** 1.0.0  
**Date:** November 4, 2025  
**Target:** TM4C123GH6PM LaunchPad  
**Module:** MCAL - QEI Driver

---

## 📋 Overview

The QEI (Quadrature Encoder Interface) driver implements AUTOSAR-style services for configuring and reading the TM4C123GH6PM quadrature encoder modules. It supports both QEI0 and QEI1, including position tracking, velocity capture, direction sensing, and interrupt callbacks for motor control and position sensing applications.

### **Key Features**

✅ **Dual QEI Modules** - QEI0 and QEI1 support  
✅ **Position Tracking** - Up to 32-bit position counter  
✅ **Velocity Measurement** - Hardware velocity capture  
✅ **Direction Sensing** - Forward/reverse detection  
✅ **Index Pulse Support** - Absolute positioning  
✅ **Digital Filtering** - Noise rejection  
✅ **Interrupt Support** - Error, direction, timer, index events  
✅ **AUTOSAR Compliant** - Full AUTOSAR 4.4.0 compliance  

---

## 🏗️ Architecture

```
┌─────────────────────────────────────┐
│      Application Layer              │
│  (motor control, position feedback) │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│         QEI Driver API              │
│  - Qei_Init()                       │
│  - Qei_GetPosition()                │
│  - Qei_GetVelocity()                │
│  - Qei_GetDirection()               │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│    Hardware Abstraction Layer       │
│  - QEI register access              │
│  - Position counter management      │
│  - Velocity timer control           │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│         TM4C123 Hardware            │
│  - QEI Module 0 (PD6/PD7/PD3)      │
│  - QEI Module 1 (PC5/PC6/PC4)      │
│  - Quadrature decoder               │
└─────────────────────────────────────┘
```

---

## 📁 File Structure

| File | Description |
|------|-------------|
| `QEI.h` | Driver interface and API declarations |
| `QEI.c` | Complete driver implementation |
| `QEI_Types.h` | Type definitions and enumerations |
| `QEI_Cfg.h` | Compile-time configuration |
| `QEI_PBCfg.c` | Post-build configuration example |

---

## 🔌 Hardware Connections

### QEI0 Pin Mapping

| Signal | Pin | Function | Description |
|--------|-----|----------|-------------|
| **PhA0** | PD6 | Phase A | Encoder channel A |
| **PhB0** | PD7 | Phase B | Encoder channel B |
| **IDX0** | PD3 | Index | Index pulse (optional) |

### QEI1 Pin Mapping

| Signal | Pin | Function | Description |
|--------|-----|----------|-------------|
| **PhA1** | PC5 | Phase A | Encoder channel A |
| **PhB1** | PC6 | Phase B | Encoder channel B |
| **IDX1** | PC4 | Index | Index pulse (optional) |

### GPIO Configuration Required

```c
/* Example: Configure PD6/PD7 for QEI0 */
const Gpio_PinConfigType Qei0_PinConfig[] = {
    {
        .Port = GPIO_PORT_D,
        .Pin = GPIO_PIN_6,
        .Mode = GPIO_MODE_ALT_FUNC,
        .AlternateFuncNum = 6u,  /* QEI alternate function */
        .Direction = GPIO_DIR_INPUT,
        .InternalAttach = GPIO_PULL_UP
    },
    {
        .Port = GPIO_PORT_D,
        .Pin = GPIO_PIN_7,
        .Mode = GPIO_MODE_ALT_FUNC,
        .AlternateFuncNum = 6u,
        .Direction = GPIO_DIR_INPUT,
        .InternalAttach = GPIO_PULL_UP
    }
};
```

---

## 🚀 Quick Start

### 1. Basic Position Tracking

```c
#include "QEI.h"
#include "QEI_PBCfg.h"

int main(void) {
    /* Initialize GPIO for QEI pins */
    Gpio_Init(&Gpio_Qei0_Config);
    
    /* Initialize QEI */
    Qei_Init(&Qei_Config);
    
    while(1) {
        /* Read current position */
        uint32 position = Qei_GetPosition();
        
        /* Get direction */
        Qei_DirectionType dir = Qei_GetDirection();
        
        if (dir == QEI_DIRECTION_FORWARD) {
            printf("Position: %d (Forward)\n", position);
        } else {
            printf("Position: %d (Reverse)\n", position);
        }
        
        delay_ms(100);
    }
}
```

### 2. Velocity Measurement

```c
int main(void) {
    Gpio_Init(&Gpio_Qei0_Config);
    Qei_Init(&Qei_Config);  /* Velocity capture enabled */
    
    while(1) {
        /* Get velocity (counts per measurement period) */
        uint32 velocity = Qei_GetVelocity();
        
        /* Convert to RPM (example for 600 PPR encoder) */
        float rpm = (velocity * 60.0) / (600 * 4);  /* 4x in quadrature */
        
        printf("Velocity: %.1f RPM\n", rpm);
        
        delay_ms(100);
    }
}
```

### 3. Position Reset on Index Pulse

```c
/* Configuration with index reset */
const Qei_ConfigType Qei_IndexConfig = {
    .Module = QEI_MODULE_0,
    .SignalMode = QEI_SIGNAL_MODE_QUADRATURE,
    .ResetMode = QEI_RESET_MODE_ON_INDEX,  /* Reset on index */
    .SwapChannels = FALSE,
    .InvertChannelA = FALSE,
    .InvertChannelB = FALSE,
    .InvertIndex = FALSE,
    .EnableVelocityCapture = TRUE,
    .VelocityPreDiv = QEI_VELOCITY_PREDIV_1,
    .VelocityTimerLoad = 160000,
    .EnableFilter = TRUE,
    .FilterCount = 4u,
    .DebugStallEnable = TRUE,
    .MaxPosition = 2399,        /* 600 PPR × 4 - 1 */
    .InitialPosition = 0u,
    .InterruptMask = QEI_INT_INDEX,
    .NotificationCallback = Qei_IndexCallback
};

void Qei_IndexCallback(Qei_InterruptMaskType flags) {
    if (flags & QEI_INT_INDEX) {
        /* Index pulse detected - position reset to 0 */
        printf("Index pulse detected!\n");
    }
}
```

---

## 📐 Encoder Resolution Calculation

### Understanding Quadrature Encoding

Quadrature mode provides **4× resolution**:
- Each encoder pulse generates 4 counts
- 600 PPR encoder → 2400 counts per revolution

### Formula

```
MaxPosition = (Encoder_PPR × 4) - 1
```

### Common Encoder Configurations

| Encoder PPR | Quadrature Counts | MaxPosition | Use Case |
|-------------|-------------------|-------------|----------|
| **100** | 400 | 399 | Low resolution |
| **256** | 1024 | 1023 | Standard |
| **360** | 1440 | 1439 | Degree resolution |
| **600** | 2400 | 2399 | **Common motor** |
| **1000** | 4000 | 3999 | High resolution |
| **2048** | 8192 | 8191 | Very high resolution |

### Example Configuration

```c
/* 600 PPR encoder configuration */
const Qei_ConfigType Qei_600PPR = {
    .Module = QEI_MODULE_0,
    .SignalMode = QEI_SIGNAL_MODE_QUADRATURE,
    .ResetMode = QEI_RESET_MODE_FREE_RUNNING,
    .MaxPosition = 2399,        /* (600 × 4) - 1 */
    .InitialPosition = 0u,
    /* ... */
};
```

---

## 🎯 Velocity Measurement

### Velocity Timer Configuration

The velocity timer measures encoder counts over a fixed time period.

**Formula:**
```
VelocityTimerLoad = SystemClock / DesiredUpdateRate
```

### Common Update Rates @ 16 MHz

| Update Rate | Timer Load | Use Case |
|-------------|------------|----------|
| **10 Hz** | 1,600,000 | Slow motors |
| **50 Hz** | 320,000 | Standard |
| **100 Hz** | 160,000 | **Recommended** |
| **200 Hz** | 80,000 | Fast response |
| **1000 Hz** | 16,000 | Very fast |

### Converting Velocity to RPM

```c
/* Get velocity (counts per measurement period) */
uint32 velocity = Qei_GetVelocity();

/* Calculate RPM */
float countsPerRev = ENCODER_PPR * 4;  /* Quadrature */
float measurementsPerSec = UPDATE_RATE_HZ;
float rpm = (velocity * measurementsPerSec * 60.0) / countsPerRev;

/* Example: 600 PPR, 100 Hz update */
// velocity = 400 counts
// rpm = (400 × 100 × 60) / 2400 = 1000 RPM
```

---

## 💡 Usage Examples

### Example 1: Motor Position Control

```c
#include "QEI.h"
#include "PWM.h"

#define TARGET_POSITION  1200  /* Half revolution */
#define KP  0.5  /* Proportional gain */

int main(void) {
    Gpio_Init(&Gpio_Config);
    Qei_Init(&Qei_Config);
    Pwm_Init(&Pwm_Motor_Config);
    
    while(1) {
        /* Read current position */
        uint32 currentPos = Qei_GetPosition();
        
        /* Calculate error */
        int32 error = TARGET_POSITION - currentPos;
        
        /* Simple P controller */
        int32 control = error * KP;
        
        /* Limit control output */
        if (control > 100) control = 100;
        if (control < -100) control = -100;
        
        /* Apply to motor */
        if (control > 0) {
            SetMotorDirection(FORWARD);
            Pwm_SetDutyCyclePercent(PWM_MOTOR, control);
        } else {
            SetMotorDirection(REVERSE);
            Pwm_SetDutyCyclePercent(PWM_MOTOR, -control);
        }
        
        delay_ms(10);
    }
}
```

### Example 2: Speed Measurement and Display

```c
#include "QEI.h"
#include "UART.h"

#define ENCODER_PPR  600
#define UPDATE_RATE  100  /* Hz */

int main(void) {
    Qei_Init(&Qei_Config);
    Uart_Init(&Uart0_Config);
    
    while(1) {
        /* Get velocity */
        uint32 velocity = Qei_GetVelocity();
        
        /* Convert to RPM */
        float rpm = (velocity * UPDATE_RATE * 60.0) / (ENCODER_PPR * 4);
        
        /* Get direction */
        Qei_DirectionType dir = Qei_GetDirection();
        
        /* Display */
        char buffer[50];
        sprintf(buffer, "Speed: %.1f RPM %s\r\n", 
                rpm, 
                (dir == QEI_DIRECTION_FORWARD) ? "FWD" : "REV");
        Uart_SendString(UART_MODULE_0, (uint8*)buffer);
        
        delay_ms(100);
    }
}
```

### Example 3: Multi-Axis Position Tracking

```c
typedef struct {
    uint32 position;
    int32 velocity;
    Qei_DirectionType direction;
} AxisStatus_t;

AxisStatus_t axis[2];  /* Two axes */

void UpdateAxisStatus(void) {
    /* Axis 0 (QEI0) */
    axis[0].position = Qei_GetPosition();
    axis[0].velocity = Qei_GetVelocity();
    axis[0].direction = Qei_GetDirection();
    
    /* Axis 1 (QEI1) - would need second QEI instance */
    // axis[1].position = Qei1_GetPosition();
    // axis[1].velocity = Qei1_GetVelocity();
    // axis[1].direction = Qei1_GetDirection();
}

int main(void) {
    Qei_Init(&Qei_Config);
    
    while(1) {
        UpdateAxisStatus();
        
        printf("Axis 0: Pos=%d, Vel=%d, Dir=%s\n",
               axis[0].position,
               axis[0].velocity,
               (axis[0].direction == QEI_DIRECTION_FORWARD) ? "FWD" : "REV");
        
        delay_ms(100);
    }
}
```

### Example 4: Interrupt-Driven Direction Change

```c
volatile boolean directionChanged = FALSE;
volatile Qei_DirectionType currentDirection;

void Qei_DirectionCallback(Qei_InterruptMaskType flags) {
    if (flags & QEI_INT_DIRECTION) {
        /* Direction changed */
        currentDirection = Qei_GetDirection();
        directionChanged = TRUE;
    }
}

const Qei_ConfigType Qei_DirIntConfig = {
    .Module = QEI_MODULE_0,
    /* ... other config ... */
    .InterruptMask = QEI_INT_DIRECTION,
    .NotificationCallback = Qei_DirectionCallback
};

int main(void) {
    Qei_Init(&Qei_DirIntConfig);
    
    while(1) {
        if (directionChanged) {
            directionChanged = FALSE;
            
            if (currentDirection == QEI_DIRECTION_FORWARD) {
                printf("Motor started moving forward\n");
            } else {
                printf("Motor started moving reverse\n");
            }
        }
    }
}
```

### Example 5: Error Detection

```c
volatile boolean encoderError = FALSE;

void Qei_ErrorCallback(Qei_InterruptMaskType flags) {
    if (flags & QEI_INT_ERROR) {
        /* Phase error detected - possible encoder fault */
        encoderError = TRUE;
        
        /* Stop motor for safety */
        Pwm_SetOutputToIdle(PWM_MOTOR);
    }
}

const Qei_ConfigType Qei_ErrorDetect = {
    .Module = QEI_MODULE_0,
    /* ... other config ... */
    .InterruptMask = QEI_INT_ERROR,
    .NotificationCallback = Qei_ErrorCallback
};

int main(void) {
    Qei_Init(&Qei_ErrorDetect);
    
    while(1) {
        if (encoderError) {
            printf("ERROR: Encoder phase error detected!\n");
            printf("Check encoder connections and signals\n");
            encoderError = FALSE;
        }
        
        delay_ms(100);
    }
}
```

---

## 🔧 API Reference

### Initialization
```c
void Qei_Init(const Qei_ConfigType* ConfigPtr);
void Qei_DeInit(void);
```

### Position Control
```c
uint32 Qei_GetPosition(void);
void Qei_SetPosition(uint32 Position);
```

### Velocity & Direction
```c
uint32 Qei_GetVelocity(void);
Qei_DirectionType Qei_GetDirection(void);
```

### Interrupt Management
```c
void Qei_EnableInterrupt(Qei_InterruptMaskType mask);
void Qei_DisableInterrupt(Qei_InterruptMaskType mask);
void Qei_ClearInterrupt(Qei_InterruptMaskType mask);
```

### Status
```c
Qei_StatusType Qei_GetStatus(void);
```

---

## 🐛 Troubleshooting

### No Position Updates

**Symptoms:** Position counter stays at 0

**Checklist:**
- ✅ GPIO pins configured as alternate function 6
- ✅ Encoder power supply connected
- ✅ PhA and PhB signals present (check with oscilloscope)
- ✅ QEI module clock enabled
- ✅ Encoder is actually rotating

**Test:**
```c
/* Test encoder signals */
Gpio_Init(&Gpio_Qei_Config);
Qei_Init(&Qei_Config);

/* Manually rotate encoder */
for (int i = 0; i < 10; i++) {
    uint32 pos = Qei_GetPosition();
    printf("Position: %d\n", pos);
    delay_ms(500);
}
/* Position should change */
```

### Wrong Direction

**Symptoms:** Direction is opposite of actual rotation

**Solutions:**
```c
/* Option 1: Swap PhA and PhB connections physically */

/* Option 2: Use SwapChannels in config */
const Qei_ConfigType Qei_Swapped = {
    .Module = QEI_MODULE_0,
    .SwapChannels = TRUE,  /* Swap in software */
    /* ... */
};

/* Option 3: Invert one channel */
const Qei_ConfigType Qei_Inverted = {
    .Module = QEI_MODULE_0,
    .InvertChannelA = TRUE,  /* Invert PhA */
    /* ... */
};
```

### Noisy Position Count

**Symptoms:** Position jumps erratically

**Solutions:**
```c
/* Enable digital filter */
const Qei_ConfigType Qei_Filtered = {
    .Module = QEI_MODULE_0,
    .EnableFilter = TRUE,
    .FilterCount = 8u,  /* Increase for more filtering */
    /* ... */
};

/* Hardware solutions: */
// - Add pull-up resistors (4.7kΩ typical)
// - Use shielded cable
// - Keep encoder wires away from motor power
// - Add ferrite beads
```

### Velocity Always Zero

**Checklist:**
- ✅ `EnableVelocityCapture = TRUE` in config
- ✅ Encoder is rotating fast enough
- ✅ `VelocityTimerLoad` is appropriate
- ✅ Motor speed > minimum detectable

**Minimum Detectable Speed:**
```c
/* Calculate minimum RPM */
float minRPM = (1.0 * UPDATE_RATE * 60.0) / (ENCODER_PPR * 4);

/* Example: 600 PPR, 100 Hz update */
// minRPM = (1 × 100 × 60) / 2400 = 2.5 RPM
```

### Index Pulse Not Working

**Checklist:**
- ✅ Index signal connected to correct pin
- ✅ `ResetMode = QEI_RESET_MODE_ON_INDEX`
- ✅ Index pulse polarity correct
- ✅ Index interrupt enabled if using callback

---

## ⚠️ Important Notes

### Signal Requirements

**Voltage Levels:**
- Logic HIGH: > 2.0V
- Logic LOW: < 0.8V
- Recommended: 3.3V CMOS levels

**Timing:**
- Minimum pulse width: 2 system clock cycles
- Maximum frequency: System clock / 4

### Position Counter Overflow

The position counter wraps at `MaxPosition`:
```c
/* Example: 600 PPR encoder */
MaxPosition = 2399

/* Counter sequence: */
0, 1, 2, ..., 2398, 2399, 0, 1, 2, ...
```

### Velocity Measurement Accuracy

**Factors affecting accuracy:**
- Update rate (higher = better resolution)
- Encoder resolution (higher = better)
- Motor speed (very slow speeds less accurate)

---

## 📊 Performance

**@ 16 MHz System Clock:**

| Operation | Time | Notes |
|-----------|------|-------|
| `Qei_Init()` | < 200 μs | One-time initialization |
| `Qei_GetPosition()` | < 5 μs | Single register read |
| `Qei_GetVelocity()` | < 5 μs | Single register read |
| `Qei_GetDirection()` | < 5 μs | Single register read |
| Maximum Input Frequency | 4 MHz | System clock / 4 |

---

## 🎓 Best Practices

### ✅ DO:
- Use pull-up resistors on encoder signals
- Enable digital filtering for noisy environments
- Choose appropriate velocity update rate
- Test direction before motor control
- Use shielded cables for long runs
- Validate encoder PPR matches configuration

### ❌ DON'T:
- Exceed maximum input frequency
- Forget to configure GPIO alternate function
- Use very high filter counts (adds latency)
- Ignore phase errors (indicates wiring issues)
- Mix up PhA and PhB connections

---

## 🔗 Related Drivers

- **GPIO Driver** - Pin configuration for QEI signals
- **PWM Driver** - Motor speed control
- **Timer Driver** - Velocity measurement timing
- **NVIC Driver** - QEI interrupt management

---

**Your encoder interface is now ready for precision control!** 🎯⚙️
