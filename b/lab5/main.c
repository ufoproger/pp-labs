// mpicc -o main main.c && mpirun main -np 3 && mout main.1 out
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

// #define DEBUG DEBUG

#define EPSILON			1e-9
#define DELTA_COLLECT 	0
#define DELTA_PTP 		1

#define N 				100

int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);

	srand(time(NULL));

	int root_rank = 0;
	int i, j;
	int world_size;
	int world_rank;
	int namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);	
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Get_processor_name(processor_name, &namelen);

	if (argc > 1) {
		root_rank = atoi(argv[1]);
	}

	if (root_rank >= world_size) {
		root_rank = 0;
	}

	if (world_rank == root_rank) {
		printf("Количество процессов: %d шт, корневой процесс %d.\n", world_size, root_rank);
		fflush(stdout);
	}

	double data[N];

	if (world_rank == root_rank) {
		for (i = 0; i < N; ++i) {
			data[i] = (double)(rand() % 100 / (rand() % 10 + 1));
		}
	}

	double delta[2];

	MPI_Barrier(MPI_COMM_WORLD);
	delta[DELTA_COLLECT] = -MPI_Wtime();

	for (i = 0; i < N; ++i) {
		MPI_Bcast(&data[i], 1, MPI_DOUBLE, root_rank, MPI_COMM_WORLD);
	}

	delta[DELTA_COLLECT] += MPI_Wtime();

#ifdef DEBUG
	double sum = 0.;

	for (i = 0; i < N; ++i) {
		sum += data[i];
	}

	printf("sum (после T_collect) = %lf.\n", sum);
	fflush(stdout);
#endif

	MPI_Barrier(MPI_COMM_WORLD);
	delta[DELTA_PTP] = -MPI_Wtime();

	if (world_rank == root_rank) {
		for (i = 0; i < N; ++i) {
			for (j = 0; j < world_size; ++j) {
				if (j != root_rank) {
					MPI_Send(&data[i], 1, MPI_DOUBLE, j, 0, MPI_COMM_WORLD);
				}
			}	
		}
	}
	else {
		for (i = 0; i < N; ++i) {
			MPI_Recv(&data[i], 1, MPI_DOUBLE, root_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
	}

	delta[DELTA_PTP] += MPI_Wtime();

#ifdef DEBUG
	sum = 0.;

	for (i = 0; i < N; ++i) {
		sum += data[i];
	}

	printf("sum (после T_ptp) = %lf.\n", sum);
	fflush(stdout);
#endif

	if (world_rank == root_rank) {
		printf("T_collect = %lf сек.\n", delta[DELTA_COLLECT]);
		printf("T_ptp = %lf сек.\n", delta[DELTA_PTP]);

		int won;

		if (fabs(delta[DELTA_COLLECT] - delta[DELTA_PTP]) < EPSILON) {
			won = -1;
			puts("T_collect и T_ptp равны.");
		}
		else if (delta[DELTA_COLLECT] < delta[DELTA_PTP]) {
			won = DELTA_COLLECT;
			puts("T_collect < T_ptp.");
		}
		else {
			won = DELTA_PTP;
			puts("T_collect > T_ptp.");
		}

		printf("T_collect быстрее T_ptp в %.2lf раза.\n", delta[DELTA_PTP] / delta[DELTA_COLLECT]);
		fflush(stdout);

		FILE *fp = fopen("main.log", "a");

		if (fp != NULL) {
			if (ftell(fp) == 0) {
				fprintf(fp, "%5s %5s %10s %10s %10s\n", "procs", "root", "T_collect", "T_ptp", "won");
			}

			fprintf(fp, "%5d %5d %10lf %10lf ", world_size, root_rank, delta[DELTA_COLLECT], delta[DELTA_PTP]);

			switch (won) {
				case -1:
					fprintf(fp, "%10s\n", "all");
					break;

				case DELTA_COLLECT:
					fprintf(fp, "%10s\n", "T_collect");
					break;

				case DELTA_PTP:
					fprintf(fp, "%10s\n", "T_ptp");
					break;
			}

			fclose(fp);
		}
	}

	MPI_Finalize();

	return 0;
}
