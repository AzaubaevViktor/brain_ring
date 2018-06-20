#include <Arduino.h>
#include "CyberLib.h"
#include <printf.h>

#include "nrf_init/nrf_init.h"

MyRadio *_radio;


void setup()
{
    Serial.begin(115200);
    Serial.print("Hi!\n");
    printf_begin();
    _radio = new MyRadio(PIPE_RED);
    _radio->slaveMode();
    _radio->printDetails();
}

long i = 0;

void loop() {
    SlaveData sd;
    memset(&sd, 0, 32);
    MasterData md;
    memset(&md, 0, 32);

    sd.btnTime = micros();
    if (_radio->slaveReceive(sd, md)) {
      printMD(md);
    }
}
