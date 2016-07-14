# Algoritmo de Floyd-Wharsall con MPI

Se realiza la implementación paralela, sobre MPI, del algoritmo de Floyd-Warshall para el cálculo de los caminos máss cortos entre todos los pares de vértices en un grafo ponderado.
Está basada en una descomposición unidimensional, por bloques, de filas consecutivas de las matrices intermedias en cada iteración ([algoritmo de Floyd-Wharsall](https://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm)).

### Compilación

Simplemente ejecutar en el directorio donde se encuentra fw.c:

```
make
```

### Uso

Ejecutar el siguiente código con mpirun, y el archivo con los hosts a usar:

```
Uso: mpirun fw <numero de vértices>
```

**Sólo** si el número de nodos es menor o igual que 10, se muestra por pantalla