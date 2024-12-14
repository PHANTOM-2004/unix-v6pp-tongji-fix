#include <stdio.h>
#include <sys.h>

int main1() {
  int i, pid, ppid;
  pid = getpid();
  ppid = getppid(pid);
  printf("%d %d\n", pid, ppid);

  for (i = 0; i < 3; i++)
    if (fork() == 0) {
      pid = getpid();
      ppid = getppid(pid);
      printf("%d %d\n", pid, ppid);
    }
  sleep(2);
  return 0;
}
