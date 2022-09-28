#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#define FALSE 0
#define TRUE 1

int alarmEnabled = FALSE;
int transmissionCount = 0;

void transmit(int signal) {
    transmissionCount++;
}

int sendTransmission() {
    while (transmissionCount <= 3) {
        transmit(signal);
    }
}