#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

int num = 18;
int defaultLimit = 3;

int result = 0;
bool * processes;

int processId, processSize;

int isSecure(int x1, int y1, int x2, int y2) {
  if (
    ((abs(x1 - x2) == 1) && (abs(y1 - y2) == 2)) || 
    ((abs(y1 - y2) == 1) && (abs(x1 - x2) == 2))
  ) {
    return false;
  }

  if (x1 == x2) {
    return false;
  }
  if (y1 == y2) {
    return false;
  }

  if (abs(x1 - x2) == abs(y1 - y2)) {
    return false;
  }

  return true;
}

int isSecureCurrent(int current[num], int x, int y) {
  int k;
  for (k = 0; k < num; k++) {
    if (current[k] == -1) {
      return true;
    }
    if (!isSecure(k, current[k], x, y)) {
      return false;
    }
  }
  return true;
}

int getFreeProcess() {
  while (true) {
    int i;
    for (i = 1; i < processSize; i++) {
      if (processes[i]) {
        processes[i] = false;
        return i;
      }
    }
    return 0;
  }
}

void freeProcess(int i) {
  processes[i] = true;
}

int waitForProcess(int i) {
  int jobResult = 0;

  MPI_Status stat;
  MPI_Recv(&jobResult, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &stat);

  result += jobResult;

  freeProcess(stat.MPI_SOURCE);

  return stat.MPI_SOURCE;
}

void maharajah(int current[num], int counter, int limit, int * localResult) {
  if (counter == num) {
    *localResult += 1;
    return;
  }
  if (limit != num && counter == limit) {
    int freeProcessId = 0;
    while (freeProcessId == 0) {
      freeProcessId = getFreeProcess();
      if (freeProcessId == 0) {
        waitForProcess(MPI_ANY_SOURCE);
      }
    }
    
    MPI_Send (current, num, MPI_INT, freeProcessId, 1, MPI_COMM_WORLD);
    
    return;
  }

  int j;
  for (j = 0; j < num; j++) {
    if (isSecureCurrent(current, counter, j)) {
      current[counter] = j;
      maharajah(current, counter + 1, limit, localResult);
      current[counter] = -1;
    }
  }
}

void findMaharajah() {
  int current[num];

  int i;
  for (i = 0; i < num; i++) {
    current[i] = -1;
  }

  int localResult = 0;
  maharajah(current, 0, defaultLimit, &localResult);
}

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &processId);
	MPI_Comm_size(MPI_COMM_WORLD, &processSize);
  
  if(!processId) {
    printf("Process: %d \n", processSize);

    double t1, t2, dt;
    t1 = MPI_Wtime();
  
    bool processesData[processSize];
    processes = processesData;
    int i;
    for (i = 0; i < processSize; i++) {
      processes[i] = true;
    }

    // запустить махараджей
    findMaharajah();

    for (i = 1; i < processSize; i++) {
      if (!processes[i]) {
        waitForProcess(i);
      }
    }
    
    int exitCode[num];
    exitCode[0] = -1;
    for (i = 1; i < processSize; i++) {
      MPI_Send(&exitCode, num, MPI_INT, i, 1, MPI_COMM_WORLD);
    }

    t2 = MPI_Wtime();
    dt = t2 - t1;

    printf("Результат: %d \n", result);
    printf("Время: %fс \n", dt);
  } 
  else {
    MPI_Status stat;
    while (true) {
      int current[num];
      // если получили сообщение - делаем работу, отправляем результат
      MPI_Recv(&current, num, MPI_INT, 0, 1, MPI_COMM_WORLD, &stat);
      // если сообщение exit - выходим
      if (current[0] == -1) {
        break;
      } else {
        int localResult = 0;
        maharajah(current, defaultLimit, num, &localResult);
        MPI_Send (&localResult, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
      }
    }
  }

  MPI_Finalize();
  return 0;
}