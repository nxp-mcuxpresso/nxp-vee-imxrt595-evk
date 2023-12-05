IF(NOT DEFINED FPU)
    SET(FPU "-mfloat-abi=hard -mfpu=fpv5-sp-d16")
ENDIF()

SET(CMAKE_ASM_FLAGS_DEBUG " \
    ${CMAKE_ASM_FLAGS_DEBUG} \
    -DDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -mcpu=cortex-m33 \
    -mthumb \
    ${FPU} \
")

SET(CMAKE_C_FLAGS_DEBUG " \
    ${CMAKE_C_FLAGS_DEBUG} \
    -DCPU_MIMXRT595SFFOC_cm33 \
    -DDEBUG \
    -mcpu=cortex-m33 \
    -mthumb -nostdlib \
    -O0 \
    -fno-common \
    -g3 \
    -Wall \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin-specs=nano.specs \
    -MMD \
    -MP \
    ${FPU} \
")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG " \
    -Xlinker --gc-sections \
    -Xlinker -print-memory-usage \
    -Xlinker --sort-section=alignment \
    -Xlinker -Map=${ProjDirPath}/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}.map \
    -u _printf_float \
    -L${ProjDirPath} \
    -T${ProjDirPath}/mimxrt595_freertos-bsp_Debug.ld \
")

SET(CMAKE_ASM_FLAGS_RELEASE " \
    ${CMAKE_ASM_FLAGS_RELEASE} \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -mcpu=cortex-m33 \
    -mthumb \
    ${FPU} \
")

SET(CMAKE_C_FLAGS_RELEASE " \
    ${CMAKE_C_FLAGS_RELEASE} \
    -DCPU_MIMXRT595SFFOC_cm33 \
    -mcpu=cortex-m33 \
    -mthumb -nostdlib \
    -O3 \
    -fno-common \
    -g3 \
    -Wall \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin-specs=nano.specs \
    -MMD \
    -MP \
    ${FPU} \
")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE " \
    -Xlinker --gc-sections \
    -Xlinker -print-memory-usage \
    -Xlinker --sort-section=alignment \
    -Xlinker -Map=${ProjDirPath}/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}.map \
    -u _printf_float \
    -L${ProjDirPath} \
    -T${ProjDirPath}/mimxrt595_freertos-bsp_Debug.ld \
")
