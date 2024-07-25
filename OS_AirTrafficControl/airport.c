#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/msg.h>
// #include "common.h"
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
struct confirmation{
    long mesg_type;
    bool confirm;
} c;
struct termination{
    long mesg_type;
    bool terminate;
} t;
#define MAX_RUNWAYS 10
#define MAX_AIRPORTS 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


typedef struct {
    struct planeDetails planeDetailsObject;
    int *weights;
    bool *runwayAvailable;
    int numberOfRunways;
    int msgid;
} ThreadArgs;

void *threadFunctionDeparture(void *args) {
   
    ThreadArgs threadArgs = *((ThreadArgs *)args);
    
    bool found = false;
    int ind = -1;
    
    int maxCap=threadArgs.runwayAvailable[1];
    for (int i = 1; i < threadArgs.numberOfRunways; i++) {
        if(maxCap<threadArgs.weights[i])
        {
            maxCap=threadArgs.weights[i];
        }
    }
    while (!found) {
        pthread_mutex_lock(&mutex);
        if(maxCap>threadArgs.planeDetailsObject.totalPlaneWeight)
        {

        for (int i = 1; i < threadArgs.numberOfRunways; i++) {
            if (threadArgs.runwayAvailable[i] && threadArgs.weights[i] >= threadArgs.planeDetailsObject.totalPlaneWeight) {
                if (ind == -1 || threadArgs.weights[ind] >= threadArgs.weights[i]) {
                    ind = i;
                }
            }
        }
        }
        else
        {
            if (threadArgs.runwayAvailable[0] && threadArgs.weights[0] >= threadArgs.planeDetailsObject.totalPlaneWeight) {
                if (ind == -1 || threadArgs.weights[ind] >= threadArgs.weights[0]) {
                    ind = 0;
                }
            }
        }
        if (ind != -1) {
            threadArgs.runwayAvailable[ind] = false;
            found = true;
        }
        pthread_mutex_unlock(&mutex);
    }

   
    fflush(stdout);
    sleep(3); // Boarding/loading process
    threadArgs.runwayAvailable[ind] = true;
    sleep(2); // Plane taking off
    printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n",
           threadArgs.planeDetailsObject.planeId, ind, threadArgs.planeDetailsObject.departure);
    
    fflush(stdout);
    // Send message to air traffic controller
    message.planeDetailsObject = threadArgs.planeDetailsObject;
    message.mesg_type = 22;
    
    message.planeDetailsObject.reqFromPlane = false;
    
           
            
    message.planeDetailsObject.takeoffRequest=true;
    if (msgsnd(threadArgs.msgid, &message, sizeof(message), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
   
    pthread_exit(NULL);
}

void *threadFunctionArrival(void *args) {
    ThreadArgs threadArgs = *((ThreadArgs *)args);
    bool found = false;
    int ind = -1;
    int maxCap=threadArgs.runwayAvailable[1];
    for (int i = 1; i < threadArgs.numberOfRunways; i++) {
        if(maxCap<threadArgs.weights[i])
        {
            maxCap=threadArgs.weights[i];
        }
    }
    while (!found) {
        pthread_mutex_lock(&mutex);
        if(maxCap>threadArgs.planeDetailsObject.totalPlaneWeight)
        {

        for (int i = 1; i < threadArgs.numberOfRunways; i++) {
            if (threadArgs.runwayAvailable[i] && threadArgs.weights[i] >= threadArgs.planeDetailsObject.totalPlaneWeight) {
                if (ind == -1 || threadArgs.weights[ind] >= threadArgs.weights[i]) {
                    ind = i;
                }
            }
        }
        }
        else
        {
            if (threadArgs.runwayAvailable[0] && threadArgs.weights[0] >= threadArgs.planeDetailsObject.totalPlaneWeight) {
                if (ind == -1 || threadArgs.weights[ind] >= threadArgs.weights[0]) {
                    ind = 0;
                }
            }
        }
        if (ind != -1) {
            threadArgs.runwayAvailable[ind] = false;
            found = true;
        }
        pthread_mutex_unlock(&mutex);
    }
     
    sleep(3); // UnBoarding/Unloading process
    
    pthread_mutex_lock(&mutex);
    threadArgs.runwayAvailable[ind] = true;
    sleep(2); //Plane landing
    printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n",
           threadArgs.planeDetailsObject.planeId, ind, threadArgs.planeDetailsObject.arrival);
    fflush(stdout);
    pthread_mutex_unlock(&mutex);
    // Send message to air traffic controller
    message.planeDetailsObject = threadArgs.planeDetailsObject;
    message.mesg_type = 22;
    message.planeDetailsObject.takeoffRequest=false;
    message.planeDetailsObject.reqFromPlane = false;

    if (msgsnd(threadArgs.msgid, &message, sizeof(message), 0) == -1) {
        perror("msgsnd");
        
        exit(EXIT_FAILURE);
    }
    
    pthread_exit(NULL);
}

void getAirportDetails(int *airNumber, int *noOfRunways) {
    printf("Enter Airport Number:\n");
    fflush(stdout);
    scanf("%d", airNumber);
    fflush(stdout);
    printf("Enter number of Runways:\n");
    fflush(stdout);
    scanf("%d", noOfRunways);
    (stdout);
}

void getLoadCapacities(int loadCapacities[], int numberOfRunways) {
    printf("Enter loadCapacity of Runways (give as a space separated list in a single line):\n");
    fflush(stdout);
    for (int i = 1; i < numberOfRunways; i++) {
        scanf("%d", &loadCapacities[i]);
        fflush(stdout);
    }
    loadCapacities[0]=15000;
     
}
bool check(int msgid,int airportNumber){
    struct termination t2;
    if(msgrcv(msgid,&t2,sizeof(t2),(airportNumber*200)+10,IPC_NOWAIT) == -1){
        return false;
    }
    t2.mesg_type=(airportNumber*200)+10;
    
    return true;
}

bool check2(int msgid){
    if(msgrcv(msgid,&t,sizeof(t),5001,IPC_NOWAIT) == -1){
         
        return false;
    }
    if(msgsnd(msgid,&t,sizeof(t),0)==-1){
            perror("msgsnd");
            exit(EXIT_FAILURE);
    }
    return true;
}

int main() {
    key_t key;
    int msgid;

    // ftok to generate unique key
    key = ftok("airTrafficController.c", 'A');

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
    if(check2(msgid)==true)
    {
        printf("cleanup initiated at airport\n");
        fflush(stdout);
        return 0;
    }
    int airportNumber;
    int numberOfRunways;
    bool terminated=false;
    getAirportDetails(&airportNumber, &numberOfRunways);
    numberOfRunways++; // Considering one runway is reserved for backup

    int loadCapacities[MAX_RUNWAYS];
    bool runwayAvailable[MAX_RUNWAYS];
    getLoadCapacities(loadCapacities, numberOfRunways);
    
   
    // Initialize runway availability
    for (int i = 0; i < numberOfRunways; i++) {
        runwayAvailable[i] = true;
    }

    

    // Main loop to handle messages
    while (true) {
        bool flag=check(msgid,airportNumber);
        
        if(flag){
            break;
        }
        if(msgrcv(msgid, &message, sizeof(message), airportNumber+10, IPC_NOWAIT) == -1)
        {
            continue;
        }
         
        // Create thread arguments
        ThreadArgs threadArgs;
        threadArgs.planeDetailsObject = message.planeDetailsObject;
        threadArgs.weights = loadCapacities;
        threadArgs.runwayAvailable = runwayAvailable;
        threadArgs.numberOfRunways = numberOfRunways;
        threadArgs.msgid = msgid;
        
       
        // Create thread based on departure or arrival
        pthread_t thread;
        if (threadArgs.planeDetailsObject.departure == airportNumber) {
            int threadResult = pthread_create(&thread, NULL, threadFunctionDeparture, (void *)&threadArgs);
            if (threadResult != 0) {
                fprintf(stderr, "Error creating thread: %d\n", threadResult);
                fflush(stdout);
                exit(EXIT_FAILURE);
            }
        } else {
            int threadResult = pthread_create(&thread, NULL, threadFunctionArrival, (void *)&threadArgs);
            if (threadResult != 0) {
                fprintf(stderr, "Error creating thread: %d\n", threadResult);
                fflush(stdout);
                exit(EXIT_FAILURE);
            }
        }
    }
    
    
    return 0;
}
