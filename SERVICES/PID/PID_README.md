# PID Controller Driver

Generic PID controller with anti-windup and derivative filtering. Suitable for motor speed/position control (e.g., EMG49 motor).

## Files
- `PID.h / PID.c` – API and implementation
- `PID_Types.h` – Type definitions
- `PID_Cfg.h` – Compile-time options
- `PID_PBCfg.c` – Example configuration for motor speed

## Features
- P, I, D terms with:
  - Integrator clamping (anti-windup)
  - Derivative on measurement (reduces kick)
  - Derivative low-pass filter (alpha 0-1)
- Output limiting
- Sample-time aware gains
- Error detection (DET)
- Version info API

## API (core)
```c
void PID_Init(const PID_ConfigType* cfg, PID_StateType* state);
void PID_Reset(PID_StateType* state);
Std_ReturnType PID_Compute(const PID_ConfigType* cfg,
                           PID_StateType* state,
                           float32 setpoint,
                           float32 measurement,
                           float32* output);
Std_ReturnType PID_SetTunings(PID_ConfigType* cfg, float32 Kp, float32 Ki, float32 Kd);
Std_ReturnType PID_SetOutputLimits(PID_ConfigType* cfg, float32 outMin, float32 outMax);
Std_ReturnType PID_SetSampleTime(PID_ConfigType* cfg, float32 sampleTimeSec);
```

## Example usage
```c
#include "PID.h"

PID_StateType pidState;

int main(void)
{
    float32 out = 0.0f;

    /* Init PID */
    PID_Init(&PID_MotorSpeed_Config, &pidState);

    while (1)
    {
        float32 setpoint = 50.0f;     /* desired speed */
        float32 measurement = GetSpeed(); /* your function */

        if (PID_Compute(&PID_MotorSpeed_Config, &pidState, setpoint, measurement, &out) == E_OK)
        {
            Motor_SetDuty(out);  /* apply output */
        }

        delay_ms(10); /* match SampleTimeSec */
    }
}
```

## Tuning tips
- Start with Kp, increase until stable response
- Add Ki to remove steady-state error (watch windup; clamp integrator)
- Add small Kd to damp overshoot; increase derivative filter alpha for less noise (0.1–0.3)
- Ensure SampleTimeSec matches your control loop period
- Set output and integrator limits to your actuator range

## Notes
- `DerivativeOnMeasurement = TRUE` is recommended to avoid derivative kick.
- `DerivativeFilterAlpha` in [0..1]; higher = less filtering (faster).
- Keep SampleTimeSec > 0; adjust Ki, Kd accordingly (they are per-second gains).

