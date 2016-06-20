#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>

#define maxm 1000
#define maxn 1000

void vermatriz(int lda, double a[][lda], int, int, char []);
void vervector(double a[], int m, char nombre[]);
void matvec(int filas, int columnas, double a[][columnas], double vector[], double *vector_solucion);


void
main(int argc, char **argv) {

	double matriz[maxm][maxn], vector[maxn], vectorSolucion[maxm]; //Matriz, vector y solución
	int filas_por_proceso, filas_padre, auxPosicion;
	int i, j; //Contadores
	int numFilas, numColumnas; //Ancho y alto de la matriz
	int idProceso, totalProcesos; //Info procesos MPI
	MPI_Status estado; //Estado MPI

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &idProceso);
	MPI_Comm_size(MPI_COMM_WORLD, &totalProcesos);
	
	if(idProceso==0){ //Si soy el proceso padre
		//Pedimos todos los datos por pantalla
		printf("Numero de filas (1 - %d): ",maxm);
		scanf("%d",&numFilas);

		while ((numFilas <= 0) || (numFilas > maxm)) {
			printf("Numero de filas (1 - %d): ",maxm);
			scanf("%d",&numFilas);
		}

		printf("Numero de columnas (1 - %d): ",maxn);
		scanf("%d",&numColumnas);

		while ((numColumnas <= 0) || (numColumnas > maxn)) {
			printf("Numero de columnas (1 - %d): ",maxn);
			scanf("%d",&numColumnas);
		}

		//Rellenamos la matriz
		for (i=0; i<numFilas; i++){
			for (j=0; j<numColumnas; j++){
				matriz[i][j] =  i+j;
			}
		}
		//Rellenamos el vector
		for (j=0; j<numColumnas; j++) {
			vector[j]= j;
		}

		//Mostramos la matriz y el vector
		if ((numFilas <= 10) && (numColumnas <= 10)) {
			vermatriz(maxn, matriz, numFilas, numColumnas, "A");
			printf("\n");
			vervector(vector, numColumnas, "x");
		}

		//Hacemos la subdivision igual que en la práctica CONTAR
		filas_por_proceso=numFilas/totalProcesos;
		filas_padre=auxPosicion=filas_por_proceso+(numFilas%totalProcesos); 


		//Después le enviamos la fila entera a cada proceso y el vector por el que lo tiene que multiplicar
		for(i=1; i<totalProcesos; i++){
			MPI_Send( &filas_por_proceso, 1, MPI_INT, i, 23, MPI_COMM_WORLD); //Enviamos a cada hijo, la cantidad de filas que tiene que multiplicar (filas_por_proceso)
			MPI_Send( &numColumnas, 1, MPI_INT, i, 23, MPI_COMM_WORLD); //Además, cuantas columnas tiene que reservar para realizar el cálculo (numColumnas)	
			//A cada proceso le decimos que coja desde auxPosicion que recorre cada fila (global) y vaya cogiendo cada fila hasta completar su asignación
			for(j=0; j<filas_por_proceso;j++){
				MPI_Send( &matriz[auxPosicion+j][0], numColumnas, MPI_DOUBLE, i, 23, MPI_COMM_WORLD);
			}
			//Enviamos el vector al proceso hijo
			MPI_Send( &vector, numFilas, MPI_DOUBLE, i, 23, MPI_COMM_WORLD);
			//Desplazamos auxPosicion a la siguiente subdivisión
			auxPosicion+=filas_por_proceso;
		}

		//Multiplicamos con la funcion tal y como dice en la práctica
		matvec(numFilas, numColumnas, matriz, vector, &vectorSolucion[0]);
		
		//Realizamos el algoritmo inverso. Recibimos el vector que nos envian los hijos.
		//Va recibiendo de cada proceso y lo va almacenando en vectorSolucion
		auxPosicion=filas_padre;
		for(i=1; i<totalProcesos; i++){
			MPI_Recv(&vectorSolucion[auxPosicion], filas_por_proceso, MPI_DOUBLE, i, 32, MPI_COMM_WORLD, &estado);
			auxPosicion+=filas_por_proceso;
		}
		vervector(vectorSolucion, numFilas, "A*x");
		
	}else{ //Si soy un hijo
		//Recibimos el alto y ancho de la matriz que tenemos que calcular
		MPI_Recv (&numFilas, 1, MPI_INT, 0, 23, MPI_COMM_WORLD, &estado);
		MPI_Recv (&numColumnas, 1, MPI_INT, 0, 23, MPI_COMM_WORLD, &estado);

		//Recibimos la matrix auxiliar (más pequeña) del proceso padre
		for(j=0; j<numFilas; j++){
			MPI_Recv (&matriz[j][0], numColumnas, MPI_DOUBLE, 0, 23, MPI_COMM_WORLD, &estado);
		}
		//Recibimos el vector del padre
		MPI_Recv (&vector, numColumnas, MPI_DOUBLE, 0, 23, MPI_COMM_WORLD, &estado);

		//Multiplicamos con la funcion tal y como dice en la práctica
		matvec(numFilas, numColumnas, matriz, vector, &vectorSolucion[0]);

		//Enviamos al padre el resultado de matvec (almacenado en vectorSolucion)
		MPI_Send(&vectorSolucion, numFilas, MPI_DOUBLE, 0, 32, MPI_COMM_WORLD);

	}
	MPI_Finalize();
} 

void vermatriz(int lda, double a[][lda], int m, int n, char nombre[]) {
	int i,j;
	printf("%4s =", nombre);
	printf("%6d ", 0);
	for (j=1;j<n;j++){
		printf("%7d ",j);
	}

	printf("\n");

	for (i=0;i<m;i++){
		printf("%8d:",i);
		for (j=0;j<n;j++){
			printf("%7.3f ",a[i][j]);
		}
		printf("\n");
	}      
}

void vervector(double a[], int m, char nombre[]) {
	int i;
	printf("%6s = ",nombre);
	for (i=0;i<m;i++){
		printf("%7.3f ",a[i]);
		if ((i+1)%5  == 0) {
			printf("\n\t\t");
		}
	}
	printf("\n"); 
}

//Devuelve el resultado de multiplicar una matriz por un vector.
//El resultado se almacena en vector_solucion. Le pasamos el puntero a la posicion de inicio de vector_solucion
void matvec(int filas, int columnas, double matriz[][maxn], double vector[], double *vector_solucion){
	int i,j;
	//Inicializamos el vector a 0
	for(i=0; i<filas; i++){
		vector_solucion[i]=0;
	}
	for(i=0; i<filas; i++){
		for(j=0; j<columnas; j++){
			vector_solucion[i]+=matriz[i][j]*vector[j];
		}
	}
}