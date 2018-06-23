#include <Arduino.h>
#include "CyberLib.h"
#include <printf.h>

// For display
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "../nrf_init/nrf_init.h"
#include "slave.h"

#define elif else if

// MASTER // сильно не бейте, ибо я не смог скомпилить это из двух файлов.
class Master {
public:
    uint32_t roundTime = 10 * 1000000; // в микросекундах
    MyRadio *radio;
    MasterData &md = *new MasterData();
    SlaveData sd[4];
    Slave slave[4];
    // Кнопки
    bool lBtn = 0;
    bool rBtn = 0;
    uint32_t lastLBtn = 0;
    uint32_t lastRBtn = 0;
    uint32_t minBtnTime = 500000; // us

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
    noInterrupts();
    for (int i = 0; i < 4; ++i) {
        radio->changePipe(pipeSlave[i]);

        md.needDrop = (sd[i].btnTime != 0);
        // Нужно ли сбрасывать нажатие кнопки на slave
        radio->masterSend(sd[i], md);

        // slave[i].applyNetInfo(md.endSend - md.startSend, md.error);
        printMD(md);
        Serial.print(md.endSend - md.startSend);
        Serial.print(" ");
        Serial.print(md.error);
        Serial.print("; ");
        // Выставляем время соединения для отображения качества соединения

        if (md.error) continue;

        uint32_t masterTime = (md.endSend + md.startSend) / 2;
        slave[i].addMeasure(masterTime, sd[i].slaveTime);
        if (0 != sd[i].btnTime) {
            slave[i].addPress(sd[i].btnTime);
        }
    }
    Serial.println();
    interrupts();
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
    // Опрос кнопок.
    noInterrupts();
    if (lBtn) {
        if (lastLBtn + minBtnTime > micros()) {
            lBtn = false;
        } else {
            lastLBtn = micros();
        }
    }

    if (rBtn) {
        if (lastRBtn + minBtnTime > micros()) {
            rBtn = false;
        } else {
            lastRBtn = micros();
        }
    }
    bool L = lBtn;
    bool R = rBtn;

    lBtn = false; // сбрасываем значения
    rBtn = false;
    interrupts();

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

#define LEVEL_0_us (15000)
#define LEVEL_9_us (1500)

#define A_SIGNAL_LEVEL (-3. / (LEVEL_0_us - LEVEL_9_us))
#define B_SIGNAL_LEVEL (-A_SIGNAL_LEVEL * LEVEL_0_us)

uint8_t calcSignalLevel(uint32_t ping) {
    if (ping > LEVEL_0_us) return 0;
    if (ping < LEVEL_9_us) return 3;
    return A_SIGNAL_LEVEL * ping + B_SIGNAL_LEVEL;
}

LiquidCrystal_I2C lcd(0x27, 16, 2);

uint8_t lcdNet0[8] = {16, 16, 28, 0, 0, 0};
uint8_t lcdNet1[8] = {16, 16, 28, 0, 0, 16};
uint8_t lcdNet2[8] = {16, 16, 28, 0, 4, 20};
uint8_t lcdNet3[8] = {16, 16, 29, 1, 5, 21};

uint8_t btr0[8] = {4, 31, 17, 17, 17, 31};
uint8_t btr1[8] = {4, 31, 17, 17, 31, 31};
uint8_t btr2[8] = {4, 31, 17, 31, 31, 31};
uint8_t btr3[8] = {4, 31, 31, 31, 31, 31};

void setup() {
    lcd.begin();
    lcd.backlight();
    lcd.clear();
    lcd.print(" BrainRing v0.1 ");
    lcd.createChar(0, lcdNet0);
	lcd.createChar(1, lcdNet1);
	lcd.createChar(2, lcdNet2);
	lcd.createChar(3, lcdNet3);
	lcd.createChar(4, btr0);
	lcd.createChar(5, btr1);
	lcd.createChar(6, btr2);
	lcd.createChar(7, btr3);
    lcd.setCursor(0, 1);
    lcd.print("  Charging...   ");

    master = new Master(true);
    master->interruptsInit(lInt, rInt);
    lcd.clear();
}
#define _DEBUG

uint8_t fps = 0;

void loop() {
    // шаг
    uint32_t start = micros();
    // Работа со slave
    master->query();
    master->logic();
    uint32_t dt = master->step();

#ifdef DEBUG
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
#endif
    // Оотображение -- динамик, дисплей, светодиоды
    // lcd.clear();
    for (int i = 0; i < 4; ++i) {
        lcd.setCursor(i*4 + 1, 0);
        if (master->slave[i].errors > 0.9) {
            lcd.print("  ");
            continue;
        }

        int link = calcSignalLevel(master->slave[i].linkTime);
        Serial.print(master->slave[i].linkTime);
        Serial.print(" ");
        lcd.print((char) link);
        lcd.print((char) 4);
    }
    Serial.println();

    lcd.setCursor(0, 1);
    switch (master->md.state) {
        case S_WAITING: lcd.print("Wait    ");
            break;
        case S_FALSESTART: lcd.print("FlSt ");
            lcd.print(master->md.player);
            lcd.print(" ");
            break;
        case S_GAME: lcd.print("Game ");
            lcd.print(master->timeLeft / 1000000);
            lcd.print(" ");
            break;
        case S_ANSWER:
            lcd.print("Answ ");
            lcd.print(master->md.player);
            break;
    }

    lcd.setCursor(14, 1);
    lcd.print(fps);

    delay_ms(10);

    fps = 1000000 / (micros() - start);
}

























// Это сраный atom, нужно место под автодополнение
