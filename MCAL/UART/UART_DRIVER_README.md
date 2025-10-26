# UART Driver Documentation

**Version:** 1.0.0  
**Date:** October 26, 2025  
**Target:** TM4C123GH6PM LaunchPad  
**Module:** MCAL - UART Driver

---

## 📋 Overview

The UART (Universal Asynchronous Receiver/Transmitter) driver provides bidirectional serial communication capabilities for the TM4C123GH6PM microcontroller. This AUTOSAR-compliant driver enables reliable data transmission and reception over serial interfaces.

### **Key Features**

✅ **Multiple UART Modules** - Supports UART0-UART7  
✅ **Configurable Baud Rates** - 9600 to 921600 bps  
✅ **Data Format Options** - 5/6/7/8 data bits, 1/2 stop bits, parity control  
✅ **FIFO Support** - Hardware FIFOs for efficient data handling  
✅ **Interrupt Support** - RX/TX interrupt-driven operation  
✅ **Polling Mode** - Non-blocking data transfer  
✅ **Flow Control** - Hardware RTS/CTS support  

---

## 🏗️ Architecture

```
┌─────────────────────────────────────┐
│      Application Layer              │
│  (main.c, command processing)       │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│         UART Driver API             │
│  - Uart_Init()                      │
│  - Uart_SendByte()                  │
│  - Uart_ReceiveByte()               │
│  - Uart_SendString()                │
│  - Uart_IsRxDataAvailable()         │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│    Hardware Abstraction Layer       │
│  - Register access                  │
│  - Clock configuration              │
│  - Interrupt handling               │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│         TM4C123 Hardware            │
│  - UART0-UART7 modules              │
│  - GPIO alternate functions         │
└─────────────────────────────────────┘
```

---

## 📁 File Structure

| File | Lines | Description |
|------|-------|-------------|
| `Uart.h` | 250+ | Driver interface and type definitions |
| `Uart.c` | 422 | Driver implementation |
| `Uart_Cfg.h` | 50+ | Compile-time configuration |
| `Uart_PBCfg.c` | 30+ | Post-build configuration |

---

## 🔧 Configuration

### **Compile-Time Configuration (Uart_Cfg.h)**

```c
#define UART_DEV_ERROR_DETECT           FALSE
#define UART_VERSION_INFO_API           TRUE
#define UART_INTERRUPT_MODE_ENABLE      TRUE
#define UART_FIFO_ENABLE                TRUE
#define UART_MAX_MODULES                8
```

### **Runtime Configuration Example**

```c
const Uart_ConfigType Uart0_Config = {
    .Module = UART_MODULE_0,
    .BaudRate = UART_BAUD_115200,
    .DataBits = UART_DATA_BITS_8,
    .Parity = UART_PARITY_NONE,
    .StopBits = UART_STOP_BITS_1,
    .FlowControl = UART_FLOW_CONTROL_NONE,
    .FifoEnable = TRUE,
    .RxInterruptEnable = FALSE,
    .TxInterruptEnable = FALSE,
    .RxCallback = NULL_PTR,
    .TxCallback = NULL_PTR
};
```

---

## 🚀 API Reference

### **Initialization**

#### `Uart_Init()`
```c
void Uart_Init(const Uart_ConfigType* ConfigPtr);
```
**Description:** Initializes UART module with specified configuration  
**Parameters:** ConfigPtr - Pointer to configuration structure  
**Returns:** void  

---

### **Data Transmission**

#### `Uart_SendByte()`
```c
Std_ReturnType Uart_SendByte(Uart_ModuleType Module, uint8 Data);
```
**Description:** Sends a single byte  
**Parameters:**  
- Module - UART module (UART_MODULE_0 to UART_MODULE_7)
- Data - Byte to transmit

**Returns:** E_OK or E_NOT_OK  

#### `Uart_SendString()`
```c
Std_ReturnType Uart_SendString(Uart_ModuleType Module, const uint8* String);
```
**Description:** Sends null-terminated string  
**Parameters:**  
- Module - UART module
- String - Pointer to string

**Returns:** E_OK or E_NOT_OK  

---

### **Data Reception**

#### `Uart_ReceiveByte()`
```c
Std_ReturnType Uart_ReceiveByte(Uart_ModuleType Module, uint8* Data);
```
**Description:** Receives a single byte (non-blocking)  
**Parameters:**  
- Module - UART module
- Data - Pointer to store received byte

**Returns:** E_OK if data available, E_NOT_OK otherwise  

#### `Uart_IsRxDataAvailable()`
```c
boolean Uart_IsRxDataAvailable(Uart_ModuleType Module);
```
**Description:** Checks if data is available in RX FIFO  
**Parameters:** Module - UART module  
**Returns:** TRUE if data available, FALSE otherwise  

---

## 💡 Usage Examples

### **Example 1: Simple Echo**

```c
#include "Uart.h"

int main(void) {
    uint8 receivedByte;
    
    /* Initialize UART0 */
    Uart_Init(&Uart0_Config);
    
    /* Send welcome message */
    Uart_SendString(UART_MODULE_0, (const uint8*)"Echo Test\r\n");
    
    while(1) {
        /* Check for received data */
        if (Uart_IsRxDataAvailable(UART_MODULE_0)) {
            /* Receive byte */
            if (Uart_ReceiveByte(UART_MODULE_0, &receivedByte) == E_OK) {
                /* Echo back */
                Uart_SendByte(UART_MODULE_0, receivedByte);
            }
        }
    }
}
```

### **Example 2: Command Processing**

```c
void ProcessCommand(void) {
    uint8 cmd;
    
    if (Uart_IsRxDataAvailable(UART_MODULE_0)) {
        Uart_ReceiveByte(UART_MODULE_0, &cmd);
        
        switch(cmd) {
            case '1':
                Uart_SendString(UART_MODULE_0, (const uint8*)"Command 1\r\n");
                break;
            case '2':
                Uart_SendString(UART_MODULE_0, (const uint8*)"Command 2\r\n");
                break;
            default:
                Uart_SendString(UART_MODULE_0, (const uint8*)"Unknown\r\n");
                break;
        }
    }
}
```

### **Example 3: Interrupt-Driven Reception**

```c
/* Callback function */
void Uart_RxCallback(void) {
    uint8 data;
    Uart_ReceiveByte(UART_MODULE_0, &data);
    /* Process data */
}

/* Configuration with interrupt */
const Uart_ConfigType Uart0_IntConfig = {
    .Module = UART_MODULE_0,
    .BaudRate = UART_BAUD_115200,
    .DataBits = UART_DATA_BITS_8,
    .Parity = UART_PARITY_NONE,
    .StopBits = UART_STOP_BITS_1,
    .FlowControl = UART_FLOW_CONTROL_NONE,
    .FifoEnable = TRUE,
    .RxInterruptEnable = TRUE,      /* Enable RX interrupt */
    .TxInterruptEnable = FALSE,
    .RxCallback = Uart_RxCallback,  /* Set callback */
    .TxCallback = NULL_PTR
};
```

---

## 🔌 Hardware Connections

### **UART0 (Most Common)**
- **PA0** → UART0 RX (Receive)
- **PA1** → UART0 TX (Transmit)
- Connected to USB debug port on LaunchPad

### **Other UART Modules**
| Module | RX Pin | TX Pin | Notes |
|--------|--------|--------|-------|
| UART1 | PB0/PC4 | PB1/PC5 | Alternate locations |
| UART2 | PD6 | PD7 | |
| UART3 | PC6 | PC7 | |
| UART4 | PC4 | PC5 | |
| UART5 | PE4 | PE5 | |
| UART6 | PD4 | PD5 | |
| UART7 | PE0 | PE1 | |

---

## ⚙️ Baud Rate Calculation

**Formula:**
```
BRD = System Clock / (16 × Baud Rate)
IBRD = Integer part of BRD
FBRD = Integer((Fractional part of BRD × 64) + 0.5)
```

**Example (115200 baud @ 16 MHz):**
```
BRD = 16,000,000 / (16 × 115200) = 8.680555
IBRD = 8
FBRD = Integer((0.680555 × 64) + 0.5) = 44
```

---

## 🐛 Troubleshooting

### **No Data Received**
- ✅ Check baud rate matches on both sides
- ✅ Verify GPIO pins configured for UART alternate function
- ✅ Ensure UART module clock is enabled
- ✅ Check cable connections (TX → RX, RX → TX)

### **Garbled Data**
- ✅ Baud rate mismatch
- ✅ Wrong data format (bits/parity/stop)
- ✅ Check system clock configuration

### **Data Loss**
- ✅ Enable FIFO
- ✅ Use interrupt mode for high-speed communication
- ✅ Process received data quickly

---

## 📊 Performance

**@ 16 MHz System Clock:**

| Baud Rate | Max Throughput | Typical Use Case |
|-----------|----------------|------------------|
| 9600 | 960 bytes/sec | Slow sensors |
| 19200 | 1.92 KB/sec | Basic communication |
| 38400 | 3.84 KB/sec | Standard serial |
| 57600 | 5.76 KB/sec | Fast serial |
| 115200 | 11.52 KB/sec | Debug/terminal |
| 230400 | 23.04 KB/sec | High-speed data |
| 460800 | 46.08 KB/sec | Very high-speed |
| 921600 | 92.16 KB/sec | Maximum speed |

---

## 🎓 Best Practices

1. **Always check return values** from send/receive functions
2. **Use FIFO** for better performance
3. **Enable interrupts** for high-speed or background communication
4. **Add timeouts** for blocking operations
5. **Validate baud rate** calculation for accuracy
6. **Use flow control** for reliable long-distance communication
7. **Buffer incoming data** to prevent loss

---

## 📚 Related Drivers

- **GPIO Driver** - Pin configuration for UART
- **NVIC Driver** - Interrupt management
- **DMA Driver** - High-speed data transfer (future)

---

## 📝 Revision History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | Oct 26, 2025 | Initial release |

---

**End of UART Driver Documentation**
