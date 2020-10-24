#include <stdio.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 

struct {
    int id;
    int priority;
} person;

struct {
    struct person;
    struct node* next;
} node;

struct {
    struct node* head;
    int length;
} waitingRoom;

int queueInWaitingRoom() {
    
}

// Global variables for output
int numEmptyChairs;
int numStudentsWaiting;
int notifications;
int studentsCurrentlyHelped;
int tutoringSessionsCompleted;

int main() {

    return 0;
}

void tutor() {

}

void student() {

}

void coordinator() {

}