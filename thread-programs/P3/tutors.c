#include <stdio.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h>
#include <stdlib.h>

static int numStudents = 1;
static int numTutors = 1;
static int chairs = 10;
static int helps = 1;

struct person {
    int id;
    int priority;
};

struct node {
    struct person* student;
    struct node* next;
};

struct LinkedList {
    struct node* head;
    int length;
};

/**
 * Insert to the LinkedList of students in waiting room
 */
void insert(struct LinkedList *waitingRoom, struct person *student) {

    // Iterator and new node
    struct node *cur = waitingRoom->head;
    struct node *newNode = malloc(sizeof(struct node));
    newNode->student = student;

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

struct person* dequeue(struct LinkedList *waitingRoom) {
    // Delete the head and free the memory given to the head node
    struct node *deleteNode = waitingRoom->head;
    waitingRoom->head = deleteNode->next;
    
    // Return the student structure for the tutor to see this
    struct person *nextInLine = deleteNode->student;
    free(deleteNode);
    return nextInLine;
}

/**
 * Print the waiting room for debugging purposes
 */
void printWaitingRoom(struct LinkedList *waitingRoom) {
    printf("Waiting Room: \n");
    struct node *cur = waitingRoom->head;
    while (cur != NULL) {
        printf("Student ID: %d, Priority %d\n", cur->student->id, cur->student->priority);
        cur = cur->next;
    }
}

/**
 * Queue in waiting room - Insert unless waiting room is full
 */
int queueInWaitingRoom(struct LinkedList *waitingRoom, struct person *student) {

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
    struct LinkedList *waitingRoom = malloc(sizeof(struct LinkedList));
    waitingRoom->length = 0;
    struct person *student1 = malloc(sizeof(struct person));
    struct person *student2 = malloc(sizeof(struct person));
    struct person *student3 = malloc(sizeof(struct person));
    struct person *student4 = malloc(sizeof(struct person));
    student1->id = 521;
    student2->id = 632;
    student3->id = 400;
    student4->id = 753;
    student1->priority = 3;
    student2->priority = 4;
    student3->priority = 1;
    student4->priority = 9;
    insert(waitingRoom, student1);
    insert(waitingRoom, student2);
    insert(waitingRoom, student3);
    insert(waitingRoom, student4);
    dequeue(waitingRoom);
    printWaitingRoom(waitingRoom);
    return 0;
}

void tutor() {

}

void student() {

}

void coordinator() {

}