################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/TI Code Composer/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/bin/cl430" -vmspx --abi=eabi --data_model=restricted --use_hw_mpy=F5 --include_path="D:/Google Drive/Local Projects/_ECE322/CodeStudioWorkspace/Assignment 34/driverlib/MSP430FR5xx_6xx" --include_path="C:/TI Code Composer/ccsv6/ccs_base/msp430/include" --include_path="C:/TI Code Composer/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/include" --advice:power=all --advice:hw_config="all" -g --define=__MSP430FR6989__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

myClocksWithCrystals.obj: ../myClocksWithCrystals.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/TI Code Composer/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/bin/cl430" -vmspx --abi=eabi --data_model=restricted --use_hw_mpy=F5 --include_path="D:/Google Drive/Local Projects/_ECE322/CodeStudioWorkspace/Assignment 34/driverlib/MSP430FR5xx_6xx" --include_path="C:/TI Code Composer/ccsv6/ccs_base/msp430/include" --include_path="C:/TI Code Composer/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/include" --advice:power=all --advice:hw_config="all" -g --define=__MSP430FR6989__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="myClocksWithCrystals.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

myGpio.obj: ../myGpio.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/TI Code Composer/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/bin/cl430" -vmspx --abi=eabi --data_model=restricted --use_hw_mpy=F5 --include_path="D:/Google Drive/Local Projects/_ECE322/CodeStudioWorkspace/Assignment 34/driverlib/MSP430FR5xx_6xx" --include_path="C:/TI Code Composer/ccsv6/ccs_base/msp430/include" --include_path="C:/TI Code Composer/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/include" --advice:power=all --advice:hw_config="all" -g --define=__MSP430FR6989__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="myGpio.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

myLcd.obj: ../myLcd.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/TI Code Composer/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/bin/cl430" -vmspx --abi=eabi --data_model=restricted --use_hw_mpy=F5 --include_path="D:/Google Drive/Local Projects/_ECE322/CodeStudioWorkspace/Assignment 34/driverlib/MSP430FR5xx_6xx" --include_path="C:/TI Code Composer/ccsv6/ccs_base/msp430/include" --include_path="C:/TI Code Composer/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/include" --advice:power=all --advice:hw_config="all" -g --define=__MSP430FR6989__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="myLcd.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


