#include <Arduino.h>
#include "CyberLib.h"
#include <printf.h>

#include "../nrf_init/nrf_init.h"

// Прерывания для кнопки (вешается на 3й пин)
uint32_t btnTime = 0;

void setBtn() {
    btnTime = micros();
}


MyRadio *radio;

void setup()
{
    // прерывание
    pinMode(3, INPUT_PULLUP);
    attachInterrupt(1, setBtn, FALLING);

    Serial.begin(115200);
    Serial.print("Hi!\n");
    printf_begin();
    radio = new MyRadio(PIPE_RED);
    radio->slaveMode();
    radio->printDetails();
}

SlaveData sd;
MasterData md;

void loop() {
    sd.btnTime = btnTime;
    if (radio->slaveReceive(sd, md)) {
      printMD(md);
      if (md.needDrop) {
          btnTime = 0;
      }
      printf("bT: %" PRIu32 "\r\n", btnTime);
    }
}
