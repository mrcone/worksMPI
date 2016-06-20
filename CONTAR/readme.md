# CONTAR en MPI

Se pretende comparar el comportamiento de un algoritmo paralelo y su version secuencial, obteniendo ciertas medidas de paralelismo como son el speed-up y la eficiencia. 
El problema que nos interesa es un algoritmo secuencial que cuenta el numero de veces que un determinado numero aparece en un conjunto grande de elementos almacenados en un vector. 

### Compilación

Simplemente ejecutar en el directorio donde se encuentra contar_paralelo.c:

```
make
```

### Uso

Ejecutar el siguiente código con mpirun, y el número de procesadores a usar:

```
Uso: mpirun -np <numero_de_procesadores> contar_paralelo <tamaño_vector>
```

Se puede integrar también la opción `-hostname <fichero>` para establecer diferentes máquinas