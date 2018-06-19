#include <Arduino.h>
#include "CyberLib.h"
#include <printf.h>

#include "nrf_init/nrf_init.h"

MyRadio *_radio;


void setup()
{
    for (int i = 0; i < 10; ++i) {
        D13_Inv;
        delay_ms(100);
    }
    D13_Low;
    Serial.begin(115200);
    Serial.print("Hi!\n");
    printf_begin();
    _radio = new MyRadio(PIPE_MASTER);
    _radio->masterMode();
    _radio->printDetails();
    _radio->changePipe(PIPE_RED);
}

void loop() {
    SlaveData sd;
    memset(&sd, 0, 32);
    MasterData md;
    memset(&md, 0, 32);
    long start = micros();

    for (int i = 1; i <= 2; ++i) {
        md.state = S_FALSESTART;
        md.player = 2;
        _radio->changePipe(pipeSlave[i - 1]);
        _radio->masterSend(sd, md);

        // printf("PLAYER: %d\r\n", i);
        // printf("Time: %" PRIu32 "\r\n", md.endSend - md.startSend);
        // printf("Error: %" PRIu8 "\r\n", md.error);
        // printSD(sd);
        uint32_t tm = md.endSend - md.startSend;
        if (md.error) {
            tm = 1;
        }
        printf("%" PRIu32 ",", tm);

    }
    printf("\r\n");

    delay_us(1000000/30 - (micros() - start));
}
