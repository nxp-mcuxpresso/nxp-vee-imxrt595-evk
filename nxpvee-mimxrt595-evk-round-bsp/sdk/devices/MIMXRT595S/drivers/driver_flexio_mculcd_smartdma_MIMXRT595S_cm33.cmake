include_guard()
message("driver_flexio_mculcd_smartdma component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/fsl_flexio_mculcd_smartdma.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)


include(driver_flexio_mculcd_MIMXRT595S_cm33)

include(driver_lpc_smartdma_MIMXRT595S_cm33)

