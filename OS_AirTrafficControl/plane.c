#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>
// #include "common.h"

#define READ_END 0
#define WRITE_END 1
struct planeDetails {
    int totalPlaneWeight;
    int arrival;
    int departure;
    int planeId;
    int typeOfPlane;
    int numOfItems;
    bool arrived;
    bool takeoffRequest;
    bool reqFromPlane;
};
struct mesg_buffer {
    long mesg_type;
    struct planeDetails planeDetailsObject;
} message;
struct termination{
    long mesg_type;
    bool terminate;
} t;
void getPlaneDetails(int *planeID, int *planeType) {
    printf("Enter Plane ID:\n");
    fflush(stdout);
    scanf("%d", planeID);
    printf("Enter Plane Type:\n");
    fflush(stdout);
    scanf("%d", planeType);
}

int getPassengerLuggageWeight() {
    int weight = 0;
    printf("Enter Weight of Your Luggage:\n");
    fflush(stdout);
    scanf("%d", &weight);
    return weight;
}

int getPassengerBodyWeight() {
    int weight = 0;
    printf("Enter Your Body Weight:\n");
    fflush(stdout);
    scanf("%d", &weight);
    return weight;
}

void getPassengerInfo(struct planeDetails *planeDetailsObject) {
    int numOfSeats = 0;
    int totalPlaneWeight = 525;
    printf("Enter Number of Occupied Seats:\n");
    fflush(stdout);
    scanf("%d", &numOfSeats);
    planeDetailsObject->numOfItems = numOfSeats;

    for (int i = 0; i < numOfSeats; i++) {
        int fd[2];
        if (pipe(fd) == -1) {
            fprintf(stderr, "PIPE failed");
            return;
        }
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            return;
        } else if (pid == 0) { // Child process
            close(fd[READ_END]);
            int weightLuggage = getPassengerLuggageWeight();
            int bodyWeight = getPassengerBodyWeight();
            write(fd[WRITE_END], &weightLuggage, sizeof(int));
            write(fd[WRITE_END], &bodyWeight, sizeof(int));
            close(fd[WRITE_END]);
            exit(EXIT_SUCCESS);
        }
        wait(NULL);
        close(fd[WRITE_END]);
        int read_msg;
        while (read(fd[READ_END], &read_msg, sizeof(int)) > 0) {
            totalPlaneWeight += read_msg;
        }
        close(fd[READ_END]);
        wait(NULL);
    }

    planeDetailsObject->totalPlaneWeight = totalPlaneWeight;
}

void getCargoInfo(struct planeDetails *planeDetailsObject) {
    int numOfCargoItems=0;
    int averageWeight=0;
    printf("Enter Number of Cargo Items:\n");
    fflush(stdout);
    scanf("%d", &numOfCargoItems);
    planeDetailsObject->numOfItems = numOfCargoItems;

    printf("Enter the Average Weight of Cargo Items\n");
    fflush(stdout);
    scanf("%d", &averageWeight);
    planeDetailsObject->totalPlaneWeight = 150 + (numOfCargoItems * averageWeight);
}

void printSuccessLanding(struct planeDetails *planeDetailsObject){
    printf("Plane %d has successfully traveled from Airport %d to Airport %d!\n", planeDetailsObject->planeId, planeDetailsObject->departure, planeDetailsObject->arrival);
}


void getArrivalAndDeparture(struct planeDetails *planeDetailsObject) {
    printf("Enter Airport Number for Departure:\n");
    fflush(stdout);
    scanf("%d", &planeDetailsObject->departure);
    printf("Enter Airport Number for Arrival:\n");
    fflush(stdout);
    scanf("%d", &planeDetailsObject->arrival);
}
bool check(int msgid){
    if(msgrcv(msgid,&t,sizeof(t),5001,IPC_NOWAIT) == -1){
        printf("No message recived\n");
        return false;
    }
    printf("Message Present\n");
    if(msgsnd(msgid,&t,sizeof(t),0)==-1){
            perror("msgsnd");
            exit(EXIT_FAILURE);
    }
    return true;
}
int main() {
    struct planeDetails planeDetailsObject;
    
    key_t key;
    int msgid;
    key = ftok("airTrafficController.c", 'A'); // Change 'A' to any unique character

    // Check if ftok succeeded
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // msgget creates a message queue and returns identifier
    msgid = msgget(key, 0666 | IPC_CREAT);

    // Check if msgget succeeded
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    


    getPlaneDetails(&planeDetailsObject.planeId, &planeDetailsObject.typeOfPlane);
    
    if (planeDetailsObject.typeOfPlane == 1) {
        getPassengerInfo(&planeDetailsObject);
    } else {
        getCargoInfo(&planeDetailsObject);
    }

    getArrivalAndDeparture(&planeDetailsObject);
   
    if(check(msgid)){
        printf("Departure request declined by ATC!");
        fflush(stdout);
        return 0;
    }
    message.mesg_type = 22;
    message.planeDetailsObject = planeDetailsObject;
    message.planeDetailsObject.arrived = false;
    message.planeDetailsObject.reqFromPlane = true;

    if (msgsnd(msgid, &message, sizeof(message), 0) == -1) {
        printf("Request Declined by ATC!");
        exit(0);
    }
    
    msgrcv(msgid, &message, sizeof(message), planeDetailsObject.planeId, 0);
    message.planeDetailsObject.arrived = true;
    message.planeDetailsObject.reqFromPlane = true;
    message.mesg_type=22;
   
    sleep(30);
    if (msgsnd(msgid, &message, sizeof(message), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
   
    // Landing request wait
    
    msgrcv(msgid, &message, sizeof(message), planeDetailsObject.planeId, 0);
    
    bool k=check(msgid);
    printSuccessLanding(&planeDetailsObject);
    
    return 0;
}
