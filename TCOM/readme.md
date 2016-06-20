# TIEMPO DE LATENCIA en MPI

Cuando se establece una comunicación mediante protocolo MPI, comúnmente, no se tiene en cuenta el coste de las comunicaciones.
Realmente, no es necesario si la latencia de la red es baja o no infiere mucho a la hora de establecer resultados de un experimento de dado. No obstante, pueden darse experimentos en los que se necesite restar la latencia de comunicación para determinar el determinado coste de un algoritmo, sin comunicación. 

### Compilación

Simplemente ejecutar en el directorio donde se encuentra tcom.c:

```
make
```

### Uso

Ejecutar el siguiente código con mpirun, y el archivo con los hosts a usar:

```
Uso: mpirun -hostfile <fichero_hosts> tcom
```

**Sólo** se deben usar dos máquinas en el fichero de hosts, ya que sólo tendrá en cuenta el tiempo entre 2 máquinas

##Explicación teórica

Los parametros β y τ indican respectivamente la latencia necesaria para el envío de un mensaje y el tiempo necesario para enviar un byte.
´´
T com = β + τ · Tam_Mensaje
´´
Lo parametros se pueden estimar de la siguiente forma:
*El valor de β sera el tiempo necesario en enviar un mensaje sin datos.
*El valor de τ sera el tiempo requerido para enviar un mensaje menos el tiempo de latencia, dividido por el numero de bytes del mensaje.

