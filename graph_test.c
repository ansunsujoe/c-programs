#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#define check(exp, msg) if(exp) {} else {\
   printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
   exit();}
#define tol 55

/**
 * The following code is added/modified by Ansun Sujoe (AXS180210)
 * and Henry Schliebe (HJS170002)
 * Testcase for printing tick values for the graph pdf
 i*/

// Function to spin
void spin()
{
	int i = 0;
  int j = 0;
  int k = 0;
	for(i = 0; i < 50; ++i)
	{
		for(j = 0; j < 100000; ++j)
		{
			k = j % 10;
      k = k + 1;
     }
	}
}

void print(struct pstat *st)
{
   int i;
   for(i = 0; i < NPROC; i++) {
      if (st->inuse[i]) {
          printf(1, "pid: %d tickets: %d ticks: %d\n", st->pid[i], st->tickets[i], st->ticks[i]);
      }
   }
}

int main()
{
   // Processes that will be forked
   int pid1;
   int pid2;

   // Set parent process to 10 tickets (Process C)
   check(settickets(10) == 0, "settickets");

   pid1 = fork();
   
   // First child process set for 20 tickets (Process B)
   if (pid1 == 0) {
     // Set tickets and spin until interrupted
     settickets(20);
     for (;;) {
       spin();
     }
   }
   else {
     // Second child process set for 30 tickets (Process A)
     pid2 = fork();
     if (pid2 == 0) {
       // Set tickets and spin until interrupted
       settickets(30);
       for (;;) {
         spin();
       }
     }
   }
   
   int j;
   struct pstat st;

   // Sleep for some time then print getpinfo
   // Take note of processes 3, 4, and 5 that each have 10, 20, and 30
   // tickets respectively.
   for (j = 0; j < 40; j++) {
     sleep(5);
     if (getpinfo(&st) != 0) {
       goto Cleanup;
     }
     printf(1, "\n **** Time %d ****\n", j);
     print(&st);
   }
   
Cleanup:
   while (wait() > 0);
   kill(pid1);
   kill(pid2);
   exit();
}

/* End of code added/modified */
