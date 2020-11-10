#include <stdio.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

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

struct queueNode {
    int studentID;
    struct queueNode* next;
};

struct Queue {
    struct queueNode* first;
    struct queueNode* last;
} queueForCoordinator;

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

/**
 * Insert in coordinator waiting structure
 */
void insertForCoordinator(int id) {
    struct queueNode *newNode = malloc(sizeof(struct queueNode));
    newNode->studentID = id;

    // If head is null
    if (queueForCoordinator.first == NULL) {
        struct queueNode *oldHead = queueForCoordinator.first;
        queueForCoordinator.first = newNode;
        queueForCoordinator.last = newNode;
        newNode->next = oldHead;
    }
    
    // Set as the tail of the queue
    else {
        queueForCoordinator.last->next = newNode;
        queueForCoordinator.last = newNode;
    }
    
}

/**
 * Delete from coordinator waiting structure
 */
int takeStudentFromWaiting() {
    int firstInLine = queueForCoordinator.first->studentID;
    struct queueNode *newFirst = queueForCoordinator.first->next;
    free(queueForCoordinator.first);
    queueForCoordinator.first = newFirst;
    if (queueForCoordinator.first == NULL) {
        queueForCoordinator.last = NULL;
    }
    return firstInLine;
}

/**
 * Enter a student into the registry with the correct information about id and priority
 */
void registerStudent(struct person* student, int id, int priority) {
    student->id = id;
    student->priority = priority;
    sem_init(&student->tutorSem, 0, 0);
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
sem_t registryMutex;
sem_t coordinatorSignal;
sem_t queueMutex;

struct LinkedList *waitingRoom;
struct person** registry;

static void* tutor(void* id) {

    int tutorID = (int) id;
    int studentID;
    int s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    printf("Tutor %d ready\n", tutorID);

    while (1) {
        // Wait for coordinator signal
        sem_wait(&coordinatorSignal);

        // Get student from queue
        printf("Tutor %d has been called by coordinator\n", tutorID);
        sem_wait(&queueMutex);
        studentID = dequeue(waitingRoom)->id;
        printf("Tutor %d is about to call student %d\n", tutorID, studentID);
        sem_post(&queueMutex);

        // Signal student
        sem_post(&registry[studentID]->tutorSem);

        // Wait for student to be done
        // This is where the tutoring is done
        printf("Tutor %d is in session with student %d\n", tutorID, studentID);
        usleep(1);
        sem_wait(&registry[studentID]->tutorSem);
    }
    return NULL;
    
}

static void* student(void* id) {
    // Iterate for helps needed
    int studentID = (int) id;
    int i;

    for (i = 0; i < helps; i++) {
        // Program
        usleep(programmingTime);
        printf("Student %d ready for tutoring\n", studentID);
        // Enter the waiting room
        // While there is no empty chair then go back to programming
        while (numEmptyChairs == 0) {
            usleep(programmingTime);
        }

        // Sit down and increment the number of chairs
        sem_wait(&waitingRoomMutex);
        numEmptyChairs--;
        insertForCoordinator(studentID);
        printf("Student %d is waiting now, numChairs = %d\n", studentID, numEmptyChairs);
        sem_post(&waitingRoomMutex);

        // Signal the coordinator
        sem_post(&queueWaitingStudent);
        printf("Student %d has been queued\n", studentID);

        // Wait for tutor
        sem_wait(&registry[studentID]->tutorSem);

        // Relinquish a waiting room chair
        printf("Student %d called for tutoring\n", studentID);
        sem_wait(&waitingRoomMutex);
        numEmptyChairs++;
        sem_post(&waitingRoomMutex);

        // Get tutored
        usleep(tutoringTime);

        // Signal tutor that it's over
        sem_post(&registry[studentID]->tutorSem);
        printf("Student %d has signaled that tutoring is over\n", studentID);

        // Decrement the number of helps
        registry[studentID]->priority--;

    }
    printf("Student %d is done\n", studentID);
    return NULL;
}

static void* coordinator() {
    int studentID;
    int s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    while (1) {
        printf("Coordinator in the loop\n");
        // Wait until student comes
        sem_wait(&queueWaitingStudent);

        // Take student out of the "waiting for coordinator" queue
        sem_wait(&waitingRoomMutex);
        studentID = takeStudentFromWaiting();
        sem_post(&waitingRoomMutex);

        // Put student in queue for tutor
        sem_wait(&queueMutex);
        insert(waitingRoom, registry[studentID]);
        sem_post(&queueMutex);

        // Signal tutor
        sem_post(&coordinatorSignal);
    }
    return NULL;
}

int main() {
    printf("Running\n");
    waitingRoom = malloc(sizeof(struct LinkedList));
    // struct person* registry[numStudents];
    registry = malloc((numStudents) * sizeof(struct person*));

    // Initialize (register) students
    long i;
    for (i = 0; i < numStudents; i++) {
        registry[i] =  malloc(sizeof(struct person));
        registerStudent(registry[i], i, helps);
    }

    // Initialize semaphores
    sem_init(&waitingRoomMutex, 0, 1);
    sem_init(&registryMutex, 0, 1);
    sem_init(&queueMutex, 0, 1);
    sem_init(&queueWaitingStudent, 0, 0);
    sem_init(&coordinatorSignal, 0, 0);

    // Initialize number of chairs
    numEmptyChairs = chairs;

    // Initialize thread storage
    pthread_t *studentsTha;
    pthread_t *tutorsTha;
    pthread_t *coordinatorTha;

    // Allocate memory for threads
    studentsTha = malloc(sizeof(pthread_t) * numStudents);
    tutorsTha = malloc(sizeof(pthread_t) * numTutors);
    coordinatorTha = malloc(sizeof(pthread_t));

    // Initialize coordinator thread
    pthread_create(&coordinatorTha[0], NULL, coordinator, NULL);

    // Initialize tutor threads
    for (i = 0; i < numTutors; i++) {
        pthread_create(&tutorsTha[i], NULL, tutor, (void *) i);
    }

    // Initialize student threads
    for (i = 0; i < numStudents; i++) {
        pthread_create(&studentsTha[i], NULL, student, (void *) i);
    }

    // Wait for student threads to end
    for (i = 0; i < numStudents; i++) {
        pthread_join(studentsTha[i], NULL);
    }

    // Cancel tutor threads
    for (i = 0; i < numTutors; i++) {
        assert(pthread_cancel(tutorsTha[i]) == 0);
    }

    // Cancel coordinator threads
    assert(pthread_cancel(coordinatorTha[0]) == 0);

    // Clean up student records
    for (i = 0; i < numStudents; i++) {
        free(registry[i]);
    }

    // Clean up data structures
    free(registry);
    free(waitingRoom);

    return 0;
}