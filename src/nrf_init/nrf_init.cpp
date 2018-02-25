#include "nrf_init.h"


MyRadio::MyRadio(pipe_t myPipe) {
  memcpy(this->myPipe, myPipe, 5);
  init();
}

void MyRadio::init() {
  radio.begin();
  delay(2000);

  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();
  radio.setRetries(0, 5);                 // Smallest time between retries, max no. of retries
  // One retry ~500us

  radio.setPayloadSize(32);

  radio.setPALevel(RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_2MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  radio.setCRCLength(RF24_CRC_16); // длинна контрольной суммы 8-bit or 16-bit
  // соответствует уровням:    -18dBm,      -12dBm,      -6dBM,           0dBm
  radio.setChannel(0x7f);         // установка канала
  radio.openWritingPipe((byte *) myPipe); // На запись
  // radio.openReadingPipe(1, (byte *) "Red__"); // открыть канал на приём

  radio.powerUp();               // включение или пониженное потребление powerDown - powerUp
  radio.printDetails();
}

void MyRadio::masterMode() {
  // Receiver mode
  radio.stopListening();
};

void MyRadio::slaveMode() {
  // Listening mode
  radio.startListening();
}

Answer MyRadio::masterSend(uint8_t data[32], pipe_t pipe) {
  Answer a;
  radio.openReadingPipe(1, (byte *) pipe);
  a.start = micros();
  return a;
}
