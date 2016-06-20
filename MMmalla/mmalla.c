#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdlib.h>

//Definiciones
#define TRUE 1
#define FALSE 0
#define KMAXBLOQ 100
#define USO1 "¡El número de procesos debe ser un cuadrado perfecto!\n\tUSO: mpirun -np X² mmalla\n"
#define USO2 "El número de bloques es entre 0 o 100\n"
#define USO3 "Tamaño de los bloques: "

//Calcula el producto matrizA*matrizB y lo almacena en matrizC
void mult(double matrizA[], double matrizB[], double *matrizC, int tam) {
    int i,j,k;
    for (i=0; i< tam; i++) {
        for (j=0; j< tam; j++) {
            for (k=0; k< tam; k++) {
                matrizC[i*tam+j]=matrizC[i*tam+j]+matrizA[i*tam+k]*matrizB[k*tam+j];
            }
        }
    }
}

int comprobarParametros(int totalProcesos, int idProcesos){
    int raizProcesos=sqrt(totalProcesos), blq;

    if (raizProcesos != (int)raizProcesos) {
        MPI_Finalize();
        if (idProcesos == 0) 
            printf(USO1);
        return 0;
    }

    if ( idProcesos == 0 ) {
        printf(USO3);
        scanf("%d",&blq);
        return blq;
    } 
}

void vermatriz(int lda, double a[][lda], int m, int n, char nombre[]) {
    int i,j;
    printf("%4s =\n",nombre);

    for (j=0;j<n;j++){
        printf("\t%d",j);
    }

    printf("\n");
    for (i=0;i<m;i++){
        printf("%2d:",i);
        for (j=0;j<n;j++){
            printf("\t%3.1f ",a[i][j]);
        }
        printf("\n");
    }      
}

/** isNumber: Comprueba si la cadena de carácteres pasada por parámetro es un número
 *  return bool
 **/
int isNumber(char *numeroEntrada) {
    int i;
    size_t tamanyoVector = strlen(numeroEntrada);
    int correcto=TRUE;

    for (i = 0; i < tamanyoVector && correcto; i++) {
        if (!isdigit(numeroEntrada[i])) {
            correcto=FALSE;
        }
    }
    return correcto;
}

/** compruebaBloques: Comprueba el tamaño del bloque que recibe es correcto, en caso contrario finaliza MPI
 *
 **/
void compruebaBloques(int bloqTam, int idProceso){
    if ((bloqTam < 1) || ( bloqTam > KMAXBLOQ )) {
        if (idProceso == 0) {
            printf(USO2);
        }
        MPI_Finalize();
        return 0;
    } 
}


main(int argc, char **argv) {

	int tamMatriz, bloqtam;
    int i=0,j=0,k=0, valorIdenti;
	int *mifila;
	double *matrizA, *matrizB, *matrizC, *matrizATMP, *buffer;
	int filaActual, columnaActual;
    int procSup, procInf;
	double raizProcesos;
	int tamBuffer;
    int totalErrores=0;

    //Variables y directivas MPI
    int totalProcesos, idProceso;
    MPI_Status estado;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &idProceso);
	MPI_Comm_size(MPI_COMM_WORLD, &totalProcesos);

    //Comprobacion de parámetros (cuadrado perfecto y tamaño de bloque)
    bloqtam=comprobarParametros(totalProcesos, idProceso);
    tamMatriz=sqrt(totalProcesos);

    //Si la raiz es correcta, comunicamos al resto los valores
	MPI_Bcast(&tamMatriz,1,MPI_INT,0,MPI_COMM_WORLD);	  
	MPI_Bcast(&bloqtam,1,MPI_INT,0,MPI_COMM_WORLD);

    //Se comprueba, y el padre muestra si no es correcto
    compruebaBloques(bloqtam, idProceso);

    filaActual = idProceso/tamMatriz;
    columnaActual = idProceso % tamMatriz;

    //Los procesos situados en la misma columna (procSup y procInf)
    if ( filaActual == 0 ) { //Si es la primera fila, accedemos a la última
        procSup=totalProcesos-tamMatriz+idProceso;
    }else{ //Si no, a la de arriba simplemente
        procSup=idProceso-tamMatriz;
    }    
    
    if ( filaActual == (tamMatriz-1) ) { //De igual manera ocurre con la fila inferior
        procInf=idProceso-totalProcesos+tamMatriz;
    }else{
        procInf=idProceso+tamMatriz;
    }    

    //Definimos las direcciones de memoria de las matrices y
    //del vector mi fila para los numeros de los procesos
    mifila = (int *)malloc(sizeof(int) * (tamMatriz-1));
    matrizA = (double *)malloc(sizeof(double) * bloqtam*bloqtam);
    matrizB = (double *)malloc(sizeof(double) * bloqtam*bloqtam);
    matrizC = (double *)malloc(sizeof(double) * bloqtam*bloqtam);
    matrizATMP = (double *)malloc(sizeof(double) * bloqtam*bloqtam);

    //Inicializamos la matrizA como se indica en la práctica
    //Y la matriz C a 0
    for (i=0; i< bloqtam*bloqtam; i++) {
        matrizA[i]=(float)i*(filaActual*columnaActual+1);
        matrizA[i]/=bloqtam*bloqtam;
        matrizC[i]=0.0;
    }

    //Almacenamos los numeros de orden de los procesos en la fila del proceso
	for (i=1; i<=tamMatriz; i++) {
		if (filaActual*tamMatriz+i-1 != idProceso) {
            mifila[j]=filaActual*tamMatriz;
			mifila[j]+=i-1;
			j+=1;
		}
	}	 

    //Creamos la matriz identifidad en B
	for (i=0; i<bloqtam; i++) {
		for (j=0; j<bloqtam; j++) {
            valorIdenti=0;
			if (filaActual == columnaActual) {
				if (i == j) {
                    valorIdenti=1;
                }
			}
            matrizB[j*bloqtam+i]=valorIdenti;
		}
	}	 

    //¿Cuanto espacio necesitamos en el buffer para el envio?
	MPI_Pack_size(bloqtam*bloqtam, MPI_DOUBLE, MPI_COMM_WORLD, &tamBuffer);	  
	//Además de lo que ocupa el buffer, le añadimos la cabecera
    tamBuffer = tamMatriz*(tamBuffer + MPI_BSEND_OVERHEAD);
    //Y creamos el buffer para MPI
	buffer = (double *)malloc(tamBuffer);
	MPI_Buffer_attach( buffer, tamBuffer);

    if(idProceso==0){
        printf("Proceso \tErrores\n");
        printf("-----------------------\n");
    }

    //Cálculo de cannon
	for (i=0; i<tamMatriz; i++) {
        //Si columna=mod(fila+k, r)
		if (columnaActual == (filaActual+i)%tamMatriz) {
			for (j=0; j<tamMatriz-1; j++) { 
                MPI_Send(matrizA,bloqtam*bloqtam, MPI_DOUBLE, mifila[j], 32, MPI_COMM_WORLD);
            }
            //C=C+A*B
			mult(matrizA,matrizB,&matrizC[0],bloqtam);
		} else { //Si no, recibimos matrizA del proceso que envia mifila 
			MPI_Recv(matrizATMP,bloqtam*bloqtam, MPI_DOUBLE,filaActual*tamMatriz+(filaActual+i)%tamMatriz,32,MPI_COMM_WORLD,&estado);	   
			mult(matrizATMP, matrizB, matrizC, bloqtam);
		}

        //P(i-j) manda su bloque a P(i-1,j).
        //El proceso P(0j) manda su bloque B a P(tamMatriz-1,j)
		MPI_Bsend(matrizB, bloqtam*bloqtam, MPI_DOUBLE, procSup, 23, MPI_COMM_WORLD);
		MPI_Recv(matrizB, bloqtam*bloqtam, MPI_DOUBLE, procInf, 23, MPI_COMM_WORLD,&estado);

	}

    /*if(idProceso==1){
        vermatriz(tamMatriz, matrizA, tamMatriz, tamMatriz, "matrizA");
    }*/

    //Recorremos las dos matrices A y C
    //Si no son iguales sumamos error
	for (i=0; i<bloqtam*bloqtam; i++) {
		if (matrizA[i]-matrizC[i]!=0) {
            totalErrores=totalErrores+1;
        }
	}

    printf("  %d \t\t %d\n",idProceso, totalErrores); 

    //Liberamos la memoria del buffer y salimos finalizamos MPI
    MPI_Buffer_detach(buffer, &tamBuffer);
    MPI_Finalize();

}
