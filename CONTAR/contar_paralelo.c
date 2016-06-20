#include <stdio.h> 
#include <stdlib.h> 
#include <sys/time.h> 
#include <malloc.h>
#include <mpi.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>

//Definiciones
#define TRUE 1
#define FALSE 0
static int nclock;

//Datos por defecto
#define MAXENTERO 1000 // El array de enteros estará entre 1 y MAXENTERO
#define REPETICIONES 10000 // Simula vector todavia mayor sin ocupar memoria
#define NUMBUSCADO 10 // Numero buscado


/** ctimer: Devuelve el timestamp absoluto
 *	return double
 **/
double ctimer(void){
	struct timeval tp;
	struct timezone tzp;
	double diff;
	nclock=sysconf(_SC_CLK_TCK);
	gettimeofday(&tp, &tzp);
	diff=(double)tp.tv_sec+(double)tp.tv_usec/1.0e6;
	return diff;
}

/** isNumber: Comprueba si la cadena de carácteres pasada por parámetro es un número
 *	return bool
 **/
int isNumber(char *numeroEntrada) {
    int i;
    long valor;
    size_t tamanyoVector = strlen(numeroEntrada);
    int correcto=TRUE;


    for (i = 0; i < tamanyoVector && correcto; i++) {
        if (!isdigit(numeroEntrada[i])) {
            correcto=FALSE;
        }
    }

    return correcto;
}

/** comprobarArgumentos: Checkea los argumentos de entrada e indica si son o no correctos
 *						Si no son correctos, sale del programa.
 *	return bool
 **/
int comprobarArgumentos(int argc, char *argv[]){
	if (argc != 2) { 
		printf ("Uso: contar_paralelo <tamaño vector> \n"); 
		return FALSE; 
	}

	int tamanyo = atoi(argv[1]);
	if(!isNumber(argv[1]) || tamanyo <= 0){
		printf ("Uso: contar_paralelo <tamaño vector (tipo numero >0)> \n"); 
		return FALSE; 
	}
}

/** buscarNumero: busca el numero NUMBUSCADO en el vector que se le pase pro parámetro y devuelve el numero de veces que aprece**/
int buscarNumero(int *subVector, int subTamanyo){
	int numVeces=0, i, j;
	//Buscamos el número NUMBUSCADO en el vector.
	//Repetimos la operación REPETICIONES veces
	for (i=0; i<REPETICIONES; i++) {
		for (j=0; j<subTamanyo; j++) {
			if (subVector[j] == NUMBUSCADO) {
				numVeces++;
			}
		}
	}
	return numVeces;
}

/** procesoSecuencial: Rellena un vector aleatoriamente y busca el numero NUMBUSCADO en todo el vector REPETICIONES veces
 *
 **/
double procesoSecuencial(int *vector, int tamanyo){
	int i,j,a; //Contadores
	double t_InicioSec, t_finSec, t_totalSec; //Tiempos de ejecución
	
	t_InicioSec=ctimer(); //Tomamos el tiempo inicial
	a=buscarNumero(vector, tamanyo);
	t_finSec=ctimer();	//Tomamos tiempo final

	t_totalSec = t_finSec - t_InicioSec;
	//Devolvemos el valor en segundos del proceso secuencial.
	return t_totalSec; 
}

int main (int argc, char *argv[]) { 
	srand(time(NULL));
	int *vector;
	int i, auxPosicion; //Contadores
	int idProceso, totalProcesos;
	int asignacionHijos, asignacionFinal;
	long int numVeces=0;
	long int numVecesRecibida=0;
  	double tiempoInicio, tiempoFin, tiempoSec, tiempoPar;
  	MPI_Status estado; //Variable que almacenará el estado de MPI en función de la rutina a la que se lo pasemos

	MPI_Init(&argc,&argv); //Iniciamos la rutina de MPI

	//Si es el proceso padre, comprueba los argumentos y hace el proceso secuencial
	if(idProceso==0){
		if(!comprobarArgumentos(argc, argv)){
			MPI_Finalize();
			return 0;
		}
	}

	int tamanyo=atoi(argv[1]);
	MPI_Comm_rank(MPI_COMM_WORLD, &idProceso); //Se establece cual es la ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &totalProcesos); //Se establece cual es el número total de procesos

	if(idProceso==0){ //Si es el proceso padre
		//Reservamos memoria para el tamaño del vector el tamaño del vector (numero por pantalla * el tamaño de un entero)
		vector=malloc(sizeof(int)*tamanyo); 
		for (i=0; i<tamanyo; i++) { //Rellenamos el vector con números aleatorios entre 1 y MAXENTERO
			vector[i] =1+ ((double)(MAXENTERO)* rand()) / RAND_MAX; 
		} 
		//Ejecutamos el proceso secuencial y lo almacenamos en la variable tiempoSec
		tiempoSec=procesoSecuencial(vector, tamanyo);		

		//La asignación de los hijos se hace de forma simple 
		asignacionHijos=tamanyo/totalProcesos;
		//Si la división no es exacta, es el padre el encargado de hacer los procesos sobrantes
		asignacionFinal=auxPosicion=asignacionHijos+tamanyo%totalProcesos;

		tiempoInicio=ctimer(); //Iniciamos el tiempo

		for (i=1;i<totalProcesos;i++) {
			//Para cada proceso le enviamos la dirección del vector y el tamaño que tiene que leer (asignaciónHijos-1)
			MPI_Send( &vector[auxPosicion], asignacionHijos-1, MPI_LONG, i, 23, MPI_COMM_WORLD);
			auxPosicion+=asignacionHijos; //Incrementamos a la siguiente posición del vector
		}	

	}else{ //Si es un proceso hijo
		//Anteriormente hemos calculado el tamaño que tiene que recibir cada hijo
		asignacionFinal=tamanyo/totalProcesos;
		vector=malloc(sizeof(int)*asignacionFinal);
		//Recibimos el vector y el tamaño que tenemos qie leer del mismo
		MPI_Recv( vector, asignacionFinal, MPI_LONG, MPI_ANY_SOURCE, 23, MPI_COMM_WORLD, &estado);
	}

	//Como asignacionFinal vale diferente para el padre que para los hijos, pero deben de hacer lo mismo
	//Lanzamos el método y almacenamos el número de veces que aparece NUMBUSCADO en cada uno de ellos
	numVeces=buscarNumero(vector, asignacionFinal);

	if(idProceso==0) {
		//Recibimos todos los numVeces de todos los procesos y los sumamos
		for (i=1;i<totalProcesos;i++) {
	    	MPI_Recv ( &numVecesRecibida, 1, MPI_LONG, MPI_ANY_SOURCE, 23, MPI_COMM_WORLD, &estado);
	     	numVeces += numVecesRecibida;
		}	

		tiempoFin=ctimer(); //Finalizamos el tiempo
		tiempoPar = tiempoFin - tiempoInicio;
		//Emitimos informe
		printf("N,\tTamVector,\tnumVeces,\tt. Secuencial,\tt. Paralelo\n");
		printf("%d|\t%5.0f|\t%d|\t\t%5.4f|\t\t%5.4f\n", totalProcesos, (double)tamanyo, numVeces, tiempoSec , tiempoPar);

	}else{
		//Si es un proceso hijo, envía numVeces al padre
		MPI_Send( &numVeces, 1, MPI_LONG, 0, 23, MPI_COMM_WORLD);
	}

	free(vector); //Liberamos la memoria del vector
	MPI_Finalize(); //Finalizamos la rutina MPI
}