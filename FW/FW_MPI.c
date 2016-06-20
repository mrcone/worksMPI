#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>

void Definir_Grafo(int n,float **dist,int **caminos){
// Inicializamos la matriz de pesos y la de caminos para el algoritmos de Floyd-Warshall. 
// Un 0 en la matriz de pesos indica que no hay arco.
// Para la matriz de caminos se supone que los vertices se numeran de 1 a n.
	int i,j;
	for (i = 0; i < n; ++i) {
		for (j = 0; j < n; ++j) {
			if (i==j){
				dist[i][j]=0;
			}else {
				dist[i][j]= (11.0 * rand() / ( RAND_MAX + 1.0 )); // aleatorios 0 <= dist < 11
				dist[i][j] = ((double)((int)(dist[i][j]*10)))/10; // truncamos a 1 decimal
				if (dist[i][j] < 2) {
					dist[i][j]=0; // establecemos algunos a 0 
				}
			}

			if (dist[i][j] != 0){
				caminos[i][j] = i+1;
			}
			else{
				caminos[i][j] = 0;
			}
		}
	}
}

void calcula_camino(float **a, int **b, int n) {
	int i,count=2, count2;
	int anterior; 
	int *camino;
	int inicio=-1, fin=-1;

	while ((inicio < 0) || (inicio >n) || (fin < 0) || (fin > n)) {
		printf("Vertices inicio y final: (0 0 para salir) ");
		scanf("%d %d",&inicio, &fin);
	}
	while ((inicio != 0) && (fin != 0)) {
		anterior = fin;
		while (b[inicio-1][anterior-1] != inicio) {
			anterior = b[inicio-1][anterior-1];
			count = count + 1;
		}
		count2 = count;
		camino = malloc(count * sizeof(int));
		anterior = fin;
		camino[count-1]=fin;
		while (b[inicio-1][anterior-1] != inicio) {
			anterior = b[inicio-1][anterior-1];
			count = count - 1;
			camino[count-1]=anterior;
		}
		camino[0] = inicio;
		printf("\nCamino mas corto de %d a %d:\n", inicio, fin);
		printf("		  Peso: %5.1f\n", a[inicio-1][fin-1]);
		printf("		Camino: ");
		for (i=0; i<count2; i++) printf("%d ",camino[i]);
			printf("\n");
		free(camino);
		inicio = -1;
		while ((inicio < 0) || (inicio >n) || (fin < 0) || (fin > n)) {
			printf("Vertices inicio y final: (0 0 para salir) ");
			scanf("%d %d",&inicio, &fin);
		}

	}
}

float **Crear_matriz_pesos_consecutivo(int Filas, int Columnas) {
// crea un array de 2 dimensiones en posiciones contiguas de memoria
	float *mem_matriz;
	float **matriz;
	int fila, col;
	if (Filas <=0) 
	{
		printf("El numero de filas debe ser mayor que cero\n");
		return;
	}
	if (Columnas <= 0)
	{
		printf("El numero de filas debe ser mayor que cero\n");
		return;
	}
	mem_matriz = malloc(Filas * Columnas * sizeof(float));
	if (mem_matriz == NULL) 
	{
		printf("Insuficiente espacio de memoria\n");
		return;
	}
	matriz = malloc(Filas * sizeof(float *));
	if (matriz == NULL) 
	{
		printf ("Insuficiente espacio de memoria\n");
		return;
	}
	for (fila=0; fila<Filas; fila++)
		matriz[fila] = mem_matriz + (fila*Columnas);
	return matriz;
}

int **Crear_matriz_caminos_consecutivo(int Filas, int Columnas) {
// crea un array de 2 dimensiones en posiciones contiguas de memoria
	int *mem_matriz;
	int **matriz;
	int fila, col;
	if (Filas <=0) 
	{
		printf("El numero de filas debe ser mayor que cero\n");
		return;
	}
	if (Columnas <= 0)
	{
		printf("El numero de filas debe ser mayor que cero\n");
		return;
	}
	mem_matriz = malloc(Filas * Columnas * sizeof(int));
	if (mem_matriz == NULL) 
	{
		printf("Insuficiente espacio de memoria\n");
		return;
	}
	matriz = malloc(Filas * sizeof(int *));
	if (matriz == NULL) 
	{
		printf ("Insuficiente espacio de memoria\n");
		return;
	}
	for (fila=0; fila<Filas; fila++)
		matriz[fila] = mem_matriz + (fila*Columnas);
	return matriz;
}

void printMatrizCaminos(int **a, int fila, int col) {
	int i, j;
	char buffer[10];
	printf("	");
	for (i = 0; i < col; ++i){
		j=sprintf(buffer, "%c%d",'V',i+1 );
		printf("%4s", buffer);
	}
	printf("\n");
	for (i = 0; i < fila; ++i) {
		j=sprintf(buffer, "%c%d",'V',i+1 );
		printf("%4s", buffer);
		for (j = 0; j < col; ++j)
			printf("%4d", a[i][j]);
		printf("\n");
	}
	printf("\n");
}

void printMatrizPesos(float **a, int fila, int col) {
	int i, j;
	char buffer[10];
	printf("	  ");
	for (i = 0; i < col; ++i){
		j=sprintf(buffer, "%c%d",'V',i+1 );
		printf("%6s", buffer);
	}
	printf("\n");
	for (i = 0; i < fila; ++i) {
		j=sprintf(buffer, "%c%d",'V',i+1 );
		printf("%6s", buffer);
		for (j = 0; j < col; ++j)
			printf("%6.1f", a[i][j]);
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char *argv[]) {

	float **dist;
	int **caminos;

	int idProceso, totalProcesos, seccion, numVertices=10, resto=0,start;
	int filaInf, tam, recorrer, c;
	double t1,t2;

	MPI_Status estado;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&totalProcesos);
	MPI_Comm_rank(MPI_COMM_WORLD,&idProceso);
	int senderK=0;
	int i, j, k;
	int *auxC;
	float *aux;
	if (argc != 1) {
		numVertices=atoi(argv[1]);
	}

	resto= numVertices%totalProcesos; //El sobrante de la división de procesos

	if (idProceso == 0) {
		filaInf=0;
		recorrer=resto; //Establecemos el contador en el primer segmento
		seccion= (numVertices-resto)/totalProcesos; //Repartimos el numero de filas entre el número total de procesos
		start = seccion + resto;

		tam = start;
		c=numVertices;//Tamaño que usará para crear la matriz de pesos y caminos  
		t1 = MPI_Wtime();
	}else{
		seccion= (numVertices-resto)/totalProcesos;
		start = seccion +resto;
		filaInf = resto + seccion*idProceso;
		tam = seccion;
		recorrer = 0;
		c=seccion;
	}

	//Definimos las matrices de peso y caminos. Y se define el grago exácfinEnviosente igual que la parte secuencial
	dist = Crear_matriz_pesos_consecutivo(c,numVertices);
	caminos = Crear_matriz_caminos_consecutivo(c,numVertices);

	//Si es el padre, envía sus filas e ignora la cantidad de elementos y el tipo que recibe (el recvcant y el type)
	if (idProceso == 0) {	
		Definir_Grafo(numVertices,dist,caminos);
		if (numVertices <= 10) {
			printMatrizPesos(dist,numVertices,numVertices);
			printMatrizCaminos(caminos,numVertices,numVertices);
		} 
		MPI_Scatter(&dist[resto][0], numVertices*seccion, MPI_FLOAT, MPI_IN_PLACE,numVertices*seccion, MPI_FLOAT,0,MPI_COMM_WORLD); 
		MPI_Scatter(&caminos[resto][0], numVertices*seccion, MPI_INT, MPI_IN_PLACE,numVertices*seccion, MPI_INT,0,MPI_COMM_WORLD);

	}else { //El hijo envía sus filas al resto de procesos y establece donde los va a recibir
		MPI_Scatter(&dist[resto][0], numVertices*seccion, MPI_FLOAT, &dist[recorrer][0],numVertices*seccion, MPI_FLOAT,0,MPI_COMM_WORLD); 
		MPI_Scatter(&caminos[resto][0], numVertices*seccion, MPI_INT, &caminos[recorrer][0],numVertices*seccion, MPI_INT,0,MPI_COMM_WORLD);
	}

	aux=(float *)malloc(sizeof(float) * numVertices);
	auxC=(int *)malloc(sizeof(int) * numVertices);

	//En la iteración k, cada tarea, además de su fila, necesita los valores de la fila k de pesos y caminos. 
	//Hay que buscar la tarea que tenga la fila y difundirlas con Bcast
	//Empieza el algoritmo de FW  
	for (k=0; k<numVertices; k++) {
		for (i=0;i<totalProcesos;i++){
			//Buscamos el proceso que tiene la fila K para enviarsela al resto de procesos
			if ((k >= resto + seccion*i) && (k<= start + seccion*i)){
				senderK = i;
			}
		}

		//Si soy yo el que tiene que enviar el proceso al resto de hijos
		//Creo el vector con mi fila
		if (idProceso == senderK) {
			for (i=0;i<numVertices;i++) {
				aux[i] = dist[k-filaInf][i];
				auxC[i]=caminos[k-filaInf][i];
			}
		}

		//Se envían los vectores al resto de procesos
		MPI_Bcast(&aux[0], numVertices, MPI_FLOAT, senderK, MPI_COMM_WORLD);
		MPI_Bcast(&auxC[0], numVertices, MPI_INT, senderK, MPI_COMM_WORLD);

		//En las lineas filas del proceso, recorremos todas las columnas

		for (i = 0; i < tam; ++i) {
			for (j = 0; j < numVertices; ++j) {
				//Y si no existe camino, hacemos la comprobación de que sea menor Y QUE NO SEA INFINITO
				if ((dist[i][k] * aux[j] != 0) ) {
					if ((dist[i][k] + aux[j] < dist[i][j]) || (dist[i][j] == 0)) {
						dist[i][j] = dist[i][k] + aux[j];
						caminos[i][j]=auxC[j];
					}
				}
			}
		}
	}

	//Total de elementos a enviar de la matriz. 
    //Recorremos tantas filas como valga recorrer.
    //Con MPI_IN_PLACE hacemos que el buffer de salida sea el mismo que el buffer de entrada
	int totalElementosEnv=numVertices*seccion;
	if (idProceso == 0) { 
        MPI_Gather(MPI_IN_PLACE, totalElementosEnv, MPI_FLOAT, &dist[recorrer][0], totalElementosEnv, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Gather(MPI_IN_PLACE, totalElementosEnv, MPI_INT, &caminos[recorrer][0], totalElementosEnv, MPI_INT, 0, MPI_COMM_WORLD);		
	}else { 
        MPI_Gather(&dist[recorrer][0], totalElementosEnv, MPI_FLOAT, &dist[recorrer][0], totalElementosEnv, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Gather(&caminos[recorrer][0], totalElementosEnv, MPI_INT, &caminos[recorrer][0], totalElementosEnv, MPI_INT, 0, MPI_COMM_WORLD);
	}

	if (idProceso == 0) {
		t2 = MPI_Wtime();
		printf("Tiempo total %f \n",t2-t1);		

		if (numVertices <= 10) {
			printMatrizPesos(dist,numVertices,numVertices);
			printMatrizCaminos(caminos,numVertices,numVertices);
		}
        //Calculamos los caminos hasta meter 00 y salir
		calcula_camino(dist, caminos, numVertices);
	}

	free(dist);
	free(caminos);  
	MPI_Finalize();
}