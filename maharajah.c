#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

int result = 0;

int num = 18;

int isSecure(int x1, int y1, int x2, int y2) {
  if (
    ((abs(x1 - x2) == 1) && (abs(y1 - y2) == 2)) || 
    ((abs(y1 - y2) == 1) && (abs(x1 - x2) == 2))
  ) {
    return 0;
  }

  if (x1 == x2) {
    return 0;
  }
  if (y1 == y2) {
    return 0;
  }

  if (abs(x1 - x2) == abs(y1 - y2)) {
    return 0;
  }

  return 1;
}

int isSecureCurrent(int current[num], int x, int y) {
  int k;
  for (k = 0; k < num; k++) {
    if (current[k] == -1) {
      return 1;
    }
    if (isSecure(k, current[k], x, y) == 0) {
      return 0;
    }
  }
  return 1;
}

void maharajah(int current[num], int counter) {
  if (counter == num) {
    result += 1;
    return;
  }
  int j;
  for (j = 0; j < num; j++) {
    if (isSecureCurrent(current, counter, j) == 1) {
      current[counter] = j;
      maharajah(current, counter + 1);
      current[counter] = -1;
    }
  }
}

void findMaharajah() {
  int current[num];

  int i;
  for (i = 0; i < num; i++)
  {
    current[i] = -1;
  }

  maharajah(current, 0);
}

int main(void)
{
  // struct timeval stop, start;
  // gettimeofday(&start, NULL);
  double t1, t2, dt;
  t1 = MPI_Wtime();

  findMaharajah();

  t2 = MPI_Wtime();
  dt = t2 - t1;
  // gettimeofday(&stop, NULL);

  printf("Результат: %d \n", result);
  printf("Время: %fс \n", dt);
  // printf("Время: %lu \n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec); 
  return 0;
}