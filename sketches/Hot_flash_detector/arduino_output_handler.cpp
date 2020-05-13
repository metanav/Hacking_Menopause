#include "output_handler.h"
#include "Arduino.h"
#include "M5Atom.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Fujitsu.h>

const uint16_t kIrLed = 12;
IRFujitsuAC ac(kIrLed);

uint8_t DisBuff[2 + 5 * 5 * 3];

void setBuff(uint8_t Rdata, uint8_t Gdata, uint8_t Bdata)
{
  DisBuff[0] = 0x05;
  DisBuff[1] = 0x05;
  for (int i = 0; i < 25; i++)
  {
    DisBuff[2 + i * 3 + 0] = Rdata;
    DisBuff[2 + i * 3 + 1] = Gdata;
    DisBuff[2 + i * 3 + 2] = Bdata;
  }
}

void HandleOutput(tflite::ErrorReporter* error_reporter, int kind, float *last_max_readings, int len) {
  // The first time this method runs, set up our LED
  static bool is_initialized = false;
  if (!is_initialized) {
    M5.begin(true, false, true);
    ac.begin();
    delay(200);
    ac.setModel(ARRAH2E);
    ac.setSwing(kFujitsuAcSwingOff);
    ac.setMode(kFujitsuAcModeCool);
    ac.setFanSpeed(kFujitsuAcFanHigh);
    ac.setTemp(20);
    ac.setCmd(kFujitsuAcCmdTurnOn);
    is_initialized = true;
  }

  float x, y;

  for (uint8_t j = 0; j < len; j++) {
    if ( j < 10) {
      x += last_max_readings[j] / 10;
    } else {
      y += last_max_readings[j] / 10;
    }
    Serial.print(last_max_readings[j]);
    if (j < 19) {
      Serial.print(",");
    } else {
      Serial.println("\n");
    }
  }

  Serial.println(kind);
  Serial.println(x);
  Serial.println(y);

  // Check if it is a person and last 20 readings has difference of 3 degree C; it's hot flash!
  if (kind == 0 && (y - x) > 1.0) {
    setBuff(0xff, 0x00, 0x00);
    M5.dis.displaybuff(DisBuff);
    ac.send();
  } else {
    setBuff(0x00, 0x00, 0x00);
    M5.dis.displaybuff(DisBuff);
  }
}
