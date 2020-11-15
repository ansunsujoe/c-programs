#include <stdio.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>

int numStudents = 4;
int numTutors = 1;
int chairs = 10;
int helps = 1;

int programmingTime = 2000;
int tutoringTime = 10;

struct person {
    int id;
    int priority;
    sem_t tutorSem;
    sem_t studentSem;
    int tutorHelping;
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
};

/**
 * This is a debugging method
 * To catch segmentation faults (SIGSEGV)
 */
void hsignal(int i) {
  printf("Segmentation fault here in thread %d!!!\n", pthread_self());
  pthread_exit(0);
}

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
        // printf("Started empty!!\n");
        waitingRoom->head = newNode;
        newNode->next = NULL;
        return;
    }

    // If the student has a greater priority than the head
    if (student->priority > cur->student->priority) {
        waitingRoom->head = newNode;
        newNode->next = cur;
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
    newNode->next = NULL;
    return;
}

struct person* dequeue(struct LinkedList *waitingRoom) {
    if (waitingRoom->head != NULL) {
        // Delete the head and free the memory given to the head node
        struct node *deleteNode = waitingRoom->head;
        waitingRoom->head = deleteNode->next;
        
        // Return the student structure for the tutor to see this
        struct person *nextInLine = deleteNode->student;
        free(deleteNode);
        return nextInLine;
    }
    return NULL;
}

/**
 * Print the waiting room for debugging purposes
 */
void printWaitingRoom(struct LinkedList *waitingRoom) {
    printf("\nWaiting Room: \n");

    // If the waiting room has no one
    if (waitingRoom->head == NULL) {
        return;
    }
    
    // Start the iteration
    struct node *cur = waitingRoom->head;
    while (cur != NULL) {
        if (cur->next == NULL) {
            printf("Student ID: %d, Priority %d - Next is NULL\n", cur->student->id, cur->student->priority);
        }
        else {
            printf("Student ID: %d, Priority %d - Next is Student ID: %d, Priority %d\n", cur->student->id, cur->student->priority, cur->next->student->id, cur->next->student->priority);
        }
        
        cur = cur->next;
    }
    printf("\n");
    return;
}

void printWaitCoordinatorArea(struct Queue *queueForCoordinator) {
    printf("\nCoordinator Waiting Area: \n");

    // Stop if head is null
    if (queueForCoordinator->first == NULL) {
        return;
    }

    // Iterate using the cur variable
    struct queueNode *cur = queueForCoordinator->first;
    while (cur != NULL) {
        printf("Student ID: %d\n", cur->studentID);
        cur = cur->next;
    }

    printf("\n");
    return;
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
void insertForCoordinator(int id, struct Queue *queueForCoordinator) {
    struct queueNode *newNode = malloc(sizeof(struct queueNode));
    newNode->studentID = id;

    // If head is null
    if (queueForCoordinator->first == NULL) {
        queueForCoordinator->first = newNode;
        queueForCoordinator->last = newNode;
        newNode->next = NULL;
    }
    
    // Set as the tail of the queue
    else {
        queueForCoordinator->last->next = newNode;
        queueForCoordinator->last = newNode;
        newNode->next = NULL;
    }
    
}

/**
 * Delete from coordinator waiting structure
 */
int takeStudentFromWaiting(struct Queue *queueForCoordinator) {
    struct queueNode *firstNode = queueForCoordinator->first;
    int firstInLine = queueForCoordinator->first->studentID;
    struct queueNode *newFirst = queueForCoordinator->first->next;

    // Free queue node if it is not null
    // if (firstNode != NULL) {
    //    free(firstNode);
    // }
    
    queueForCoordinator->first = newFirst;
    if (queueForCoordinator->first == NULL) {
        queueForCoordinator->last = NULL;
    }
    return firstInLine;
}

/**
 * Enter a student into the registry with the correct information about id and priority
 */
void registerStudent(struct person* student, int id, int priority) {
    student->id = id;
    student->priority = priority;
    // sem_init(&student->tutorSem, 0, 0);
    student->tutorHelping = -1;

}

// Global variables for output
int numEmptyChairs = 0;
int numStudentsWaiting = 0;
int notifications = 0;
int studentsCurrentlyHelped = 0;
int tutoringSessionsCompleted = 0;

// Semaphores
sem_t waitingRoomMutex;
sem_t queueWaitingStudent;
sem_t registryMutex;
sem_t coordinatorSignal;
sem_t queueMutex;
sem_t tutoringStatsMutex;
sem_t numNotificationsMutex;

struct LinkedList *waitingRoom;
struct Queue *queueForCoordinator;
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
        struct person* studentToTutor = dequeue(waitingRoom);

        // If there was nothing in the queue to receive, then the studentID will be null
        if (studentToTutor == NULL) {
            printf("Uhoh we got bad news, student is null.\n");
            sem_post(&queueMutex);
            continue;
        }
        
        studentID = studentToTutor->id;
        printWaitingRoom(waitingRoom);
        printf("Tutor %d is about to call student %d\n", tutorID, studentID);
        sem_post(&queueMutex);

        // Increment the students currently helped
        sem_wait(&tutoringStatsMutex);
        studentsCurrentlyHelped++;
        sem_post(&tutoringStatsMutex);

        // Signal student
        registry[studentID]->tutorHelping = tutorID;
        sem_post(&registry[studentID]->studentSem);

        // Wait for student to be done
        // This is where the tutoring is done
        printf("Tutor %d is in session with student %d\n", tutorID, studentID);
        usleep(1);
        printf("Tutor is waiting on Student %d to signal tutoring over\n", studentID);
        sem_wait(&registry[studentID]->tutorSem);
        printf("TUTOR %d is signaled by student %d and we're done\n", tutorID, studentID);

        // Update the number of tutoring sessions completed
        sem_wait(&tutoringStatsMutex);
        tutoringSessionsCompleted++;
        studentsCurrentlyHelped--;
        printf("T: Student %d tutored by Tutor %d. Students tutored now: %d. Total sessions tutored: %d\n", studentID, tutorID, studentsCurrentlyHelped, tutoringSessionsCompleted);
        sem_post(&tutoringStatsMutex);
    }
    return NULL;
    
}

static void* student(void* id) {
    // Iterate for helps needed
    int studentID = (int) id;
    int i = 0;
    while (i < helps) {
        // Program
        usleep(programmingTime);
        printf("Student %d ready for tutoring\n", studentID);
        // Enter the waiting room
        // While there is no empty chair then go back to programming
        while (numEmptyChairs == 0) {
            printf("S: Student %d found no empty chair. Will try again later.\n", studentID);
            usleep(programmingTime);
        }

        // Sit down and increment the number of chairs
        sem_wait(&waitingRoomMutex);
        if (numEmptyChairs == 0) {
            sem_post(&waitingRoomMutex);
            printf("Just released the mutex\n");
            printf("S: Student %d found no empty chair. Will try again later.\n", studentID);
            continue;
        }
        numEmptyChairs--;
        printf("S: Student %d takes a seat. Empty chairs = %d.\n", studentID, numEmptyChairs);
        insertForCoordinator(studentID, queueForCoordinator);
        printWaitCoordinatorArea(queueForCoordinator);
        sem_post(&waitingRoomMutex);

        // Increment the number of signals to the coordinator
        // We are going to do a post to the coordinator
        sem_wait(&numNotificationsMutex);
        notifications++;
        sem_post(&queueWaitingStudent);
        printf("Student %d has just notified the coordinator\n", studentID);
        sem_post(&numNotificationsMutex);

        // Wait for tutor
        sem_wait(&registry[studentID]->studentSem);

        // Relinquish a waiting room chair
        sem_wait(&waitingRoomMutex);
        numEmptyChairs++;
        printf("Student %d called for tutoring, numChairs = %d\n", studentID, numEmptyChairs);
        sem_post(&waitingRoomMutex);

        // Get tutored
        usleep(tutoringTime);
        usleep(1);
        printf("Student %d is about to be done with tutoring\n", studentID);
        // Signal tutor that it's over
        sem_post(&registry[studentID]->tutorSem);
        printf("Student %d has signaled that tutoring is over.\n", studentID);
        printf("S: Student %d received help from tutor %d\n", studentID, registry[studentID]->tutorHelping);

        // Decrement the number of helps
        registry[studentID]->priority--;

        // Iterator
        i++;
        printf("Student %d priority is now %d and has completed %d iterations\n", studentID, registry[studentID]->priority, i);
    }
    printf("STUDENT %d is done\n", studentID);
    return NULL;
}

static void* coordinator() {
    int studentID;
    int s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    while (1) {
        // printf("Coordinator in the loop\n");
        // Wait until student comes
        sem_wait(&queueWaitingStudent);

        // Take student out of the "waiting for coordinator" queue
        sem_wait(&waitingRoomMutex);
        studentID = takeStudentFromWaiting(queueForCoordinator);
        printWaitCoordinatorArea(queueForCoordinator);
        sem_post(&waitingRoomMutex);

        // Print out the information
        printf("C: Student %d with priority %d in the queue. Waiting students now = %d. Total requests = %d.\n", studentID, registry[studentID]->priority, numStudentsWaiting, notifications);

        // Put student in queue for tutor
        sem_wait(&queueMutex);
        insert(waitingRoom, registry[studentID]);
        printWaitingRoom(waitingRoom);
        sem_post(&queueMutex);

        // Signal tutor
        sem_post(&coordinatorSignal);
    }
    return NULL;
}

int main(int argc, char *argv[]) {

    // Signal catching for segmentation fault
    sigset(SIGSEGV, hsignal);

    // Get the arguments
    if (argc != 5) {
        printf("Incorrect amount of arguments\n");
        exit(1);
    }
    else {
        numStudents = atoi(argv[1]);
        numTutors = atoi(argv[2]);
        chairs = atoi(argv[3]);
        helps = atoi(argv[4]);
    }

    waitingRoom = malloc(sizeof(struct LinkedList));
    queueForCoordinator = malloc(sizeof(struct Queue));
    registry = malloc((numStudents) * sizeof(struct person*));

    // Initialize (register) students
    long i;
    for (i = 0; i < numStudents; i++) {
        registry[i] = malloc(sizeof(struct person));
        registerStudent(registry[i], i, helps);
        sem_init(&registry[i]->tutorSem, 0, 0);
        sem_init(&registry[i]->studentSem, 0, 0);
    }

    // Initialize semaphores
    sem_init(&waitingRoomMutex, 0, 1);
    sem_init(&registryMutex, 0, 1);
    sem_init(&queueMutex, 0, 1);
    sem_init(&tutoringStatsMutex, 0, 1);
    sem_init(&numNotificationsMutex, 0, 1);
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