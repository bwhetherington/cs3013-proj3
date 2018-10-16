#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include "main.h"

msg_t *MAILBOXES;
sem_t *SENDS;
sem_t *RECVS;

int LAST_MAILBOX;
int SUM_TO;
int NUM_THREADS;

msg_t newMsg(int iSender, int type, int value1, int value2) {
    msg_t msg;
    msg.iSender = iSender;
    msg.type = type;
    msg.value1 = value1;
    msg.value2 = value2;
    return msg;
}

/*
 * iTo: mailbox to send to
 * msg: message to be sent
 */
void SendMsg(int iTo, msg_t *msg) {
    // Wait for a message to be sent
    sem_t *send = SENDS + iTo;
    sem_wait(send);

    // Copy data of message to mailbox with the specified ID
    MAILBOXES[iTo] = *msg;

    // Allow another to be received from the mailbox
    sem_t *recv = RECVS + iTo;
    sem_post(recv);
}

/*
 * iTo: mailbox to receive from
 * msg: message struct to be filled in with received message
 */
void RecvMsg(int iFrom, msg_t *msg) {
    // Wait for a message to be received
    sem_t *recv = RECVS + iFrom;
    sem_wait(recv);

    // Copy message from mailbox to specified address
    *msg = MAILBOXES[iFrom];

    // Allow another to be sent to the mailbox
    sem_t *send = SENDS + iFrom;
    sem_post(send);
}

void addRange(void *input) {
    // Receive message
    int mailbox = (int) input;

    msg_t recv;
    RecvMsg(mailbox, &recv);

    int sum = 0;
    for (int i = recv.value1; i <= recv.value2; i++) {
        sum += i;
    }

    msg_t final = newMsg(mailbox, ALLDONE, sum, 0);
    SendMsg(LAST_MAILBOX, &final);
}

void collectSums(int total, int numThreads) {
    msg_t recv;
    int sum = 0;
    for (int i = 0; i < numThreads; i++) {
        // Waiting for message for sum
        RecvMsg(LAST_MAILBOX, &recv);
        sum += recv.value1;
    }
    printf("The total for 1 to %d using %d threads is %d.\n", total, numThreads, sum);
}

static void cleanupSems(int len, sem_t *sems) {
    for (int i = 0; i < len; i++) {
        sem_destroy(sems + i);
    }
}

// Works unless you give it 
void splitSections(int total, int numSections, range_t *dest) {
    // Calculate right desired size;

    int desiredSize = total / numSections;
    int j = 0;
    int count = 0;
    int from = 1;
    int to;
    for (int i = 1; i <= total; i++) {
        to = i;
        count++;
        if (count == desiredSize) {
            count = 0;
            range_t section;
            section.from = from;
            section.to = to;
            dest[j] = section;
            j++;
            from = to + 1;
        }
    }
    dest[numSections - 1].to = total;
}

int main(int argc, char **argv) {
    // Check argument count
    if (argc != 3) {
        fprintf(stderr, "Invalid argument count. Expected 2, found %d.\n", argc - 1);
        exit(-1);
    }

    int numThreads = atoi(argv[1]);
    int countTo = atoi(argv[2]);

    if (numThreads < 1) {
        fprintf(stderr, "At least 1 thread is required.\n");
        exit(-1);
    }

    if (numThreads > countTo) {
        fprintf(stderr, "The maximum number of threads allowed is %d.\n", countTo);
        exit(-1);
    }

    SUM_TO = countTo;
    NUM_THREADS = numThreads;

    int numMailboxes = numThreads + 1;

    MAILBOXES = (msg_t *) malloc(sizeof (msg_t) * numMailboxes);
    LAST_MAILBOX = numThreads;

    // Create sendersm receivers
    SENDS = (sem_t *) malloc(sizeof (sem_t) * numMailboxes);
    RECVS = (sem_t *) malloc(sizeof (sem_t) * numMailboxes);
    for (int i = 0; i < numMailboxes; i++) {
        sem_init(SENDS + i, 0, 1);
        sem_init(RECVS + i, 0, 0);
    }

    pthread_t ids[numMailboxes];

    range_t ranges[numThreads];
    splitSections(countTo, numThreads, ranges);

    // Send messages
    for (int i = 0; i < numThreads; i++) {
        // Calculate subrange
        range_t range = ranges[i];
        msg_t msg = newMsg(LAST_MAILBOX, RANGE, range.from, range.to);
        SendMsg(i, &msg);
    }

    for (int i = 0; i < numThreads; i++) {
        // Create threads here
        // All child threads need to wait until they have a message, then can go
        // printf("%d\n", i);
        pthread_create(ids + i, NULL, addRange, i);
    }

    //

    // Receive all messages in the `sum` mailbox
    collectSums(countTo, numThreads);

    cleanupSems(numMailboxes, SENDS);
    cleanupSems(numMailboxes, RECVS);

    // printf("Counting to %d using %d threads.\n", countTo, numThreads);
}