#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
Debugger: lldb ./executable
Compile: gcc main.c -o main
Asm: clang -S exec -O<level> -o output
 */

typedef struct {
int *ptr;
int length;
int size;
} Array;

Array initArray() {
  Array array;
  array.size = 10;
  array.length = 0;
  array.ptr = malloc(array.size * sizeof(int));
  return array;
}

void addToArray(Array* array, int value){
  if(array->length > array->size) {
    array->size += 10;
    array->ptr = realloc(array->ptr, array->size * sizeof(int));
    if(array->ptr != 0) { // Cleanup moved buffer
      memset(array->ptr+(array->size-9), 0, 10*sizeof(int)); // 9 Because our pointer can be seen as 1. Following standard c logic, we think of it as location 0.
    }
  }

  array->ptr[array->length] = value;
  array->length++;
  
}

void getValueAt(Array* array, int placement) {
  int val = array->ptr[placement];
  printf("Addr: %d, Offset: %d (%d), Value: %d\n", array->ptr, placement*sizeof(int), placement, val);
}

void getArrayValues(Array* array){
  for(int i = 0; i<=array->size; i++) {
    getValueAt(array, i);
  }
}

void freeArray(Array* array) {
free(array->ptr);
array->ptr = NULL;
}

int main() {
  Array a = initArray();
  for(int i = 0; i <= 10; i++){
  addToArray(&a, i);
  }
  getArrayValues(&a);
  addToArray(&a, 11);
  getArrayValues(&a);
  freeArray(&a);
}

/**

float add(float a, float b) { return a + b; }

float mult(float a, float b) { return a * b; }

float divide(float a, float b) {
  if (b == 0) {
    return 0;
  }
  return a / b;
}

int main() {
  int choice;
  float res = 0;
  float num;

  while (1) {
    printf("Result: %f\n", res);
    printf("Choice: \n");
    printf("0 - Set result\n");
    printf("1 - Add a value\n");
    printf("2 - Multiply with a value\n");
    printf("3 - Divide by a value\n");

    scanf("%d", &choice);

    if (choice >= 1 && choice <= 3) {
      printf("Input value: ");
      scanf("%f", &num);
    }
    switch (choice) {
    case 0:
      printf("Input result buffer: ");
      scanf("%f", &res);
      break;

    case 1:
      res = add(res, num);
      break;

    case 2:
      res = mult(res, num);
      break;

    case 3:
      res = divide(res, num);
      break;
    }
  }
}
*/
