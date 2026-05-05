# FreeRTOS Tasks Documentation

**Version:** 1.1.0  
**Date:** January 9, 2026  
**Module:** SERVICES/RTOS  
**Framework:** FreeRTOS 10.x

---

## 📋 Overview

The Tasks module creates and manages all FreeRTOS tasks for the UGV low-level controller. It defines task priorities, stack sizes, periods, and inter-task communication queues.

### **Key Features**

✅ **5 Application Tasks** - Safety, Control, Sensor, Comm, Thermal  
✅ **Priority-Based Scheduling** - Safety highest, thermal lowest  
✅ **3 Inter-Task Queues** - Command, feedback, and safety status  
✅ **Resource Management** - Integrated with ResourceMap  
✅ **Error Hooks** - Stack overflow and malloc failure detection  

---

## 🏗️ Task Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    TASK HIERARCHY                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Priority 4 ▶ Safety Task   (10ms) ─► WDG, Current, Thermal    │
│  Priority 3 ▶ Control Task  (10ms) ─► PID, Motor commands      │
│  Priority 2 ▶ Sensor Task   (20ms) ─► IMU, GPS, Encoders       │
│  Priority 1 ▶ Comm Task     (20ms) ─► ROS UART, Commands       │
│  Priority 1 ▶ Thermal Task (1000ms) ─► Fan control             │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Task Parameters

| Task | Priority | Stack (words) | Period | Responsibilities |
|------|:--------:|:-------------:|:------:|------------------|
| Safety | 4 | 128 | 10ms | WDG feed, current monitoring, thermal shutdown |
| Control | 3 | 256 | 10ms | PID control, motor commands, encoder feedback |
| Sensor | 2 | 256 | 20ms | IMU reading, GPS parsing, sensor fusion |
| Comm | 1 | 256 | 20ms | ROS UART RX/TX, command parsing |
| Thermal | 1 | 128 | 1000ms | Temperature monitoring, fan control |

---

## 📁 File Structure

| File | Description |
|------|-------------|
| `Tasks_Init.h` | Task API declarations |
| `Tasks_Init.c` | Task creation and queue management |
| `FreeRTOSConfig.h` | FreeRTOS kernel configuration |

---

## 🚀 Quick Start

### 1. Start the System

```c
#include "SERVICES/SYSTEM/System_Services.h"
#include "SERVICES/RTOS/Tasks_Init.h"

int main(void) {
    /* Initialize all drivers */
    System_Init();
    
    /* Create tasks and start scheduler */
    Tasks_Init();  /* Never returns */
    
    for (;;) { }
}
```

### 2. Access Queues

```c
#include "Tasks_Init.h"

/* Get queue handles */
QueueHandle_t cmdQueue = Tasks_GetWheelSpeedCmdQueue();
QueueHandle_t fbQueue = Tasks_GetSensorFeedbackQueue();
```

---

## 📡 Inter-Task Communication

### Queue Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    DATA FLOW                                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  [ROS/RPi] ──UART──► Comm ──Queue_WheelSpeedCmd──► Control      │
│                                                                 │
│  Sensor ──Queue_SensorFeedback──► Comm ──UART──► [ROS/RPi]      │
│                                                                 │
│  Safety ──Queue_SafetyStatus──► Comm ──UART──► [ROS/RPi]        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Queue Details

| Queue | Producer | Consumer | Data Size | Depth |
|-------|----------|----------|-----------|:-----:|
| Queue_WheelSpeedCmd | Comm | Control | 8 bytes | 4 |
| Queue_SensorFeedback | Sensor | Comm | 24 bytes | 4 |
| Queue_SafetyStatus | Safety | Comm | 12 bytes | 2 |

---

## 🔧 API Reference

### Initialization
```c
void Tasks_Init(void);                    /* Create all tasks, start scheduler */
BaseType_t Tasks_CreateQueues(void);      /* Create queues */
BaseType_t Tasks_CreateAll(void);         /* Create tasks */
```

### Queue Access
```c
QueueHandle_t Tasks_GetWheelSpeedCmdQueue(void);
QueueHandle_t Tasks_GetSensorFeedbackQueue(void);
```

---

## ⚙️ FreeRTOS Configuration

Key settings in `FreeRTOSConfig.h`:

| Setting | Value | Description |
|---------|-------|-------------|
| `configCPU_CLOCK_HZ` | 80000000 | 80 MHz system clock |
| `configTICK_RATE_HZ` | 100 | 10ms tick period |
| `configTOTAL_HEAP_SIZE` | 8192 | 8 KB heap |
| `configMAX_PRIORITIES` | 5 | Priority levels 0-4 |
| `configMINIMAL_STACK_SIZE` | 128 | 512 bytes minimum |

---

## 🔄 Task Timing

```
Time (ms)
│
0────────────10───────────20───────────30───────────40───────────50
    │           │           │           │           │           │
    ▼           ▼           ▼           ▼           ▼           ▼
  Safety     Safety      Safety      Safety      Safety      Safety
  Control    Control     Control     Control     Control     Control
              Sensor                  Sensor                  Sensor
              Comm                    Comm                    Comm
```

### Execution Order (same priority)

When tasks have the same priority (Comm and Thermal):
- Round-robin scheduling with time slicing
- Earlier created task runs first if both ready

---

## 🛡️ Error Handling

### Stack Overflow Hook

```c
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
    /* Called when stack overflow detected */
    taskDISABLE_INTERRUPTS();
    for (;;) { }  /* System halts */
}
```

### Malloc Failed Hook

```c
void vApplicationMallocFailedHook(void) {
    /* Called when pvPortMalloc() fails */
    taskDISABLE_INTERRUPTS();
    for (;;) { }  /* System halts */
}
```

---

## 💡 Usage Examples

### Example: Sending Command to Control

```c
/* In Comm Task */
void App_CommTask_HandleCommand(const char* cmd) {
    WheelSpeedCmdType wheelCmd;
    
    if (ParseWheelCommand(cmd, &wheelCmd)) {
        QueueHandle_t queue = Tasks_GetWheelSpeedCmdQueue();
        xQueueOverwrite(queue, &wheelCmd);
    }
}
```

### Example: Publishing Sensor Data

```c
/* In Sensor Task */
void App_SensorTask_PublishData(void) {
    SensorFeedbackType feedback;
    
    feedback.LeftEncoderTicks = encoder0.PositionCounts;
    feedback.RightEncoderTicks = encoder1.PositionCounts;
    feedback.YawDegrees = imuYaw;
    
    QueueHandle_t queue = Tasks_GetSensorFeedbackQueue();
    xQueueOverwrite(queue, &feedback);
}
```

---

## ⚠️ Important Notes

### Initialization Order

```
1. System_Init()        ← All drivers initialized
2. ResourceMap_Init()   ← Mutexes and queues created
3. Tasks_CreateQueues() ← Task queues created
4. Tasks_CreateAll()    ← Tasks created
5. vTaskStartScheduler()← Scheduler starts
```

### Priority Inversion

Encoder mutex access between Sensor (P2) and Control (P3):
- FreeRTOS mutexes have priority inheritance
- Higher priority task won't be blocked indefinitely

### Stack Usage

Monitor with `uxTaskGetStackHighWaterMark()`:

```c
UBaseType_t remaining = uxTaskGetStackHighWaterMark(NULL);
/* remaining * 4 = bytes remaining */
```

---

## 🐛 Troubleshooting

### System Hangs at Startup

**Checklist:**
- ✅ `configTOTAL_HEAP_SIZE` sufficient (≥8192)
- ✅ All task stacks large enough
- ✅ `ResourceMap_Init()` returns E_OK

### Queue Operations Fail

**Checklist:**
- ✅ Queue created before task uses it
- ✅ Queue handle is not NULL
- ✅ Data size matches queue item size

### Task Not Running

**Debug:**
```c
eTaskState state = eTaskGetState(TaskHandle);
/* taskREADY, taskRUNNING, taskBLOCKED, taskSUSPENDED */
```

---

## 📚 Related Modules

- **ResourceMap** - Additional queues and mutexes
- **App_SafetyTask** - Highest priority, WDG owner
- **App_ControlTask** - Motor control loop
- **App_SensorTask** - Sensor data collection
- **App_CommTask** - ROS communication
- **App_ThermalTask** - Temperature management

---

**Your FreeRTOS tasks are ready to run!** ⚡🔄
