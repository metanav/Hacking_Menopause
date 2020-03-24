#include "adc.h"

// Edge Board Pin Definitions
#define SF_EDGE_PIN_MIC0    11
const am_hal_gpio_pincfg_t g_SF_EDGE_PIN_MIC0 =
{
  .uFuncSel       = AM_HAL_PIN_11_ADCSE2,
};
#define SF_EDGE_PIN_MIC1    29
const am_hal_gpio_pincfg_t g_SF_EDGE_PIN_MIC1 =
{
  .uFuncSel       = AM_HAL_PIN_29_ADCSE1,
};


#define NUM_SLOTS (2)
#define ADC_SAMPLES_PER_SLOT (1024)

#define ADC_SAMPLE_BUF_SIZE (NUM_SLOTS * ADC_SAMPLES_PER_SLOT)      // Note: there seem to be memry limitations after 2048 channel-samples. Only a portion of RAM gets turned on though, it seems.


uint32_t g_ui32ADCSampleBuffer[ADC_SAMPLE_BUF_SIZE];                // This is the raw buffer that is filled by the ADC during DMA

am_hal_adc_sample_t SampleBuffer0[ADC_SAMPLES_PER_SLOT];            // These buffers are basically the same exact thing expect the type
am_hal_adc_sample_t SampleBuffer1[ADC_SAMPLES_PER_SLOT];            // is a structure with names for the actual value (.ui32Sample) and the slot number (.ui32Slot)
// And actually I just noticed that that type has TWO 32-bit fields in each element

//
// ADC Device Handle.
//
static void *g_ADCHandle;
// static void *g_ADCHandle1;

//
// ADC DMA complete flag.
//
volatile bool                   g_bADCDMAComplete;

//
// ADC DMA error flag.
//
volatile bool                   g_bADCDMAError;


//*****************************************************************************
//
// Interrupt handler for the ADC.
//
//*****************************************************************************
void
am_adc_isr(void)
{
  uint32_t ui32IntMask;

  //
  // Read the interrupt status.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_status(g_ADCHandle, &ui32IntMask, false))
  {
    am_util_stdio_printf("Error reading ADC0 interrupt status\n");
  }

  //
  // Clear the ADC interrupt.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_clear(g_ADCHandle, ui32IntMask))
  {
    am_util_stdio_printf("Error clearing ADC0 interrupt status\n");
  }

  //
  // If we got a DMA complete, set the flag.
  //
  if (ui32IntMask & AM_HAL_ADC_INT_DCMP)
  {
    g_bADCDMAComplete = true;
  }

  //
  // If we got a DMA error, set the flag.
  //
  if (ui32IntMask & AM_HAL_ADC_INT_DERR)
  {
    g_bADCDMAError = true;
  }
}

//*****************************************************************************
//
// Set up the core for sleeping, and then go to sleep.
//
//*****************************************************************************
void
sleep(void)
{
  //
  // Disable things that can't run in sleep mode.
  //
#if (0 == ADC_EXAMPLE_DEBUG)
  am_bsp_debug_printf_disable();
#endif

  //
  // Go to Deep Sleep.
  //
  am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

  //
  // Re-enable peripherals for run mode.
  //
#if (0 == ADC_EXAMPLE_DEBUG)
  am_bsp_debug_printf_enable();
#endif
}

//*****************************************************************************
//
// Configure the ADC.
//
//*****************************************************************************
void
adc_config_dma(void)
{
  am_hal_adc_dma_config_t       ADCDMAConfig;

  //
  // Configure the ADC to use DMA for the sample transfer.
  //
  ADCDMAConfig.bDynamicPriority = true;
  ADCDMAConfig.ePriority = AM_HAL_ADC_PRIOR_SERVICE_IMMED;
  ADCDMAConfig.bDMAEnable = true;
  ADCDMAConfig.ui32SampleCount = ADC_SAMPLE_BUF_SIZE;
  ADCDMAConfig.ui32TargetAddress = (uint32_t)g_ui32ADCSampleBuffer;
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_dma(g_ADCHandle, &ADCDMAConfig))
  {
    am_util_stdio_printf("Error - configuring ADC DMA failed.\n");
  }

  //
  // Reset the ADC DMA flags.
  //
  g_bADCDMAComplete = false;
  g_bADCDMAError = false;
}

//*****************************************************************************
//
// Configure the ADC.
//
//*****************************************************************************
void
adc_config0(void)
{
  am_hal_adc_config_t           ADCConfig;
  am_hal_adc_slot_config_t      ADCSlotConfig;

  //
  // Initialize the ADC and get the handle.
  //
  if ( AM_HAL_STATUS_SUCCESS != am_hal_adc_initialize(0, &g_ADCHandle) )
  {
    am_util_stdio_printf("Error - reservation of the ADC0 instance failed.\n");
  }

  //
  // Power on the ADC.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_power_control(g_ADCHandle,
      AM_HAL_SYSCTRL_WAKE,
      false) )
  {
    am_util_stdio_printf("Error - ADC0 power on failed.\n");
  }

  //
  // Set up the ADC configuration parameters. These settings are reasonable
  // for accurate measurements at a low sample rate.
  //
  ADCConfig.eClock             = AM_HAL_ADC_CLKSEL_HFRC_DIV2; // AM_HAL_ADC_CLKSEL_HFRC;
  ADCConfig.ePolarity          = AM_HAL_ADC_TRIGPOL_RISING;
  ADCConfig.eTrigger           = AM_HAL_ADC_TRIGSEL_SOFTWARE;
  ADCConfig.eReference         = AM_HAL_ADC_REFSEL_INT_2P0;  //AM_HAL_ADC_REFSEL_INT_1P5;
  ADCConfig.eClockMode         = AM_HAL_ADC_CLKMODE_LOW_LATENCY;
  ADCConfig.ePowerMode         = AM_HAL_ADC_LPMODE0;
  ADCConfig.eRepeat            = AM_HAL_ADC_REPEATING_SCAN;
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure(g_ADCHandle, &ADCConfig))
  {
    am_util_stdio_printf("Error - configuring ADC0 failed.\n");
  }


  //
  // Set up an ADC slot (2)
  //
  ADCSlotConfig.eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_1;  //AM_HAL_ADC_SLOT_AVG_128;
  ADCSlotConfig.ePrecisionMode  = AM_HAL_ADC_SLOT_14BIT;
  // ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE0;
  ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE2;
  ADCSlotConfig.bWindowCompare  = false;
  ADCSlotConfig.bEnabled        = true;
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 2, &ADCSlotConfig))
  {
    am_util_stdio_printf("Error - configuring ADC Slot 2 failed.\n");
  }

  //
  // Set up an ADC slot (1)
  //
  ADCSlotConfig.eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_1;  //AM_HAL_ADC_SLOT_AVG_128;
  ADCSlotConfig.ePrecisionMode  = AM_HAL_ADC_SLOT_14BIT;
  // ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE0;
  ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE1;
  ADCSlotConfig.bWindowCompare  = false;
  ADCSlotConfig.bEnabled        = true;
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 1, &ADCSlotConfig))
  {
    am_util_stdio_printf("Error - configuring ADC Slot 1 failed.\n");
  }

  //
  // Configure the ADC to use DMA for the sample transfer.
  //
  adc_config_dma();

  //
  // For this example, the samples will be coming in slowly. This means we
  // can afford to wake up for every conversion.
  //
  am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_DERR | AM_HAL_ADC_INT_DCMP );

  //
  // Enable the ADC.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_enable(g_ADCHandle))
  {
    am_util_stdio_printf("Error - enabling ADC0 failed.\n");
  }
}


//*****************************************************************************
//
// Initialize the ADC repetitive sample timer A3.
//
//*****************************************************************************
void
init_timerA3_for_ADC(void)
{
  //
  // Start a timer to trigger the ADC periodically (1 second).
  //
  am_hal_ctimer_config_single(3, AM_HAL_CTIMER_TIMERA,
                              AM_HAL_CTIMER_HFRC_12MHZ    |
                              AM_HAL_CTIMER_FN_REPEAT     |
                              AM_HAL_CTIMER_INT_ENABLE);

  am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA3);

  am_hal_ctimer_period_set(3, AM_HAL_CTIMER_TIMERA, 750, 374);

  //
  // Enable the timer A3 to trigger the ADC directly
  //
  am_hal_ctimer_adc_trigger_enable();

  //
  // Start the timer.
  //
  am_hal_ctimer_start(3, AM_HAL_CTIMER_TIMERA);
}

void init_audio()
{
//  // Set the clock frequency
//  if (AM_HAL_STATUS_SUCCESS != am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0))
//  {
//    am_util_stdio_printf("Error - configuring the system clock failed.\n");
//  }
//
//
//  // Set the default cache configuration and enable it.
//  if (AM_HAL_STATUS_SUCCESS != am_hal_cachectrl_config(&am_hal_cachectrl_defaults))
//  {
//    am_util_stdio_printf("Error - configuring the system cache failed.\n");
//  }
//  if (AM_HAL_STATUS_SUCCESS != am_hal_cachectrl_enable())
//  {
//    am_util_stdio_printf("Error - enabling the system cache failed.\n");
//  }


  // Enable only the first 512KB bank of Flash (0).  Disable Flash(1)
  if (AM_HAL_STATUS_SUCCESS != am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_FLASH_512K))
  {
    am_util_stdio_printf("Error - configuring the flash memory failed.\n");
  }


  am_util_stdio_printf("OK\n");

  // Start the CTIMER A3 for timer-based ADC measurements.
  init_timerA3_for_ADC();

  // Enable interrupts.
  NVIC_EnableIRQ(ADC_IRQn);
  am_hal_interrupt_master_enable();

  // Set a pin(s) to act as our ADC input
  am_hal_gpio_pinconfig(SF_EDGE_PIN_MIC0, g_SF_EDGE_PIN_MIC0);
  am_hal_gpio_pinconfig(SF_EDGE_PIN_MIC1, g_SF_EDGE_PIN_MIC1);

  // Configure the ADC
  adc_config0();

  // Trigger the ADC sampling for the first time manually.
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_sw_trigger(g_ADCHandle))
  {
    am_util_stdio_printf("Error - triggering the ADC0 failed.\n");
  }

}

uint32_t get_avg_readings()
{
  uint32_t avg_reading = 0;

  if (g_bADCDMAComplete)
  {

    // Check for DMA errors.
    if (g_bADCDMAError)
    {
      am_util_stdio_printf("DMA Error occured\n");
      while (1);
    }


    // Check if the ADC DMA completion interrupt occurred.
    if (g_bADCDMAComplete)
    {
      // For slot 1:
      uint32_t slotCount = 0;
      for ( uint32_t indi = 0; indi < ADC_SAMPLE_BUF_SIZE; indi++) {
        am_hal_adc_sample_t temp;

        temp.ui32Slot   = AM_HAL_ADC_FIFO_SLOT(g_ui32ADCSampleBuffer[indi]);
        temp.ui32Sample = AM_HAL_ADC_FIFO_SAMPLE(g_ui32ADCSampleBuffer[indi]);

        if ( temp.ui32Slot == 1 ) {
          SampleBuffer1[slotCount] = temp;
          slotCount++;
        }
      }


      slotCount = 0;
      for ( uint32_t indi = 0; indi < ADC_SAMPLE_BUF_SIZE; indi++) {
        am_hal_adc_sample_t temp;

        temp.ui32Slot   = AM_HAL_ADC_FIFO_SLOT(g_ui32ADCSampleBuffer[indi]);
        temp.ui32Sample = AM_HAL_ADC_FIFO_SAMPLE(g_ui32ADCSampleBuffer[indi]);

        if ( temp.ui32Slot == 2 ) {
          SampleBuffer0[slotCount] = temp;
          slotCount++;
        }
      }

      // Print out the results over UART for visual verification
      for (uint32_t indi = 0; indi < ADC_SAMPLES_PER_SLOT; indi++) {
        //am_util_stdio_printf("%d, %d\n", SampleBuffer0[indi].ui32Slot, SampleBuffer1[indi].ui32Slot);           // Show slot numbers
        //am_util_stdio_printf("%d, %d\n", SampleBuffer0[indi].ui32Sample, SampleBuffer1[indi].ui32Sample);    // Show values
        avg_reading +=  ((SampleBuffer0[indi].ui32Sample + SampleBuffer1[indi].ui32Sample) / 2) / ADC_SAMPLES_PER_SLOT;
      }


      //
      // Reset the DMA completion and error flags.
      //
      g_bADCDMAComplete = false;

      //
      // Re-configure the ADC DMA.
      //
      adc_config_dma();

      //
      // Clear the ADC interrupts.
      //
      if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_clear(g_ADCHandle, 0xFFFFFFFF))
      {
        am_util_stdio_printf("Error - clearing the ADC0 interrupts failed.\n");
      }

      //
      // Trigger the ADC sampling for the first time manually.
      //
      if (AM_HAL_STATUS_SUCCESS != am_hal_adc_sw_trigger(g_ADCHandle))
      {
        am_util_stdio_printf("Error - triggering the ADC0 failed.\n");
      }

      am_util_stdio_printf("AVG = %d\n", avg_reading);
    }
  }

  return avg_reading;

}
