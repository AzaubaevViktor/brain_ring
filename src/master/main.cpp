#include <Arduino.h>
#include "CyberLib.h"
#include <printf.h>

#include "../nrf_init/nrf_init.h"
#include "slave.h"

#define elif else if

// MASTER // сильно не бейте, ибо я не смог скомпилить это из двух файлов.
class Master {
public:
    uint32_t roundTime = 5 * 1000000; // в микросекундах
    MyRadio *radio;
    MasterData md;
    SlaveData sd[4];
    Slave slave[4];
    bool lBtn = 0;
    bool rBtn = 0;
    bool changeState = false;
    uint32_t timeLeft = 0; // сколько времени осталось на игру
    uint32_t lastStepTime = 0; // время предыдущего шага

    uint32_t referTime = 1;
    // Опорное время. Используется при арбитраже
    // Выставляется по нажатию кнопки.

    Master(bool debug);
    void query();
    void getPlayer();
    void logic();
    uint32_t step();

    void interruptsInit(void (*l)(), void (*r)());
    void lBtnSet();
    void rBtnSet();
};

void Master::lBtnSet() {
    lBtn = true;
}

void Master::rBtnSet() {
    rBtn = true;
}

void Master::interruptsInit(void (*l)(), void (*r)()) {
    attachInterrupt(0, l, FALLING);
    attachInterrupt(1, r, FALLING);
}

// В прерываниях нельзя использовать функции из класса
Master::Master(bool debug) {
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);

    md.state = S_WAITING;

    Serial.begin(115200);
    printf_begin();
    printf("Master\n\r");
    radio = new MyRadio(PIPE_MASTER);
    radio->masterMode();
    radio->changePipe(PIPE_RED);
    radio->printDetails();
}

void Master::query() {
    // Опрос slave
    for (int i = 0; i < 4; ++i) {
        radio->changePipe(pipeSlave[i]);

        md.needDrop = (sd[i].btnTime != 0);
        // Нужно ли сбрасывать нажатие кнопки на slave
        radio->masterSend(sd[i], md);

        slave[i].linkTime = md.endSend - md.startSend;
        // Выставляем время соединения для отображения качества соединения

        if (md.error) continue;

        uint32_t masterTime = (md.endSend + md.startSend) / 2;
        slave[i].addMeasure(masterTime, sd[i].slaveTime);
        if (0 != sd[i].btnTime) {
            slave[i].addPress(sd[i].btnTime);
        }
    }
    md.needDrop = false;
}

void Master::getPlayer() {
    // Ищем игрока, который нажал на кнопку
    int8_t player = -1;
    uint32_t minTime = micros();

    for (int i = 0; i < 4; ++i) {
        uint32_t btnTime = slave[i].btnTime;
        if (btnTime < referTime) continue;
        if (btnTime < minTime) {
            minTime = btnTime;
            player = i;
        }
    }

    md.player = player;
}

void Master::logic() {
    // Логика работы master, смена состояний
    // Опрос кнопок. Инвертированное -- так как pullup
    bool L = lBtn;
    bool R = rBtn;
    lBtn = false; // сбрасываем значения
    rBtn = false;
    getPlayer();
    int8_t player = md.player;
    bool S = -1 != player; // нажал ли кто-нибудь из slave кнопку

    if (R || L) referTime = micros();

    changeState = false;
    switch (md.state) {
        case S_WAITING: {
            if (S) {
                md.state = S_FALSESTART;
                changeState = true;
            } elif (L) {
                md.state = S_GAME;
                changeState = true;
            }
            break;
        }

        case S_FALSESTART: {
            if (R) {
                md.state = S_WAITING;
                changeState = true;
            } elif (L) {
                md.state = S_GAME;
                changeState = true;
            }
            break;
        }

        case S_GAME: {
            if (S) {
                md.state = S_ANSWER;
                changeState = true;
            } elif (L) {
                md.state = S_WAITING;
                changeState = true;
            } elif (0 == timeLeft) {
                md.state = S_WAITING;
                changeState = true;
            }
            break;
        }

        case S_ANSWER: {
            if (L) {
                md.state = S_WAITING;
                changeState = true;
            } elif (R) {
                md.state = S_GAME;
                changeState = true;
            }
            break;
        }
    }

    return;
}

uint32_t Master::step() {
    uint32_t dt = micros() - lastStepTime;
    lastStepTime = micros();

    switch (md.state) {
        case S_WAITING: {
            timeLeft = 0;
            break;
        }

        case S_FALSESTART: {
            timeLeft = 0;
            break;
        }

        case S_GAME: {
            if (0 == timeLeft) {
                timeLeft = roundTime;
            } else {
                if (dt > timeLeft) {
                    timeLeft = 0;
                    // состояние меняется в ::logic
                } else {
                    timeLeft -= dt;
                }
            }
            break;
        }

        case S_ANSWER: {

            break;
        }
    }

    return dt;
};


// END MASTER //

Master *master;

void lInt() {
    master->lBtnSet();
}

void rInt() {
    master->rBtnSet();
}

void setup() {
    master = new Master(true);
    master->interruptsInit(lInt, rInt);
}

void loop() {
    // шаг

    master->query();
    master->logic();
    uint32_t dt = master->step();

    printf("State: %d\n\r", master->md.state);
    printf("Player: %d\n\r", master->md.player);
    printf("Dt: %" PRIu32 "\n\r", dt);
    printf("TimeLeft: %" PRIu32 "\n\r", master->timeLeft);
    printf("Refer: %" PRIu32 "\n\r", master->referTime);

    for (int i = 0; i < 4; ++i) {
        printSD(master->sd[i]);
        Serial.print("Time: ");
        Serial.print(master->slave[i].alpha);
        Serial.print("; ");
        Serial.print(master->slave[i].beta);
        Serial.print("\n\r");

        printf("==================\n\r");
    }
    // Оотображение -- динамик, дисплей, светодиоды
    delay_ms(300);
}

























// Это сраный atom, нужно место под автодополнение
