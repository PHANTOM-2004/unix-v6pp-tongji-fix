#include <stdio.h>
#include <sys.h>

int main1() {
  int const pid = getpid();
  int const ppid = getppid(pid);

  printf("current process pid[%d]\n", pid);
  printf("parent process pid[%d]\n", ppid);
  return 0;
}
