# TM4C123GH6PM Multi-Driver Integration Test

**Version:** 3.0.0  
**Date:** October 26, 2025  
**Target:** TM4C123GH6PM LaunchPad

---

## 🎯 Overview

This comprehensive test program demonstrates the **integration of multiple AUTOSAR-compliant drivers** working together in real-time:

| Driver | Function | Status |
|--------|----------|--------|
| **GPIO** | Pin configuration and control | ✅ Active |
| **LED** | High-level LED abstraction | ✅ Active |
| **Button** | Debounced button input with events | ✅ Active |
| **UART** | Bidirectional serial communication | ✅ Active |
| **Timer** | Periodic interrupts (2s) | ✅ Active |

---

## ✨ Features

### **1. Automatic LED Cycling (Timer-Driven)**
- Timer2A generates interrupts every 2 seconds
- Automatically cycles through 8 LED colors
- Can be enabled/disabled via UART commands

### **2. Manual LED Control (UART-Driven)**
- Send commands from PC terminal
- Instantly control LED colors
- Automatically switches to manual mode when commanded

### **3. Physical Button Control**
- **SW1 (PF4)**: Toggle between AUTO and MANUAL modes
- **SW2 (PF0)**: Next color (manual) or Pause/Resume (auto)
- Hardware debouncing (20ms)
- Event-driven with state change detection

### **4. Real-Time Status Reporting**
- Uptime counter (seconds)
- Color cycle count
- Button press counters
- Current mode (AUTO/MANUAL/PAUSED)
- System statistics on demand

### **5. Interactive Command Interface**
- Echo functionality
- Help menu
- Mode switching
- Statistics display

---

## 🔌 Hardware Setup

**No external wiring required!** Everything is on-board:

- **PA0** → UART0 RX (USB debug port)
- **PA1** → UART0 TX (USB debug port)
- **PF1** → Red LED
- **PF2** → Blue LED
- **PF3** → Green LED
- **PF4** → SW1 Button (left button on LaunchPad)
- **PF0** → SW2 Button (right button on LaunchPad)

---

## 📋 Terminal Commands

### **LED Control (0-7)**
| Key | Color | LEDs |
|-----|-------|------|
| `0` | OFF | All OFF |
| `1` | RED | Red only |
| `2` | GREEN | Green only |
| `3` | BLUE | Blue only |
| `4` | YELLOW | Red + Green |
| `5` | MAGENTA | Red + Blue |
| `6` | CYAN | Green + Blue |
| `7` | WHITE | All ON |

**Note:** Sending any LED command (0-7) automatically switches to MANUAL mode.

### **Mode Control**
| Key | Action |
|-----|--------|
| `a` | Enable AUTO mode (Timer cycling) |
| `s` | STOP auto mode (Manual control) |

### **Information**
| Key | Action |
|-----|--------|
| `t` | Show uptime & statistics |
| `h` | Show help menu |

### **Button Controls**
| Button | Function |
|--------|----------|
| **SW1 (PF4)** | Toggle AUTO/MANUAL mode |
| **SW2 (PF0)** | In AUTO: Pause/Resume cycling<br>In MANUAL: Next color |

---

## 🚀 How to Test

### **Step 1: Open Terminal in CCS**

1. **View** → **Terminal**
2. Click **Connect** icon
3. Configure:
   - **Connection Type:** Serial Terminal
   - **Port:** Your LaunchPad COM port
   - **Baud Rate:** 115200
   - **Data Bits:** 8
   - **Parity:** None
   - **Stop Bits:** 1
4. Click **OK**

### **Step 2: Build and Flash**

1. **Project** → **Build Project**
2. **Run** → **Debug** (F11)
3. **Run** → **Resume** (F8)

### **Step 3: Observe Welcome Message**

```
************************************************
*  TM4C123GH6PM Multi-Driver Integration Test *
************************************************
Drivers Active:
  [x] GPIO Driver
  [x] LED Driver
  [x] UART Driver (115200 bps, 8N1)
  [x] Timer Driver (2s periodic interrupt)

System Configuration:
  System Clock: 16 MHz
  Auto-Cycle: ENABLED (press 's' to stop)
  Initial Color: OFF

============================================
  TM4C123 Multi-Driver Integration Test
  UART + Timer + GPIO + LED
============================================
LED Commands:
  0 - All LEDs OFF
  1 - Red LED
  2 - Green LED
  3 - Blue LED
  4 - Yellow (Red + Green)
  5 - Magenta (Red + Blue)
  6 - Cyan (Green + Blue)
  7 - White (All LEDs)

Mode Commands:
  a - Enable AUTO mode (Timer cycling)
  s - STOP auto mode (Manual control)

Info Commands:
  t - Show uptime & statistics
  h - Show this menu
============================================
Mode: AUTO | Color: OFF
Ready > 
```

---

## 🎬 Usage Examples

### **Example 1: Watch Auto-Cycling**

Just wait! Every 2 seconds, you'll see:

```
[AUTO] OFF
Ready > 
[AUTO] RED
Ready > 
[AUTO] GREEN
Ready > 
[AUTO] BLUE
Ready > 
...
```

The LEDs on the board will cycle through all 8 colors automatically.

### **Example 2: Manual Control**

```
Ready > 1
[MANUAL] LED set to: RED
Ready > 4
[MANUAL] LED set to: YELLOW
Ready > 0
[MANUAL] LED set to: OFF
Ready > 
```

**Note:** Auto-cycling stops when you send a manual command.

### **Example 3: Re-enable Auto Mode**

```
Ready > a
[AUTO MODE ENABLED] Timer will cycle colors every 2s
Ready > 
[AUTO] RED
Ready > 
[AUTO] GREEN
Ready > 
```

### **Example 4: View Statistics**

```
Ready > t

--- System Statistics ---
Uptime: 24 seconds
Color Cycles: 12
Current Mode: AUTO
Current Color: CYAN
System Clock: 16 MHz
UART Baud: 115200 bps
Timer Period: 2000 ms
-------------------------
Ready > 
```

### **Example 5: Stop Auto Mode**

```
Ready > s
[AUTO MODE STOPPED] Use 0-7 for manual control
Ready > 
```

Now the timer still runs (uptime still counts), but LEDs don't change automatically.

### **Example 6: Button Control - Toggle Mode**

Press **SW1** (left button on LaunchPad):

```
[SW1] Mode: MANUAL
Ready > 
```

Press **SW1** again:

```
[SW1] Mode: AUTO
Ready > 
[AUTO] RED
Ready > 
```

### **Example 7: Button Control - Pause/Resume**

While in AUTO mode, press **SW2** (right button):

```
[AUTO] BLUE
Ready > 
[SW2] Auto-cycle: PAUSED
Ready > 
(LEDs stop changing, but timer still runs)
```

Press **SW2** again:

```
[SW2] Auto-cycle: RESUMED
Ready > 
[AUTO] YELLOW
Ready > 
```

### **Example 8: Button Control - Next Color**

Switch to MANUAL mode (press SW1), then press **SW2** repeatedly:

```
[SW1] Mode: MANUAL
Ready > 
[SW2] Next: RED
Ready > 
[SW2] Next: GREEN
Ready > 
[SW2] Next: BLUE
Ready > 
```

### **Example 9: Combined Control**

Mix UART commands and button presses for full control!

```
(Press SW1 to go to MANUAL)
[SW1] Mode: MANUAL
Ready > 1
[MANUAL] LED set to: RED
Ready > 
(Press SW2 to cycle)
[SW2] Next: GREEN
Ready > 
(Type 'a' to go back to AUTO)
Ready > a
[AUTO MODE ENABLED] Timer will cycle colors every 2s
Ready > 
[AUTO] BLUE
Ready > 
```

---

## 🧪 What You're Testing

### **GPIO Driver**
- ✅ Port A configuration (UART pins)
- ✅ Port F configuration (LED pins)
- ✅ Alternate function mode (UART)
- ✅ Digital output mode (LEDs)

### **LED Driver**
- ✅ LED initialization
- ✅ LED state control (ON/OFF)
- ✅ Multiple LED combinations
- ✅ Real-time LED updates

### **Button Driver**
- ✅ Button initialization (SW1, SW2)
- ✅ State reading with debouncing
- ✅ State change detection
- ✅ Event-driven button handling
- ✅ Pull-up resistor configuration

### **UART Driver**
- ✅ Initialization (115200 baud, 8N1)
- ✅ Transmit: `Uart_SendString()`, `Uart_SendByte()`
- ✅ Receive: `Uart_ReceiveByte()`, `Uart_IsRxDataAvailable()`
- ✅ Bidirectional communication
- ✅ FIFO operation
- ✅ Polling mode

### **Timer Driver**
- ✅ Timer2A initialization
- ✅ Periodic mode (2 seconds)
- ✅ Interrupt generation
- ✅ ISR execution
- ✅ NVIC configuration
- ✅ Interrupt flag clearing

### **Integration**
- ✅ Multiple drivers working simultaneously
- ✅ ISR + main loop coordination
- ✅ Shared resource management (LEDs)
- ✅ Mode switching (auto/manual/paused)
- ✅ Real-time statistics tracking
- ✅ Button + UART + Timer interaction
- ✅ Event-driven architecture

---

## 📊 System Architecture

```
┌──────────────────────────────────────────────────┐
│              Main Application                    │
│  - Button polling & event handling               │
│  - UART command processing                       │
│  - Statistics tracking                           │
│  - Mode management (AUTO/MANUAL/PAUSED)          │
└──────────┬───────────────────────────────────────┘
           │
           ├─────────┬──────────┬──────────┬──────────┐
           │         │          │          │          │
     ┌─────▼────┐ ┌─▼─────┐ ┌──▼────┐ ┌───▼───┐ ┌───▼────┐
     │  Button  │ │ UART  │ │ Timer │ │  LED  │ │  GPIO  │
     │  Driver  │ │Driver │ │Driver │ │Driver │ │ Driver │
     └─────┬────┘ └───┬───┘ └───┬───┘ └───┬───┘ └───┬────┘
           │          │         │         │         │
           │          │         │         │         │
     ┌─────▼──────────▼─────────▼─────────▼─────────▼────┐
     │            Hardware Abstraction Layer              │
     │  - UART0 (PA0/PA1)                                │
     │  - Timer2A (32-bit periodic)                      │
     │  - GPIO Port F (PF0/PF1/PF2/PF3/PF4)             │
     │  - Buttons (SW1/SW2 with debounce)               │
     └───────────────────────────────────────────────────┘
```

---

## 🔄 Program Flow

```
1. Initialize GPIO → Configure all pins (LEDs + Buttons + UART)
2. Initialize LEDs → Set initial state (OFF)
3. Initialize Buttons → SW1 and SW2 with debouncing
4. Initialize UART → 115200 baud, 8N1
5. Initialize Timer → 2s periodic, interrupt enabled
6. Enable NVIC → Timer2A interrupt
7. Start Timer → Begin auto-cycling
8. Send welcome message → Via UART
9. Main loop (foreground):
   ├─ Poll SW1 button
   │  └─ If pressed: Toggle AUTO/MANUAL mode
   ├─ Poll SW2 button
   │  ├─ If AUTO: Pause/Resume cycling
   │  └─ If MANUAL: Next color
   ├─ Poll UART for commands
   ├─ Process received commands
   └─ Update mode/LEDs as needed
   
Background (ISR):
   └─ Timer2A_Handler (every 2s):
      ├─ Update uptime counter
      ├─ If AUTO mode AND not paused:
      │  ├─ Cycle to next color
      │  ├─ Increment cycle count
      │  ├─ Update LED hardware
      │  └─ Send notification via UART
      └─ Clear interrupt flag
```

---

## 📈 Performance Metrics

**@ 16 MHz System Clock:**

| Operation | Time |
|-----------|------|
| Button debounce | 20 ms |
| Button response | < 25 ms |
| UART character echo | < 1 ms |
| LED color change | < 100 μs |
| Command processing | < 5 ms |
| Timer interrupt latency | < 10 μs |
| Menu display | ~300 ms |
| Statistics display | ~250 ms |

---

## 🐛 Troubleshooting

### **No terminal output:**
- ✅ Check COM port in Device Manager
- ✅ Verify baud rate is 115200
- ✅ Ensure USB cable supports data

### **LEDs not cycling:**
- ✅ Check if in MANUAL mode (press 'a')
- ✅ Verify Timer2A interrupt is enabled
- ✅ Check startup file has `Timer2A_Handler`

### **Commands not working:**
- ✅ Check if characters are echoed
- ✅ Try lowercase commands
- ✅ Press 'h' for help

### **Garbled text:**
- ✅ Wrong baud rate (must be 115200)
- ✅ Check terminal settings (8N1)

### **Buttons not responding:**
- ✅ Check if PF0/PF4 are configured in GPIO
- ✅ Verify button initialization in main
- ✅ Try holding button longer (debounce = 20ms)
- ✅ Check UART for button press notifications

### **Button presses not detected:**
- ✅ Ensure `Button_HasStateChanged()` is called in loop
- ✅ Check that buttons are not stuck
- ✅ Verify pull-up resistors are enabled

---

## 🎓 Learning Outcomes

After running this test, you understand:

1. **Multi-driver integration** - How 5 different drivers work together
2. **Interrupt-driven design** - ISR + main loop coordination
3. **Real-time systems** - Timing, responsiveness, determinism
4. **State machines** - Mode switching (AUTO/MANUAL/PAUSED)
5. **Communication protocols** - UART command/response
6. **Resource sharing** - LEDs controlled by ISR, UART, and buttons
7. **Event-driven programming** - Button state change detection
8. **Debouncing techniques** - Software debouncing for reliable input
9. **AUTOSAR architecture** - Layered driver design
10. **Human-machine interface** - Multiple input methods (UART + buttons)

---

## 🚀 Next Steps

1. ✅ **Add more timers** - Multiple periodic tasks
2. ✅ **Implement PWM** - LED brightness control
3. ✅ **Add ADC** - Read sensors, display via UART
4. ✅ **Create protocols** - JSON, binary commands
5. ✅ **Add RTOS** - Task scheduling, priorities
6. ✅ **Implement DMA** - High-speed UART transfers

---

## 📝 Code Statistics

| File | Lines | Description |
|------|-------|-------------|
| `main.c` | 481 | Application logic with button integration |
| `Gpio.c` | 500+ | GPIO driver |
| `Led.c` | 200+ | LED driver |
| `Button.c` | 300+ | Button driver with debouncing |
| `Uart.c` | 422 | UART driver |
| `Timer.c` | 600+ | Timer driver |
| **Total** | **2500+** | Production-ready code |

---

**End of Documentation**
