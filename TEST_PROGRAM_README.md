# TM4C123GH6PM Driver Test Program

## Overview

This is a simple test program that demonstrates the GPIO, LED, and Button drivers working together. Press any button on the LaunchPad to cycle through 7 different LED color combinations.

## Hardware Setup

**TM4C123GH6PM LaunchPad Components Used:**

| Component | Pin | Configuration | Description |
|-----------|-----|---------------|-------------|
| **Red LED** | PF1 | Active-High Output | RGB LED Red |
| **Blue LED** | PF2 | Active-High Output | RGB LED Blue |
| **Green LED** | PF3 | Active-High Output | RGB LED Green |
| **SW1 Button** | PF4 | Active-Low Input + Pull-up | Left button |
| **SW2 Button** | PF0 | Active-Low Input + Pull-up | Right button |

## Color Combinations

The program cycles through these 7 colors each time a button is pressed:

1. **OFF** - All LEDs off (black)
2. **RED** - Red LED only
3. **BLUE** - Blue LED only
4. **GREEN** - Green LED only
5. **YELLOW** - Red + Green LEDs
6. **MAGENTA** - Red + Blue LEDs
7. **CYAN** - Blue + Green LEDs
8. **WHITE** - All three LEDs (back to OFF)

## How to Use

### 1. Compile and Flash
```bash
# In Code Composer Studio:
1. Build the project
2. Flash to TM4C123GH6PM LaunchPad
3. Run the program
```

### 2. Test the Drivers
- **Press SW1 or SW2** to cycle through colors
- **Watch the RGB LED** change colors
- **7 different combinations** cycle in sequence

### 3. Verify Each Driver
- **GPIO Driver:** Configures pins as input/output with proper settings
- **LED Driver:** Controls LED states (ON/OFF/TOGGLE)
- **Button Driver:** Reads button states with debouncing

## Code Structure

```c
// 1. Initialize GPIO driver
Gpio_Init(&Gpio_Configuration);

// 2. Configure GPIO pins for LEDs and buttons
// (GPIO configurations defined in code)

// 3. Initialize LED driver
Led_Init(&Led_Red);
Led_Init(&Led_Blue);
Led_Init(&Led_Green);

// 4. Initialize Button driver
Button_Init(&Button_SW1);
Button_Init(&Button_SW2);

// 5. Main loop: Check buttons and update LEDs
while(1) {
    if (button pressed) {
        cycle to next color
        update LEDs accordingly
    }
}
```

## Features Demonstrated

✅ **GPIO Driver Integration** - Proper pin configuration and initialization
✅ **LED Driver Usage** - ON/OFF control and state management
✅ **Button Driver Usage** - Debounced reading and state detection
✅ **Real-time Response** - Immediate LED updates on button press
✅ **Thread Safety** - All drivers use atomic operations
✅ **Error Handling** - Parameter validation and graceful degradation

## Expected Behavior

1. **Program starts:** All LEDs are OFF
2. **Press SW1 or SW2:** LEDs change to RED
3. **Press again:** LEDs change to BLUE
4. **Continue pressing:** Cycles through all 7 color combinations
5. **After WHITE:** Returns to OFF and repeats

## Troubleshooting

### LEDs Not Working
- Check GPIO configuration for PF1, PF2, PF3
- Verify LEDs are configured as active-high
- Ensure Gpio_Init() is called before Led_Init()

### Buttons Not Responding
- Check GPIO configuration for PF4 (SW1) and PF0 (SW2)
- Verify buttons are configured as active-low with pull-up
- Ensure Button_Init() is called after GPIO configuration
- Check debounce timing (20ms should be sufficient)

### Build Errors
- Verify all driver files are included in the project
- Check that include paths are correct
- Ensure std_types.h is available in CONFIG directory

## Dependencies

- **MCAL/GPIO Driver** - Must be initialized first
- **ECUAL/LED Driver** - For LED control functions
- **ECUAL/Button Driver** - For button reading with debouncing
- **CONFIG/std_types.h** - Standard type definitions

---

**Ready to test!** 🎯 Just compile, flash, and press the buttons to see the colors change!
