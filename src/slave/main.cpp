#include <Arduino.h>
#include "CyberLib.h"
#include <printf.h>

#include "../nrf_init/nrf_init.h"

// Прерывания для кнопки (вешается на 3й пин)
uint32_t btnTime = 0;

SlaveData sd;
MasterData md;

void setBtn() {
    if ((S_GAME == md.state) || (S_WAITING == md.state)) {
        btnTime = micros();
    }
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
    radio = new MyRadio(PIPE_GREEN);
    radio->slaveMode();
    radio->printDetails();
}



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
