#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <assert.h>

static int nthread = 1;
static int round = 0;

struct synchronizer {
  pthread_mutex_t sync_mutex;
  pthread_cond_t sync_cond;
  int nthread; // # threads that have reached this round of the synchronizer
  int round;     // Synchronizer round
} sstate;

static void
sync_init(void)
{
  assert(pthread_mutex_init(&sstate.sync_mutex, NULL) == 0);
  assert(pthread_cond_init(&sstate.sync_cond, NULL) == 0);
  sstate.nthread = 0;
}

static void 
synchronizer()
{
  // Acquire the lock
  pthread_mutex_lock(&sstate.sync_mutex);

  // Another thread has arrived at the room
  sstate.nthread++;

  // If the thread is the last thread
  if (sstate.nthread == nthread) {

    // Start the next round, the threads waiting will be 0
    sstate.nthread = 0;
    sstate.round++;
    
    // Signal the rest of the threads (everyone but itself)
    for (int i = 0; i < nthread - 1; i++) {
      pthread_cond_signal(&sstate.sync_cond);
    } 
  }

  // Otherwise we need to wait for everyone else
  else {
    pthread_cond_wait(&sstate.sync_cond, &sstate.sync_mutex);
  }

  // Release lock
  pthread_mutex_unlock(&sstate.sync_mutex);
  return;
}

static void *
thread(void *xa)
{
  long n = (long) xa;
  long delay;
  int i;

  for (i = 0; i < 20000; i++) {
    int t = sstate.round;
    assert (i == t);
    synchronizer();
    usleep(random() % 100);
  }
  return NULL;
}

int
main(int argc, char *argv[])
{
  pthread_t *tha;
  void *value;
  long i;
  double t1, t0;

  if (argc < 2) {
    fprintf(stderr, "%s: %s nthread\n", argv[0], argv[0]);
    exit(-1);
  }
  nthread = atoi(argv[1]);
  tha = malloc(sizeof(pthread_t) * nthread);
  srandom(0);

  sync_init();

  for(i = 0; i < nthread; i++) {
    assert(pthread_create(&tha[i], NULL, thread, (void *) i) == 0);
  }
  for(i = 0; i < nthread; i++) {
    assert(pthread_join(tha[i], &value) == 0);
  }
  printf("OK; passed\n");
  return 0;
}
