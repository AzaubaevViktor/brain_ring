#include <Arduino.h>
#include "CyberLib.h"


#include <printf.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>

RF24 radio(4, 8);

const byte pipe_in[6] = "Mastr";
const byte pipe_out[6] = "Green";
uint8_t askPipe = 0;

uint32_t data = 0;

typedef struct _Command {
  uint32_t timestamp; // 4b
  uint8_t cmd; // 1b
} Command;


void setup()
{
  Serial.begin(115200);
  printf_begin();
  radio.begin();
  delay(2000);

  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();
  radio.setRetries(0, 5);                 // Smallest time between retries, max no. of retries
  // One retry ~500us
  radio.setPayloadSize(32);

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_2MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  radio.setCRCLength(RF24_CRC_16); // длинна контрольной суммы 8-bit or 16-bit
  // соответствует уровням:    -18dBm,      -12dBm,      -6dBM,           0dBm
  radio.setChannel(0x7f);         // установка канала
  radio.openWritingPipe(pipe_out); // На запись
  radio.openReadingPipe(1,pipe_in); // открыть канал на приём

  radio.powerUp();               // включение или пониженное потребление powerDown - powerUp
  radio.printDetails();
  radio.startListening();

}

unsigned long count = 0;

struct Data {
  uint32_t count;
  uint8_t number;
};

void loop() {
  if(radio.available(&askPipe)) {
    count = micros();
    radio.read(&data, 4);
    Data toSend = {count, 1};
    radio.writeAckPayload(askPipe, &toSend, 5);
    printf("Pipe: %d; Receive: %lu; Send: %lu\n\r", askPipe, data, count);
  }
}
