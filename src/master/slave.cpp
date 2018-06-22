#include "slave.h"

void Slave::addMeasure(uint32_t masterTime, uint32_t slaveTime) {
    prevMasterTime = curMasterTime;
    curMasterTime = masterTime;

    prevSlaveTime = curSlaveTime;
    curSlaveTime = slaveTime;

    if (prevSlaveTime > curSlaveTime) {
        // Такое может произойти, например, при выключении slave
        // Или при перезагрузке
        prevSlaveTime = 0;
    }

    if ((prevMasterTime == 0) || (prevSlaveTime == 0)) {
        // Считать alpha и beta бессмысленно
        return;
    }

    alpha = (curMasterTime - prevMasterTime) / (curSlaveTime - prevSlaveTime);
    beta = curMasterTime - alpha * curSlaveTime;
}


void Slave::addPress(uint32_t btnPressSlaveTime) {
    btnTime = alpha * btnPressSlaveTime + beta;
}
