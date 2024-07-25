
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>


struct termination{
    long mesg_type;
    bool terminate;
} t;

int main(){
    char c='N';
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
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    msgid = msgget(key, 0644|IPC_CREAT);   
    if (msgid == -1){
        printf("error in creating message queue\n");
        exit(1);
    }
    while(1){
        printf("Do you want the Air Traffic Control System to terminate?(Y for Yes and N for No)");

        scanf(" %c",&c);
        if(c=='N'){
            continue;
        }
        else{
            break;
        }
        
    }
    printf("Message Sent\n");
    struct termination t2;
    t2.mesg_type=101;
    t2.terminate=true;
    if (msgsnd(msgid, &t2, sizeof(t2), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
    }
   
    return 0;
}
