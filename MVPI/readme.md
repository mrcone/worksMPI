# Multiplicación Matriz-Vector en MPI

Se desea realizar la implementación paralela del producto matriz-vector, utilizando el algoritmo basado en el producto interno, con distribucióon equitativa de todas las filas de la matriz entre los procesos. 

### Compilación

Simplemente ejecutar en el directorio donde se encuentra mvmpi.c:

```
make
```

### Uso

Ejecutar el siguiente código con mpirun, y el archivo con los hosts a usar:

```
Uso: mpirun -np <numero_procesos> mvmpi
```

Si la matriz y el vector son menor que 10, se mostrarán por pantalla, en caso contrario sólo se mostrará el vector solución