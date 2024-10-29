#include <stdio.h>

int version = 1;

int sum(int a, int b) {
  int cnt;
  version = 2;
  cnt = a + b;
  return cnt;
}

void main1() {
  int a, b, res;
  a = 1;
  b = 2;
  res = sum(a, b);
  printf("result=%d\n", res);
}
