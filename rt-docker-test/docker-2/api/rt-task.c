#include <sched.h>
#include <stdio.h>
int main(int argc, char **argv)
{
  /* Setting scheduler policy and priority*/
  struct sched_param param;
  int thread_priority = argc > 1 ? atoi(argv[1]) : 0;
  param.sched_priority = thread_priority; 
  sched_setscheduler(0, SCHED_RR, &param);
 
  /* Set thread id */
  int thread_id = argc > 2 ? atoi(argv[2]) : -1;
  
  /* Infinity loop */
  int n = 0;
  while(1) {
    n = n + 1;
    if (!(n % 10000000)) {

      /* Get the CPU Core used*/
      int current_cpu = sched_getcpu();

      /* Print: CPU, Thread ID, Sched Policy, Priority, Timer(n)*/
      printf("%d,%d,SCHED_RR,%d,%d\n",current_cpu, thread_id, thread_priority, n);

      //printf("Inst:%s RR Prio %d running (n=%d)\n",argv[2], atoi(argv[1]),n);
    }
  }
}