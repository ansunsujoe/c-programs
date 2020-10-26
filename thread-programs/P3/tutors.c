#include <stdio.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h>
#include <stdlib.h>

static int numStudents = 4;
static int numTutors = 1;
static int chairs = 10;
static int helps = 1;

static int programmingTime = 2000;
static int tutoringTime = 10;

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

void registerStudent(struct person* student, int id, int priority) {
    student->id = id;
    student->priority = priority;
}

// Global variables for output
int numEmptyChairs;
int numStudentsWaiting;
int notifications;
int studentsCurrentlyHelped;
int tutoringSessionsCompleted;

// Semaphores
sem_t queuedInWaiting;
sem_t tutorReady;

void tutor(int id) {

}

void student(int id) {
    // Iterate for helps needed
    int i;
    for (i = 0; i < helps; i++) {
        // Program
        usleep(programmingTime);

        // Alert the coordinator so the coordinator will get you in
        sem_wait(&queuedInWaiting);

        // Now you are in front of the line. A tutor will wake you next
        sem_wait(&tutorReady);

        // Simulate tutoring time
        usleep(tutoringTime);
    }
}

void coordinator() {
    
}

int main() {
    struct LinkedList *waitingRoom = malloc(sizeof(struct LinkedList));
    struct person* registry[numStudents];
    waitingRoom->length = 0;

    // Initialize (register) students
    int i;
    for (i = 0; i < numStudents; i++) {
        registry[i] =  malloc(sizeof(struct person));
        registerStudent(registry[i], i + 3001, helps);
    }

    for (i = 0; i < numStudents; i++) {
        insert(waitingRoom, registry[i]);
    }

    dequeue(waitingRoom);
    printWaitingRoom(waitingRoom);

    // Clean up student records
    for (i = 0; i < numStudents; i++) {
        free(registry[i]);
    }

    return 0;
}