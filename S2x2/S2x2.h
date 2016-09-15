#define S2_GENERAL_ID   3

// General Mode Command.
#define S2_SETUP_ID     0x01
#define S2_SETUP_GAME   0x02
#define S2_START_POLL   0x03
#define S2_STOP_POLL    0x04
#define S2_DETAILS      0x05

#define S2_LED_COLOR    0x60

// Game Mode Command.
#define S2_IS_ALIVE     0xFF
#define S2_EXIT_GAME    0x12

#define S2_ACK          0x55AA

enum S2State {
  S2_STATE_GENERAL,
  S2_STATE_GAME
};

enum S2Response {
  S2_RESP_ACK,
  S2_RESP_DETAILS
};

