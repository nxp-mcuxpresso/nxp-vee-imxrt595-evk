/*
 * Copyright 2019-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2022 MicroEJ Corp. This file has been modified by MicroEJ Corp.
 * 1. Remove irrelevant code
 */

#include "display_support.h"
#include "fsl_gpio.h"
#include "board.h"
#include "fsl_dc_fb_dsi_cmd.h"
#include "fsl_rm67162.h"
#include "fsl_mipi_dsi.h"
#include "fsl_inputmux.h"
#include "fsl_mipi_dsi_smartdma.h"
#include "fsl_power.h"
#include "pin_mux.h"
#include "mej_disable.h"
#include "power_manager.h"

#include "trace_platform.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void BOARD_InitSmartDMA(void)
{
    RESET_ClearPeripheralReset(kINPUTMUX_RST_SHIFT_RSTn);

    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, 0, kINPUTMUX_MipiIrqToSmartDmaInput);

    /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */
    INPUTMUX_Deinit(INPUTMUX);

    POWER_DisablePD(kPDRUNCFG_APD_SMARTDMA_SRAM);
    POWER_DisablePD(kPDRUNCFG_PPD_SMARTDMA_SRAM);
    POWER_ApplyPD();

    SMARTDMA_InitWithoutFirmware();
    NVIC_EnableIRQ(SDMA_IRQn);

    NVIC_SetPriority(SDMA_IRQn, 3);
}

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * RM67162 panel
 */

/* Definitions for MIPI. */
#define DEMO_MIPI_DSI MIPI_DSI_HOST
#define DEMO_MIPI_DSI_LANE_NUM 1
#define DEMO_MIPI_DSI_IRQn MIPI_IRQn

/*
 * The max TX array size:
 *
 * 1. One byte in FIFO is reserved for DSC command
 * 2. One pixel should not be split to two transfer.
 */
#define DEMO_DSI_TX_ARRAY_MAX \
    (((FSL_DSI_TX_MAX_PAYLOAD_BYTE - 1U) / BUFFER_BYTE_PER_PIXEL) * BUFFER_BYTE_PER_PIXEL)

typedef struct _dsi_mem_write_ctx
{
    volatile bool onGoing;
    const uint8_t *txData;
    uint32_t leftByteLen;
    uint8_t dscCmd;
} dsi_mem_write_ctx_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void BOARD_PullPanelResetPin(bool pullUp);
static void BOARD_PullPanelPowerPin(bool pullUp);
static void BOARD_InitMipiDsiClock(void);
static status_t BOARD_DSI_Transfer(dsi_transfer_t *xfer);
static status_t BOARD_DSI_MemWrite(uint8_t virtualChannel, const uint8_t *data, uint32_t length);
/*******************************************************************************
 * Variables
 ******************************************************************************/
#if DEMO_RM67162_USE_DSI_SMARTDMA
static dsi_smartdma_handle_t s_dsiDriverHandle;
#else
static dsi_mem_write_ctx_t s_dsiMemWriteCtx;
static dsi_transfer_t s_dsiMemWriteXfer = {0};
static dsi_handle_t s_dsiDriverHandle;
static uint8_t s_dsiMemWriteTmpArray[DEMO_DSI_TX_ARRAY_MAX];
#endif

static uint32_t mipiDsiTxEscClkFreq_Hz;
static uint32_t mipiDsiDphyBitClkFreq_Hz;

static mipi_dsi_device_t dsiDevice = {
    .virtualChannel = 0,
    .xferFunc       = BOARD_DSI_Transfer,
    .memWriteFunc   = BOARD_DSI_MemWrite,
};

static const rm67162_resource_t rm67162Resource = {
    .dsiDevice    = &dsiDevice,
    .pullResetPin = BOARD_PullPanelResetPin,
    .pullPowerPin = BOARD_PullPanelPowerPin,
};

static display_handle_t rm67162Handle = {
    .resource = &rm67162Resource,
    .ops      = &rm67162_ops,
};

const dc_fb_dsi_cmd_config_t s_panelConfig = {
    .commonConfig =
        {
            .resolution   = FSL_VIDEO_RESOLUTION(DEMO_PANEL_WIDTH, DEMO_PANEL_HEIGHT),
            .hsw          = 0, /* Not used. */
            .hfp          = 0, /* Not used. */
            .hbp          = 0, /* Not used. */
            .vsw          = 0, /* Not used. */
            .vfp          = 0, /* Not used. */
            .vbp          = 0, /* Not used. */
            .controlFlags = 0,
            .dsiLanes     = DEMO_MIPI_DSI_LANE_NUM,
            .pixelFormat  = BUFFER_PIXEL_FORMAT,
        },
#ifdef MEJ_DISABLED_DISPLAY_TEARING
    .useTEPin = false, // do not use TE signal (framerate is slower)
#else
    .useTEPin = true,
#endif
};

static dc_fb_dsi_cmd_handle_t s_dcFbDsiCmdHandle = {
    .dsiDevice   = &dsiDevice,
    .panelHandle = &rm67162Handle,
};

const dc_fb_t g_dc = {
    .ops     = &g_dcFbOpsDsiCmd,
    .prvData = &s_dcFbDsiCmdHandle,
    .config  = &s_panelConfig,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

static void BOARD_PullPanelResetPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(GPIO, BOARD_MIPI_RST_PORT, BOARD_MIPI_RST_PIN, 1);
    }
    else
    {
        GPIO_PinWrite(GPIO, BOARD_MIPI_RST_PORT, BOARD_MIPI_RST_PIN, 0);
    }
}

static void BOARD_PullPanelPowerPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(GPIO, BOARD_MIPI_POWER_PORT, BOARD_MIPI_POWER_PIN, 1);
    }
    else
    {
        GPIO_PinWrite(GPIO, BOARD_MIPI_POWER_PORT, BOARD_MIPI_POWER_PIN, 0);
    }
}

static status_t BOARD_DSI_Transfer(dsi_transfer_t *xfer)
{
    status_t ret;
    ret = DSI_TransferBlocking(DEMO_MIPI_DSI, xfer);
    return ret;
}

#if !DEMO_RM67162_USE_DSI_SMARTDMA
static status_t BOARD_DsiMemWriteSendChunck(void)
{
    uint32_t curSendLen;
    uint32_t i;

    curSendLen =
        DEMO_DSI_TX_ARRAY_MAX > s_dsiMemWriteCtx.leftByteLen ? s_dsiMemWriteCtx.leftByteLen : DEMO_DSI_TX_ARRAY_MAX;

    s_dsiMemWriteXfer.txDataType = kDSI_TxDataDcsLongWr;
    s_dsiMemWriteXfer.dscCmd     = s_dsiMemWriteCtx.dscCmd;
    s_dsiMemWriteXfer.txData     = s_dsiMemWriteTmpArray;
    s_dsiMemWriteXfer.txDataSize = curSendLen;

    /* For each pixel, the MIPI DSI sends out low byte first, but according to
     * the MIPI DSC spec, the high byte should be send first, so swap the pixel byte
     * first.
     */
    for (i = 0; i < curSendLen; i += 2)
    {
        s_dsiMemWriteTmpArray[i]     = *(s_dsiMemWriteCtx.txData + 1);
        s_dsiMemWriteTmpArray[i + 1] = *(s_dsiMemWriteCtx.txData);

        s_dsiMemWriteCtx.txData += 2;
    }

    s_dsiMemWriteCtx.leftByteLen -= curSendLen;
    s_dsiMemWriteCtx.dscCmd = kMIPI_DCS_WriteMemoryContinue;

    return DSI_TransferNonBlocking(DEMO_MIPI_DSI, &s_dsiDriverHandle, &s_dsiMemWriteXfer);
}

static void BOARD_DsiMemWriteCallback(MIPI_DSI_HOST_Type *base, dsi_handle_t *handle, status_t status, void *userData)
{
    if ((kStatus_Success == status) && (s_dsiMemWriteCtx.leftByteLen > 0))
    {
        status = BOARD_DsiMemWriteSendChunck();

        if (kStatus_Success == status)
        {
            return;
        }
    }

    s_dsiMemWriteCtx.onGoing = false;
    MIPI_DSI_MemoryDoneDriverCallback(status, &dsiDevice);
}
#else
static void BOARD_DsiMemWriteCallback(MIPI_DSI_HOST_Type *base,
                                      dsi_smartdma_handle_t *handle,
                                      status_t status,
                                      void *userData)
{

    MIPI_DSI_MemoryDoneDriverCallback(status, &dsiDevice);
    power_manager_enable_low_power(POWER_SDMA, LOW_POWER_AUTHORIZED);

    /* Request power_manager to power down this buffer */
    power_manager_framebuffer_request_power_off();
 
    TRACE_HW_TASK_STOP(HW_TASK_SDMA_ID);

}
#endif

static status_t BOARD_DSI_MemWrite(uint8_t virtualChannel, const uint8_t *data, uint32_t length)
{
#if DEMO_RM67162_USE_DSI_SMARTDMA
    dsi_smartdma_write_mem_transfer_t xfer = {
        .inputFormat  = kDSI_SMARTDMA_InputPixelFormatRGB565,
        .outputFormat = kDSI_SMARTDMA_OutputPixelFormatRGB565,
        .data         = data,
        .dataSize     = length,
        .virtualChannel = virtualChannel
    };

    power_manager_enable_low_power(POWER_SDMA, LOW_POWER_FORBIDDEN);
    TRACE_HW_TASK_START(HW_TASK_SDMA_ID);

    return DSI_TransferWriteMemorySMARTDMA(DEMO_MIPI_DSI, &s_dsiDriverHandle, &xfer);
#else
    status_t status;

    if (s_dsiMemWriteCtx.onGoing)
    {
        return kStatus_Fail;
    }

    s_dsiMemWriteXfer.virtualChannel = virtualChannel;
    s_dsiMemWriteXfer.flags          = kDSI_TransferUseHighSpeed;
    s_dsiMemWriteXfer.sendDscCmd     = true;

    s_dsiMemWriteCtx.onGoing     = true;
    s_dsiMemWriteCtx.txData      = data;
    s_dsiMemWriteCtx.leftByteLen = length;
    s_dsiMemWriteCtx.dscCmd      = kMIPI_DCS_WriteMemoryStart;

    status = BOARD_DsiMemWriteSendChunck();

    if (status != kStatus_Success)
    {
        /* Memory write does not start actually. */
        s_dsiMemWriteCtx.onGoing = false;
    }

    return status;
#endif
}

static void BOARD_InitMipiDsiClock(void)
{
    POWER_DisablePD(kPDRUNCFG_APD_MIPIDSI_SRAM);
    POWER_DisablePD(kPDRUNCFG_PPD_MIPIDSI_SRAM);
    POWER_DisablePD(kPDRUNCFG_PD_MIPIDSI);
    POWER_ApplyPD();

    /* RxClkEsc max 60MHz, TxClkEsc 12 to 20MHz. */
    CLOCK_AttachClk(kFRO_DIV16_to_MIPI_DPHYESC_CLK);
    /* RxClkEsc = 12MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivDphyEscRxClk, 1);
    /* TxClkEsc = 12MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivDphyEscTxClk, 1);
    mipiDsiTxEscClkFreq_Hz = CLOCK_GetMipiDphyEscTxClkFreq();

    /*
     * DPHY supports up to 895.1MHz bit clock.
     * When choose the DPHY clock frequency, consider the panel frame rate and
     * resolution.
     *
     * RM67162 controller maximum total bit rate is:
     *  - 500Mbps of 2 data lanes 24-bit data format
     *  - 360Mbps of 2 data lanes 18-bit data format
     *  - 320Mbps of 2 data lanes 16-bit data format
     *
     * Here the clock is not set to maximum value, because to remove tearing effect,
     * the frame data send time should be
     * 1. Shorter than one panel refresh time, OR
     * 2. Longer than one panel refresh time and shorter than two panel refresh time.
     *
     * The data send time might be impacted by system bus load. In other words, if
     * data send time is shorter than one panel refresh time with low system bus load,
     * it might be longer then one panel refresh time with high system bus load.
     * For safe, the MIPI bit clock is set to low, so that the data send time
     * is always longer than one panel refresh and shorter than two panel refresh time.
     *
     * Here use AUX1 PLL clock.
     * AUX1 PLL clock is system pll clock * 18 / pfd.
     * system pll clock is 384MHz defined in clock_config.c
     */
    CLOCK_AttachClk(kAUX1_PLL_to_MIPI_DPHY_CLK);
    /* PFD value is in the range of 12~35. */
    /* initial demo had systemPLL= 528MHz MIPI clock = 226.3MHz
     * reduced system PLL to save power so changed dividers here to get approximatively
     * same MIPI clock and then same framerate :
     * systemPLL = 384MHz - MIPI clock = 230.4MHz. */ 
    CLOCK_InitSysPfd(kCLOCK_Pfd3, 30);
    CLOCK_SetClkDiv(kCLOCK_DivDphyClk, 1);

    mipiDsiDphyBitClkFreq_Hz = CLOCK_GetMipiDphyClkFreq();

}

#ifndef MEJ_DISABLED_DISPLAY_TEARING
static void BOARD_InitMipiPanelTEPin(void)
{
    const gpio_pin_config_t tePinConfig = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic  = 0,
    };

    /*
     * TE pin configure method:
     *
     * The TE pin interrupt is like this:
     *
     *            VSYNC
     *         +--------+
     *         |        |
     *         |        |
     * --------+        +----------------
     *
     * 1. If one frame send time is shorter than one frame refresh time, then set
     *    TE pin interrupt at the start of VSYNC.
     * 2. If one frame send time is longer than one frame refresh time, and shorter
     *    than two frames refresh time, then set TE pin interrupt at the end of VSYNC.
     * 3. If one frame send time is longer than two frame refresh time, tearing effect
     *    could not be removed.
     *
     * For RM67162 @60Hz frame rate, frame refresh time is 16.7 ms. After test,
     * one frame send time is longer than one frame refresh time. So TE interrupt is
     * set to end of VSYNC.
     */

    gpio_interrupt_config_t tePinIntConfig = {kGPIO_PinIntEnableEdge, kGPIO_PinIntEnableLowOrFall};

    GPIO_PinInit(GPIO, BOARD_MIPI_TE_PORT, BOARD_MIPI_TE_PIN, &tePinConfig);

    GPIO_SetPinInterruptConfig(GPIO, BOARD_MIPI_TE_PORT, BOARD_MIPI_TE_PIN, &tePinIntConfig);

    GPIO_PinEnableInterrupt(GPIO, BOARD_MIPI_TE_PORT, BOARD_MIPI_TE_PIN, kGPIO_InterruptA);

    NVIC_SetPriority(GPIO_INTA_IRQn, 3);

    NVIC_EnableIRQ(GPIO_INTA_IRQn);
}
#endif

static status_t BOARD_InitLcdPanel(void)
{
    const gpio_pin_config_t pinConfig = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 0,
    };

    GPIO_PinInit(GPIO, BOARD_MIPI_POWER_PORT, BOARD_MIPI_POWER_PIN, &pinConfig);
    GPIO_PinInit(GPIO, BOARD_MIPI_RST_PORT, BOARD_MIPI_RST_PIN, &pinConfig);

#ifndef MEJ_DISABLED_DISPLAY_TEARING
   BOARD_InitMipiPanelTEPin();
#endif

    return kStatus_Success;
}

/* Smart panel TE pin IRQ handler. */
void BOARD_DisplayTEPinHandler(void)
{
#ifndef MEJ_DISABLED_DISPLAY_TEARING
	uint32_t intStat = GPIO_PortGetInterruptStatus(GPIO, BOARD_MIPI_TE_PORT, kGPIO_InterruptA);

	if (intStat & (1UL << BOARD_MIPI_TE_PIN))
	{
		GPIO_PortClearInterruptFlags(GPIO, BOARD_MIPI_TE_PORT, kGPIO_InterruptA, (1UL << BOARD_MIPI_TE_PIN));
		DC_FB_DSI_CMD_TE_IRQHandler(&g_dc);
	}
#endif

}

static void BOARD_SetMipiDsiConfig(void)
{
    dsi_config_t dsiConfig;
    dsi_dphy_config_t dphyConfig;

    /*
     * dsiConfig.numLanes = 4;
     * dsiConfig.enableNonContinuousHsClk = false;
     * dsiConfig.autoInsertEoTp = true;
     * dsiConfig.numExtraEoTp = 0;
     * dsiConfig.htxTo_ByteClk = 0;
     * dsiConfig.lrxHostTo_ByteClk = 0;
     * dsiConfig.btaTo_ByteClk = 0;
     */
    DSI_GetDefaultConfig(&dsiConfig);
    dsiConfig.numLanes       = DEMO_MIPI_DSI_LANE_NUM;
    dsiConfig.autoInsertEoTp = true;

    DSI_GetDphyDefaultConfig(&dphyConfig, mipiDsiDphyBitClkFreq_Hz, mipiDsiTxEscClkFreq_Hz);

    /* Init the DSI module. */
    DSI_Init(DEMO_MIPI_DSI, &dsiConfig);

    /* Init DPHY. There is not DPHY PLL, the ref clock is not used. */
    DSI_InitDphy(DEMO_MIPI_DSI, &dphyConfig, 0);
}

status_t BOARD_InitDisplayInterface(void)
{
    /* 1. Assert MIPI DPHY reset. */
    RESET_SetPeripheralReset(kMIPI_DSI_PHY_RST_SHIFT_RSTn);

    /* 2. Setup clock. */
    BOARD_InitMipiDsiClock();

    /* 3. Configures peripheral. */
    RESET_ClearPeripheralReset(kMIPI_DSI_CTRL_RST_SHIFT_RSTn);
    BOARD_SetMipiDsiConfig();

    /* 4. Deassert reset. */
    RESET_ClearPeripheralReset(kMIPI_DSI_PHY_RST_SHIFT_RSTn);

    /* 5. Configure the panel. */
    return BOARD_InitLcdPanel();
}

status_t BOARD_PrepareDisplayController(void)
{
    status_t status;

    status = BOARD_InitDisplayInterface();

#if DEMO_RM67162_USE_DSI_SMARTDMA
    if (kStatus_Success == status)
    {
        BOARD_InitSmartDMA();
        status = DSI_TransferCreateHandleSMARTDMA(DEMO_MIPI_DSI, &s_dsiDriverHandle, BOARD_DsiMemWriteCallback, NULL);
    }

    return status;

#else

    if (kStatus_Success == status)
    {
        /*
         * Suggest setting to low priority. Because a new DSI transfer is prepared
         * in the callback BOARD_DsiMemWriteCallback, so the core spends more time
         * in ISR. Setting the low priority, then the important ISR won't be blocked.
         */
        NVIC_SetPriority(DEMO_MIPI_DSI_IRQn, 6);
    }
    else
    {
        return status;
    }

    memset(&s_dsiMemWriteCtx, 0, sizeof(dsi_mem_write_ctx_t));

    /* Create the MIPI DSI trasnfer handle for non-blocking data trasnfer. */
    return DSI_TransferCreateHandle(DEMO_MIPI_DSI, &s_dsiDriverHandle, BOARD_DsiMemWriteCallback, NULL);
#endif
}
