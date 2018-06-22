#ifndef SLAVE_H
#define SLAVE_H

#include <stdint.h>

class Slave {
public:
    // Время нажатия кнопки относительно master
    uint32_t btnTime = 0;
    // Коэффициенты преобразования времени m = a*s + b
    float alpha = 0;
    float beta = 0;

    uint32_t prevSlaveTime = 0;
    uint32_t curSlaveTime = 0;

    uint32_t prevMasterTime = 0;
    uint32_t curMasterTime = 0;

    uint32_t linkTime = 0;
    // Время соединения со slave

    void addMeasure(uint32_t masterTime, uint32_t slaveTime);
    void addPress(uint32_t btnPressSlaveTime);    
};

#endif
