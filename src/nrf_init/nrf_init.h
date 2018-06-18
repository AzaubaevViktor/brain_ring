#ifndef NRF_INIT
#define NRF_INIT

// #include <printf.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>

typedef char pipe_t[6];

struct SlaveData {
  // Заполняет slave
  uint32_t btnTime = 0; // 4b
  uint8_t dataOk = 0; // 1b
  bool isBtn = false; // 1b
  // Заполняет master при приёме
  bool success = false; // 1b
  bool isData = false; // 1b
  uint8_t color = 0; // 1b
  uint8_t _nop[23]; // 23b
};

struct MasterData {
  uint8_t dataOk = 0; // 1b
  // Заполняет master
  uint32_t masterTime = 0; // 4b
  // 0 -- UNKNOWN
  // 1 -- Время не идёт
  // 2 -- Время идёт
  // 3 -- Кто-то отвечает
  uint8_t state = 0; // 1b
  bool falseStart[4] = {0}; // 4b
  // Номер slave, который отвечает
  uint8_t answer = 0; // 1b

  // Заполняет slave при приёме
  uint32_t start = 0; // 4b
  uint32_t end = 0; // 4b
  bool success = false; // 1b
  bool isData = false; // 1b
  uint8_t _nop[11]; // 11b
};

void printSD(SlaveData &sd);
void printMD(MasterData &md);

static_assert(sizeof(SlaveData) == 32, "sizeof(SlaveData) != 32");
static_assert(sizeof(MasterData) == 32, "sizeof(MasterData) != 32");

class MyRadio {
public:
  pipe_t myPipe;
  RF24 &radio = *new RF24(4, 8);
  MyRadio(pipe_t myPipe);
  void masterMode();
  void slaveMode();
  void printDetails();
  void slaveSend(SlaveData &sd, MasterData &md);
  void masterReceiveFromSlave(SlaveData &sd, MasterData &md);
  void masterSendToSlave(uint8_t askPipeN, MasterData &md);
private:
  void init();
};

#define PIPE_MASTER "Mastr"
#define PIPE_RED "Red__"
#define PIPE_GREEN "Green"
#define PIPE_BLUE "Blue_"
#define PIPE_YELLOW "Yellw"

const byte PipeMaster[6] = "Mastr";

const byte PipeSlave[4][6] = {
  "Red__", "Green", "Blue_", "Yellw"
};


#endif /* end of include guard: NRF_INIT */
