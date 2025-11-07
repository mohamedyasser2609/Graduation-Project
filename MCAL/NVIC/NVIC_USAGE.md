# NVIC Driver Documentation

**Version:** 1.0.0  
**Date:** November 4, 2025  
**Target:** TM4C123GH6PM LaunchPad  
**Module:** MCAL - NVIC Driver

---

## 📋 Overview

The NVIC (Nested Vectored Interrupt Controller) driver provides an AUTOSAR-compliant abstraction for enabling, prioritizing, and managing interrupts on the TM4C123GH6PM. It configures vector priorities, handles pending/active statuses, and supports software-triggered interrupts.

### **Key Features**

✅ **Interrupt Management** - Enable/disable any interrupt vector  
✅ **Priority Control** - 8 priority levels with grouping support  
✅ **Pending/Active Status** - Query and control interrupt states  
✅ **Software Triggers** - Generate interrupts programmatically  
✅ **Priority Grouping** - Preemption and subpriority configuration  
✅ **AUTOSAR Compliant** - Full AUTOSAR 4.4.0 compliance  

---

## 🏗️ Architecture

```
┌─────────────────────────────────────┐
│      Application Layer              │
│  (ISRs, interrupt handlers)         │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│         NVIC Driver API             │
│  - NVIC_Init()                      │
│  - NVIC_EnableIRQ()                 │
│  - NVIC_SetPriority()               │
│  - NVIC_ClearPendingIRQ()           │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│    Hardware Abstraction Layer       │
│  - NVIC register access             │
│  - Priority encoding                │
│  - Vector table management          │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│      ARM Cortex-M4 NVIC             │
│  - 138 interrupt vectors            │
│  - 8-level priority system          │
│  - Nested interrupt support         │
└─────────────────────────────────────┘
```

---

## 📁 File Structure

| File | Description |
|------|-------------|
| `NVIC.h` | Driver interface and API declarations |
| `NVIC.c` | Complete driver implementation |
| `NVIC_Types.h` | Type definitions and vector enums |
| `NVIC_Cfg.h` | Compile-time configuration |
| `NVIC_PBCfg.c` | Post-build interrupt table |

---

## 🚀 Quick Start

### 1. Basic Initialization

```c
#include "NVIC.h"
#include "NVIC_PBCfg.h"

int main(void) {
    /* Initialize NVIC with configuration */
    NVIC_Init(&NVIC_Config);
    
    /* Your application code */
    while(1) {
        /* Interrupts are now active */
    }
}
```

### 2. Enable/Disable Interrupts

```c
/* Enable UART0 interrupt */
NVIC_EnableIRQ(NVIC_UART0_IRQ);

/* Disable UART0 interrupt */
NVIC_DisableIRQ(NVIC_UART0_IRQ);

/* Enable Timer0A interrupt */
NVIC_EnableIRQ(NVIC_TIMER0A_IRQ);
```

### 3. Set Interrupt Priority

```c
/* Set UART0 to highest priority (0) */
NVIC_SetPriority(NVIC_UART0_IRQ, 0);

/* Set Timer0A to medium priority (4) */
NVIC_SetPriority(NVIC_TIMER0A_IRQ, 4);

/* Set GPIO Port F to lowest priority (7) */
NVIC_SetPriority(NVIC_GPIOF_IRQ, 7);
```

### 4. Manage Pending Interrupts

```c
/* Set interrupt pending (trigger software interrupt) */
NVIC_SetPendingIRQ(NVIC_TIMER0A_IRQ);

/* Clear pending interrupt */
NVIC_ClearPendingIRQ(NVIC_TIMER0A_IRQ);

/* Check if interrupt is active */
if (NVIC_IsActiveIRQ(NVIC_UART0_IRQ)) {
    /* UART0 ISR is currently executing */
}
```

---

## 🎯 Priority Grouping Explained

The TM4C123 uses 3 bits for priority (8 levels: 0-7). Priority grouping determines how these bits are split between **preemption priority** and **subpriority**.

### Priority Group Options

| Group | Preempt Bits | Sub Bits | Preempt Levels | Sub Levels | Use Case |
|-------|--------------|----------|----------------|------------|----------|
| **NVIC_PRIGROUP_XXX** | 3 | 0 | 8 | 1 | **Most common** - Full preemption |
| NVIC_PRIGROUP_XXY | 2 | 1 | 4 | 2 | Balanced preemption/sub |
| NVIC_PRIGROUP_XYY | 1 | 2 | 2 | 4 | More subpriorities |
| NVIC_PRIGROUP_YYY | 0 | 3 | 1 | 8 | No preemption |

### What's the Difference?

**Preemption Priority:**
- Higher priority interrupts can interrupt lower priority ones
- Creates nested interrupts

**Subpriority:**
- Only matters when multiple interrupts are pending simultaneously
- Does NOT allow nesting

### Example Configuration

```c
/* Configuration in NVIC_PBCfg.c */
const NVIC_ConfigType NVIC_Config = {
    .PriorityGrouping = NVIC_PRIGROUP_XXX,  /* All preemption, no sub */
    .InterruptCount = 3,
    .Interrupts = NVIC_InterruptConfigTable
};

/* Interrupt table */
const NVIC_InterruptConfigEntryType NVIC_InterruptConfigTable[] = {
    {NVIC_UART0_IRQ,    0, TRUE},   /* Highest - can interrupt all */
    {NVIC_TIMER0A_IRQ,  2, TRUE},   /* Medium - can interrupt priority 3-7 */
    {NVIC_GPIOF_IRQ,    7, TRUE}    /* Lowest - cannot interrupt others */
};
```

---

## 📊 Common Interrupt Vectors

### Peripheral Interrupts

| Vector | IRQ Number | Description |
|--------|------------|-------------|
| `NVIC_UART0_IRQ` | 5 | UART0 RX/TX |
| `NVIC_UART1_IRQ` | 6 | UART1 RX/TX |
| `NVIC_TIMER0A_IRQ` | 19 | Timer 0A timeout |
| `NVIC_TIMER0B_IRQ` | 20 | Timer 0B timeout |
| `NVIC_TIMER1A_IRQ` | 21 | Timer 1A timeout |
| `NVIC_GPIOA_IRQ` | 0 | GPIO Port A |
| `NVIC_GPIOB_IRQ` | 1 | GPIO Port B |
| `NVIC_GPIOC_IRQ` | 2 | GPIO Port C |
| `NVIC_GPIOD_IRQ` | 3 | GPIO Port D |
| `NVIC_GPIOE_IRQ` | 4 | GPIO Port E |
| `NVIC_GPIOF_IRQ` | 30 | GPIO Port F |
| `NVIC_ADC0_SEQ0_IRQ` | 14 | ADC0 Sequence 0 |
| `NVIC_I2C0_IRQ` | 8 | I2C0 Master/Slave |
| `NVIC_PWM0_FAULT_IRQ` | 9 | PWM0 Fault |

---

## 💡 Usage Examples

### Example 1: UART Interrupt Setup

```c
#include "NVIC.h"
#include "UART.h"

/* UART0 ISR */
void UART0_Handler(void) {
    uint8 data;
    
    /* Clear UART interrupt */
    Uart_ClearInterrupt(UART_MODULE_0);
    
    /* Read received data */
    Uart_ReceiveByte(UART_MODULE_0, &data);
    
    /* Process data */
    ProcessReceivedData(data);
}

int main(void) {
    /* Initialize NVIC */
    NVIC_Init(&NVIC_Config);
    
    /* Set UART0 priority to high (1) */
    NVIC_SetPriority(NVIC_UART0_IRQ, 1);
    
    /* Enable UART0 interrupt */
    NVIC_EnableIRQ(NVIC_UART0_IRQ);
    
    /* Initialize UART with interrupts enabled */
    Uart_Init(&Uart0_IntConfig);
    
    while(1) {
        /* UART data handled in ISR */
    }
}
```

### Example 2: Timer Interrupt with Priority

```c
#include "NVIC.h"
#include "Timer.h"

volatile uint32 systemTick = 0;

/* Timer0A ISR - 1ms system tick */
void Timer0A_Handler(void) {
    /* Clear timer interrupt */
    Timer_ClearInterrupt(TIMER_MODULE_0, TIMER_BLOCK_A, TIMER_INT_TIMEOUT);
    
    /* Increment system tick */
    systemTick++;
}

int main(void) {
    /* Initialize NVIC */
    NVIC_Init(&NVIC_Config);
    
    /* Set Timer0A to highest priority (0) - critical timing */
    NVIC_SetPriority(NVIC_TIMER0A_IRQ, 0);
    
    /* Enable Timer0A interrupt */
    NVIC_EnableIRQ(NVIC_TIMER0A_IRQ);
    
    /* Initialize timer for 1ms periodic */
    Timer_Init(&Timer_1ms_Config);
    Timer_Start(TIMER_MODULE_0, TIMER_BLOCK_A);
    
    while(1) {
        /* Use systemTick for timing */
        if (systemTick >= 1000) {
            /* 1 second elapsed */
            systemTick = 0;
        }
    }
}
```

### Example 3: GPIO Interrupt (Button Press)

```c
#include "NVIC.h"
#include "GPIO.h"

volatile boolean buttonPressed = FALSE;

/* GPIO Port F ISR */
void GPIOF_Handler(void) {
    /* Clear GPIO interrupt */
    Gpio_ClearInterrupt(GPIO_PORT_F, GPIO_PIN_4);
    
    /* Set flag */
    buttonPressed = TRUE;
}

int main(void) {
    /* Initialize NVIC */
    NVIC_Init(&NVIC_Config);
    
    /* Set GPIO F to medium priority (4) */
    NVIC_SetPriority(NVIC_GPIOF_IRQ, 4);
    
    /* Enable GPIO F interrupt */
    NVIC_EnableIRQ(NVIC_GPIOF_IRQ);
    
    /* Initialize GPIO with interrupt on PF4 (SW1) */
    Gpio_Init(&Gpio_Button_Config);
    
    while(1) {
        if (buttonPressed) {
            buttonPressed = FALSE;
            /* Handle button press */
            ToggleLED();
        }
    }
}
```

### Example 4: Nested Interrupts

```c
/* High priority interrupt can interrupt low priority */

/* Priority 0 - Highest (critical timing) */
void Timer0A_Handler(void) {
    /* This can interrupt UART0_Handler */
    Timer_ClearInterrupt(TIMER_MODULE_0, TIMER_BLOCK_A, TIMER_INT_TIMEOUT);
    /* Critical timing code */
}

/* Priority 3 - Medium */
void UART0_Handler(void) {
    /* Timer0A can interrupt this ISR */
    Uart_ClearInterrupt(UART_MODULE_0);
    /* UART processing */
}

/* Priority 7 - Lowest */
void GPIOF_Handler(void) {
    /* Both Timer0A and UART0 can interrupt this */
    Gpio_ClearInterrupt(GPIO_PORT_F, GPIO_PIN_4);
    /* Button handling */
}

int main(void) {
    NVIC_Init(&NVIC_Config);
    
    /* Set priorities */
    NVIC_SetPriority(NVIC_TIMER0A_IRQ, 0);  /* Highest */
    NVIC_SetPriority(NVIC_UART0_IRQ, 3);    /* Medium */
    NVIC_SetPriority(NVIC_GPIOF_IRQ, 7);    /* Lowest */
    
    /* Enable all */
    NVIC_EnableIRQ(NVIC_TIMER0A_IRQ);
    NVIC_EnableIRQ(NVIC_UART0_IRQ);
    NVIC_EnableIRQ(NVIC_GPIOF_IRQ);
    
    while(1);
}
```

---

## 🔧 API Reference

### Initialization
```c
void NVIC_Init(const NVIC_ConfigType* ConfigPtr);
```

### Interrupt Control
```c
void NVIC_EnableIRQ(NVIC_InterruptVectorType Vector);
void NVIC_DisableIRQ(NVIC_InterruptVectorType Vector);
```

### Priority Management
```c
void NVIC_SetPriority(NVIC_InterruptVectorType Vector, NVIC_PriorityType Priority);
NVIC_PriorityType NVIC_GetPriority(NVIC_InterruptVectorType Vector);
```

### Pending/Active Control
```c
void NVIC_SetPendingIRQ(NVIC_InterruptVectorType Vector);
void NVIC_ClearPendingIRQ(NVIC_InterruptVectorType Vector);
boolean NVIC_IsActiveIRQ(NVIC_InterruptVectorType Vector);
```

### Software Interrupt
```c
void NVIC_GenerateSoftwareInterrupt(NVIC_InterruptVectorType Vector);
```

---

## 🐛 Troubleshooting

### Interrupt Not Firing

**Checklist:**
- ✅ `NVIC_EnableIRQ()` called for the vector
- ✅ Peripheral interrupt enabled in peripheral registers
- ✅ ISR function name matches vector table exactly
- ✅ Global interrupts not disabled (`PRIMASK` clear)
- ✅ Priority not masked by `BASEPRI`

**Example:**
```c
/* WRONG - ISR name mismatch */
void UART0_ISR(void) { }  /* Should be UART0_Handler */

/* CORRECT */
void UART0_Handler(void) { }
```

### Nested Interrupts Not Working

**Checklist:**
- ✅ Priority grouping allows preemption (`NVIC_PRIGROUP_XXX`)
- ✅ Interrupt priorities are different
- ✅ Higher priority interrupt is enabled
- ✅ Lower priority ISR is running when higher priority triggers

### Interrupt Firing Continuously

**Checklist:**
- ✅ Interrupt flag cleared in ISR
- ✅ Peripheral condition causing interrupt is resolved
- ✅ `NVIC_ClearPendingIRQ()` called if needed

**Example:**
```c
void UART0_Handler(void) {
    /* MUST clear interrupt source */
    Uart_ClearInterrupt(UART_MODULE_0);  /* Clear UART flag */
    
    /* Process data */
    uint8 data;
    Uart_ReceiveByte(UART_MODULE_0, &data);
}
```

### Wrong ISR Executing

**Checklist:**
- ✅ Vector number is correct
- ✅ ISR name matches startup code vector table
- ✅ No duplicate ISR definitions

---

## ⚠️ Important Notes

### ISR Naming Convention

ISR names **must** match the vector table in `startup_TM4C123.c`:

```c
/* Correct ISR names */
void UART0_Handler(void);
void Timer0A_Handler(void);
void GPIOF_Handler(void);

/* WRONG - will not be called */
void UART0_ISR(void);
void Timer0A_IRQHandler(void);
```

### Priority Values

- **0** = Highest priority
- **7** = Lowest priority
- Lower number = higher priority (can interrupt higher numbers)

### Critical Sections

To temporarily disable interrupts:
```c
/* Disable all interrupts */
__disable_irq();

/* Critical code */
criticalVariable++;

/* Re-enable interrupts */
__enable_irq();
```

---

## 📊 Performance

- **Enable/Disable**: < 10 CPU cycles
- **Set Priority**: < 15 CPU cycles
- **ISR Entry Overhead**: ~12 cycles (hardware stacking)
- **ISR Exit Overhead**: ~12 cycles (hardware unstacking)
- **Nested ISR Entry**: +6 cycles

---

## 🎓 Best Practices

### ✅ DO:
- Clear interrupt flags in ISR
- Keep ISRs short and fast
- Use priorities appropriately
- Test interrupt nesting scenarios
- Document ISR execution time

### ❌ DON'T:
- Use blocking delays in ISRs
- Perform heavy processing in ISRs
- Forget to clear interrupt sources
- Use same priority for all interrupts
- Call non-reentrant functions from ISRs

---

## 📚 Related Drivers

- **GPIO Driver** - GPIO pin interrupts
- **UART Driver** - Serial communication interrupts
- **Timer Driver** - Periodic and one-shot interrupts
- **ADC Driver** - Conversion complete interrupts

---

**Your interrupts are now properly configured!** ⚡🎯
