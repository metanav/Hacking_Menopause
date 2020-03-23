#include <Wire.h>
#include "ble_api.h"
#include "SparkFun_LIS2DH12.h"
#include "tf_adc.h"

#define ACCEL_ADDRESS 0x19
#define APP_SENSOR_OVERRIDE
//#define BLE_PERIPHERAL_NAME "Artemis BLE" // Up to 29 characters

SPARKFUN_LIS2DH12 lis2dh12;

void setup() {

  SERIAL_PORT.begin(115200);
  delay(1000);
  SERIAL_PORT.printf("Apollo3 Arduino BLE Example. Compiled: %s\n", __TIME__);

  Wire1.begin(); //Accel is on IOM3 and defined in the variant file as Wire1.

  //By default the SparkFun library uses Wire. We need to begin with Wire1 on the Edge/Edge2.
  if (lis2dh12.begin(ACCEL_ADDRESS, Wire1) == false)
  {
    Serial.println("Accelerometer not detected. Are you sure you did a Wire1.begin()? Freezing...");
    while (1);
  }

  // Initialize ADC to read MEMS Mic input
  initADC();
  enableAdcInterrupts();
  
  pinMode(LED_BUILTIN, OUTPUT);
  set_led_low();

  // Configure the peripheral's advertised name:
  //setAdvName(BLE_PERIPHERAL_NAME);

  // Boot the radio.
  HciDrvRadioBoot(0);

  // Initialize the main ExactLE stack.
  exactle_stack_init();

  // Start the "Sensor" profile.
  SensorStart();
}

void loop() {

  //
  // Calculate the elapsed time from our free-running timer, and update
  // the software timers in the WSF scheduler.
  //
  update_scheduler_timers();
  wsfOsDispatcher();

  //
  // Enable an interrupt to wake us up next time we have a scheduled event.
  //
  set_next_wakeup();

  am_hal_interrupt_master_disable();

  //
  // Check to see if the WSF routines are ready to go to sleep.
  //
  if ( wsfOsReadyToSleep() )
  {
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
  }
  am_hal_interrupt_master_enable();
}

bool_t AppReadGyro(int16_t *pX, int16_t *pY, int16_t *pZ)
{

  while (lis2dh12.available() == false) ;

  *pX = lis2dh12.getRawX();
  *pY = lis2dh12.getRawY();
  *pZ = lis2dh12.getRawZ();

  SERIAL_PORT.print(lis2dh12.getX());
  SERIAL_PORT.print(", ");
  SERIAL_PORT.print(lis2dh12.getY());
  SERIAL_PORT.print(", ");
  SERIAL_PORT.print(lis2dh12.getZ());
  SERIAL_PORT.println("");
  return TRUE;
}

// ( ( (float)lsb / 64.0f ) / 4.0f ) + 25.0f;
bool_t AppReadTemp(int16_t *pTemp)
{
  while (lis2dh12.available() == false) ;

  *pTemp = lis2dh12.getRawTemperature();
  SERIAL_PORT.println(lis2dh12.getTemperature());
  return TRUE;
}


bool_t AppReadSound(int32_t *pSound)
{
  *pSound = audioSample;
  SERIAL_PORT.println(audioSample);
  return TRUE;
}
