# Transparent UART Bridge Mode

**Simple bidirectional communication between PC and Raspberry Pi through TM4C**

---

## 🎯 What It Does

The TM4C acts as a **transparent bridge** between your PC and Raspberry Pi:

```
    PC Serial Monitor
          ↕
    UART0 (USB)
          ↕
      TM4C123
          ↕
    UART1 (PB0/PB1)
          ↕
   Raspberry Pi 5
```

**Everything you type in the serial monitor is sent to the Raspberry Pi.**  
**Everything the Raspberry Pi sends is displayed in your serial monitor.**

---

## 🔌 Hardware Connections

### **TM4C ↔ Raspberry Pi**

| TM4C Pin | Function | RPi Pin | GPIO | Notes |
|----------|----------|---------|------|-------|
| **PB1** | TX | **Pin 10** | GPIO 15 (RX) | TM4C → RPi |
| **PB0** | RX | **Pin 8** | GPIO 14 (TX) | RPi → TM4C |
| **GND** | Ground | **Pin 6** | GND | **MUST CONNECT!** |

**⚠️ CRITICAL:** Common ground is mandatory! Don't skip the GND connection.

---

## 🚀 Quick Start

### **Step 1: Flash TM4C**

1. Build the project:
   ```
   Project → Clean
   Project → Build
   ```

2. Flash to board:
   ```
   Run → Debug → Resume
   ```

### **Step 2: Open Serial Monitor**

- **Baud Rate:** 115200
- **Data Bits:** 8
- **Parity:** None
- **Stop Bits:** 1

You should see:
```
========================================
  Transparent UART Bridge (TM4C)      
========================================
PC   <-> UART0 (PA0/PA1) @ 115200 baud
RPi  <-> UART1 (PB0/PB1) @ 115200 baud

Mode: Transparent Bridge
- Type here -> Sent to RPi
- RPi data  -> Shown here

Ready! Start typing...
```

### **Step 3: Test Without RPi (Loopback)**

Before connecting RPi, test the TM4C:

1. **Short PB0 and PB1** on the TM4C (connect them together)
2. Type in serial monitor
3. You should see your text echoed back (loopback test)
4. If this works, UART1 is functioning correctly

### **Step 4: Connect Raspberry Pi**

1. **Enable UART on RPi:**
   ```bash
   sudo raspi-config
   # Interface Options → Serial Port
   # Login shell: NO
   # Serial port hardware: YES
   sudo reboot
   ```

2. **Test from RPi side:**
   ```bash
   # Install minicom
   sudo apt install minicom
   
   # Open serial port
   minicom -b 115200 -D /dev/serial0
   
   # Type something - should appear in TM4C serial monitor!
   ```

---

## 💬 Usage Examples

### **Example 1: Send Text from PC to RPi**

**In PC Serial Monitor:**
```
Hello Raspberry Pi!
```

**On Raspberry Pi (if running minicom or Python script):**
```
Hello Raspberry Pi!
```

### **Example 2: Receive Data from RPi**

**On Raspberry Pi:**
```bash
echo "Hello from RPi!" > /dev/serial0
```

**In PC Serial Monitor:**
```
Hello from RPi!
```

### **Example 3: Python Script on RPi**

```python
#!/usr/bin/env python3
import serial
import time

# Open serial port
ser = serial.Serial('/dev/serial0', 115200, timeout=1)

# Send data
ser.write(b"Hello from Python!\n")

# Read data
while True:
    if ser.in_waiting > 0:
        data = ser.read(ser.in_waiting)
        print(f"Received: {data.decode('utf-8', errors='ignore')}")
    time.sleep(0.1)
```

---

## 📊 Statistics

Every 10 seconds, you'll see transfer statistics:

```
[STATS] PC->RPi: 245 bytes | RPi->PC: 128 bytes
```

This shows:
- **PC->RPi:** Total bytes sent from PC to Raspberry Pi
- **RPi->PC:** Total bytes received from Raspberry Pi

---

## 🔧 Troubleshooting

### **Problem: Nothing appears when I type**

**Check:**
1. Serial monitor is connected to correct COM port
2. Baud rate is 115200
3. TM4C is powered and programmed

### **Problem: Garbled characters**

**Check:**
1. Baud rate matches on both sides (115200)
2. Both devices use 8N1 (8 data bits, no parity, 1 stop bit)
3. Ground connection is solid

### **Problem: Can send but not receive from RPi**

**Check:**
1. RPi UART is enabled (`sudo raspi-config`)
2. TX/RX are crossed (TX → RX, RX → TX)
3. Ground is connected
4. RPi is actually sending data

### **Problem: Loopback test fails**

**Check:**
1. PB0 and PB1 are properly shorted
2. UART1 GPIO configuration is correct
3. Rebuild and reflash the code

---

## 🐍 Simple RPi Test Script

Save as `uart_test.py` on Raspberry Pi:

```python
#!/usr/bin/env python3
"""
Simple UART test for TM4C bridge
"""
import serial
import time

def main():
    print("TM4C UART Bridge Test")
    print("=" * 40)
    
    try:
        # Open serial port
        ser = serial.Serial(
            port='/dev/serial0',
            baudrate=115200,
            timeout=1
        )
        
        print("Serial port opened successfully!")
        print("Type 'quit' to exit\n")
        
        # Send test message
        ser.write(b"Hello from Raspberry Pi!\n")
        print("Sent: Hello from Raspberry Pi!")
        
        # Echo loop
        while True:
            # Read from serial
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting)
                text = data.decode('utf-8', errors='ignore')
                print(f"Received: {text}", end='')
                
                # Echo back
                ser.write(data)
                
                # Check for quit
                if 'quit' in text.lower():
                    break
            
            time.sleep(0.01)
        
        ser.close()
        print("\nTest completed!")
        
    except serial.SerialException as e:
        print(f"Error: {e}")
    except KeyboardInterrupt:
        print("\nInterrupted by user")

if __name__ == "__main__":
    main()
```

**Run it:**
```bash
chmod +x uart_test.py
python3 uart_test.py
```

---

## 🎓 Understanding the Code

### **Main Loop (Simplified):**

```c
while (1) {
    /* Forward PC → Raspberry Pi */
    ForwardPCToRPi();
    
    /* Forward Raspberry Pi → PC */
    ForwardRPiToPC();
}
```

### **ForwardPCToRPi():**
```c
void ForwardPCToRPi(void) {
    uint8 receivedByte;
    
    while (Uart_IsRxDataAvailable(PC_UART_MODULE)) {
        Uart_ReceiveByte(PC_UART_MODULE, &receivedByte);
        Uart_SendByte(ROS2_UART_MODULE, receivedByte);  // Forward!
    }
}
```

### **ForwardRPiToPC():**
```c
void ForwardRPiToPC(void) {
    uint8 receivedByte;
    
    while (Uart_IsRxDataAvailable(ROS2_UART_MODULE)) {
        Uart_ReceiveByte(ROS2_UART_MODULE, &receivedByte);
        Uart_SendByte(PC_UART_MODULE, receivedByte);  // Forward!
    }
}
```

**That's it!** Simple byte-by-byte forwarding in both directions.

---

## 🔄 Switching Between Modes

### **To Use GPS Reader:**
In `main_gps.c`, change line 8:
```c
#if 0  /* Change to #if 1 */
```

In `main_ros2.c`, add at top:
```c
#if 0  /* Comment out ROS2 main */
```

### **To Use ROS2 Bridge:**
Keep current configuration (ROS2 active, GPS commented)

---

## 📝 Next Steps

Once basic communication works:

1. ✅ **Test text communication** (this mode)
2. ✅ **Develop protocol** (add packet structure if needed)
3. ✅ **Integrate with ROS 2** (create ROS node)
4. ✅ **Add sensor data** (GPS, IMU, etc.)
5. ✅ **Implement motor control** (receive commands)

---

## 💡 Tips

- **Use Ctrl+C/Ctrl+V** in serial monitor for quick testing
- **Send JSON** for structured data: `{"cmd":"move","speed":100}`
- **Add newlines** (`\n`) to separate messages
- **Monitor statistics** to verify data flow
- **Test loopback first** before connecting RPi

---

**Your TM4C is now a transparent UART bridge! Type away and communicate with your Raspberry Pi!** 🔗💬
