#include "nrf_init.h"

void printMD(MasterData &md) {
  printf("dataOk: %" PRIu8 "\r\n"
         "masterTime: %" PRIu32 "\r\n"
         "state: %" PRIu8 "\r\n"
         "falseStart: %" PRIu8 " %" PRIu8 " %" PRIu8 " %" PRIu8 "\r\n"
         "answer: %" PRIu8 "\r\n"
         "start: %" PRIu32 "\r\n"
         "end: %" PRIu32 "\r\n"
         "success: %" PRIu8 "\r\n"
         "isData: %" PRIu8 "\r\n"
         "=====================\r\n",
         md.dataOk, md.masterTime, md.state,
         md.falseStart[0], md.falseStart[1],md.falseStart[2], md.falseStart[3],
         md.answer, md.start, md.end, md.success, md.isData
       );
}

void printSD(SlaveData &sd) {
  printf("btnTime: %" PRIu32 "\r\n"
         "dataOk: %" PRIu8 "\r\n"
         "isBtn: %" PRIu8 "\r\n"
         "success: %" PRIu8 "\r\n"
         "isData: %" PRIu8 "\r\n"
         "color: %" PRIu8 "\r\n"
         "=====================\r\n",
         sd.btnTime, sd.dataOk, sd.isBtn, sd.success,
         sd.isData, sd.color
       );
}


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
  // radio.openReadingPipe(1, (byte *) ); // открыть канал на приём

};

void MyRadio::printDetails() {
  radio.printDetails();
};

void MyRadio::slaveMode() {
  // Receiver mode
  radio.openReadingPipe(1, (byte *) PIPE_MASTER);
  radio.startListening();
  radio.stopListening();
  radio.powerUp();               // включение или пониженное потребление powerDown - powerUp
};

void MyRadio::masterMode() {
  // Listening mode
  radio.openReadingPipe(1, (byte *) PIPE_RED);
  radio.openReadingPipe(2, (byte *) PIPE_GREEN);
  radio.openReadingPipe(3, (byte *) PIPE_BLUE);
  radio.openReadingPipe(4, (byte *) PIPE_YELLOW);
  radio.startListening();
  radio.powerUp();               // включение или пониженное потребление powerDown - powerUp
}



void MyRadio::slaveSend(SlaveData &sd, MasterData &md) {
  // Отправляет SlaveData, Принимает MasterData
  uint32_t start, end;
  sd.dataOk = 13;
  start = micros();
  if (radio.write(&sd, 32)) { // Передать данные
    // delayMicroseconds(1000);
    if (!radio.available()) { // Если нет данных
      end = micros();
      md.success = true;
    } else { // Если данные есть
      while (radio.available()) {
        end = micros();
        radio.read(&md, 32);
        if (13 == md.dataOk) {
          md.success = true;
          md.isData = true;
        }
      }
    }
  } else {printf("Err\n");}
  md.start = start;
  md.end = end;
};

void MyRadio::masterReceiveFromSlave(SlaveData &sd, MasterData &md) {
  // Принимает SlaveData, отправляет MasterData
  uint8_t askPipe = 0;
  md.dataOk = 13;

  if (radio.available(&askPipe)) {
    radio.read(&sd, 32);
    if (13 == sd.dataOk) {
      radio.writeAckPayload(askPipe, &md, 24);

      sd.success = true;
      sd.isData = true;
      sd.color = askPipe;
    }
    printf("! %d !\n", askPipe);
  }
}

void MyRadio::masterSendToSlave(uint8_t askPipeN, MasterData &md) {
  // Отправляет MasterData
  md.dataOk = 13;
  radio.writeAckPayload(askPipeN, &md, 32);
}
