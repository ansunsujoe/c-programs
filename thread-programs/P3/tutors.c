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
    sem_t tutorSem;
};

struct node {
    struct person* student;
    struct node* next;
};

struct LinkedList {
    struct node* head;
};

/**
 * Insert to the LinkedList of students in waiting room
 */
void insert(struct LinkedList *waitingRoom, struct person *student) {

    // Iterator and new node
    struct node *cur = waitingRoom->head;
    struct node *newNode = malloc(sizeof(struct node));
    newNode->student = student;

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

    insert(waitingRoom, student);
    return 0;
}

void registerStudent(struct person* student, int id, int priority) {
    student->id = id;
    student->priority = priority;
    sem_init(student->tutorSem, 0, 0);
}

// Global variables for output
int numEmptyChairs;
int numStudentsWaiting;
int notifications;
int studentsCurrentlyHelped;
int tutoringSessionsCompleted;

// Semaphores
sem_t waitingRoomMutex;
sem_t queueWaitingStudent;
sem_t tutorWaitingStudents;

struct LinkedList *waitingRoom;

void tutor(int id) {
    // Wait for coordinator signal

    // Get student from queue

    // Signal student

    // Wait for student to be done
}

void student(int id) {
    // Iterate for helps needed
    int i;
    for (i = 0; i < helps; i++) {
        // Program
        usleep(programmingTime);

        // Enter the waiting room
        // While there is no empty chair then go back to programming
        while (numEmptyChairs == 0) {
            usleep(programmingTime);
        }

        // Sit down and increment the number of chairs
        sem_wait(&waitingRoomMutex);
        numEmptyChairs--;
        sem_post(&waitingRoomMutex);

        // Signal the coordinator
        sem_post(queueWaitingStudent);

        // Wait for tutor
        sem_wait(tutorWaitingStudents);

        // Get tutored
        usleep(tutoringTime);

        // Signal tutor that it's over
        // sem_post(registry);

        // Decrement the number of helps

        // Terminate thread if the number of helps is 0

    }
}

void coordinator() {
    // Wait until student comes

    // Enter student in queue

    // Signal tutor
}

int main() {
    waitingRoom = malloc(sizeof(struct LinkedList));
    struct person* registry[numStudents];

    // Initialize (register) students
    int i;
    for (i = 0; i < numStudents; i++) {
        registry[i] =  malloc(sizeof(struct person));
        registerStudent(registry[i], i, helps);
    }

    // Initialize semaphores
    sem_init(&waitingRoomMutex, 0, 0);
    sem_init(&queueWaitingStudent, 0, 0);
    sem_init(&tutorWaitingStudents, 0, numTutors);

    // Initialize number of chairs
    numEmptyChairs = chairs;

    // Clean up student records
    for (i = 0; i < numStudents; i++) {
        free(registry[i]);
    }

    return 0;
}