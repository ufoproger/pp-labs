// mpicc -std=c99 -o main main.c && mpirun main test -np 3 && mout main.1 out

#include <string.h>
#include <stdio.h>
#include <mpi.h>

// Цель бенчмарка
#define G_OP_TYPE_COLLECT	1
#define G_OP_TYPE_PTP		2

// Корневой процесс
#define ROOT 	0

// http://mpitutorial.com/tutorials/mpi-broadcast-and-collective-communication/
// Глава "Broadcasting with MPI_Send and MPI_Recv"
void custom_bcast(void* data, int count, MPI_Datatype datatype, int root, MPI_Comm communicator) {
	int world_rank;
	int world_size;
	
	MPI_Comm_rank(communicator, &world_rank);
	MPI_Comm_size(communicator, &world_size);

	if (world_rank == root) {		
		for (int i = 0; i < world_size; i++) {
			if (i != world_rank) {
				MPI_Send(data, count, datatype, i, 0, communicator);
			}
		}
	} else {
		MPI_Recv(data, count, datatype, root, 0, communicator, MPI_STATUS_IGNORE);
	}
}

int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);

	int g_op_type = 0;

	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "collect") == 0) {
			g_op_type = G_OP_TYPE_COLLECT;
			break;
		}
		else if (strcmp(argv[i], "ptp") == 0) {
			g_op_type = G_OP_TYPE_PTP;
			break;
		}
	}

	int size;
	int rank;
	int namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];

	MPI_Comm_size(MPI_COMM_WORLD, &size);	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Get_processor_name(processor_name, &namelen);

	if (rank == ROOT) {
		printf("Количество процессов: %d шт.\n", size);
		fflush(stdout);
	}

	printf("Выполняется процесс %d на %s.\n", rank, processor_name);
	fflush(stdout);
	
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();

	return 0;
}
