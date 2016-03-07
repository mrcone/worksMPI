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
#define MAXENTERO 100 // El array de enteros estará entre 1 y MAXENTERO
#define REPETICIONES 100 // Simula vector todavia mayor sin ocupar memoria
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
	int i,j; //Contadores
	double tiempoInicio, tiempoFin, tiempoTotal; //Tiempos de ejecución
	int numVeces; //Numero de veces que aparece un número en el vector
	
	tiempoInicio=ctimer(); //Tomamos el tiempo inicial
	numVeces=buscarNumero(vector, tamanyo);
	tiempoFin=ctimer();	//Tomamos tiempo final

	tiempoTotal = tiempoFin - tiempoInicio;

	//Mostramos la información del proceso secuencial
	printf ("Veces que aparece el %d en %d repeticiones del vector de tamaño %d: %d veces, el %5.2f%\n", NUMBUSCADO, REPETICIONES, tamanyo, numVeces,(100.0*numVeces)/(tamanyo*REPETICIONES)); 
	  
	return tiempoTotal;
}

int main (int argc, char *argv[]) { 
	srand(time(NULL));
	int *vector;
	int i, auxPosicion; //Contadores
	int idProceso, totalProcesos;
	int asignacionHijos, asignacionPadre;
	int numVecesTotal=0, numVecesRemoto=0;
	MPI_Init(&argc,&argv);
  	MPI_Status estado;

	//Si es el proceso padre, comprueba los argumentos y hace el proceso secuencial
	if(!comprobarArgumentos(argc, argv)){
		MPI_Finalize();
		return 0;
	}

	int tamanyo=atoi(argv[1]);
	MPI_Comm_rank(MPI_COMM_WORLD, &idProceso);
	MPI_Comm_size(MPI_COMM_WORLD, &totalProcesos);

	if(idProceso==0){
		//Reservamos memoria para el tamaño del vector el tamaño del vector (numero por pantalla * el tamaño de un entero)
		vector=malloc(sizeof(int)*tamanyo); 
		for (i=0; i<tamanyo; i++) { //Rellenamos el vector con números aleatorios entre 1 y MAXENTERO
			vector[i] =1+ ((double)(MAXENTERO)* rand()) / RAND_MAX; 
		} 

		printf ("Tiempo de proceso: %f seg\n", procesoSecuencial(vector, tamanyo));

		asignacionHijos=tamanyo/totalProcesos;
		asignacionPadre=auxPosicion=asignacionHijos+tamanyo%totalProcesos;
		printf("El proceso padre 0 analiza desde 0 hasta %d (%d)\n", asignacionPadre-1, asignacionPadre);
		for (i=1;i<totalProcesos;i++) {
			printf("El proceso hijo %d analiza desde %d hasta %d (%d)\n", i, auxPosicion, auxPosicion+asignacionHijos-1, asignacionHijos);
			MPI_Send( &vector[auxPosicion], asignacionHijos-1, MPI_INT, i, 23, MPI_COMM_WORLD);
			auxPosicion+=asignacionHijos; 
		}

		numVecesTotal=buscarNumero(vector, asignacionPadre);

		//Recibimos todos los numVeces de todos los procesos y los sumamos
		for (i=1;i<totalProcesos;i++) {
        	MPI_Recv ( &numVecesRemoto, 1, MPI_INT, MPI_ANY_SOURCE, 23, MPI_COMM_WORLD, &estado);
         	numVecesTotal = numVecesTotal + numVecesRemoto;
		}		

	}else{
		printf("Proceso %d a\n", idProceso);
		asignacionHijos=tamanyo/totalProcesos;
		vector=malloc(sizeof(int)*asignacionHijos);

		MPI_Recv( &vector, asignacionHijos, MPI_INT, 0, idProceso, MPI_COMM_WORLD, &estado);

		int vecesParcial=buscarNumero(vector, asignacionHijos);

		printf("Proceso %d c\n", idProceso);
		MPI_Send( &vecesParcial, 1, MPI_INT, 0, 23, MPI_COMM_WORLD);
		printf("Proceso %d ha contado %d veces el numero\n ", idProceso, vecesParcial);
	}
	free(vector); 
	MPI_Finalize();
}