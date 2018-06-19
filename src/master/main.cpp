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
    _radio = new MyRadio(PIPE_MASTER);
    _radio->masterMode();
    _radio->printDetails();
}

void loop() {
    SlaveData sd;
    memset(&sd, 0, 32);
    MasterData md;
    memset(&md, 0, 32);


    md.state = S_FALSESTART;
    md.player = 2;
    _radio->masterSend(sd, md);

    printf("Time: %" PRIu32 "\r\n", md.endSend - md.startSend);
    printf("Error: %" PRIu8 "\r\n", md.error);
    printSD(sd);

    delay_ms(2000);

}
