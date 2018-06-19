#include "nrf_init.h"

void printMD(MasterData &md) {
  printf("dataCheck: %" PRIu8 "\r\n"
         "state    : %" PRIu8 "\r\n"
         "player   : %" PRIu8 "\r\n"
         "startSend: %" PRIu32 "\r\n"
         "endSend  : %" PRIu32 "\r\n"
         "error    : %" PRIu8  "\r\n"
         "isData   : %" PRIu8 "\r\n"
         "======================\r\n",
    md.dataCheck,
    md.state,
    md.player,
    md.startSend,
    md.endSend,
    md.error,
    md.isData);
}

void printSD(SlaveData &sd) {
    printf("slaveTime: %" PRIu32 "\r\n"
           "btnTime  : %" PRIu32 "\r\n"
           "dataCheck: %" PRIu8  "\r\n"
           "error    : %" PRIu8  "\r\n"
           "======================\r\n",
    sd.slaveTime,
    sd.btnTime,
    sd.dataCheck,
    sd.error);
}


MyRadio::MyRadio(pipe_t myPipe) {
    // Загружаем имя pipe к себе
    memcpy(this->myPipe, myPipe, 5);
    init();
}

void MyRadio::init() {
    // Запускаем радио
    radio.begin();
    // Ждём две секунды на всякий случай
    delay(2000);

    // Выставляем автоматический ответ на пришедшее сообщение
    radio.setAutoAck(1);                    // Ensure autoACK is enabled
    radio.enableAckPayload();
    radio.setRetries(2, 5);                 // Smallest time between retries, max no. of retries
    // One retry ~500us

    // Размер ответа -- 32 байта
    radio.setPayloadSize(32);

    radio.setPALevel(RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
    // соответствует уровням:    -18dBm,      -12dBm,      -6dBM,           0dBm
    radio.setDataRate(RF24_1MBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
    radio.setCRCLength(RF24_CRC_16); // длинна контрольной суммы 8-bit or 16-bit
    radio.setChannel(0x7f);         // установка канала
    radio.openWritingPipe((byte *) myPipe); // На запись
};

void MyRadio::printDetails() {
  radio.printDetails();
};


void MyRadio::slaveMode() {
    // Listening mode от master
    radio.openReadingPipe(1, (byte *) PIPE_MASTER);
    radio.startListening();
    radio.powerUp();               // включение или пониженное потребление powerDown - powerUp
}


void MyRadio::masterMode() {
    // Send mode на все slave
    radio.openReadingPipe(1, (byte *) PIPE_RED);
    // radio.openReadingPipe(2, (byte *) PIPE_GREEN);
    // radio.openReadingPipe(3, (byte *) PIPE_BLUE);
    // radio.openReadingPipe(4, (byte *) PIPE_YELLOW);
    radio.startListening();
    radio.stopListening();
    radio.powerUp();               // включение или пониженное потребление powerDown - powerUp
}

void MyRadio::changePipe(byte * pipe) {
    radio.openWritingPipe((byte *) pipe);
}


void MyRadio::masterSend(SlaveData &sd, MasterData &md) {
    // отправляет slave MasterData, принимает SlaveData
    md.startSend = micros();
    md.error = NO_ERR;
    md.dataCheck = 13;
    // Передаём данные
    if (radio.write(&md, sizeof(MasterData))) {
        // Отправили, ждём ответа
        if (!radio.available()) {
            // Если нет данных
            md.error = ERR_RECEIVE;
        } else {
            // Если данные есть
            while (radio.available()) {
                radio.read(&sd, sizeof(SlaveData));
            }

            if (13 == sd.dataCheck) {
                md.isData = true;
            } else {
                md.error = ERR_CHECK;
            }
        }
    } else {
        md.error = ERR_SEND;
    }
    md.endSend = micros();
}


uint8_t MyRadio::slaveReceive(SlaveData &sd, MasterData &md) {
    // Принимает MasterData, отправляет SlaveData
    uint8_t askPipe = 0;
    sd.dataCheck = 13;

    if (radio.available(&askPipe)) {
        radio.read(&md, sizeof(MasterData));
        if (13 == md.dataCheck) {
            sd.slaveTime = micros();
            radio.writeAckPayload(askPipe, &sd, sizeof(SlaveData));

            sd.error = NO_ERR;
        } else {
            sd.error = ERR_RECEIVE;
        }
        return 1;
    } else {
        return 0;
    }
}
