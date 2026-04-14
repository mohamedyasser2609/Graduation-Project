/**
 * @file main_i2c_debug.c
 * @brief Minimal I2C Debug Test - Direct Register Access
 * @details Bypasses ECUAL/MCAL layers to diagnose I2C issues
 *
 * This test:
 * 1. Configures GPIO PB2/PB3 directly for I2C0
 * 2. Initializes I2C0 peripheral directly
 * 3. Attempts communication with MPU-6050
 * 4. Prints detailed register states for debugging
 *
 * @author Mohamed Yasser / Debug Version
 * @date Feb 07, 2026
 */

#if 0 /* Set to 1 to enable this test */

#include <stdint.h>
#include <stdbool.h>

/* ===================[Register Definitions]=================== */
/* System Control */
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_RCGCI2C_R        (*((volatile uint32_t *)0x400FE620))
#define SYSCTL_RCGCUART_R       (*((volatile uint32_t *)0x400FE618))
#define SYSCTL_PRGPIO_R         (*((volatile uint32_t *)0x400FEA08))
#define SYSCTL_PRI2C_R          (*((volatile uint32_t *)0x400FEA20))
#define SYSCTL_RCC_R            (*((volatile uint32_t *)0x400FE060))
#define SYSCTL_RCC2_R           (*((volatile uint32_t *)0x400FE070))
#define SYSCTL_PLLSTAT_R        (*((volatile uint32_t *)0x400FE168))

/* GPIO Port B Registers */
#define GPIO_PORTB_AFSEL_R      (*((volatile uint32_t *)0x40005420))
#define GPIO_PORTB_DEN_R        (*((volatile uint32_t *)0x4000551C))
#define GPIO_PORTB_ODR_R        (*((volatile uint32_t *)0x4000550C))
#define GPIO_PORTB_PUR_R        (*((volatile uint32_t *)0x40005510))
#define GPIO_PORTB_PCTL_R       (*((volatile uint32_t *)0x4000552C))
#define GPIO_PORTB_DATA_R       (*((volatile uint32_t *)0x400053FC))

/* GPIO Port A Registers (for UART0) */
#define GPIO_PORTA_AFSEL_R      (*((volatile uint32_t *)0x40004420))
#define GPIO_PORTA_DEN_R        (*((volatile uint32_t *)0x4000451C))
#define GPIO_PORTA_PCTL_R       (*((volatile uint32_t *)0x4000452C))

/* I2C0 Registers */
#define I2C0_MSA_R              (*((volatile uint32_t *)0x40020000))
#define I2C0_MCS_R              (*((volatile uint32_t *)0x40020004))
#define I2C0_MDR_R              (*((volatile uint32_t *)0x40020008))
#define I2C0_MTPR_R             (*((volatile uint32_t *)0x4002000C))
#define I2C0_MCR_R              (*((volatile uint32_t *)0x40020020))
#define I2C0_MBMON_R            (*((volatile uint32_t *)0x4002002C))

/* I2C MCS Bit Definitions */
#define I2C_MCS_ACK             0x00000008
#define I2C_MCS_STOP            0x00000004
#define I2C_MCS_START           0x00000002
#define I2C_MCS_RUN             0x00000001
#define I2C_MCS_BUSY            0x00000001
#define I2C_MCS_ERROR           0x00000002
#define I2C_MCS_ADRACK          0x00000004
#define I2C_MCS_DATACK          0x00000008
#define I2C_MCS_ARBLST          0x00000010
#define I2C_MCS_IDLE            0x00000020
#define I2C_MCS_BUSBSY          0x00000040
#define I2C_MCS_CLKTO           0x00000080

/* UART0 Registers (for debug output) */
#define UART0_DR_R              (*((volatile uint32_t *)0x4000C000))
#define UART0_FR_R              (*((volatile uint32_t *)0x4000C018))
#define UART0_IBRD_R            (*((volatile uint32_t *)0x4000C024))
#define UART0_FBRD_R            (*((volatile uint32_t *)0x4000C028))
#define UART0_LCRH_R            (*((volatile uint32_t *)0x4000C02C))
#define UART0_CTL_R             (*((volatile uint32_t *)0x4000C030))

/* MPU-6050 */
#define MPU6050_ADDR            0x68
#define MPU6050_WHO_AM_I        0x75

/* ===================[Helper Functions]=================== */

void delay(uint32_t count) {
    volatile uint32_t i;
    for (i = 0; i < count; i++);
}

/* Dummy handler required by startup file */
void Timer2A_Handler(void) {
    /* Empty - not used in this test */
}

/* Initialize PLL for 80MHz */
void PLL_Init_80MHz(void) {
    /* Use RCC2 for more options */
    SYSCTL_RCC2_R |= 0x80000000;  /* USERCC2 */
    SYSCTL_RCC2_R |= 0x00000800;  /* BYPASS2 - bypass PLL while configuring */
    
    /* Configure crystal: 16MHz */
    SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x000007C0) | 0x00000540;
    
    /* Select main oscillator */
    SYSCTL_RCC2_R &= ~0x00000070;
    
    /* Power on PLL */
    SYSCTL_RCC2_R &= ~0x00002000;
    
    /* Use 400MHz PLL */
    SYSCTL_RCC2_R |= 0x40000000;
    
    /* Set divider for 80MHz: 400/(4+1) = 80 */
    SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000) | (4 << 22);
    
    /* Wait for PLL to lock */
    while ((SYSCTL_PLLSTAT_R & 0x01) == 0);
    
    /* Clear BYPASS to use PLL */
    SYSCTL_RCC2_R &= ~0x00000800;
}

void UART0_Init(void) {
    /* Enable UART0 and Port A clocks */
    SYSCTL_RCGCUART_R |= 0x01;
    SYSCTL_RCGCGPIO_R |= 0x01;
    delay(100);
    
    /* Configure PA0/PA1 for UART */
    GPIO_PORTA_AFSEL_R |= 0x03;
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFFFFFF00) | 0x00000011;
    GPIO_PORTA_DEN_R |= 0x03;
    
    /* Configure UART0: 115200 baud, 8N1 @ 80MHz */
    UART0_CTL_R = 0;
    /* BRD = 80MHz / (16 * 115200) = 43.402778 */
    /* IBRD = 43, FBRD = int(0.402778 * 64 + 0.5) = 26 */
    UART0_IBRD_R = 43;
    UART0_FBRD_R = 26;
    UART0_LCRH_R = 0x70;  /* 8 bits, no parity, 1 stop, FIFO */
    UART0_CTL_R = 0x301;  /* Enable UART, TX, RX */
}

void UART0_SendChar(char c) {
    while (UART0_FR_R & 0x20);  /* Wait until TX FIFO not full */
    UART0_DR_R = c;
}

void UART0_SendString(const char* str) {
    while (*str) {
        UART0_SendChar(*str++);
    }
}

void UART0_SendHex8(uint8_t val) {
    const char hex[] = "0123456789ABCDEF";
    UART0_SendChar(hex[(val >> 4) & 0x0F]);
    UART0_SendChar(hex[val & 0x0F]);
}

void UART0_SendHex32(uint32_t val) {
    UART0_SendHex8((val >> 24) & 0xFF);
    UART0_SendHex8((val >> 16) & 0xFF);
    UART0_SendHex8((val >> 8) & 0xFF);
    UART0_SendHex8(val & 0xFF);
}

void UART0_SendInt16(int16_t val) {
    char buffer[8];
    int i = 0;
    uint16_t uval;
    
    if (val < 0) {
        UART0_SendChar('-');
        uval = (uint16_t)(-val);
    } else {
        uval = (uint16_t)val;
    }
    
    if (uval == 0) {
        UART0_SendChar('0');
        return;
    }
    
    while (uval > 0) {
        buffer[i++] = '0' + (uval % 10);
        uval /= 10;
    }
    
    while (i > 0) {
        UART0_SendChar(buffer[--i]);
    }
}

/* ===================[I2C Functions]=================== */

void I2C0_Init_Direct(void) {
    /* Enable I2C0 and GPIO Port B clocks */
    SYSCTL_RCGCI2C_R |= 0x01;
    SYSCTL_RCGCGPIO_R |= 0x02;
    
    /* Wait for peripherals to be ready */
    delay(1000);
    while ((SYSCTL_PRGPIO_R & 0x02) == 0);
    while ((SYSCTL_PRI2C_R & 0x01) == 0);
    
    /* Configure PB2 (SCL) and PB3 (SDA) for I2C */
    GPIO_PORTB_AFSEL_R |= 0x0C;           /* Enable alternate function on PB2, PB3 */
    GPIO_PORTB_ODR_R |= 0x08;             /* Enable open drain on SDA (PB3) only */
    GPIO_PORTB_PUR_R |= 0x0C;             /* Enable pull-up on PB2, PB3 */
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFFF00FF) | 0x00003300;  /* I2C function (AF3) */
    GPIO_PORTB_DEN_R |= 0x0C;             /* Enable digital on PB2, PB3 */
    
    /* Initialize I2C0 Master */
    I2C0_MCR_R = 0x10;                    /* Master function enable */
    
    /* Configure clock: 100kHz @ 80MHz system clock */
    /* TPR = (80MHz / (2 * 10 * 100kHz)) - 1 = 39 */
    I2C0_MTPR_R = 39;
    
    delay(1000);
}

bool I2C0_WaitBusy(void) {
    uint32_t timeout = 100000;
    while ((I2C0_MCS_R & I2C_MCS_BUSY) && timeout > 0) {
        timeout--;
    }
    return (timeout > 0);
}

uint8_t I2C0_ReadRegister(uint8_t slaveAddr, uint8_t regAddr) {
    uint32_t status;
    
    /* Wait for bus to be free */
    if (!I2C0_WaitBusy()) {
        UART0_SendString("  [TIMEOUT waiting for bus]\r\n");
        return 0xFF;
    }
    
    /* Send slave address + write bit + register address */
    I2C0_MSA_R = (slaveAddr << 1) | 0x00;  /* Write */
    I2C0_MDR_R = regAddr;
    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN;  /* Start + Run (no Stop for repeated start) */
    
    /* Wait for completion */
    if (!I2C0_WaitBusy()) {
        UART0_SendString("  [TIMEOUT after sending address]\r\n");
        return 0xFF;
    }
    
    /* Check for errors */
    status = I2C0_MCS_R;
    UART0_SendString("  MCS after write: 0x");
    UART0_SendHex8(status);
    UART0_SendString("\r\n");
    
    if (status & I2C_MCS_ERROR) {
        UART0_SendString("  [ERROR: ");
        if (status & I2C_MCS_ADRACK) UART0_SendString("ADDR_NACK ");
        if (status & I2C_MCS_DATACK) UART0_SendString("DATA_NACK ");
        if (status & I2C_MCS_ARBLST) UART0_SendString("ARB_LOST ");
        if (status & I2C_MCS_CLKTO) UART0_SendString("CLK_TIMEOUT ");
        UART0_SendString("]\r\n");
        I2C0_MCS_R = I2C_MCS_STOP;
        return 0xFF;
    }
    
    /* Repeated start for read */
    I2C0_MSA_R = (slaveAddr << 1) | 0x01;  /* Read */
    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP;  /* Start + Run + Stop (single byte) */
    
    /* Wait for completion */
    if (!I2C0_WaitBusy()) {
        UART0_SendString("  [TIMEOUT after read]\r\n");
        return 0xFF;
    }
    
    /* Check for errors */
    status = I2C0_MCS_R;
    UART0_SendString("  MCS after read: 0x");
    UART0_SendHex8(status);
    UART0_SendString("\r\n");
    
    if (status & I2C_MCS_ERROR) {
        UART0_SendString("  [READ ERROR]\r\n");
        return 0xFF;
    }
    
    return (uint8_t)I2C0_MDR_R;
}

void I2C0_WriteRegister(uint8_t slaveAddr, uint8_t regAddr, uint8_t data) {
    /* Wait for bus to be free */
    if (!I2C0_WaitBusy()) {
        return;
    }
    
    /* Send slave address + write bit */
    I2C0_MSA_R = (slaveAddr << 1) | 0x00;
    
    /* Send register address */
    I2C0_MDR_R = regAddr;
    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN;
    
    if (!I2C0_WaitBusy()) {
        return;
    }
    
    if (I2C0_MCS_R & I2C_MCS_ERROR) {
        I2C0_MCS_R = I2C_MCS_STOP;
        return;
    }
    
    /* Send data byte */
    I2C0_MDR_R = data;
    I2C0_MCS_R = I2C_MCS_RUN | I2C_MCS_STOP;
    
    I2C0_WaitBusy();
}

/* ===================[Main Function]=================== */

int main(void) {
    uint8_t whoAmI;
    
    /* Initialize PLL for 80MHz */
    PLL_Init_80MHz();
    delay(100000);
    
    /* Initialize UART for debug output */
    UART0_Init();
    delay(100000);
    
    UART0_SendString("\r\n\r\n");
    UART0_SendString("========================================\r\n");
    UART0_SendString("I2C Debug Test - Direct Register Access\r\n");
    UART0_SendString("========================================\r\n\r\n");
    
    /* Print GPIO and I2C clock status */
    UART0_SendString("1. Checking clocks...\r\n");
    UART0_SendString("   RCGCGPIO: 0x");
    UART0_SendHex32(SYSCTL_RCGCGPIO_R);
    UART0_SendString("\r\n");
    UART0_SendString("   RCGCI2C: 0x");
    UART0_SendHex32(SYSCTL_RCGCI2C_R);
    UART0_SendString("\r\n\r\n");
    
    /* Initialize I2C0 directly */
    UART0_SendString("2. Initializing I2C0...\r\n");
    I2C0_Init_Direct();
    
    /* Print GPIO Port B configuration */
    UART0_SendString("3. GPIO Port B Configuration:\r\n");
    UART0_SendString("   AFSEL: 0x");
    UART0_SendHex8(GPIO_PORTB_AFSEL_R);
    UART0_SendString(" (expect 0x0C for PB2/PB3)\r\n");
    UART0_SendString("   ODR:   0x");
    UART0_SendHex8(GPIO_PORTB_ODR_R);
    UART0_SendString(" (expect 0x08 for SDA open-drain)\r\n");
    UART0_SendString("   PUR:   0x");
    UART0_SendHex8(GPIO_PORTB_PUR_R);
    UART0_SendString(" (expect 0x0C for pull-ups)\r\n");
    UART0_SendString("   PCTL:  0x");
    UART0_SendHex32(GPIO_PORTB_PCTL_R);
    UART0_SendString(" (expect 0x--003300)\r\n");
    UART0_SendString("   DEN:   0x");
    UART0_SendHex8(GPIO_PORTB_DEN_R);
    UART0_SendString(" (expect 0x0C for digital enable)\r\n\r\n");
    
    /* Print I2C0 configuration */
    UART0_SendString("4. I2C0 Configuration:\r\n");
    UART0_SendString("   MCR:  0x");
    UART0_SendHex8(I2C0_MCR_R);
    UART0_SendString(" (expect 0x10 for master enable)\r\n");
    UART0_SendString("   MTPR: 0x");
    UART0_SendHex8(I2C0_MTPR_R);
    UART0_SendString(" (expect 0x27 = 39 for 100kHz)\r\n");
    UART0_SendString("   MCS:  0x");
    UART0_SendHex8(I2C0_MCS_R);
    UART0_SendString(" (bus status)\r\n\r\n");
    
    /* Check bus monitor */
    UART0_SendString("5. Bus Monitor (MBMON): 0x");
    UART0_SendHex8(I2C0_MBMON_R);
    UART0_SendString("\r\n");
    UART0_SendString("   SCL line: ");
    UART0_SendString((I2C0_MBMON_R & 0x02) ? "HIGH" : "LOW");
    UART0_SendString("\r\n");
    UART0_SendString("   SDA line: ");
    UART0_SendString((I2C0_MBMON_R & 0x01) ? "HIGH" : "LOW");
    UART0_SendString("\r\n\r\n");
    
    /* Try to read WHO_AM_I */
    UART0_SendString("6. Reading MPU-6050 WHO_AM_I (0x75)...\r\n");
    whoAmI = I2C0_ReadRegister(MPU6050_ADDR, MPU6050_WHO_AM_I);
    
    UART0_SendString("\r\nResult: WHO_AM_I = 0x");
    UART0_SendHex8(whoAmI);
    UART0_SendString("\r\n");
    
    if (whoAmI == 0x71) {
        UART0_SendString("SUCCESS! MPU-6050 detected.\r\n");
    } else if (whoAmI == 0x73) {
        UART0_SendString("SUCCESS! MPU-9255 detected.\r\n");
    } else if (whoAmI == 0xFF) {
        UART0_SendString("FAILED! No response from device.\r\n");
    } else {
        UART0_SendString("UNEXPECTED! Unknown device.\r\n");
    }
    
    /* Wake up MPU-6050 (clear sleep bit in PWR_MGMT_1) */
    UART0_SendString("\r\n7. Waking up MPU-6050...\r\n");
    {
        uint8_t pwrMgmt = 0x00;  /* Clear sleep bit, use internal oscillator */
        I2C0_WriteRegister(MPU6050_ADDR, 0x6B, pwrMgmt);
        delay(500000);  /* Wait for wake up */
    }
    
    /* Configure accelerometer and gyroscope */
    I2C0_WriteRegister(MPU6050_ADDR, 0x1C, 0x00);  /* Accel: +/- 2g */
    I2C0_WriteRegister(MPU6050_ADDR, 0x1B, 0x00);  /* Gyro: +/- 250 dps */
    delay(100000);
    
    UART0_SendString("MPU-6050 configured. Reading sensor data...\r\n\r\n");
    
    /* Continuous reading loop */
    while (1) {
        int16_t ax, ay, az;
        int16_t gx, gy, gz;
        uint8_t data[14];
        uint8_t i;
        
        /* Read 14 bytes starting from ACCEL_XOUT_H (0x3B) */
        for (i = 0; i < 14; i++) {
            data[i] = I2C0_ReadRegister(MPU6050_ADDR, 0x3B + i);
        }
        
        /* Parse accelerometer data */
        ax = (int16_t)((data[0] << 8) | data[1]);
        ay = (int16_t)((data[2] << 8) | data[3]);
        az = (int16_t)((data[4] << 8) | data[5]);
        
        /* Skip temp (bytes 6-7), parse gyroscope */
        gx = (int16_t)((data[8] << 8) | data[9]);
        gy = (int16_t)((data[10] << 8) | data[11]);
        gz = (int16_t)((data[12] << 8) | data[13]);
        
        /* Print data */
        UART0_SendString("Acc: ");
        UART0_SendInt16(ax); UART0_SendString(", ");
        UART0_SendInt16(ay); UART0_SendString(", ");
        UART0_SendInt16(az);
        UART0_SendString(" | Gyro: ");
        UART0_SendInt16(gx); UART0_SendString(", ");
        UART0_SendInt16(gy); UART0_SendString(", ");
        UART0_SendInt16(gz);
        UART0_SendString("\r\n");
        
        delay(1600000);  /* ~200ms at 80MHz */
    }
}

#endif
