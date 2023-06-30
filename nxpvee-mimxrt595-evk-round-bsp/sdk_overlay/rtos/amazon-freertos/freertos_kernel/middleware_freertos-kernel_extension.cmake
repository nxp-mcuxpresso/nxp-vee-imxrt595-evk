#Description: FreeRTOS NXP extension; user_visible: False
include_guard(GLOBAL)
message("middleware_freertos-kernel_extension component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)


