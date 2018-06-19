#ifndef NRF_INIT
#define NRF_INIT

// #include <printf.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>

typedef char pipe_t[6];

#define S_UNKNOWN_STATE (0)
#define S_WAITING (1)
#define S_FALSESTART (2)
#define S_ANSWER (3)
#define S_GAME (4)

#define NO_ERR (0)
#define ERR_SEND (11)
#define ERR_RECEIVE (12)
#define ERR_CHECK (13)

/* Это пиздец, товарищи. Подбор работающих параметров -- это, сука, сраный ад.
* Когда-нибудь быть может я даже этим займусь, но не сейчас точно, ибо надо
* доделать, и не факт, что при смене оно заработает так, как надо.
* Поэтому если не понимаешь, что делаешь, не меняй ничего!
* Все параметры в этой табличке:
*/

struct MasterData {
    uint8_t dataCheck = 13; // 1b
    uint8_t state = 0; // 1b смотри S_*
    uint8_t player = 0; // 1b номер игрока, которой фальшстартанул/отвечает

    uint32_t startSend = 0; // 4b
    uint32_t endSend = 0; // 4b
    uint8_t error = 0; // 1b
    uint8_t isData = 0; // 1b
    uint8_t _nop[19];
};

struct SlaveData {
    uint32_t slaveTime = 0; // 4b время на slave
    uint32_t btnTime = 0; // 4b время нажатия кнопки по slave

    uint8_t dataCheck = 0; // 1b
    uint8_t error = 0; // 1b
    uint8_t _nop[22];
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
    void changePipe(byte *pipe);
    void masterSend(SlaveData &sd, MasterData &md);
    uint8_t slaveReceive(SlaveData &sd, MasterData &md);
private:
    void init();
};

#define PIPE_MASTER "Mastr"
#define PIPE_RED "Red__"
#define PIPE_GREEN "Green"
#define PIPE_BLUE "Blue_"
#define PIPE_YELLOW "Yellw"

const byte pipeMaster[6] = PIPE_MASTER;

const byte pipeSlave[4][6] = {
    PIPE_RED, PIPE_GREEN, PIPE_BLUE, PIPE_YELLOW
};


#endif /* end of include guard: NRF_INIT */
