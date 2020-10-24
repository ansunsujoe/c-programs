#include <stdio.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h>

static int numStudents = 1;
static int numTutors = 1;
static int chairs = 10;
static int helps = 1;

typedef struct {
    int id;
    int priority;
} person;

typedef struct {
    person* student;
    node* next;
} node;

typedef struct {
    node* head;
    int length;
} LinkedList;

LinkedList *waitingRoom = {NULL, 0};

/**
 * Insert to the LinkedList of students in waiting room
 */
void insert(LinkedList *waitingRoom, person *student) {

    // Iterator and new node
    node *cur = waitingRoom->head;
    node *newNode = {student, NULL};

    // Increase the length of the list
    waitingRoom->length++;

    // If list is empty, then insert at head
    if (cur == NULL) {
        waitingRoom->head = newNode;
        return;
    }
    // If the student has a greater priority than the head
    if (student->priority > cur->student->priority) {
        newNode->next = cur;
        waitingRoom->head = newNode;
        return;
    }

    // Iterate to find where to queue the student based on priority
    while (cur->next != NULL) {
        if (student->priority > cur->next->student->priority) {
            newNode->next = cur->next;
            cur->next = newNode;
            return;
        }
        cur = cur->next;
    }

    // Lowest priority - place at end of list
    cur->next = newNode;
}

/**
 * Queue in waiting room - Insert unless waiting room is full
 */
int queueInWaitingRoom(person *student) {

    // The student will have to go back to programming and come back
    // another time
    if (waitingRoom->length >= chairs) {
        return -1;
    }
    // Priority queue insert
    else {
        insert(waitingRoom, student);
    }
    return 0;
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