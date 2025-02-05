#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/shm.h>

#define MAX_CUSTOMERS 100
#define MAX_CASHIERS 10
#define K 5  // Liczba klientów na jedną kasę
#define MAX_QUEUE_SIZE 100

// Struktura komunikatu
typedef struct {
    long mtype;
    int customer_id;
    int cashier_id;  // ID kasy, do której przypisano klienta
} message_t;

#endif
