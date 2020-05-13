#include "thermal_camera_handler.h"

#include <Arduino.h>
#include <Wire.h>
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include "constants.h"

const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640

#define TA_SHIFT 8 //Default shift for MLX90640 in open air

static float mlx90640To[768];
paramsMLX90640 mlx90640;

boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); //Sensor did not ACK
  return (true);
}

TfLiteStatus InitThermalCamera(tflite::ErrorReporter* error_reporter) {
  // Wait until we know the serial port is ready
//  while (!Serial) {
//  }
  Serial.println("MLX90640 init start!");

  Wire.begin();
  Wire.setClock(400000);

  if (isConnected() == false)
  {
    Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
    while (1);
  }

  Serial.println("MLX90640 detected!");

  //Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0) {
    Serial.println("Failed to load system parameters");
  }

  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0) {
    Serial.println("Parameter extraction failed");
  }

  Serial.println("Thermal Camera Initialized");
  delay(100);
  return kTfLiteOk;
}

float GetThermalImage(tflite::ErrorReporter* error_reporter, float* input, int length) {
  for (byte x = 0 ; x < 2 ; x++) //Read both subpages
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);

      return -100.0; // return big enough negative value to show error
    }

    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    float emissivity = 0.95;

    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
  }

  int i = 0;
  float max_temp = 0.0;
  for (uint8_t x = 0; x < 32; x++) {
    for (uint8_t y = 0; y < 24; y++) {
      input[i] = mlx90640To[24 * x + y];
      if (input[i] > max_temp) {
        max_temp = input[i];
      }
      i++;
    }
  }

  return max_temp;
}
