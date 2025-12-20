#ifndef MCAL_ADC_ADC_CFG_H_
#define MCAL_ADC_ADC_CFG_H_

#include "../../CONFIG/Std_Types.h"
#include "Adc_Types.h"

#define ADC_VENDOR_ID                    (0x1234u)
#define ADC_MODULE_ID                    (122u)
#define ADC_INSTANCE_ID                  (0u)

#define ADC_SW_MAJOR_VERSION             (1u)
#define ADC_SW_MINOR_VERSION             (0u)
#define ADC_SW_PATCH_VERSION             (0u)

#define ADC_AR_RELEASE_MAJOR_VERSION     (4u)
#define ADC_AR_RELEASE_MINOR_VERSION     (4u)
#define ADC_AR_RELEASE_PATCH_VERSION     (0u)

#define ADC_DEV_ERROR_DETECT             STD_ON
#define ADC_VERSION_INFO_API             STD_ON

#define ADC_CONFIGURED_GROUPS            (2u)

#define ADC_E_PARAM_CONFIG               (0x01u)
#define ADC_E_PARAM_POINTER              (0x02u)
#define ADC_E_UNINIT                     (0x03u)
#define ADC_E_ALREADY_INITIALIZED        (0x04u)
#define ADC_E_PARAM_GROUP                (0x05u)
#define ADC_E_BUSY                       (0x06u)
#define ADC_E_BUFFER_UNINIT              (0x07u)
#define ADC_E_PARAM_CHANNEL              (0x08u)
#define ADC_E_PARAM_TRIGGER              (0x09u)

#define ADC_INIT_SID                     (0x00u)
#define ADC_DEINIT_SID                   (0x01u)
#define ADC_SETUP_RESULT_BUFFER_SID      (0x02u)
#define ADC_START_GROUP_CONVERSION_SID   (0x03u)
#define ADC_STOP_GROUP_CONVERSION_SID    (0x04u)
#define ADC_READ_GROUP_SID               (0x05u)
#define ADC_GET_GROUP_STATUS_SID         (0x06u)
#define ADC_ENABLE_HW_TRIGGER_SID        (0x07u)
#define ADC_DISABLE_HW_TRIGGER_SID       (0x08u)
#define ADC_ENABLE_NOTIFICATION_SID      (0x09u)
#define ADC_DISABLE_NOTIFICATION_SID     (0x0Au)
#define ADC_GET_VERSION_INFO_SID         (0x0Bu)
#define ADC_MAIN_FUNCTION_HANDLING_SID   (0x0Cu)

extern const Adc_ConfigType Adc_Config;

#endif /* MCAL_ADC_ADC_CFG_H_ */

