# Encoder Driver for EMG49 Motor Encoder

## Overview

High-level encoder driver for the EMG49 motor encoder (~980 counts/rev in quadrature) that wraps the QEI driver and provides convenient functions for position and velocity measurement in various units.

## Features

- ✅ Position reading in counts, revolutions, or degrees
- ✅ Velocity reading in counts/second or RPM
- ✅ Direction detection (forward/reverse)
- ✅ Position reset and calibration
- ✅ Velocity filtering (exponential moving average)
- ✅ AUTOSAR 4.4.0 compliant
- ✅ Error detection and reporting

## EMG49 Encoder Specifications

- **Counts Per Revolution (quadrature):** ~980
- **Pre-quadrature PPR:** ~245 (980 / 4)
- **Resolution:** ~0.367 degrees per count (360° / 980)
- **Maximum Position:** 4095 counts (configurable; adjust as needed)

## API Functions

### Initialization

```c
void Encoder_Init(const Encoder_ConfigType* ConfigPtr);
void Encoder_DeInit(void);
```

### Position Reading

```c
// Get position in different units
float Encoder_GetPosition(Encoder_UnitType Unit);  // COUNTS, REVOLUTIONS, or DEGREES
uint32 Encoder_GetPositionCounts(void);            // Returns counts (0-4095)
float Encoder_GetPositionRevolutions(void);        // Returns revolutions
float Encoder_GetPositionDegrees(void);            // Returns degrees (0-360)
```

### Velocity Reading

```c
// Get velocity in different units
float Encoder_GetVelocity(Encoder_VelocityUnitType Unit);  // COUNTS_PER_SEC or RPM
uint32 Encoder_GetVelocityCountsPerSec(void);               // Returns counts/second
float Encoder_GetVelocityRPM(void);                          // Returns RPM
```

### Direction and Status

```c
Encoder_DirectionType Encoder_GetDirection(void);  // FORWARD or REVERSE
Encoder_StatusType Encoder_GetStatus(void);       // UNINIT, IDLE, or RUNNING
```

### Position Control

```c
void Encoder_ResetPosition(void);           // Reset position to 0
void Encoder_SetPosition(uint32 Position); // Set position to specific value
```

### Data Reading

```c
// Get all encoder data at once
Std_ReturnType Encoder_GetData(Encoder_DataType* DataPtr);
```

### Update Function

```c
// Call periodically for filtered velocity (recommended: 10-100ms)
void Encoder_Update(void);
```

## Usage Examples

### Example 1: Basic Position Reading

```c
#include "ECUAL/ENCODER/ENCODER.h"
#include "ECUAL/ENCODER/ENCODER_Cfg.h"

int main(void)
{
    /* Initialize encoder */
    Encoder_Init(&Encoder_Config);
    
    while(1)
    {
        /* Read position in different units */
        uint32 counts = Encoder_GetPositionCounts();
        float revolutions = Encoder_GetPositionRevolutions();
        float degrees = Encoder_GetPositionDegrees();
        
        /* Use the values */
        printf("Position: %lu counts, %.2f rev, %.1f deg\n", 
               counts, revolutions, degrees);
        
        delay_ms(100);
    }
}
```

### Example 2: Velocity and RPM Measurement

```c
int main(void)
{
    Encoder_Init(&Encoder_Config);
    
    while(1)
    {
        /* Get velocity in RPM */
        float rpm = Encoder_GetVelocityRPM();
        
        /* Get velocity in counts per second */
        uint32 countsPerSec = Encoder_GetVelocityCountsPerSec();
        
        printf("Velocity: %.1f RPM (%lu counts/sec)\n", rpm, countsPerSec);
        
        /* Update encoder (for filtered velocity) */
        Encoder_Update();
        
        delay_ms(50);
    }
}
```

### Example 3: Complete Encoder Data

```c
int main(void)
{
    Encoder_Init(&Encoder_Config);
    Encoder_DataType encoderData;
    
    while(1)
    {
        /* Get all encoder data at once */
        if (Encoder_GetData(&encoderData) == E_OK)
        {
            printf("Position: %.2f rev (%.1f deg)\n", 
                   encoderData.PositionRevolutions,
                   encoderData.PositionDegrees);
            
            printf("Velocity: %.1f RPM\n", encoderData.VelocityRPM);
            
            printf("Direction: %s\n", 
                   (encoderData.Direction == ENCODER_DIRECTION_FORWARD) ? 
                   "Forward" : "Reverse");
        }
        
        Encoder_Update();
        delay_ms(100);
    }
}
```

### Example 4: Motor Position Control

```c
#define TARGET_POSITION_REV  2.5f  /* Target: 2.5 revolutions */
#define KP  0.1f                    /* Proportional gain */

int main(void)
{
    Encoder_Init(&Encoder_Config);
    
    while(1)
    {
        /* Get current position */
        float currentPos = Encoder_GetPositionRevolutions();
        
        /* Calculate error */
        float error = TARGET_POSITION_REV - currentPos;
        
        /* Simple P-control */
        float controlSignal = KP * error;
        
        /* Apply to motor (example) */
        Motor_SetSpeed(controlSignal);
        
        Encoder_Update();
        delay_ms(10);
    }
}
```

### Example 5: Direction-Based Control

```c
int main(void)
{
    Encoder_Init(&Encoder_Config);
    Encoder_DirectionType lastDirection = ENCODER_DIRECTION_FORWARD;
    
    while(1)
    {
        Encoder_DirectionType currentDir = Encoder_GetDirection();
        
        /* Detect direction change */
        if (currentDir != lastDirection)
        {
            if (currentDir == ENCODER_DIRECTION_FORWARD)
            {
                printf("Motor started rotating forward\n");
            }
            else
            {
                printf("Motor started rotating reverse\n");
            }
            
            lastDirection = currentDir;
        }
        
        Encoder_Update();
        delay_ms(50);
    }
}
```

## Configuration

The encoder configuration is in `ENCODER_PBCfg.c`:

```c
const Encoder_ConfigType Encoder_Config = {
    .PulsesPerRevolution = 12u,              /* EMG49: 12 PPR */
    .QuadratureCountsPerRev = 48u,            /* 12 × 4 = 48 */
    .EnableVelocityFilter = TRUE,             /* Enable filtering */
    .VelocityFilterAlpha = 200u,              /* Filter coefficient */
    .VelocityTimerPeriodUs = 2000u           /* Timer period in microseconds */
};
```

### Velocity Filter Configuration

- **VelocityFilterAlpha:** 0-255
  - Higher values = less filtering (faster response, more noise)
  - Lower values = more filtering (slower response, less noise)
  - Recommended: 200 (78% new, 22% old)

## Velocity Calculation

The encoder driver converts QEI velocity readings to RPM:

```
RPM = (CountsPerSecond × 60) / QuadratureCountsPerRevolution
```

For EMG49:
```
RPM = (CountsPerSecond × 60) / 48
```

## Position Wrapping

The position counter wraps at `MaxPosition` (default: 4095). This corresponds to:
- **85.3 revolutions** (4095 / 48)
- **30,720 degrees** (4095 × 7.5)

To track absolute position over multiple revolutions, implement position accumulation in your application.

## Error Handling

All functions include error detection (if `ENCODER_DEV_ERROR_DETECT == STD_ON`):
- Returns error codes via DET
- Returns safe default values (0) on error
- Checks for uninitialized state

## Dependencies

- **QEI Driver:** `MCAL/QEI/QEI.h`
- **Standard Types:** `CONFIG/Std_Types.h`
- **DET:** `CONFIG/Det.h`

## File Structure

```
ECUAL/ENCODER/
├── ENCODER.h          # API header
├── ENCODER.c          # Implementation
├── ENCODER_Types.h    # Type definitions
├── ENCODER_Cfg.h      # Configuration header
├── ENCODER_PBCfg.c    # Post-build configuration
└── ENCODER_README.md  # This file
```

## Notes

- Call `Encoder_Update()` periodically (10-100ms) for accurate filtered velocity
- Position readings are immediate (no filtering needed)
- Velocity filtering only applies if `EnableVelocityFilter = TRUE`
- The driver automatically handles position wrapping at MaxPosition

