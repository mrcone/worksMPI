//Ejecutar con n=50; while (( n <= 100 )); do mpirun -hostfile hosts tcom $n; n=$((n+10)); done
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/time.h> 
#include <malloc.h>
#include <mpi.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include <math.h>

//Definiciones
#define TRUE 1
#define FALSE 0
#define BITS_INICIALES 2
#define BITS_FINALES 19
static int nclock;

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

/** comprobarArgumentos: Checkea los argumentos de entrada e indica si son o no correctos. Si no son correctos, sale del programa.
*	return bool
**/
int comprobarArgumentos(int argc, char *argv[]){
	if (argc != 2) { 
		printf ("Uso: tcom <Total envíos> \n"); 
		return FALSE; 
	}

	int tamanyo = atoi(argv[1]);

	if(!isNumber(argv[1]) || tamanyo <= 0){
		printf ("Uso: tcom <Número Total envíos> \n"); 
		return FALSE; 
	}
}

/** comprobarProcesos: Checkea que sólo existen dos procesos
*	return bool
**/
int comprobarProcesos(int totalP){
	if (totalP != 2){ //Si hay mas de dos procesos
		printf("El sistema tiene que ser punto a punto (dos máquinas)\n");
		printf("\tModifique el fichero HOSTS\n");
		return FALSE;
	}
}

/**
*
**/
main(int argc, char **argv) {
	//Variables MPI
	int totalProcesos, idProceso;
	MPI_Status estado;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &idProceso);
	MPI_Comm_size(MPI_COMM_WORLD, &totalProcesos);

	//Variables programa
	int enviosTotales, jBit, iEnvios, tam_mensaje;
	char byteEnviado="0"; //Para calcular el mensaje 'vacío'
	double mensajeGrande[((int)pow(2, BITS_FINALES))+1]; //Para calcular un mensaje con contenido 2^19 + 1 bit como máximo
	double tiempoIni, tiempoFin, tiempoIdaVuelta, mediaTiempo=0, beta, tau;

	if(!comprobarArgumentos(argc, argv) || !comprobarProcesos(totalProcesos)){
		MPI_Finalize();
		return 0;
	}

	enviosTotales=atoi(argv[1]);

	if(idProceso==0){ //Si es el proceso padre
		printf("-------------------------------------------------------------------------\n");
		printf("|-> Total de envios: \t\t\t\t%d veces\t\t|\n", enviosTotales);
		printf("-------------------------------------------------------------------------\n");
		
		//############Cálculo de beta###########
		//Enviamos un byte y lo recibimos enviosTotales veces.
		//Lo dividimos entre 2 y obtenemos el tiempo medio de comunicación
		//Si lo hacemos mil veces, obtenemos una media
		for (iEnvios=0; iEnvios<enviosTotales; iEnvios++){
			tiempoIni=MPI_Wtime();

			MPI_Send(&byteEnviado, 1, MPI_BYTE, 1, 123, MPI_COMM_WORLD);
			MPI_Recv(&byteEnviado, 1, MPI_BYTE, 1, 321, MPI_COMM_WORLD, &estado);

			tiempoFin=MPI_Wtime();
			tiempoIdaVuelta=(tiempoFin-tiempoIni)/2;
			mediaTiempo+=tiempoIdaVuelta;
		}
		beta=mediaTiempo/enviosTotales; //Tiempo de ida y vuelta medio
		printf("|-> BETA (latencia): \t\t\t\t%5.2F ms \t\t|\r\n", beta*1000000, beta);
		printf("-------------------------------------------------------------------------\n");
		//#########Cálculo de TAU##############
		//Enviamos una trama de tamaño 'totalBytes' de doubles 'enviosTotales' veces
		//Le quitamos la latencia a la media del tiempo medio de ida y vuelta
		//Y lo dividimos entre tantos bits como hayamos enviado
		printf("|-> TAU ([t_com - BETA] / Tamaño Mensaje):\t\t\t\t|\r\n");
		for(jBit=BITS_INICIALES; jBit<=BITS_FINALES; jBit++){ //Enviamos tramas de bytes, desde 2 hasta 19
			mediaTiempo=0;
			tam_mensaje=(int)pow(2, jBit);
			for (iEnvios=0; iEnvios<enviosTotales; iEnvios++){ //Tantos envios como haya por terminal
				tiempoIni=MPI_Wtime();

				MPI_Send(&mensajeGrande, tam_mensaje, MPI_DOUBLE, 1, 123, MPI_COMM_WORLD);
				MPI_Recv(&mensajeGrande, tam_mensaje, MPI_DOUBLE, 1, 321, MPI_COMM_WORLD, &estado);

				tiempoFin=MPI_Wtime();
				tiempoIdaVuelta=(tiempoFin-tiempoIni)/2; //Establecemos el tiempo de ida y vuelta (suponemos que es la mitad del tiempo de envio)
				mediaTiempo+=tiempoIdaVuelta;
			}
			tau=mediaTiempo/enviosTotales; //Calculamos la media de tau con todos los mensajes
			tau=tau-beta; //Le restamos el tiempo de latencia
			tau=tau/(tam_mensaje*8); //Puesto que cada double son 8 bytes, hay que dividir por el tamaño del mensaje por 8 bytes que tiene cada elemento del mensaje		
			printf("\t|-> (%5.2f - %5.2f) / 2^%d \t= \t%7.5f ms\t\t|\n", (mediaTiempo/enviosTotales)*1000000, beta*1000000, jBit, tau*1000000);		
		}
		printf("-------------------------------------------------------------------------\n");
	}else{ //Si somos el hijo (asumiriemos que sólo hay 1)
		for (iEnvios=0; iEnvios<enviosTotales; iEnvios++){ //Recibimos y volvemos a enviar el byte
			MPI_Recv(&byteEnviado, 1, MPI_BYTE, 0, 123, MPI_COMM_WORLD, &estado);
         		MPI_Send(&byteEnviado, 1, MPI_BYTE, 0, 321, MPI_COMM_WORLD);
         }

		for(jBit=BITS_INICIALES; jBit<=BITS_FINALES; jBit++){ //Recibimos y enviamos los bytes 
			tam_mensaje=(int)pow(2, jBit);
			for (iEnvios=0; iEnvios<enviosTotales; iEnvios++){
				MPI_Recv(&mensajeGrande, tam_mensaje, MPI_DOUBLE, 0, 123, MPI_COMM_WORLD, &estado);
				MPI_Send(&mensajeGrande, tam_mensaje, MPI_DOUBLE, 0, 321, MPI_COMM_WORLD);	
			}
		}
	}
	MPI_Finalize();
	return 0;
}
