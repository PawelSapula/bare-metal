#include <stdio.h>

int multiply(int val1, int val2) {
  return val1*val2;
}

int main(void) {
  printf("Hello World!");
  int a = multiply(4, 8);
  printf("%d\n", a);
  return 0;
}

