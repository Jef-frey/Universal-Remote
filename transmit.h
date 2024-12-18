#define IDLE            0
#define HEADER_MARK     1       // 9ms
#define HEADER_SPACE    2       // 4.5ms
#define LOGIC0_MARK     3       // 564us
#define LOGIC0_SPACE    4       // 564us
#define LOGIC1_MARK     5       // 564us
#define LOGIC1_SPACE    6       // 1.692us
#define STOP_BIT        7       // 564us
#define LEAD_OUT        8       // padded to 108ms
#define END             9
#define REPEAT_MARK     10      // 9ms
#define REPEAT_SPACE    11      // 2.25ms
#define REPEAT_STOP     12      // 564us
#define REPEAT_LEAD_OUT 13      // padded to 108ms
#define DEBOUNCE        14      // debounce for 4.5ms

#define TV_POWER    0x20DF10EF
#define TV_MUTE     0x20DF906F
#define TV_VOL_P    0x20DF40BF
#define TV_VOL_N    0x20DFC03F
#define TV_HOME     0x20DF3EC1
#define TV_RETURN   0x20DF14EB
#define TV_UP       0x20DF02FD
#define TV_DOWN     0x20DF827D
#define TV_LEFT     0x20DFE01F
#define TV_RIGHT    0x20DF609F
#define TV_ENTER    0x20DF22DD
#define TV_1        0x20DF8877
#define TV_2        0x20DF48B7
#define TV_3        0x20DFC837
#define TV_NETFLIX  0x20DF6A95

int unsigned nxt_tx_state;
int unsigned long tx_data;
int unsigned long sampler;

void pwm_initialization();

int unsigned long switch_select(int unsigned long data_1, int unsigned long data_2);

