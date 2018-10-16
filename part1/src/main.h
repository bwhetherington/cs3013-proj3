#define RANGE 1
#define ALLDONE 2

typedef struct {
    int iSender;
    int type;
    int value1;
    int value2;
} msg_t;

typedef struct {
    int from;
    int to;
} range_t;

msg_t newMsg(int iSender, int type, int value1, int value2);

/*
 * iTo: mailbox to send to
 * msg: message to be sent
 */
void SendMsg(int iTo, msg_t *msg);

/*
 * iTo: mailbox to receive from
 * msg: message struct to be filled in with received message
 */
void RecvMsg(int iFrom, msg_t *msg);

void addRange(void *input);

void collectSums(int total, int numThreads);

void splitSections(int total, int numSections, range_t *dest);