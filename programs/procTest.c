#include <stdio.h>
#include <sys.h>

void main1() {
  int i, j, k, pid, ppid;
  if (fork()) {
    // 2#
    sleep(2);
    for (k = 1; k < 6; k++) {
      printf("%d,%d; ", k, getppid(k));
    }
    printf("\n");
  } else {
    int const ws = 0;
    // 3#
    if (fork()) {
      if (fork()) {
        sleep(1);
        // 3#
        pid = getpid();
        ppid = getppid(pid);
        // for (k = 0; k < ws; k++) {
        //   i = wait(&j);
        //   printf("Process %d#:My child %d is finished with exit status %d\n",
        //          pid, i, j);
        // }
        printf("Process %d# finished: My father is %d\n", pid, ppid);
        exit(ppid);
      } else {
        // 5#
        pid = getpid();
        ppid = getppid(pid);
        printf("Process %d# finished: My father is %d\n", pid, ppid);
        exit(ppid);
      }
    } else {
      // 4#
      pid = getpid();
      ppid = getppid(pid);
      printf("Process %d# finished: My father is %d\n", pid, ppid);
      exit(ppid);
    }
  }
}
