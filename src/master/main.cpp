#include <Arduino.h>
#include "CyberLib.h"
#include <printf.h>

#include "nrf_init/nrf_init.h"

byte data = 0;
uint32_t _read = 0;
uint8_t askPipe = 0;

MyRadio *_radio;


void setup()
{
  Serial.begin(115200);
  Serial.print("Hi!\n");
  printf_begin();
  _radio = new MyRadio(PIPE_MASTER);
  _radio->masterMode();
}


int32_t tm = 0;
uint32_t prevDiff = 0;
uint32_t diff = 0;

void loop() {
  RF24 &radio = _radio->radio;

  radio.openReadingPipe(1, (byte *) "Red__");
  // radio.openReadingPipe(2, (byte *) "Green");

  printf("Now sending `%lu+epsilon` as payload.\n\r", micros());
  unsigned long time = micros();

  if ( radio.write(&time, 4) ) {                         // Send the counter variable to the other radio
    if(!radio.available(&askPipe)) {                             // If nothing in the buffer, we got an ack but it is blank
      tm = micros() - time;
      printf(" > Got blank response from %d. round-trip delay: %lu microseconds\n\r", askPipe, micros()-time);

    } else {
      while(radio.available(&askPipe) ) {                      // If an ack with payload was received
        tm = micros() - time;
        radio.read( &_read, 4 );                  // Read it, and display the response time
        uint8_t id = 0;
        radio.read(&id, 1);
        printf(" > Got response `%lu` from %d (id:%d), round-trip delay: %ld microseconds\n\r", _read, askPipe, id, tm);
        prevDiff = diff;
        if (_read > (time + tm / 2)) {
          diff = _read - (time + tm / 2);
        } else {
          diff = (time + tm / 2) - _read;
        }
        printf("Diff: %lu; Drift: %ld\n\r", diff, abs(diff - prevDiff));
        data++;                                  // Increment the counter variable
      }
    }

  } else {
    tm = micros() - time;
    printf(" > Sending failed. Delay: %ld\n\r", tm);
  }          // If no ack response, sending failed

  delay(1000);
}
