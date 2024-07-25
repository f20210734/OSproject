#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/msg.h>
// #include "common.h"
#define ATC "AirTrafficController.txt"
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
struct termination{
    long mesg_type;
    bool terminate;
} t;
struct mesg_buffer {
    long mesg_type;
    struct planeDetails planeDetailsObject;
} message;
struct confirmation{
    long mesg_type;
    bool confirm;
} c;

int getNumberOfAirports() {
    int a=0;
    printf("Enter the number of airports to be handled/managed:");
    fflush(stdout);
    scanf(" %d", &a); // Notice the space before %d to clear whitespace
    while (getchar() != '\n'); // Clear input buffer
    return a;
}
bool flag=false;
bool flag1=true;
bool check(int numberOfAirports,int msgid){
    struct termination t2;
        
        if(msgrcv(msgid,&t2,sizeof(t2),101,IPC_NOWAIT) == -1){
            return true;
        }  
        
            t2.mesg_type=101;
           
           
            if(msgsnd(msgid,&t2,sizeof(t2),0)==-1){
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
        struct termination t3;
        t3.mesg_type=5001;
        t3.terminate=true;
        if(flag1==true)
        {

        if(msgsnd(msgid,&t3,sizeof(t3),0)==-1){
            perror("msgsnd3");
            exit(EXIT_FAILURE);
            }
        }
        
        flag1=false;
        return false;
    }
void closeAirports(int msgid,int numberOfAirports)
{
               for(int i=0;i<numberOfAirports;i++){
                struct termination t3;
                t3.terminate=true;
                t3.mesg_type=((i+1)*200)+10;
                if(msgsnd(msgid,&t3,sizeof(t),0) == -1){
                    
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
            }
            
}
int main() {
    int numberOfAirports = 0;
    numberOfAirports = getNumberOfAirports();
    fflush(stdout); 
    FILE *atc = fopen(ATC, "w");
    if (atc == NULL) {
        perror("Error opening earnings file");
        exit(EXIT_FAILURE);
    }
    key_t key;
    int msgid;

    key = ftok("airTrafficController.c", 'A');
    if (key == -1){
        printf("error in creating unique key\n");
        fflush(stdout);
        exit(1);
    }
    msgid = msgget(key, 0666|IPC_CREAT);   
    if (msgid == -1){
        printf("error in creating message queue\n");
        fflush(stdout);
        exit(1);
    }
   
    int planesInFlight=0;
    while (check(numberOfAirports,msgid) || planesInFlight!=0 ) {
      
    fflush(stdout);
        if (msgrcv(msgid, &message, sizeof(message), 22, IPC_NOWAIT) == -1) {
            continue;
        }
               

        if (message.planeDetailsObject.reqFromPlane == true) {
            // If message was from a plane
        
            
            if (message.planeDetailsObject.arrived == false) {
                planesInFlight++;
                fprintf(atc, "Plane %d has departed from Airport %d and will land at Airport %d. \n", message.planeDetailsObject.planeId, message.planeDetailsObject.departure,message.planeDetailsObject.arrival);
                fflush(atc);
                message.mesg_type = message.planeDetailsObject.departure + 10;
            } else {  
                message.mesg_type = message.planeDetailsObject.arrival + 10;    
            }
        } else {
           
            // If message was not from a plane
            message.mesg_type = message.planeDetailsObject.planeId;
            if(message.planeDetailsObject.takeoffRequest==false)
                planesInFlight--;

        }
        // Send message
            
        if (msgsnd(msgid, &message, sizeof(message), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
    }
    closeAirports(msgid,numberOfAirports);
    sleep(3);
   
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    fclose(atc);
    return 0;
}
