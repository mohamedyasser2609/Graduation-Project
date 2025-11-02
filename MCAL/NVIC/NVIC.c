/**
 * @file NVIC.c
 * @brief NVIC Driver Implementation for TM4C123GH6PM
 */

#include "NVIC.h"
#include "tm4c123gh6pm.h"

/* ===================[Private Defines]=================== */
#define NVIC_EN_BASE_ADDR            (0xE000E100u)
#define NVIC_DIS_BASE_ADDR           (0xE000E180u)
#define NVIC_PEND_BASE_ADDR          (0xE000E200u)
#define NVIC_UNPEND_BASE_ADDR        (0xE000E280u)
#define NVIC_ACTIVE_BASE_ADDR        (0xE000E300u)
#define NVIC_PRIORITY_BASE_ADDR      (0xE000E400u)

#define NVIC_REG32(base, index)      (*((volatile uint32*)((base) + ((uint32)(index) * 4u))))
#define NVIC_BIT_POS(vector)         ((uint32)(vector) & 0x1Fu)
#define NVIC_REG_INDEX(vector)       ((uint32)(vector) >> 5)

#define NVIC_PRIORITY_FIELD_SHIFT(vector)   (((uint32)(vector) & 0x3u) * 8u + 5u)
#define NVIC_PRIORITY_FIELD_MASK(vector)    (0x7u << NVIC_PRIORITY_FIELD_SHIFT(vector))

#define NVIC_APINT_VECTKEY_VALUE     (0x05FAu)
#define NVIC_APINT_PRIGROUP_MASK     (0x00000700u)

/* ===================[Private Variables]=================== */
static boolean NVIC_ModuleInitialized = FALSE;

/* ===================[Private Helper Functions]=================== */
static boolean NVIC_IsVectorValid(NVIC_InterruptVectorType vector)
{
    return (vector < NVIC_NUM_INTERRUPTS);
}

static volatile uint32* NVIC_GetPriorityReg(NVIC_InterruptVectorType vector)
{
    uint32 index = ((uint32)vector) >> 2;
    return (volatile uint32*)(NVIC_PRIORITY_BASE_ADDR + (index * 4u));
}

static void NVIC_InternalSetPriority(NVIC_InterruptVectorType vector, NVIC_PriorityType priority)
{
    volatile uint32* reg = NVIC_GetPriorityReg(vector);
    uint32 fieldMask = NVIC_PRIORITY_FIELD_MASK(vector);
    uint32 shift = NVIC_PRIORITY_FIELD_SHIFT(vector);
    uint32 value = *reg;

    value &= ~fieldMask;
    value |= ((uint32)(priority & 0x7u) << shift);
    *reg = value;
}

static NVIC_PriorityType NVIC_InternalGetPriority(NVIC_InterruptVectorType vector)
{
    volatile uint32* reg = NVIC_GetPriorityReg(vector);
    uint32 shift = NVIC_PRIORITY_FIELD_SHIFT(vector);
    uint32 value = *reg;

    return (NVIC_PriorityType)((value >> shift) & 0x7u);
}

static void NVIC_InternalWriteReg(uint32 baseAddr, NVIC_InterruptVectorType vector, boolean setBit)
{
    uint32 index = NVIC_REG_INDEX(vector);
    uint32 bit = NVIC_BIT_POS(vector);
    volatile uint32* reg = (volatile uint32*)(baseAddr + (index * 4u));

    if (setBit == TRUE)
    {
        *reg |= (1u << bit);
    }
    else
    {
        *reg &= ~(1u << bit);
    }
}

static uint32 NVIC_InternalReadReg(uint32 baseAddr, NVIC_InterruptVectorType vector)
{
    uint32 index = NVIC_REG_INDEX(vector);
    volatile uint32* reg = (volatile uint32*)(baseAddr + (index * 4u));
    uint32 bit = NVIC_BIT_POS(vector);

    return (*reg & (1u << bit));
}

/* ===================[Public API]=================== */
void NVIC_Init(const NVIC_ConfigType* ConfigPtr)
{
#if (NVIC_DEV_ERROR_DETECT == STD_ON)
    if (ConfigPtr == NULL_PTR)
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_INIT_SID, NVIC_E_PARAM_POINTER);
        return;
    }

    if (NVIC_ModuleInitialized == TRUE)
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_INIT_SID, NVIC_E_ALREADY_INITIALIZED);
        return;
    }
#endif

    /* Configure priority grouping */
    uint32 apintValue = NVIC_APINT_R & 0x0000FFFFu;
    apintValue &= ~NVIC_APINT_PRIGROUP_MASK;
    apintValue |= ((uint32)ConfigPtr->PriorityGrouping << 8u);
    NVIC_APINT_R = ((uint32)NVIC_APINT_VECTKEY_VALUE << 16u) | apintValue;

    /* Apply interrupt configuration */
    if ((ConfigPtr->Interrupts != NULL_PTR) && (ConfigPtr->InterruptCount > 0u))
    {
        uint16 idx;
        for (idx = 0u; idx < ConfigPtr->InterruptCount; idx++)
        {
            NVIC_InterruptVectorType vector = ConfigPtr->Interrupts[idx].Vector;
            NVIC_PriorityType priority = ConfigPtr->Interrupts[idx].Priority;
            boolean enable = ConfigPtr->Interrupts[idx].Enable;

            if (!NVIC_IsVectorValid(vector))
            {
#if (NVIC_DEV_ERROR_DETECT == STD_ON)
                Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_INIT_SID, NVIC_E_PARAM_VECTOR);
#endif
                continue;
            }

            NVIC_InternalSetPriority(vector, priority);

            if (enable == TRUE)
            {
                NVIC_InternalWriteReg(NVIC_EN_BASE_ADDR, vector, TRUE);
            }
            else
            {
                NVIC_InternalWriteReg(NVIC_DIS_BASE_ADDR, vector, TRUE);
            }
        }
    }

    NVIC_ModuleInitialized = TRUE;
}

void NVIC_EnableIRQ(NVIC_InterruptVectorType Vector)
{
#if (NVIC_DEV_ERROR_DETECT == STD_ON)
    if (NVIC_ModuleInitialized == FALSE)
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_ENABLE_IRQ_SID, NVIC_E_UNINIT);
        return;
    }
    if (!NVIC_IsVectorValid(Vector))
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_ENABLE_IRQ_SID, NVIC_E_PARAM_VECTOR);
        return;
    }
#endif

    NVIC_InternalWriteReg(NVIC_EN_BASE_ADDR, Vector, TRUE);
}

void NVIC_DisableIRQ(NVIC_InterruptVectorType Vector)
{
#if (NVIC_DEV_ERROR_DETECT == STD_ON)
    if (NVIC_ModuleInitialized == FALSE)
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_DISABLE_IRQ_SID, NVIC_E_UNINIT);
        return;
    }
    if (!NVIC_IsVectorValid(Vector))
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_DISABLE_IRQ_SID, NVIC_E_PARAM_VECTOR);
        return;
    }
#endif

    NVIC_InternalWriteReg(NVIC_DIS_BASE_ADDR, Vector, TRUE);
}

void NVIC_SetPriority(NVIC_InterruptVectorType Vector, NVIC_PriorityType Priority)
{
#if (NVIC_DEV_ERROR_DETECT == STD_ON)
    if (NVIC_ModuleInitialized == FALSE)
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_SET_PRIORITY_SID, NVIC_E_UNINIT);
        return;
    }
    if (!NVIC_IsVectorValid(Vector))
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_SET_PRIORITY_SID, NVIC_E_PARAM_VECTOR);
        return;
    }
    if (Priority > NVIC_MAX_PRIORITY_LEVEL)
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_SET_PRIORITY_SID, NVIC_E_PARAM_PRIORITY);
        return;
    }
#endif

    NVIC_InternalSetPriority(Vector, Priority);
}

NVIC_PriorityType NVIC_GetPriority(NVIC_InterruptVectorType Vector)
{
#if (NVIC_DEV_ERROR_DETECT == STD_ON)
    if (NVIC_ModuleInitialized == FALSE)
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_GET_PRIORITY_SID, NVIC_E_UNINIT);
        return 0u;
    }
    if (!NVIC_IsVectorValid(Vector))
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_GET_PRIORITY_SID, NVIC_E_PARAM_VECTOR);
        return 0u;
    }
#endif

    return NVIC_InternalGetPriority(Vector);
}

void NVIC_SetPendingIRQ(NVIC_InterruptVectorType Vector)
{
#if (NVIC_DEV_ERROR_DETECT == STD_ON)
    if (NVIC_ModuleInitialized == FALSE)
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_SET_PENDING_IRQ_SID, NVIC_E_UNINIT);
        return;
    }
    if (!NVIC_IsVectorValid(Vector))
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_SET_PENDING_IRQ_SID, NVIC_E_PARAM_VECTOR);
        return;
    }
#endif

    NVIC_REG32(NVIC_PEND_BASE_ADDR, NVIC_REG_INDEX(Vector)) = (1u << NVIC_BIT_POS(Vector));
}

void NVIC_ClearPendingIRQ(NVIC_InterruptVectorType Vector)
{
#if (NVIC_DEV_ERROR_DETECT == STD_ON)
    if (NVIC_ModuleInitialized == FALSE)
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_CLEAR_PENDING_IRQ_SID, NVIC_E_UNINIT);
        return;
    }
    if (!NVIC_IsVectorValid(Vector))
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_CLEAR_PENDING_IRQ_SID, NVIC_E_PARAM_VECTOR);
        return;
    }
#endif

    NVIC_REG32(NVIC_UNPEND_BASE_ADDR, NVIC_REG_INDEX(Vector)) = (1u << NVIC_BIT_POS(Vector));
}

boolean NVIC_IsActiveIRQ(NVIC_InterruptVectorType Vector)
{
#if (NVIC_DEV_ERROR_DETECT == STD_ON)
    if (NVIC_ModuleInitialized == FALSE)
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_IS_ACTIVE_IRQ_SID, NVIC_E_UNINIT);
        return FALSE;
    }
    if (!NVIC_IsVectorValid(Vector))
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_IS_ACTIVE_IRQ_SID, NVIC_E_PARAM_VECTOR);
        return FALSE;
    }
#endif

    return (NVIC_InternalReadReg(NVIC_ACTIVE_BASE_ADDR, Vector) != 0u) ? TRUE : FALSE;
}

void NVIC_GenerateSoftwareInterrupt(NVIC_InterruptVectorType Vector)
{
#if (NVIC_DEV_ERROR_DETECT == STD_ON)
    if (NVIC_ModuleInitialized == FALSE)
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_GENERATE_SW_IRQ_SID, NVIC_E_UNINIT);
        return;
    }
    if (!NVIC_IsVectorValid(Vector))
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_GENERATE_SW_IRQ_SID, NVIC_E_PARAM_VECTOR);
        return;
    }
#endif

    NVIC_SW_TRIG_R = (uint32)Vector;
}

#if (NVIC_VERSION_INFO_API == STD_ON)
void NVIC_GetVersionInfo(Std_VersionInfoType* versionInfoPtr)
{
#if (NVIC_DEV_ERROR_DETECT == STD_ON)
    if (versionInfoPtr == NULL_PTR)
    {
        Det_ReportError(NVIC_MODULE_ID, NVIC_INSTANCE_ID, NVIC_GET_VERSION_INFO_SID, NVIC_E_PARAM_POINTER);
        return;
    }
#endif

    versionInfoPtr->vendorID = NVIC_VENDOR_ID;
    versionInfoPtr->moduleID = NVIC_MODULE_ID;
    versionInfoPtr->sw_major_version = NVIC_SW_MAJOR_VERSION;
    versionInfoPtr->sw_minor_version = NVIC_SW_MINOR_VERSION;
    versionInfoPtr->sw_patch_version = NVIC_SW_PATCH_VERSION;
}
#endif

