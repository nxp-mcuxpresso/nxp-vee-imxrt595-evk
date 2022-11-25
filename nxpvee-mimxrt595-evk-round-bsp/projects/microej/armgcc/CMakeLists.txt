# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.0)

# CURRENT DIRECTORY
SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/.)
message(${EXECUTABLE_OUTPUT_PATH})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath})

project(mimxrt595_freertos-bsp C)

set(MCUX_DEVICE MIMXRT595S_cm33)
set(PROJECT_NAME mimxrt595_freertos-bsp)
set(MCUX_SDK_PROJECT_NAME ${PROJECT_NAME}.axf)

# config to select component, the format is CONFIG_USE_${component}
set(CONFIG_USE_component_serial_manager_uart_MIMXRT595S_cm33 true)
set(CONFIG_USE_driver_flexcomm_usart_MIMXRT595S_cm33 true)

add_executable(${MCUX_SDK_PROJECT_NAME}
    "${ProjDirPath}/../board/src/board.c"
    "${ProjDirPath}/../board/src/clock_config.c"
    "${ProjDirPath}/../board/src/display_support.c"
    "${ProjDirPath}/../board/src/pin_mux.c"
    "${ProjDirPath}/../freertos/src/SEGGER_SYSVIEW_Config_FreeRTOS.c"
    "${ProjDirPath}/../simple_gfx_app/src/simple_gfx_app_imp.c"
    "${ProjDirPath}/../main/src/cpuload.c"
    "${ProjDirPath}/../main/src/cpuload_impl_FreeRTOS.c"
    "${ProjDirPath}/../main/src/fault_handlers.c"
    "${ProjDirPath}/../main/src/interrupts.c"
    "${ProjDirPath}/../main/src/LLBSP.c"
    "${ProjDirPath}/../main/src/LLEXT_RES_impl.c"
    "${ProjDirPath}/../main/src/LLMJVM_FreeRTOS.c"
    "${ProjDirPath}/../main/src/main.c"
    "${ProjDirPath}/../main/src/microjvm_main.c"
    "${ProjDirPath}/../main/src/monitor.c"
    "${ProjDirPath}/../main/src/os_support.c"
    "${ProjDirPath}/../main/src/stack_overflow_impl_FreeRTOS.c"
    "${ProjDirPath}/../main/src/time_hardware_timer.c"
    "${ProjDirPath}/../main/src/trace_platform.c"
    "${ProjDirPath}/../osal/src/osal_FreeRTOS.c"
    "${ProjDirPath}/../lowpower/src/power_manager.c"
    "${ProjDirPath}/../lowpower/src/fsl_tickless_rtc.c"
    "${ProjDirPath}/../thirdparty/freetype/src/wrappers/ft_base_wrapper.c"
    "${ProjDirPath}/../thirdparty/freetype/src/wrappers/ft_otf_wrapper.c"
    "${ProjDirPath}/../thirdparty/freetype/src/wrappers/ft_ttf_wrapper.c"
    "${ProjDirPath}/../thirdparty/freetype/src/wrappers/ft_vector_wrapper.c"
    "${ProjDirPath}/../thirdparty/harfbuzz/src/hb-alloc.c"
    "${ProjDirPath}/../thirdparty/systemview/src/SEGGER_RTT.c"
    "${ProjDirPath}/../thirdparty/systemview/src/SEGGER_SYSVIEW.c"
    "${ProjDirPath}/../thirdparty/systemview-freertos/src/SEGGER_SYSVIEW_FreeRTOS.c"
    "${ProjDirPath}/../trace/src/LLMJVM_MONITOR_sysview.c"
    "${ProjDirPath}/../trace/src/LLTRACE_sysview.c"
    "${ProjDirPath}/../ui/src/buttons_helper.c"
    "${ProjDirPath}/../ui/src/buttons_manager.c"
    "${ProjDirPath}/../ui/src/display_dma.c"
    "${ProjDirPath}/../ui/src/display_framebuffer.c"
    "${ProjDirPath}/../ui/src/display_impl.c"
    "${ProjDirPath}/../ui/src/display_utils.c"
    "${ProjDirPath}/../ui/src/display_vglite.c"
    "${ProjDirPath}/../ui/src/drawing_vglite.c"
    "${ProjDirPath}/../ui/src/event_generator.c"
    "${ProjDirPath}/../ui/src/framerate.c"
    "${ProjDirPath}/../ui/src/framerate_impl_FreeRTOS.c"
    "${ProjDirPath}/../ui/src/LLDW_PAINTER_impl.c"
    "${ProjDirPath}/../ui/src/LLUI_DISPLAY_impl.c"
    "${ProjDirPath}/../ui/src/LLUI_INPUT_impl.c"
    "${ProjDirPath}/../ui/src/LLUI_PAINTER_impl.c"
    "${ProjDirPath}/../ui/src/touch_helper.c"
    "${ProjDirPath}/../ui/src/touch_manager.c"
    "${ProjDirPath}/../ui/src/vg_drawer.c"
    "${ProjDirPath}/../ui/src/vglite_path.c"
    "${ProjDirPath}/../util/src/mej_debug.c"
    "${ProjDirPath}/../util/src/mej_math.c"
    "${ProjDirPath}/../util/src/pool.c"
    "${ProjDirPath}/../vg/src/freetype_bitmap_helper.c"
    "${ProjDirPath}/../vg/src/LLVG_BVI_impl.c"
    "${ProjDirPath}/../vg/src/LLVG_FONT_freetype.c"
    "${ProjDirPath}/../vg/src/LLVG_FONT_PAINTER_freetype_bitmap.c"
    "${ProjDirPath}/../vg/src/LLVG_FONT_PAINTER_freetype_vglite.c"
    "${ProjDirPath}/../vg/src/LLVG_FONT_stub.c"
    "${ProjDirPath}/../vg/src/LLVG_GRADIENT_impl.c"
    "${ProjDirPath}/../vg/src/LLVG_impl.c"
    "${ProjDirPath}/../vg/src/LLVG_MATRIX_impl.c"
    "${ProjDirPath}/../vg/src/LLVG_PATH_impl.c"
    "${ProjDirPath}/../vg/src/LLVG_PATH_PAINTER_vglite.c"
    "${ProjDirPath}/../vg/src/LLVG_PATH_stub.c"
    "${ProjDirPath}/../vg/src/LLVG_vglite.c"
    "${ProjDirPath}/../vg/src/microvg_helper.c"
    "${ProjDirPath}/../vglite_support/vglite_support.c"
    "${ProjDirPath}/../vglite_window/vglite_window.c"
    "${ProjDirPath}/../stub/src/stub.c"
    "${ProjDirPath}/../../../sdk/devices/MIMXRT595S/mcuxpresso/startup_mimxrt595s_cm33.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME}  PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../platform/inc
    ${ProjDirPath}/../board/inc
    ${ProjDirPath}/../freertos/inc
    ${ProjDirPath}/../main/inc
    ${ProjDirPath}/../lowpower/inc
    ${ProjDirPath}/../trace/inc
    ${ProjDirPath}/../ui/inc
    ${ProjDirPath}/../util/inc
    ${ProjDirPath}/../vg/inc
    ${ProjDirPath}/../thirdparty/systemview/inc
    ${ProjDirPath}/../thirdparty/systemview-freertos/inc
    ${ProjDirPath}/../thirdparty/freetype/inc
    ${ProjDirPath}/../osal/inc
    ${ProjDirPath}/../platform/inc
    ${ProjDirPath}/../vglite_support
    ${ProjDirPath}/../vglite_window
    ${ProjDirPath}/../thirdparty/harfbuzz/inc
)

add_definitions(
    -D__USE_CMSIS
    -DDEBUG
    -DFSL_SDK_DRIVER_QUICK_ACCESS_ENABLE=1
    -DBOOT_HEADER_ENABLE=1
    -DCPU_MIMXRT595SFFOB_cm33
    -DBOARD_ENABLE_PSRAM_CACHE=0
    -DFLEXIO_MCULCD_DATA_BUS_WIDTH=8
    -DDBI_FLEXIO_USE_SMARTDMA=1
    -DCUSTOM_VGLITE_MEMORY_CONFIG=1
    -DVG_RESOLVE_ENGINE=0
    -DVG_PE_COLOR_KEY=0
    -DVG_IM_INDEX_FORMAT=0
    -DVG_AYUV_INPUT_OUTPUT=0
    -DVG_DOUBLE_IMAGE=0
    -DVG_RECTANGLE_STRIP_MODE=0
    -DVG_MMU=0
    -DVG_DRIVER_SINGLE_THREAD=1
    -DVG_IM_FILTER=0
    -DVG_IM_YUV_PACKET=1
    -DVG_IM_YUV_PLANAR=0
    -DVG_PE_YUV_PACKET=1
    -DVG_TARGET_TILED=1
    -DVG_COMMAND_CALL=1
    -DVG_SHARE_BUFFER_IM_16K=0
    -DVG_OFFLINE_MODE=0
    -DVG_RESOLUTION_2880=0
    -DVG_PE_PREMULTIPLY=0
    -DVG_POST_CONVERTER=0
    -DVG_PRE_CONVERTER=0
    -DVG_RENDER_BY_MESH=0
    -DVG_TARGET_FAST_CLEAR=0
    -DVG_BUFFER_NUMBER_OF_TARGET=0
    -DVG_VIDEO_CLEAR_CONTROL=0
    -DVG_VIDEO_CONTROL=0
    -DVGLITE_TST_FIRMWARE=0
    -DVG_LITE_SYS_GPU_CTRL=0
    -DSERIAL_PORT_TYPE_UART=1
    -DFSL_RTOS_FREE_RTOS
    -DPRINTF_FLOAT_ENABLE=1
    -DENABLE_SVIEW=1
    -DSDK_I2C_BASED_COMPONENT_USED=1
    -D_VG_LITE_IRQ_CALLBACK=1
    -DSL_WFX_PROD_KEY=1
    -DLWIP_2_1_2
    -DSDK_DEBUGCONSOLE_UART
    -DFT_CONFIG_MODULES_H=\"freetype/config/ftmodule.h\"
    -DFT2_BUILD_LIBRARY
    -DRTT_USE_ASM=0
)

SET(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../sdk/boards/evkmimxrt595/flash_config/
    ${ProjDirPath}/../../../sdk/components/ft3267
    ${ProjDirPath}/../../../sdk/components/serial_manager
    ${ProjDirPath}/../../../sdk/components/pca9420
    ${ProjDirPath}/../../../sdk/components/lists
    ${ProjDirPath}/../../../sdk/components/uart
    ${ProjDirPath}/../../../sdk/components/video
    ${ProjDirPath}/../../../sdk/components/video/display
    ${ProjDirPath}/../../../sdk/components/video/display/dbi
    ${ProjDirPath}/../../../sdk/components/video/display/dbi/flexio
    ${ProjDirPath}/../../../sdk/components/video/display/dc
    ${ProjDirPath}/../../../sdk/components/video/display/dc/ssd1963
    ${ProjDirPath}/../../../sdk/components/video/display/dc/lcdif
    ${ProjDirPath}/../../../sdk/components/video/display/dc/dsi_cmd
    ${ProjDirPath}/../../../sdk/components/video/display/fbdev
    ${ProjDirPath}/../../../sdk/components/video/display/mipi_dsi_cmd
    ${ProjDirPath}/../../../sdk/components/video/display/rm67162
    ${ProjDirPath}/../../../sdk/components/video/display/rm68191
    ${ProjDirPath}/../../../sdk/components/video/display/rm68200
    ${ProjDirPath}/../../../sdk/CMSIS/Core/Include
    ${ProjDirPath}/../../../sdk/CMSIS/DSP
    ${ProjDirPath}/../../../sdk/devices/MIMXRT595S
    ${ProjDirPath}/../../../sdk/devices/MIMXRT595S/drivers
    ${ProjDirPath}/../../../sdk/devices/MIMXRT595S/utilities
    ${ProjDirPath}/../../../sdk/devices/MIMXRT595S/utilities/debug_console/
    ${ProjDirPath}/../../../sdk/middleware/vglite
    ${ProjDirPath}/../../../sdk/rtos/amazon-freertos/freertos_kernel

)

# include modules
include(driver_pca9420_MIMXRT595S_cm33)
include(component_usart_adapter_MIMXRT595S_cm33)
include(component_lists_MIMXRT595S_cm33)
include(component_serial_manager_MIMXRT595S_cm33)
include(component_serial_manager_uart_MIMXRT595S_cm33)  
include(device_MIMXRT595S_CMSIS_MIMXRT595S_cm33)
include(driver_cache_cache64_MIMXRT595S_cm33)
include(driver_clock_MIMXRT595S_cm33)
include(driver_common_MIMXRT595S_cm33)
include(driver_ctimer_MIMXRT595S_cm33)
include(driver_dbi_flexio_smartdma_MIMXRT595S_cm33)
include(driver_dc-fb-dsi-cmd_MIMXRT595S_cm33)
include(driver_display-rm67162_MIMXRT595S_cm33)
include(driver_display-rm68191_MIMXRT595S_cm33)
include(driver_display-rm68200_MIMXRT595S_cm33)
include(driver_fbdev_MIMXRT595S_cm33)
include(driver_flash_config_evkmimxrt595_MIMXRT595S_cm33)
include(driver_flexcomm_i2c_MIMXRT595S_cm33)
include(driver_flexcomm_MIMXRT595S_cm33)
include(driver_flexcomm_usart_dma_MIMXRT595S_cm33)
include(driver_flexcomm_usart_MIMXRT595S_cm33)
include(driver_flexio_MIMXRT595S_cm33)
include(driver_flexio_spi_MIMXRT595S_cm33)
include(driver_flexspi_MIMXRT595S_cm33)
include(driver_ft3267_MIMXRT595S_cm33)
include(driver_iap_MIMXRT595S_cm33)
include(driver_inputmux_MIMXRT595S_cm33)
include(driver_lpc_dma_MIMXRT595S_cm33)
include(driver_lpc_gpio_MIMXRT595S_cm33)
include(driver_lpc_rtc_MIMXRT595S_cm33)
include(driver_lpc_smartdma_MIMXRT595S_cm33)
include(driver_mipi_dsi_MIMXRT595S_cm33)
include(driver_mipi_dsi_smartdma_MIMXRT595S_cm33)
include(driver_power_MIMXRT595S_cm33)
include(driver_powerquad_cmsis_MIMXRT595S_cm33)
include(driver_powerquad_MIMXRT595S_cm33)
include(driver_reset_MIMXRT595S_cm33)
include(driver_trng_MIMXRT595S_cm33)
include(driver_video-common_MIMXRT595S_cm33)
include(middleware_freertos-kernel_cm33_nonsecure_port)
include(middleware_freertos-kernel_heap_4)
include(middleware_freertos-kernel_MIMXRT595S_cm33)
include(middleware_vglite_MIMXRT595S_cm33)
include(utilities_misc_utilities_MIMXRT595S_cm33)
include(utility_assert_MIMXRT595S_cm33)
include(utility_debug_console_MIMXRT595S_cm33)


SET(CMAKE_ASM_FLAGS_RELEASE " \
    ${CMAKE_ASM_FLAGS_RELEASE} \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -mcpu=cortex-m33 \
    -mfloat-abi=hard \
    -mfpu=fpv5-sp-d16 \
    -mthumb \
")

SET(CMAKE_C_FLAGS_RELEASE " \
    ${CMAKE_C_FLAGS_RELEASE} \
    -DCPU_MIMXRT595SFFOC_cm33\
    -mcpu=cortex-m33\
    -mfpu=fpv5-sp-d16\
    -mfloat-abi=hard\
    -mthumb -nostdlib\
    -O3\
    -fno-common\
    -g3\
    -Wall\
    -ffunction-sections\
    -fdata-sections\
    -ffreestanding\
    -fno-builtin-specs=nano.specs\
    -MMD\
    -MP\
")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE " \
    -Xlinker --gc-sections \
    -Xlinker -print-memory-usage \
    -Xlinker --sort-section=alignment \
    -Xlinker -Map=${PROJECT_NAME}.map\
    -T mimxrt595_freertos-bsp_Debug.ld\
")

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE 
    ${ProjDirPath}/../platform/lib/microejapp.o
    ${ProjDirPath}/../platform/lib/microejruntime.a
    ${ProjDirPath}/../thirdparty/harfbuzz/lib/harfbuzz.a
)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.bin)
ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Oihex ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.hex)
