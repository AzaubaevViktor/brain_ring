#ifndef NRF_INIT
#define NRF_INIT

// #include <printf.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>

typedef char pipe_t[6];

struct Answer {
  uint8_t data[32];
  bool isData = false;
  bool success = false;
  uint32_t start = 0;
  uint32_t end = 0;
};

class MyRadio {
public:
  pipe_t myPipe;
  RF24 &radio = *new RF24(9, 10);
  MyRadio(pipe_t myPipe);
  void masterMode();
  void slaveMode();
  Answer masterSend(uint8_t data[32], pipe_t pipe);
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
