# Motor Driver Documentation

**Cytron MDD10A Rev2.0 Dual Motor Controller Driver**

---

## Overview

AUTOSAR-compliant motor driver for the Cytron MDD10A dual-channel motor controller, providing PWM speed control and GPIO direction control.

### Features
- Dual independent motor channels (Left/Right)
- PWM speed control (0-100%)
- 4 operation modes: Forward, Reverse, Brake, Coast
- Speed limiting (Min/Max)
- Direction inversion support
- Emergency stop all

---

## Hardware Configuration

### Cytron MDD10A Connections

| MDD10A Pin | TM4C Pin | Function |
|------------|----------|----------|
| PWM1 | PWM Channel (M0PWM0-M1PWM7) | Motor 1 speed |
| DIR1 | GPIO Output | Motor 1 direction |
| PWM2 | PWM Channel (M0PWM0-M1PWM7) | Motor 2 speed |
| DIR2 | GPIO Output | Motor 2 direction |
| GND | GND | Common ground |
| VIN | 5-25V | Motor power supply |

### Direction Logic

| DIR Pin | Motor Rotation |
|---------|----------------|
| LOW (0) | Forward |
| HIGH (1) | Reverse |

> **Note**: Use `InvertDirection = TRUE` in config if wiring is reversed.

---

## API Reference

### Initialization

```c
/* Initialize motor driver */
void Motor_Init(const Motor_ConfigType* ConfigPtr);

/* De-initialize motor driver */
void Motor_DeInit(void);  /* If MOTOR_DE_INIT_API == STD_ON */
```

### Speed Control

```c
/* Set motor speed (0-100%) */
Std_ReturnType Motor_SetSpeed(Motor_ChannelType Channel, Motor_SpeedType SpeedPercent);
```

### Direction Control

```c
/* Set motor direction */
Std_ReturnType Motor_SetDirection(Motor_ChannelType Channel, Motor_DirectionType Direction);

/* Direction options: */
/* MOTOR_DIRECTION_FORWARD  - Forward rotation */
/* MOTOR_DIRECTION_REVERSE  - Reverse rotation */
/* MOTOR_DIRECTION_BRAKE    - Active brake (short circuit) */
/* MOTOR_DIRECTION_COAST    - Coast (high impedance) */
```

### Combined Control

```c
/* Set speed and direction in one call */
Std_ReturnType Motor_SetSpeedAndDirection(Motor_ChannelType Channel, 
                                           Motor_SpeedType SpeedPercent, 
                                           Motor_DirectionType Direction);
```

### Stop Functions

```c
/* Stop single motor (brake mode) */
Std_ReturnType Motor_Stop(Motor_ChannelType Channel);

/* Emergency stop all motors */
void Motor_StopAll(void);
```

### Status

```c
/* Get motor status */
Motor_StatusType Motor_GetStatus(Motor_ChannelType Channel);
/* Returns: MOTOR_STATUS_UNINIT, MOTOR_STATUS_IDLE, MOTOR_STATUS_RUNNING, MOTOR_STATUS_ERROR */

/* Get all motor data */
Std_ReturnType Motor_GetData(Motor_ChannelType Channel, Motor_DataType* DataPtr);
```

---

## Usage Example

```c
#include "ECUAL/MOTOR/MOTOR.h"

extern const Motor_ConfigType Motor_Configuration;

void main(void) {
    /* Initialize motors */
    Motor_Init(&Motor_Configuration);
    
    /* Drive forward at 75% speed */
    Motor_SetSpeedAndDirection(MOTOR_CHANNEL_LEFT, 75, MOTOR_DIRECTION_FORWARD);
    Motor_SetSpeedAndDirection(MOTOR_CHANNEL_RIGHT, 75, MOTOR_DIRECTION_FORWARD);
    
    /* Delay... */
    
    /* Turn right (left faster than right) */
    Motor_SetSpeed(MOTOR_CHANNEL_LEFT, 80);
    Motor_SetSpeed(MOTOR_CHANNEL_RIGHT, 40);
    
    /* Delay... */
    
    /* Stop all motors */
    Motor_StopAll();
}
```

### Differential Drive Example

```c
void DifferentialDrive(sint8 linear, sint8 angular) {
    sint16 left = linear + angular;
    sint16 right = linear - angular;
    
    /* Clamp to 0-100 */
    left = (left < 0) ? 0 : (left > 100) ? 100 : left;
    right = (right < 0) ? 0 : (right > 100) ? 100 : right;
    
    Motor_SetSpeed(MOTOR_CHANNEL_LEFT, (Motor_SpeedType)left);
    Motor_SetSpeed(MOTOR_CHANNEL_RIGHT, (Motor_SpeedType)right);
}
```

---

## Configuration Structure

```c
typedef struct {
    Motor_ChannelType ChannelId;        /* MOTOR_CHANNEL_LEFT or MOTOR_CHANNEL_RIGHT */
    Pwm_ChannelType PwmChannel;         /* PWM channel for speed */
    Gpio_ChannelType DirectionPin;      /* GPIO pin for direction */
    boolean InvertDirection;            /* Invert direction logic */
    Motor_SpeedType MaxSpeedPercent;    /* Maximum speed limit (0-100) */
    Motor_SpeedType MinSpeedPercent;    /* Minimum speed threshold */
} Motor_ChannelConfigType;
```

---

## Safety Notes

1. **Always call `Motor_StopAll()` in error handlers**
2. **Use speed limits to protect mechanical systems**
3. **Ensure proper motor power supply (separate from MCU)**
4. **Common GND between MCU and motor driver is critical**

---

## Version Information

| Item | Value |
|------|-------|
| Module ID | MOTOR (300) |
| Driver Version | 1.0.0 |
| AUTOSAR Version | 4.4.0 |
| Hardware | Cytron MDD10A Rev2.0 |
