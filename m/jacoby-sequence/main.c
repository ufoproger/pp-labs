#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <ctype.h>
#include <ctime>
#include <sys/time.h>

#define N (int)20

#define PI (double)(4. * atanl(1))

//#define LENGTH (double)1.
#define LENGTH (double)(2 * PI)
#define EPSILON (double)10e-8
 
void right_side(double h, double **f)
{
    for (int i = 0; i <= N; ++i) {
        for (int j = 0; j <= N; ++j) {
            // f[i][j]=0;          
            f[i][j]= -2 * (sin(i * h) * cos(j * h));
        }
    }
}

int initial_data(double h, double **u)
{   
    for (int j = 0; j <= N; ++j) {
        u[0][j] = sin(0.0) * cos(h * j);
        u[N][j] = sin(N * h) * cos(h * j);

        // u[0][j] = 100 - 200 * (h * j);
        // u[n][j] = -100 + 200 * (h * j);
    }

    for (int i = 1; i < N; ++i) {
        u[i][0] = sin(h * i) * cos(0.0);
        u[i][N] = sin(h * i) * cos(N * h);

        // u[i][0] = 100 - 200 * (h * i);
        // u[i][n] = -100 + 200 * (h * i);
    }

    // interior
    for (int i = 1; i < N; ++i) {
        for (int j = 1; j < N; ++j) {
            u[i][j] = 0;
        }
    }
         
    return 1;
}

int exact_solv(double h, double **u_exact)
{       
    for (int i = 0; i <= N; ++i) {
        for (int j = 0; j <= N; ++j) {
            u_exact[i][j] = sin(h * i) * cos(h * j);
        }
    }
     
    return 1;
}

int main(int argc, char * argv[])
{
    clock_t t1, t2;
    struct timeval tv1, tv2;
    struct timezone tz;
    
    int num = 0, i_max, j_max, ind;
    double err, err_max = 1, exact_jac = 1e-8;
    double res, res_max = 1;
    double **ptr;
    double h = LENGTH / N;
    char tmp, file_name[30];
    
    FILE *out_file_u, *out_file_ex, *out_time;
    
    double **u = (double**)malloc(sizeof(double*) * (N + 1));
    double **u_new = (double**)malloc(sizeof(double*) * (N + 1));
    double **u_exact = (double**)malloc(sizeof(double*) * (N + 1));
    double **f = (double**)malloc(sizeof(double*) * (N + 1));
    
    for (int i = 0; i <= N; ++i) {
        u [i] = (double*)malloc(sizeof(double) * (N + 1));
        u_new[i] = (double*)malloc(sizeof(double) * (N + 1));
        u_exact[i] = (double*)malloc(sizeof(double) * (N + 1));
        f[i] = (double*)malloc(sizeof(double) * (N + 1));
    }
   
    right_side(h, f);
    initial_data(h, u);
    initial_data(h, u_new);
    exact_solv(h, u_exact);
    
/*
    for (i=0; i<=n; ++i)
    printf("%20.12le %20.12le | %20.12le %20.12le \n",u[0][i],u_exact[0][i],u[n][i],u_exact[n][i] );
    printf("\n*************\n");
    for (i=0; i<=n; ++i)
    printf("%20.12le %20.12le | %20.12le %20.12le \n",u[i][0],u_exact[i][0],u[i][n],u_exact[i][n] );
    printf("\n*************\n");
*/  
    
/////////////// START TIME ////////////////
    gettimeofday(&tv1, &tz);
    t1 = clock();
//////////////////////////////////////////
         
    while (num < 1000000 && (err_max > exact_jac || res_max > exact_jac)) {
        num++;
        err = 0;
        err_max = 0;
        res_max = 0;
            
        for (int i = 1; i < N; ++i) {
            for (int j = 1; j < N; ++j) {
                u_new[i][j] = 0.25 * (u[i - 1][j] + u[i + 1][j] + u[i][j - 1] + u[i][j + 1] - h * h * f[i][j]); 
                err = fabs(u[i][j] - u_new[i][j]);

                if (err > err_max) {
                    err_max = err;
                }
            }
        }

        for (int i = 1; i < N; ++i) {
            for (int j = 1; j < N; ++j) {
                res = fabs(u_new[i - 1][j] + u_new[i + 1][j] - 4 * u_new[i][j] + u_new[i][j - 1] + u_new[i][j + 1] - h * h * f[i][j]); 

                if (res_max < res) {
                    res_max = res;
                }
            }
        }

        // u = u_new;
        ptr = u;
        u = u_new;
        u_new = ptr;
    }

/////////////////////////////////////TIME END////////////////////////////////////////
    t2 = clock();
    gettimeofday(&tv2, &tz);

    sprintf(file_name, "CPU_time.txt");

    if ((out_time = fopen(file_name, "a")) == NULL) {
        printf("Error at opening of the file.\n");
        exit(1); 
    }

    fprintf(out_time, "%d: %22.14le\n", N, (double)((t2 - t1) / CLOCKS_PER_SEC)); 
    fclose(out_time);

    sprintf(file_name, "Time(gettimeofday).txt");

    if ((out_time = fopen(file_name, "a")) == NULL) {
        printf("Error at opening of the file.\n");
        exit(1);
    }   

    fprintf(out_time, "%d: %22.14le\n", N, (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec) * 1e-6);
    fclose(out_time);
//////////////////////////////////////////////////////////////////////////////////////////

    printf("\n iter=%i\tJacoby dif_max=%20.12le\tres=%20.12le\n", num, err_max, res_max);
 
    err_max = fabs(u[0][0] - u_exact[0][0]);

    for (int i = 0; i <= N; ++i) {
        for (int j = 0; j <= N; ++j) {
            err = fabs(u[i][j] - u_exact[i][j]);

            if (err > err_max) {
                err_max = err;
                i_max = i;
                j_max = j;
            }
        }
    }


    printf("\n iter=%i\terr_max=%20.12le\tu[%d][%d]=%20.12le\tu_exact[%d][%d]=%20.12le\n",num, err_max, i_max, j_max, u[i_max][j_max], i_max, j_max, u_exact[i_max][j_max]);
    // getchar();
/*
//  open file for print data about d-iteration
    if ((out_file_u=fopen("out_u.txt", "a"))==NULL) {
    printf("Error at opening of the file.\n");exit(1);
    }//if (open file)

//  open file for print data about d-iteration
    if ((out_file_ex=fopen("out_ex.txt", "a"))==NULL) {
    printf("Error at opening of the file.\n"); exit(1);
    }//if (open file)

    for (i=0; i<=n; ++i){
    for (j=0; j<=n; ++j)
        fprintf(out_file_u,"%20.12le ", u[i][j]);                   
    fprintf(out_file_u,"\n");
    }


//  fprintf(out_file,"\n *************\n");
    
    for (i=0; i<=n; ++i){
    for (j=0; j<=n; ++j)
        fprintf(out_file_ex,"%20.12le ", u_exact[i][j]);
    fprintf(out_file_ex,"\n");
    }

    fclose(out_file_u);
    fclose(out_file_ex);

*/
    for (int i = 0; i <= N; ++i) {
        free(u[i]);
        free(u_new[i]);
        free(u_exact[i]);
        free(f[i]);
    }

    free(u);
    free(u_new);
    free(u_exact);
    free(f); 

    return 0;
}


