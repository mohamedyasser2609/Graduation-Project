################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/Source/%.obj: ../FreeRTOS/Source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="D:/graduation project/ccs project workspace/Graduation_Project" --include_path="D:/graduation project/ccs project workspace/Graduation_Project/SERVICES/RTOS" --include_path="D:/graduation project/ccs project workspace/Graduation_Project/CONFIG" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --include_path="D:/graduation project/ccs project workspace/Graduation_Project/FreeRTOS/Source/include" --include_path="D:/graduation project/ccs project workspace/Graduation_Project/FreeRTOS/Source/portable/CCS/ARM_CM3" --define=ccs="ccs" --define=PART_TM4C123GH6PM -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="FreeRTOS/Source/$(basename $(<F)).d_raw" --obj_directory="FreeRTOS/Source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


