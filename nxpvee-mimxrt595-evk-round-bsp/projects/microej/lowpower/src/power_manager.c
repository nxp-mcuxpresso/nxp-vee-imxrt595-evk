/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020-2022 MicroEJ Corp. This file has been modified by MicroEJ Corp.
 */

#include <stdint.h>
#include <sni.h>
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_pca9420.h"
#include "fsl_power.h"
#include "fsl_rtc.h"
#include "pin_mux.h"
#include "power_manager.h"
#include "fsl_gpio.h"
#include "fsl_powerquad.h"
#include "vglite_support.h"

#include "SEGGER_SYSVIEW.h"

/*
 * force all unused peripherals and RAM partitions
 * to be turned off when the CPU is active.
 */

// Rams sections defined in linker script
extern uint8_t m_data_start, m_data_end;
extern uint8_t m_framebuffer0_start, m_framebuffer0_end;
extern uint8_t m_framebuffer1_start, m_framebuffer1_end;

#define SRAM_PARTITIONS_NB  (32)
#define SRAM_BASE_ADDRESS   (0x20000000)

/* Define all SRAM partition sizes and associated power management parameters.*/
typedef struct {
	uint8_t size_32k; // size of the partition in multiple of 32kBytes.
	pd_bit_t apd; //array power
	pd_bit_t ppd; //periphery power
} sram_partition_t;

sram_partition_t sram_partitions[SRAM_PARTITIONS_NB] = {
	{1 /* 32kB  */, kPDRUNCFG_APD_SRAM_IF0,  kPDRUNCFG_PPD_SRAM_IF0},
	{1 /* 32kB  */, kPDRUNCFG_APD_SRAM_IF1,  kPDRUNCFG_PPD_SRAM_IF1},
	{1 /* 32kB  */, kPDRUNCFG_APD_SRAM_IF2,  kPDRUNCFG_PPD_SRAM_IF2},
	{1 /* 32kB  */, kPDRUNCFG_APD_SRAM_IF3,  kPDRUNCFG_PPD_SRAM_IF3},
	{1 /* 32kB  */, kPDRUNCFG_APD_SRAM_IF4,  kPDRUNCFG_PPD_SRAM_IF4},
	{1 /* 32kB  */, kPDRUNCFG_APD_SRAM_IF5,  kPDRUNCFG_PPD_SRAM_IF5},
	{1 /* 32kB  */, kPDRUNCFG_APD_SRAM_IF6,  kPDRUNCFG_PPD_SRAM_IF6},
	{1 /* 32kB  */, kPDRUNCFG_APD_SRAM_IF7,  kPDRUNCFG_PPD_SRAM_IF7},
	{2 /* 64kB  */, kPDRUNCFG_APD_SRAM_IF8,  kPDRUNCFG_PPD_SRAM_IF8},
	{2 /* 64kB  */, kPDRUNCFG_APD_SRAM_IF9,  kPDRUNCFG_PPD_SRAM_IF9},
	{2 /* 64kB  */, kPDRUNCFG_APD_SRAM_IF10, kPDRUNCFG_PPD_SRAM_IF10},
	{2 /* 64kB  */, kPDRUNCFG_APD_SRAM_IF11, kPDRUNCFG_PPD_SRAM_IF11},
	{4 /* 128kB */, kPDRUNCFG_APD_SRAM_IF12, kPDRUNCFG_PPD_SRAM_IF12},
	{4 /* 128kB */, kPDRUNCFG_APD_SRAM_IF13, kPDRUNCFG_PPD_SRAM_IF13},
	{4 /* 128kB */, kPDRUNCFG_APD_SRAM_IF14, kPDRUNCFG_PPD_SRAM_IF14},
	{4 /* 128kB */, kPDRUNCFG_APD_SRAM_IF15, kPDRUNCFG_PPD_SRAM_IF15},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF16, kPDRUNCFG_PPD_SRAM_IF16},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF17, kPDRUNCFG_PPD_SRAM_IF17},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF18, kPDRUNCFG_PPD_SRAM_IF18},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF19, kPDRUNCFG_PPD_SRAM_IF19},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF20, kPDRUNCFG_PPD_SRAM_IF20},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF21, kPDRUNCFG_PPD_SRAM_IF21},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF22, kPDRUNCFG_PPD_SRAM_IF22},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF23, kPDRUNCFG_PPD_SRAM_IF23},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF24, kPDRUNCFG_PPD_SRAM_IF24},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF25, kPDRUNCFG_PPD_SRAM_IF25},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF26, kPDRUNCFG_PPD_SRAM_IF26},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF27, kPDRUNCFG_PPD_SRAM_IF27},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF28, kPDRUNCFG_PPD_SRAM_IF28},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF29, kPDRUNCFG_PPD_SRAM_IF29},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF30, kPDRUNCFG_PPD_SRAM_IF30},
	{8 /* 256kB */, kPDRUNCFG_APD_SRAM_IF31, kPDRUNCFG_PPD_SRAM_IF31},
};


/* Mask of the SRAM partitions in use for deep sleep config. */
static uint32_t used_ram_mask = 0;

/* Keep track of the framebuffer that is currently transmitted to Display */
static framebuffer_t *framebuffer_to_power_off_address = NULL;

/*
 * Some GPIO pins can be configured to wake up the device from deepsleep mode.
 * Once the device has been waked up by a GPIO pin event, entering in deepsleep
 * mode will not be allowed for some seconds(configurable for each wakeup source).
 * All these GPIO pins interrupts are associated with GPIO INTB interrupt handler.
 *
 */

/*
 * GPIO_WakeUp_Config_t struct
 */

typedef struct {
	uint8_t GPIO_Port;
	uint8_t GPIO_Pin;
	gpio_interrupt_config_t IT_config;
	int32_t IT_time;
	uint8_t lowpower_disabled_sec;
	power_domain_t power_id;
} GPIO_WakeUp_Config_t;

/*
 * GPIO wake up pins declaration
 */
GPIO_WakeUp_Config_t gpio_wakeup_config[1] = {
		{
		   BOARD_SWDIO_PORT, BOARD_SWDIO_PIN,
		   {kGPIO_PinIntEnableEdge, kGPIO_PinIntEnableLowOrFall},
		   0, 1, POWER_DEBUGGER
		},
};

/*
 * GPIO INTB interrupt handler
 */
void GPIO_INTB_IRQHandler(void)
{
	for(int i=0; i<sizeof(gpio_wakeup_config)/sizeof(GPIO_WakeUp_Config_t); i++){
		if ((1 << gpio_wakeup_config[i].GPIO_Pin) == GPIO_PortGetInterruptStatus(GPIO, gpio_wakeup_config[i].GPIO_Port, kGPIO_InterruptB))
		{
			GPIO_PinClearInterruptFlag(GPIO, gpio_wakeup_config[i].GPIO_Port, gpio_wakeup_config[i].GPIO_Pin, kGPIO_InterruptB);

			// Disable entering deepsleep for lowpower_disabled_sec seconds
			gpio_wakeup_config[i].IT_time = RTC->COUNT + gpio_wakeup_config[i].lowpower_disabled_sec;

#ifdef PM_DEBUG
			PRINTF("Awake from deepsleep for %d seconds\n", gpio_wakeup_config[i].lowpower_disabled_sec);
#endif
			GPIO_WakeUp_Power_Enable_Update();
		}
	}
}


/*
 * Enable GPIO pins wake up from deep sleep mode.
 * To be called in fsl_tickless_rtc.c before
 * entering deepsleep mode.
 *
 */
void GPIO_WakeUp_Enable(void)
{
	for(int i=0; i<sizeof(gpio_wakeup_config)/sizeof(GPIO_WakeUp_Config_t); i++){
		GPIO_PinEnableInterrupt(GPIO,
				                gpio_wakeup_config[i].GPIO_Port,
				                gpio_wakeup_config[i].GPIO_Pin,
								kGPIO_InterruptB);
	}
}

/*
 * Disable GPIO pins wake up from deep sleep mode.
 * To be called in fsl_tickless_rtc.c after leaving
 * deepsleep mode.
 *
 */
void GPIO_WakeUp_Disable(void)
{
	for(int i=0; i<sizeof(gpio_wakeup_config)/sizeof(GPIO_WakeUp_Config_t); i++){
		GPIO_PinDisableInterrupt(GPIO,
				                gpio_wakeup_config[i].GPIO_Port,
				                gpio_wakeup_config[i].GPIO_Pin,
								kGPIO_InterruptB);
	}
}

/*
 * Init GPIO wake up pins.
 *
 */
static void GPIO_WakeUp_Init(void)
{
	for(int i=0; i<sizeof(gpio_wakeup_config)/sizeof(GPIO_WakeUp_Config_t); i++)	{
		GPIO_PortInit_noreset(GPIO, gpio_wakeup_config[i].GPIO_Port);
		GPIO_SetPinInterruptConfig(GPIO,
				                   gpio_wakeup_config[i].GPIO_Port,
				                   gpio_wakeup_config[i].GPIO_Pin,
								   &gpio_wakeup_config[i].IT_config);
	}

	NVIC_EnableIRQ(GPIO_INTB_IRQn);
	EnableDeepSleepIRQ(GPIO_INTB_IRQn);
}

/*
 * Monitor the duration since the last GPIO wake up pin event
 * and release the deepsleep mode avalaibility
 *
 */
static void GPIO_WakeUp_Power_Enable_Update(void)
{
	uint16_t rtc_count = RTC->COUNT;

	for(int i=0; i<sizeof(gpio_wakeup_config)/sizeof(GPIO_WakeUp_Config_t); i++){
		if(rtc_count > (gpio_wakeup_config[i].IT_time))
		{
			power_manager_enable_low_power(gpio_wakeup_config[i].power_id, LOW_POWER_AUTHORIZED);
		}
		else
		{
			power_manager_enable_low_power(gpio_wakeup_config[i].power_id, LOW_POWER_FORBIDDEN);
		}
	}
}

static void set_ram_partitions_power(uint8_t *start, uint8_t *end, ram_power_level_t level, others_power_level_t others_level)
{
	uint8_t *partition_base, *next_partition_base;
	uint32_t i;

	//PRINTF("S 0x%X, E 0x%x\n", start, end);
	/* Loop on all SRAM partitions and determine if they can be turned off. */
	partition_base = (uint8_t *)SRAM_BASE_ADDRESS;

	for (i = 0; i < SRAM_PARTITIONS_NB; i++) {
		next_partition_base = partition_base + (sram_partitions[i].size_32k * 32 * 1024);

		if ((next_partition_base <= start) // partition before start of used area
			|| (partition_base > end))     // partition after end of used area
		{
			/* Partition unused: enable power down. */
			if(OTHERS_POWER_OFF == others_level)	{ // only if others_level is true
				POWER_EnablePD(sram_partitions[i].apd);
				POWER_EnablePD(sram_partitions[i].ppd);
				used_ram_mask &= ~(1 << i);
			}
		} else { // partition in the used area
			if(POWER_ON == level)	{ // select if it must be switch on or off
				/* Partition used: disable power down. */
				POWER_DisablePD(sram_partitions[i].apd);
				POWER_DisablePD(sram_partitions[i].ppd);
				/* Some data are mapped in this area: keep info for deep sleep. */
				used_ram_mask |= (1 << i);
			}
			else {
				POWER_EnablePD(sram_partitions[i].apd);
				POWER_EnablePD(sram_partitions[i].ppd);
				used_ram_mask &= ~(1 << i);
			}
		}
		if((OTHERS_DONT_CARE == others_level) &&  (partition_base > end))
		{
			break;
		}

		partition_base = next_partition_base;
	}

	//PRINTF("SRAM partitions enabled mask: 0x%08x\n", used_ram_mask);
}


static void set_active_power_config(void)
{
	/* Disable unused peripherals. */
	POWER_EnablePD(kPDRUNCFG_PD_AUDPLL_LDO);
	POWER_EnablePD(kPDRUNCFG_PD_AUDPLL_ANA);
	POWER_EnablePD(kPDRUNCFG_PD_ADC);
	POWER_EnablePD(kPDRUNCFG_LP_ADC);
	POWER_EnablePD(kPDRUNCFG_PD_ADC_TEMPSNS);
	POWER_EnablePD(kPDRUNCFG_PD_PMC_TEMPSNS);
	POWER_EnablePD(kPDRUNCFG_PD_ACMP);

	/* Configure RAM partitions power. */
	used_ram_mask = 0;

	/* First power on all used RAMs to computed initial used_ram_mask*/
	set_ram_partitions_power(&m_data_start, &m_data_end, POWER_ON, OTHERS_POWER_OFF); // Power on only data_ram, power_off others
	set_ram_partitions_power(&m_framebuffer0_start, &m_framebuffer0_end, POWER_ON, OTHERS_DONT_CARE); // Power on framebuffer0
	set_ram_partitions_power(&m_framebuffer1_start, &m_framebuffer1_end, POWER_ON, OTHERS_DONT_CARE); // Power on framebuffer1

	/* Limit access only to the RAM partitions in use. */
	SYSCTL0->AHB_SRAM_ACCESS_DISABLE = ~used_ram_mask;
	SYSCTL0->AXI_SRAM_ACCESS_DISABLE = ~used_ram_mask;

	/* Power Off ram not needed at startup */
	set_ram_partitions_power(&m_framebuffer1_start, &m_framebuffer1_end, POWER_OFF, OTHERS_DONT_CARE); // Power off framebuffer1

	/* DSP unused - disable its access to SRAM. */
	SYSCTL0->DSP_SRAM_ACCESS_DISABLE = 0xFFFFFFFF;

	/* PowerDown clock, RAM, and access to unused peripherals */
	CLOCK_AttachClk(kNONE_to_SDIO0_CLK);
	POWER_EnablePD(kPDRUNCFG_LP_HSPAD_SDIO0_VDET);
	POWER_EnablePD(kPDRUNCFG_PD_HSPAD_SDIO0_REF);
	POWER_EnablePD(kPDRUNCFG_APD_USDHC0_SRAM);
	POWER_EnablePD(kPDRUNCFG_PPD_USDHC0_SRAM);
	/* SDIO1 */
	CLOCK_AttachClk(kNONE_to_SDIO1_CLK);
	POWER_EnablePD(kPDRUNCFG_LP_HSPAD_SDIO1_VDET);
	POWER_EnablePD(kPDRUNCFG_PD_HSPAD_SDIO1_REF);
	POWER_EnablePD(kPDRUNCFG_APD_USDHC1_SRAM);
	POWER_EnablePD(kPDRUNCFG_PPD_USDHC1_SRAM);
	/* CASPER */
	CLOCK_DisableClock(kCLOCK_Casper);
	POWER_EnablePD(kPDRUNCFG_PPD_CASPER_SRAM);
	/* ROM */
	CLOCK_DisableClock(kCLOCK_RomCtrlr);
    POWER_EnablePD(kPDRUNCFG_PD_ROM);
	/* OTP */
	CLOCK_DisableClock(kCLOCK_OtpCtrl);
	POWER_EnablePD(kPDRUNCFG_PD_OTP);

	/* PMC */
	POWER_EnablePD(kPDRUNCFG_LP_VDD_COREREG);
	POWER_EnablePD(kPDRUNCFG_LP_PMCREF);
	POWER_EnablePD(kPDRUNCFG_PD_HVD1V8);
	//POWER_EnablePD(kPDRUNCFG_LP_PORCORE);
	POWER_EnablePD(kPDRUNCFG_LP_LVDCORE);
	POWER_EnablePD(kPDRUNCFG_PD_HVDCORE);

	/* PMC enable internal clock divider to save power. */
	PMC->CTRL |= PMC_CTRL_CLKDIVEN_MASK; // name is confusing but defined like this in the .h file

  	/* Apply power setting. */
	POWER_ApplyPD();
}

static pca9420_handle_t pca9420Handle;
/*
 * Configure the PMIC for power savings.
 * Switch between modes is triggered automatically by HW at run time
 * thanks to 2 signals from RT500 to PMIC.
 *  0b00    run mode, vddcore 0.925V (0.9V is unstable)
 *  0b01    deep sleep mode, vddcore 0.6V.
 *  0b10    deep powerdown mode, vddcore off.
 *  0b11    full deep powerdown mode vdd1v8 and vddcore off.
 */
static void pmic_config()
{
	pca9420_config_t pca9420Config;
	pca9420_modecfg_t pca9420ModeCfg[4];
	power_pad_vrange_t vrange;
	gpio_pin_config_t gpio_conf;

	POWER_SetLvdFallingTripVoltage(kLvdFallingTripVol_720);

	/* Configure I2C to communicate with PMIC.*/
	BOARD_InitI2CPmicPins();
	CLOCK_AttachClk(kFRO_DIV4_to_FLEXCOMM15);
	BOARD_PMIC_I2C_Init();

	/* Initialize the PMIC. */
	PCA9420_GetDefaultConfig(&pca9420Config);
	pca9420Config.I2C_SendFunc    = BOARD_PMIC_I2C_Send;
	pca9420Config.I2C_ReceiveFunc = BOARD_PMIC_I2C_Receive;
	PCA9420_Init(&pca9420Handle, &pca9420Config);

	/* Inform power lib that an external PMIC is used. */
	POWER_UpdatePmicRecoveryTime(1);

	/* Override default PMIC configurations */
	/* Mode 0: max perfs. */
	PCA9420_GetDefaultModeConfig(&pca9420ModeCfg[0]);
	pca9420ModeCfg[0].sw1OutVolt = kPCA9420_Sw1OutVolt0V900;
	
	/* Mode 1: VDDCORE 0.6V. */
	PCA9420_GetDefaultModeConfig(&pca9420ModeCfg[1]);
	pca9420ModeCfg[1].sw1OutVolt = kPCA9420_Sw1OutVolt0V600;

	/* Mode 2: VDDCORE low power. */
	PCA9420_GetDefaultModeConfig(&pca9420ModeCfg[2]);
	pca9420ModeCfg[2].sw1OutVolt = kPCA9420_Sw1OutVolt0V800;

	/* Mode 3: VDDCORE, VDD1V8 and VDDIO off. */
	PCA9420_GetDefaultModeConfig(&pca9420ModeCfg[3]);
	pca9420ModeCfg[3].enableSw1Out  = false;
	pca9420ModeCfg[3].enableSw2Out  = false;
	pca9420ModeCfg[3].enableLdo2Out = false;

	/* Send new configuration to PMIC. */
	PCA9420_WriteModeConfigs(&pca9420Handle, kPCA9420_Mode0, &pca9420ModeCfg[0], ARRAY_SIZE(pca9420ModeCfg));

	if (SYSCTL0->SILICONREV_ID < 0xB0002) {
		/* On boards having a chip prior to B2:
		 * gpio 4,0 controls VDDIO_2 voltage selection though RT_PIO4_0 signal: low=3.3V high=1.8V*/
		gpio_conf.pinDirection = kGPIO_DigitalOutput;
		gpio_conf.outputLogic = 1; // VDDIO_2 = 1.8V
		GPIO_PinInit(GPIO, 4, 0, &gpio_conf);
	}

	/* Configure pads voltage ranges according to PMIC settings. */
	vrange.Vdde0Range = kPadVol_171_198;
	vrange.Vdde1Range = kPadVol_171_198;
	vrange.Vdde2Range = kPadVol_171_198;
	vrange.Vdde3Range = kPadVol_300_360;
	vrange.Vdde4Range = kPadVol_171_198;

	POWER_SetPadVolRange(&vrange);

	RESET_SetPeripheralReset(kFC15_RST_SHIFT_RSTn);
	CLOCK_AttachClk(kNONE_to_FLEXCOMM15);
}


/* 
 * Table keeping track of power needs from different peripherals.
 */
static low_power_enabled_t low_power_enabled[POWER_DOMAIN_NUMBER];

/*
 * Initialize power manager.
 */
void power_manager_init(void)
{
	/* Setup power configuration during active periods. */
	set_active_power_config();

	/* Configure PMIC and associated pads. */
	pmic_config();

	/* Configure SWDIO as interrupt to wake up from deepsleep by debugger */
	GPIO_WakeUp_Init();

	/* Configure deep sleep wake-up sources other than RTC. */
	EnableDeepSleepIRQ(GPIO_INTA_IRQn);

	/* Init low_power_enabled table
	 * By default power down is globally forbidden. */
	for(int i=0; i<POWER_DOMAIN_NUMBER; i++)
	{
		low_power_enabled[i] = LOW_POWER_AUTHORIZED;
	}
	low_power_enabled[POWER_GLOBAL] = LOW_POWER_FORBIDDEN;
}

/*
 * enable or disable low power for each individual power domain.
 */

extern const clock_sys_pll_config_t g_sysPllConfig_BOARD_BootClockRUN;

void power_manager_enable_low_power(power_domain_t power_d, low_power_enabled_t enabled)
{
	//PRINTF("power_manager_enable_low_power 0x%x 0x%x\n", power_d, enabled);

	/* Register power requests from peripherals. */
	low_power_enabled[power_d] = enabled;

	/* Dynamic clock control. */
	if (power_d == POWER_GPU) {
		if (enabled == LOW_POWER_AUTHORIZED) {
			BOARD_StopVGliteClock();
		} else {
			BOARD_StartVGliteClock();
		}
	}

	if (power_d == POWER_SDMA) {
		if (enabled == LOW_POWER_AUTHORIZED) {
			CLOCK_DisableClock(kCLOCK_Smartdma);
		} else {
			CLOCK_EnableClock(kCLOCK_Smartdma);
		}
	}
}

/* Return lowest power mode possible and associated config if applicable. */
power_mode_t power_manager_get_mode(uint32_t config[4])
{

	// Update low_power_enabled for GPIO Wakeup Pins indexes
	GPIO_WakeUp_Power_Enable_Update();

	/* Subsec counter used to compensate timer during sleep starts 1s after RTC enabling. */
	if (RTC->COUNT < 1)
		return PM_ACTIVE;

	if (low_power_enabled[POWER_GLOBAL] != LOW_POWER_AUTHORIZED)
		return PM_ACTIVE;

    for(int i=0; i<POWER_DOMAIN_NUMBER; i++)
	{
        if(low_power_enabled[i] != LOW_POWER_AUTHORIZED)
		{
			return PM_SLEEP;
		}
	}

    // Config used only for deep sleep for now. Could be extended to sleep and active to have different runcfg settings
	config[0] = 0x00000000;
	config[0] |= SYSCTL0_PDSLEEPCFG0_RBB_PD_MASK;
	config[0] |= SYSCTL0_PDSLEEPCFG0_RBBSRAM_PD_MASK;
	config[0] |= SYSCTL0_PDSLEEPCFG0_SYSXTAL_PD_MASK;
	//config[0] = 0xFFFFFFFF;

	config[1] = 0x00000000;
	// Keep SDMA RAM array powered
	config[1] |= SYSCTL0_PDSLEEPCFG1_SMARTDMA_SRAM_APD_MASK;
	// Keep FLEXSPIs cache RAM array powered
	config[1] |= SYSCTL0_PDSLEEPCFG1_FLEXSPI0_SRAM_APD_MASK | SYSCTL0_PDSLEEPCFG1_FLEXSPI1_SRAM_APD_MASK ;

	config[2] = used_ram_mask; /* Keep array power for all RAM partitions in use. */

	config[3] = 0x00000000; /* Turn off all periphery power. */
	return PM_DEEP_SLEEP;
}

static int current_power_profile = POWER_PROFILE_MAX_PERFS; /* by default boot with max perfs.*/

/* Native function called directly by Java apps to request a given power profile. */
int32_t power_manager_set_profile(uint32_t power_profile)
{
	if (power_profile == current_power_profile)
		return 0;

	BOARD_FlexspiClockSafeConfig(); // move flexspi to a clock that won't be impacted by change below.

	if (power_profile == POWER_PROFILE_MAX_PERFS) {
		PRINTF("[PowerManagement] Switching to Max Performance Profile\n");
		/* MIPI clock at 230.4 MHz to keep similar rate to initial demo (was 226.3). */
		CLOCK_InitSysPfd(kCLOCK_Pfd3, 30);
		// raise voltage first then raise the clock
		SYSCTL0->PDRUNCFG0_CLR = SYSCTL0_PDRUNCFG0_PMIC_MODE0_MASK | SYSCTL0_PDRUNCFG0_PMIC_MODE1_MASK; // PMIC mode 0
		CLOCK_AttachClk(kFRO_DIV1_to_MAIN_CLK);

		/* PowerQuad enabled. */
		POWER_DisablePD(kPDRUNCFG_PPD_PQ_SRAM);
		POWER_ApplyPD();
		PQ_Init(POWERQUAD);

		/* Divide FlexspiClock to have root clock @96Mhz */
		BOARD_SetFlexspiClock(FLEXSPI1, 0U, 2U);

	} else if (power_profile == POWER_PROFILE_POWER_SAVING) {
		PRINTF("[PowerManagement] Switching to Power Saving Performance Profile\n");
		/* Disable powerquad in power saving profile. */
		PQ_Deinit(POWERQUAD);
		POWER_EnablePD(kPDRUNCFG_PPD_PQ_SRAM);
		POWER_ApplyPD();

		// lower clock first then lower voltage
		CLOCK_AttachClk(kFRO_DIV2_to_MAIN_CLK);

		SYSCTL0->PDRUNCFG0_CLR = SYSCTL0_PDRUNCFG0_PMIC_MODE0_MASK;
		SYSCTL0->PDRUNCFG0_SET = SYSCTL0_PDRUNCFG0_PMIC_MODE1_MASK; // PMIC mode 2
		/* MIPI clock at 345.6 MHz for shorter active time. */
		CLOCK_InitSysPfd(kCLOCK_Pfd3, 20);

        /* Divide FlexspiClock to have root clock @96Mhz */
		BOARD_SetFlexspiClock(FLEXSPI1, 0U, 1U);

	}

    BOARD_SetFlexspiClock(FLEXSPI0, 0U, 1U); // move back fkexspi to main clock

	SystemCoreClock = CLOCK_GetFreq(kCLOCK_CoreSysClk);
	current_power_profile = power_profile;

#if (ENABLE_SVIEW == 1)
		SEGGER_SYSVIEW_UpdateTimeStampMultiplier(SystemCoreClock);
#endif

	return 0;
}


/* Request power ON of a framebuffer */
void power_manager_framebuffer_power_on(framebuffer_t *frameBuffer) {

	uint8_t *frameBuffer_end = (uint8_t*) ((uint8_t*) frameBuffer
			+ sizeof(framebuffer_t));

	set_ram_partitions_power((uint8_t*) frameBuffer, frameBuffer_end, POWER_ON,
			OTHERS_DONT_CARE); // Power on framebuffer

	POWER_ApplyPD();
}

/* Register the next framebuffer to power OFF */
void power_manager_framebuffer_set_next_off(framebuffer_t *frameBuffer) {
	framebuffer_to_power_off_address = frameBuffer;
}

/* Power OFF previously registered framebuffer */
void power_manager_framebuffer_request_power_off(void) {

	// Returns if no framebuffer registered to be powered off
	if(NULL == framebuffer_to_power_off_address) return;

	uint8_t *frameBuffer_end =
			(uint8_t*) ((uint8_t*) framebuffer_to_power_off_address
					+ sizeof(framebuffer_t));

	if ((low_power_enabled[POWER_SDMA] != LOW_POWER_FORBIDDEN)
		 && (low_power_enabled[POWER_DMA] != LOW_POWER_FORBIDDEN)) {
		// Switch off buffer only once both SDMA and DMA transfer are over
		set_ram_partitions_power((uint8_t*) framebuffer_to_power_off_address,
				frameBuffer_end, POWER_OFF, OTHERS_DONT_CARE); // Power off framebuffer

		POWER_ApplyPD();

		framebuffer_to_power_off_address = NULL;
	}
}
